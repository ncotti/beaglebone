#ifndef IOCTL_H
#define IOCTL_H

#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/ioctl.h>
#include "ioctl_types.h"

#define DEVICE_NAME "ioctl_driver"
#define DEVICE_CLASS_NAME "cotti"
#define MINOR_NUMBER 0
#define NUMBER_OF_DEVICES 1

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nicolas G. Cotti");
MODULE_DESCRIPTION(
    "Example for ioctl usage in a LKM. This driver receives some ioctl and does the following:"
    "* IOCTL_CMD_MULTIPLY (arg): return arg * multiplier;"
    "* IOCTL_CMD_RESET: multiplier = 1;"
    "* IOCTL_CMD_READ: return multiplier;"
    "* IOCTL_CMD_WRITE(arg): multiplier = arg;");

#endif // IOCTL_H