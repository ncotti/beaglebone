#include "bmp280.h"

// i2c client
static struct i2c_client *bmp280_i2c_client;
static s32 dig_T1, dig_T2, dig_T3;

int bmp280_init(struct i2c_client *client) {
    u8 bmp280_id;
    bmp280_i2c_client = client;

    // Checking that the device slave address matches (could be many i2c devices on the same bus)
    if(client->addr != I2C_SLAVE_ADDRESS) {
        printk("Device doesn't have the right slave address. Expected: %d, got: %d\n",
            I2C_SLAVE_ADDRESS, client->addr);
            return -1;
    }

    // Read ID
    bmp280_id = i2c_smbus_read_byte_data(bmp280_i2c_client, ADDRESS_ID);
    printk("ID: 0x%x\n", bmp280_id);

    // Read calibration values
    dig_T1 = i2c_smbus_read_word_data(bmp280_i2c_client, 0x88);
    dig_T2 = i2c_smbus_read_word_data(bmp280_i2c_client, 0x8a);
    dig_T3 = i2c_smbus_read_word_data(bmp280_i2c_client, 0x8c);

    if(dig_T2 > 32767)
		dig_T2 -= 65536;

	if(dig_T3 > 32767)
		dig_T3 -= 65536;

    // Initialize device
    i2c_smbus_write_byte_data(bmp280_i2c_client, 0xf5, 0x5<<5);
    i2c_smbus_write_byte_data(bmp280_i2c_client, 0xf4, (5<<5) | (5<<2) | (3<<0) );

    return 0;
}

/// @brief Read current temperature from BMP280 sensor
/// @return temperature
s32 bmp280_read_temperature(void) {
    int var1, var2;
    s32 raw_temp;
    s32 d1, d2, d3;

    // Read Temperature
	d1 = i2c_smbus_read_byte_data(bmp280_i2c_client, 0xFA);
	d2 = i2c_smbus_read_byte_data(bmp280_i2c_client, 0xFB);
	d3 = i2c_smbus_read_byte_data(bmp280_i2c_client, 0xFC);
	raw_temp = ((d1<<16) | (d2<<8) | d3) >> 4;

	// Calculate temperature in degree
	var1 = ((((raw_temp >> 3) - (dig_T1 << 1))) * (dig_T2)) >> 11;
	var2 = (((((raw_temp >> 4) - (dig_T1)) * ((raw_temp >> 4) - (dig_T1))) >> 12) * (dig_T3)) >> 14;
	return ((var1 + var2) *5 +128) >> 8;
}