#ifndef CHAR_DEVICE_H
#define CHAR_DEVICE_H

#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/ioctl.h>
#include "bmp280.h"
#include "bmp280_types.h"
#include "log.h"

int char_device_create(const char *name);
void char_device_remove(void);

// This value can be used by "udev" rules. Check for 'SUBSYSTEM=="cotti"'.
#define DEVICE_CLASS_NAME "cotti"

// Minimum minor number that can be used.
#define MINOR_NUMBER 0

// Amount of devices that will be created
#define NUMBER_OF_DEVICES 1

#endif // CHAR_DEVICE_H
