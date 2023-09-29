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

#define DRIVER_NAME "cotti,driver"  // Can be any name here

#define DT_COMPATIBLE "cotti,driver"
#define DT_PROPERTY_LABEL "label"
#define DT_PROPERTY_MY_VALUE "my-value"
#define DT_PROPERTY_GPIO "green-led-gpios"

// It's really important to NOT include the "-gpio" in the name here
// https://www.kernel.org/doc/Documentation/driver-api/gpio/board.rst
// The function gpiod_get() appends the "-gpio" or "-gpios".
#define DT_PROPERTY_GPIO_WITHOUT_POSTFIX "green-led"

#define BASE_10 10
