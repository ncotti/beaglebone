#ifndef BMP280_TYPES_H
#define BMP280_TYPES_H

#include <linux/ioctl.h>

#define MAGIC_NUMBER 'c'
#define IOCTL_CMD_SET_FREQ  _IOR(MAGIC_NUMBER, 1, int*)
#define IOCTL_CMD_SET_MODE  _IOR(MAGIC_NUMBER, 2, int*)
#define IOCTL_CMD_SET_UNIT  _IOR(MAGIC_NUMBER, 3, int*)

#define BMP280_ID       0x58
#define BMP280_ERROR    -6666

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

/// @brief Temperature unit used at the return value
typedef enum bmp280_unit {
    CELSIUS = 0,
    KELVIN = 1,
    FAHRENHEIT = 2,
} bmp280_unit;

#endif // BMP280_TYPES_H
