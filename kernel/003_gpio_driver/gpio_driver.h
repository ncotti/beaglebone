#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>    // dev_t
#include <linux/fs.h>       // chrdev
#include <linux/cdev.h>     // cdev
#include <linux/uaccess.h>  // copy_from_user, copy_to_user
#include <linux/gpio.h>     // gpio

#define DEVICE_NAME "gpio_driver"
#define DEVICE_CLASS_NAME "my_module_class"
#define MINOR_NUMBER 0
#define NUMBER_OF_DEVICES 1
#define LED_PIN 60
#define LED_NAME "my_led"
#define LED_RESET_STATE 0
#define BUTTON_PIN 48
#define BUTTON_NAME "my_button"

// Module description
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nicolas G. Cotti");
MODULE_DESCRIPTION("A simple gpio driver for setting a LED and reading a button");
