#include <linux/init.h>
#include <linux/module.h>
#include <linux/mod_devicetable.h>
#include <linux/property.h>
#include <linux/platform_device.h>
#include <linux/of_device.h>
#include <linux/i2c.h>

#include "char_device.h"
#include "bmp280.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nicolas G. Cotti");
MODULE_DESCRIPTION("Configure an I2C device on top of the default I2C driver, "
"using the device tree.");

#define DRIVER_NAME "cotti,driver"  // Can be any name here

#define DT_CHILD_DEVICE_NAME "my_i2c_device"
#define DT_COMPATIBLE "cotti,driver"
