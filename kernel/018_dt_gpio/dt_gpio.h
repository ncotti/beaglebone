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
#include <linux/gpio/consumer.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nicolas G. Cotti");
MODULE_DESCRIPTION("Configure a GPIO directly from the device tree. TODO this module is incomplete");

#define DEVICE_NAME "dt_gpio_driver"
#define DEVICE_CLASS_NAME "cotti"
#define MINOR_NUMBER 0
#define NUMBER_OF_DEVICES 1

#define DRIVER_NAME "my_device_driver"  // Can be any name here

#define DT_COMPATIBLE "mydev"
#define DT_PROPERTY_LABEL "label"
#define DT_PROPERTY_MY_VALUE "my_value"
#define DT_PROPERTY_GPIO "green-led-gpio"

#define BASE_10 10
