#ifndef INFO_H
#define INFO_H

#include <linux/kern_levels.h>

// Can be any name here, used for logging to the kernel and naming the driver
#define DRIVER_NAME "temp-sensor"

#define WARNING(msg)  KERN_WARNING DRIVER_NAME ": " msg
#define ERROR(msg)    KERN_ERR DRIVER_NAME ": " msg
#define INFO(msg)     KERN_INFO DRIVER_NAME ": " msg

#endif // INFO_H
