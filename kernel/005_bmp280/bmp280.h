#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>    // dev_t
#include <linux/fs.h>       // chrdev
#include <linux/cdev.h>     // cdev
#include <linux/uaccess.h>  // copy_from_user, copy_to_user
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/kernel.h>
#include <linux/version.h>

#define DEVICE_NAME "bmp280_driver"
#define DEVICE_CLASS_NAME "my_module_class"
#define MINOR_NUMBER 0
#define NUMBER_OF_DEVICES 1
#define I2C_BUS_NUMBER 1
#define SLAVE_DEVICE_NAME "BMP280"
#define BMP280_SLAVE_ADDRESS 0x76

#define ADDRESS_ID 0xD0

// Module description
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nicolas G. Cotti");
MODULE_DESCRIPTION("A simple i2c driver");