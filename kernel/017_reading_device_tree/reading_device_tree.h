#include <linux/init.h>
#include <linux/module.h>
#include <linux/mod_devicetable.h>
#include <linux/property.h>
#include <linux/platform_device.h>
#include <linux/of_device.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nicolas G. Cotti");
MODULE_DESCRIPTION("A LKM to parse the device tree for a specific device and its properties");

#define DRIVER_NAME "cotti,driver"  // Can be any name here

// Properties defined in the device tree
#define DT_COMPATIBLE "cotti,driver"
#define DT_PROPERTY_LABEL "label"
#define DT_PROPERTY_MY_VALUE "my-value"
