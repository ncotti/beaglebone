#ifndef CHAR_DEVICE_H
#define CHAR_DEVICE_H

#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>

#include "bmp280.h"

#define DEVICE_NAME "dt_i2c_driver"
#define DEVICE_CLASS_NAME "cotti"
#define MINOR_NUMBER 0
#define NUMBER_OF_DEVICES 1

int char_device_create(void);
void char_device_remove(void);

#endif // CHAR_DEVICE_H
