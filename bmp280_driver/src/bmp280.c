#include "bmp280.h"

// i2c client
static s32 dig_T1, dig_T2, dig_T3;

int bmp280_init(void) {
    u8 bmp280_id;

    // Read ID
    printk("First I2C read\n");
    msleep(1000);
    bmp280_id = cotti_i2c_read(ADDRESS_ID);
    printk("ID: 0x%x\n", bmp280_id);

    // Read calibration values
    dig_T1 = cotti_i2c_read(0x88);
    dig_T2 = cotti_i2c_read(0x8a);
    dig_T3 = cotti_i2c_read(0x8c);

    if(dig_T2 > 32767)
		dig_T2 -= 65536;

	if(dig_T3 > 32767)
		dig_T3 -= 65536;

    // Initialize device
    cotti_i2c_write(0x5<<5, 0xf5);
    cotti_i2c_write((5<<5) | (5<<2) | (3<<0), 0xf4);
    printk("Wrote configuration correctly!\n");

    return 0;
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

	// Calculate temperature in degree
	var1 = ((((raw_temp >> 3) - (dig_T1 << 1))) * (dig_T2)) >> 11;
	var2 = (((((raw_temp >> 4) - (dig_T1)) * ((raw_temp >> 4) - (dig_T1))) >> 12) * (dig_T3)) >> 14;
	return ((var1 + var2) *5 +128) >> 8;
}
