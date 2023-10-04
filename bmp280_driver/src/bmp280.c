#include "bmp280.h"

// i2c client
static s32 dig_T1, dig_T2, dig_T3;

int bmp280_init(void) {
    u8 bmp280_id;
    // struct device_node *clk_node = NULL;
    // if ((temp_sensor_node = of_get_child_by_name(pdev->dev.of_node, DT_I2C_CHILD_NAME)) == NULL) {
    //     printk(ERROR("Couldn't get bmp280 node.\n"));
    //     goto i2c_ptr_error;
    // }
    // temp_sensor_error: of_node_put(temp_sensor_node);

    // Read ID
    printk(INFO("First I2C read.\n"));
    bmp280_id = cotti_i2c_read(ADDRESS_ID);
    printk(INFO("ID: 0x%x.\n"), bmp280_id);

    // Read calibration values
    dig_T1 = cotti_i2c_read(0x89) << 8 | cotti_i2c_read(0x88);
    dig_T2 = cotti_i2c_read(0x8b) << 8 | cotti_i2c_read(0x8a);
    dig_T3 = cotti_i2c_read(0x8d) << 8 | cotti_i2c_read(0x8c);
    printk(INFO("Calibration values: 0x%x 0x%x 0x%x.\n"), dig_T1, dig_T2, dig_T3);

    if(dig_T2 > 32767)
		dig_T2 -= 65536;

	if(dig_T3 > 32767)
		dig_T3 -= 65536;

    // Initialize device
    cotti_i2c_write(0x5<<5, 0xf5);
    cotti_i2c_write((5<<5) | (5<<2) | (3<<0), 0xf4);
    printk(INFO("Wrote configuration correctly.\n"));

    // if ((kthread = kthread_run(thread_function, &arg_t2, "Thread_2")) == NULL) {
    //     printk(ERROR("Couldn't create second thread.\n"));
    //     return -1;
    // }

    return 0;
}

void bmp280_deinit(void) {

}

/// @brief Read current temperature from BMP280 sensor
/// @return temperature
s32 bmp280_read_temperature(void) {
    int var1, var2;
    s32 raw_temp;
    s32 d1, d2, d3;

    // Read Temperature
	d1 = cotti_i2c_read(0xFA);
	d2 = cotti_i2c_read(0xFB);
	d3 = cotti_i2c_read(0xFC);
	raw_temp = ((d1<<16) | (d2<<8) | d3) >> 4;
    printk(INFO("Raw temp: 0x%x.\n"), raw_temp);

	// Calculate temperature in degree
	var1 = ((((raw_temp >> 3) - (dig_T1 << 1))) * (dig_T2)) >> 11;
	var2 = (((((raw_temp >> 4) - (dig_T1)) * ((raw_temp >> 4) - (dig_T1))) >> 12) * (dig_T3)) >> 14;
	return ((var1 + var2) *5 +128) >> 8;
}
