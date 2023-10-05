#ifndef BMP280_H
#define BMP280_H

#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include "cotti_i2c.h"
#include "log.h"
#include "bmp280_types.h"

int bmp280_init(struct platform_device *pdev);
void bmp280_deinit(void);
s32 bmp280_read_temperature(void);
int bmp280_set_mode(bmp280_mode mode);
int bmp280_set_frequency(bmp280_freq freq);
void bmp280_set_unit(bmp280_unit unit);
int bmp280_is_connected(void);

#define DT_I2C_CHILD_NAME           "temperature_sensor"
#define DT_PROPERTY_REG             "reg"

#define BMP280_REG_DIG_T1_LSB       0x88
#define BMP280_REG_DIG_T1_MSB       0x89
#define BMP280_REG_DIG_T2_LSB       0x8A
#define BMP280_REG_DIG_T2_MSB       0x8B
#define BMP280_REG_DIG_T3_LSB       0x8C
#define BMP280_REG_DIG_T3_MSB       0x8D
#define BMP280_REG_ID               0xD0
#define BMP280_REG_RESET            0xE0
#define BMP280_REG_STATUS           0xF3
#define BMP280_REG_CTRL_MEAS        0xF4
#define BMP280_REG_CONFIG           0xF5
#define BMP280_REG_TEMP_MSB         0xFA
#define BMP280_REG_TEMP_LSB         0xFB
#define BMP280_REG_TEMP_XLSB        0xFC

#define BMP280_TEMP_OVERSAMPLING_X1 (1 << 5)

#endif // BMP280_H
