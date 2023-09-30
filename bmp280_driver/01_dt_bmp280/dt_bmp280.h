#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/mod_devicetable.h>
#include <linux/property.h>
#include <linux/platform_device.h>
#include <linux/of_device.h>
#include <linux/i2c.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nicolas G. Cotti");
MODULE_DESCRIPTION("Configure an I2C device on top of the default I2C driver, "
"using the device tree.");

#define DEVICE_NAME "dt_i2c_driver"
#define DEVICE_CLASS_NAME "cotti"
#define MINOR_NUMBER 0
#define NUMBER_OF_DEVICES 1

#define DRIVER_NAME "cotti,driver"  // Can be any name here

#define DT_CHILD_DEVICE_NAME "my_i2c_device"
#define DT_COMPATIBLE "cotti,driver"
#define I2C_SLAVE_ADDRESS 0x76
#define ADDRESS_ID 0xD0
