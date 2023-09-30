#include "mutex.h"

/******************************************************************************
 * Static variables
******************************************************************************/

static struct task_struct *kthread1, *kthread2;
static int arg_t1 = 1, arg_t2 = 2;  // Arguments for threads must be static
static struct mutex lock;

static int thread_function(void *arg);

/******************************************************************************
 * Init and exit
******************************************************************************/

/// @brief This function is called when the module is loaded into the kernel.
static int __init my_module_init(void) {
    printk("Hello, Kernel!\n");

    // Initialize mutex variable
    mutex_init(&lock);

    // First way of initializing a kthread
    if ((kthread1 = kthread_create(thread_function, &arg_t1, "Thread_1")) == NULL) {
        printk("Couldn't create first thread\n");
        goto kthread1_error;
    }
    wake_up_process(kthread1);
    printk("Thread 1 was created, and it's running now!\n");

    // Second way of initializing a kthread
    if ((kthread2 = kthread_run(thread_function, &arg_t2, "Thread_2")) == NULL) {
        printk("Couldn't create second thread\n");
        goto kthread2_error;
    }
    printk("Thread 2 was created, and it's running now!\n");

    return 0;

    kthread2_error: kthread_stop(kthread1);
    kthread1_error: return -1;
}

/// @brief This function is called when the module is removed from the kernel.
static void __exit my_module_exit(void) {
    printk("Goodbye, Kernel\n");

    // No need to stop the threads if they return naturally
    // kthread_stop(kthread1);
    // kthread_stop(kthread2);
}

// Set entrypoint and exit point for the kernel module.
module_init(my_module_init);
module_exit(my_module_exit);

/******************************************************************************
 * Thread function
******************************************************************************/

/// @brief This function will keep executing until the module is removed.
static int thread_function(void *arg) {
    int thread_number = *(int *) arg;
    printk("Thread %d is executed\n", thread_number);

    mutex_lock(&lock);
    printk("Thread %d is in critical section!\n", thread_number);
    msleep(thread_number * 1000);
    printk("Thread %d is leaving the critical section!\n", thread_number);
    mutex_unlock(&lock);

    printk("Thread %d finished execution\n", thread_number);
    return 0;
}
