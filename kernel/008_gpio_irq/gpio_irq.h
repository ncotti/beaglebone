#include <linux/init.h>
#include <linux/module.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nicolas G. Cotti");
MODULE_DESCRIPTION("A simple gpio interrupt LKM. See the button press on the logs");

#define BUTTON_PIN 48
#define BUTTON_NAME "my_button"
#define IRQ_NAME "my_gpio_irq"
#define DEBOUNCE_TIME 500
