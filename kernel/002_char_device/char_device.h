#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>    // dev_t
#include <linux/fs.h>       // chrdev
#include <linux/cdev.h>     // cdev
#include <linux/uaccess.h>  // copy_from_user, copy_to_user

#define DEVICE_NAME "echo_driver"
#define DEVICE_CLASS_NAME "cotti"
#define MINOR_NUMBER 0
#define NUMBER_OF_DEVICES 1

// Module description
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nicolas G. Cotti");
MODULE_DESCRIPTION("Creates a device driver in the directory /dev/echo_driver");