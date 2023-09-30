#ifndef BMP280_H
#define BMP280_H

#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/i2c.h>

s32 bmp280_read_temperature(void);
int bmp280_init(struct i2c_client *client);

#define I2C_SLAVE_ADDRESS 0x76
#define ADDRESS_ID 0xD0

#endif // BMP280_H