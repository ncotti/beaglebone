#include "high_res_timer.h"

/******************************************************************************
 * Static variables
******************************************************************************/

static struct hrtimer my_hrtimer;
u64 start_time;

static enum hrtimer_restart timer_callback(struct hrtimer *timer) ;

/******************************************************************************
 * Init and exit
******************************************************************************/

/// @brief This function is called when the module is loaded into the kernel.
static int __init my_module_init(void) {
    printk("Hello, Kernel!\n");

    // Init of hrtimer. hrtimer expire after the callback, and don't need to be
    // destroyed
    hrtimer_init(&my_hrtimer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
    my_hrtimer.function = &timer_callback;
    start_time = jiffies;
    hrtimer_start(&my_hrtimer, ms_to_ktime(100), HRTIMER_MODE_REL);
    return 0;
}

/// @brief This function is called when the module is removed from the kernel.
static void __exit my_module_exit(void) {
    hrtimer_cancel(&my_hrtimer);    // cancel timer if not called, otherwise does nothing
    printk("Goodbye, Kernel\n");
}

// Set entrypoint and exit point for the kernel module.
module_init(my_module_init);
module_exit(my_module_exit);

/******************************************************************************
 * Callbacks
******************************************************************************/

static enum hrtimer_restart timer_callback(struct hrtimer *timer) {
    // Get current time
    u64 now_time = jiffies;
    printk("now_time - start_time = %u\n", jiffies_to_msecs(now_time - start_time));

    return HRTIMER_NORESTART;   // Can return HRTIMER_RESTART to make periodic
}
