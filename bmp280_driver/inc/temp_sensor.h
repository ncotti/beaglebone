#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of_device.h>
#include "char_device.h"
#include "bmp280.h"
#include "cotti_i2c.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nicolas Gabriel Cotti");
MODULE_DESCRIPTION("This module initializes the I2C2 bus interface based on a "
"device tree overlay, and then sets up a character device in the /dev/ "
"directory to read and configure the temperature sensor BMP280.");

// Can be any name here
#define DRIVER_NAME "temp-sensor"

// Value of the property "compatible" to match this driver
#define DT_COMPATIBLE "cotti,i2c"
