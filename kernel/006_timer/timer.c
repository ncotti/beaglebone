#include "timer.h"

/******************************************************************************
 * Static variables
******************************************************************************/

static struct timer_list my_timer;

void timer_callback(struct timer_list *timer);

/******************************************************************************
 * Init and exit
******************************************************************************/

/// @brief This function is called when the module is loaded into the kernel.
static int __init my_module_init(void) {
    int retval;

    printk("Hello, Kernel!\n");

    // Led initialization
    if ((retval = gpio_request(LED_PIN, LED_NAME)) != 0) {
        printk("Couldn't allocate GPIO %d\n", LED_PIN);
        goto gpio_request_error;
    }
    if (( retval = gpio_direction_output(LED_PIN, LED_RESET_STATE)) != 0) {
        printk("Couldn't set GPIO %d to output\n", LED_PIN);
        goto led_error;
    }

    // Initialize timer
    timer_setup(&my_timer, timer_callback, 0);
    // jiffies is a global variable that contains the number of system ticks since startup
    if (( retval = mod_timer(&my_timer, jiffies + msecs_to_jiffies(1000))) != 0) {
        printk("Couldn't set the timer\n");
        goto led_error;
    }

    gpio_set_value(LED_PIN, 1);

    return 0;

    led_error: gpio_free(LED_PIN);
    gpio_request_error: return retval;
}

/// @brief This function is called when the module is removed from the kernel.
static void __exit my_module_exit(void) {
    gpio_set_value(LED_PIN, LED_RESET_STATE);
    gpio_free(LED_PIN);
    del_timer(&my_timer);
    printk("Goodbye, Kernel\n");
}

// Set entrypoint and exit point for the kernel module.
module_init(my_module_init);
module_exit(my_module_exit);

/******************************************************************************
 * Callbacks
******************************************************************************/
void timer_callback(struct timer_list *timer) {
    gpio_set_value(LED_PIN, 0);
}
