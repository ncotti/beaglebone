#ifndef SEND_SIGNAL_H
#define SEND_SIGNAL_H

#include <linux/module.h>
#include <linux/init.h>
#include <linux/gpio.h>
#include <linux/cdev.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <linux/sched/signal.h>
#include <linux/ioctl.h>

#include "ioctl_types.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nicolas G. Cotti");
MODULE_DESCRIPTION(
    "Creates a device. If a program registers itself with ioctl(), "
    "then a signal will be sent every time a button is pressed."
);

#define DEVICE_NAME "signal_driver"
#define DEVICE_CLASS_NAME "cotti"
#define MINOR_NUMBER 0
#define NUMBER_OF_DEVICES 1

#define BUTTON_PIN 48
#define BUTTON_NAME "my_button"
#define IRQ_NAME "my_gpio_irq"
#define DEBOUNCE_TIME 500

#endif // SEND_SIGNAL_H
