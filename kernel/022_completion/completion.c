#include "completion.h"

/******************************************************************************
 * Static variables
******************************************************************************/

static struct timer_list my_timer;
static struct completion comp;

static void timer_callback(struct timer_list *timer);

/******************************************************************************
 * Init and exit
******************************************************************************/

/// @brief This function is called when the module is loaded into the kernel.
static int __init my_module_init(void) {
    int retval;

    printk("Hello, Kernel!\n");

    // Initialize completion
    init_completion(&comp);

    // Initialize timer
    timer_setup(&my_timer, timer_callback, 0);
    if (( retval = mod_timer(&my_timer, jiffies + msecs_to_jiffies(40))) != 0) {
        printk("Couldn't set the timer\n");
        return -1;
    }

    // After the timer was set, the code will block until a call to complete()
    if ((retval = wait_for_completion_timeout(&comp, msecs_to_jiffies(100))) == 0){
        printk("Timeout reached!\n");
    } else {
        printk("Completion cached!\n");
    }

    // Reuse completion
    reinit_completion(&comp);

    //Create new timer
    if (( retval = mod_timer(&my_timer, jiffies + msecs_to_jiffies(1000))) != 0) {
        printk("Couldn't set the timer\n");
        return -1;
    }

    // Should exit for timeout
    if ((retval = wait_for_completion_timeout(&comp, msecs_to_jiffies(100))) == 0){
        printk("Timeout reached!\n");
    } else {
        printk("Completion cached!\n");
    }

    return 0;
}

/// @brief This function is called when the module is removed from the kernel.
static void __exit my_module_exit(void) {
    del_timer(&my_timer);
    printk("Goodbye, Kernel\n");
}

// Set entrypoint and exit point for the kernel module.
module_init(my_module_init);
module_exit(my_module_exit);

/******************************************************************************
 * Callbacks
******************************************************************************/
static void timer_callback(struct timer_list *timer) {
    printk("Timer expired\n");

    // Completion set. Code can continue.
    complete(&comp);
}
