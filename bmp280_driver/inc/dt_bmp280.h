#include <linux/init.h>
#include <linux/module.h>
#include <linux/mod_devicetable.h>
#include <linux/property.h>
#include <linux/platform_device.h>
#include <linux/of_device.h>
#include <linux/of_irq.h>

#include "char_device.h"
#include "bmp280.h"
#include "cotti_i2c.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nicolas G. Cotti");
MODULE_DESCRIPTION("Configure an I2C device on top of the default I2C driver, "
"using the device tree.");

#define DRIVER_NAME "cotti,driver"  // Can be any name here

#define DT_COMPATIBLE "cotti,driver"
#define DT_PROPERTY_LABEL "label"
#define DT_PROPERTY_MY_VALUE "my-value"
#define DT_PROPERTY_CLOCK_FREQ "clock-frequency"
