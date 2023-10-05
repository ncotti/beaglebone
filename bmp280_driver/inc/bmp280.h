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

/// @brief Operation modes.
typedef enum bmp280_mode {
    SLEEP = 0,      // Don't measure
    FORCED = 1,     // Do one measure and return to sleep
    NORMAL = 3,     // Measure periodically
} bmp280_mode;

/// @brief Operation frequency [Hz]. Uses the time standby time and the measurement
/// time, assuming ultra low power oversampling (Table 14 of the datasheet).
typedef enum bmp280_freq {
    FREQ_167    = 0,    // 166,67 Hz
    FREQ_15     = 1,    // 14,71 Hz
    FREQ_8      = 2,    // 7,66 Hz
    FREQ_4      = 3,    // 3,91 Hz
    FREQ_2      = 4,    // 1,98 Hz
    FREQ_1      = 5,    // 0,99 Hz
    FREQ_0_50   = 6,    // 0,5 Hz
    FREQ_0_25   = 7,    // 0,25 Hz
} bmp280_freq;

int bmp280_init(struct platform_device *pdev);
void bmp280_deinit(void);
s32 bmp280_read_temperature(void);
void bmp280_set_mode(bmp280_mode mode);
void bmp280_set_frequency(bmp280_freq freq);

#define DT_I2C_CHILD_NAME           "temperature_sensor"
#define DT_PROPERTY_REG             "reg"

#define BMP280_ID                   0x58

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
