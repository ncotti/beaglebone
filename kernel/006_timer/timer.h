#include <linux/init.h>
#include <linux/module.h>
#include <linux/gpio.h>
#include <linux/jiffies.h>
#include <linux/timer.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nicolas G. Cotti");
MODULE_DESCRIPTION("A test for LKM timers. Expected output is 010, with a delay of 1 second");

#define LED_PIN 60
#define LED_NAME "my_led"
#define LED_RESET_STATE 0