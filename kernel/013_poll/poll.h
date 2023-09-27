#ifndef SEND_SIGNAL_H
#define SEND_SIGNAL_H

#include <linux/module.h>
#include <linux/init.h>
#include <linux/gpio.h>
#include <linux/cdev.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <linux/ioctl.h>
#include <linux/poll.h>
#include <linux/wait.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nicolas G. Cotti");
MODULE_DESCRIPTION(
    "Creates a device. If you press a button, the gpio IRQ will register, and "
    "the event POLLIN will be set. From the user app, you can wait for this "
    "event using poll() in a blocking manner."
);

#define DEVICE_NAME "poll_driver"
#define DEVICE_CLASS_NAME "cotti"
#define MINOR_NUMBER 0
#define NUMBER_OF_DEVICES 1

#define BUTTON_PIN 48
#define BUTTON_NAME "my_button"
#define IRQ_NAME "my_gpio_irq"
#define DEBOUNCE_TIME 500

#endif // SEND_SIGNAL_H
