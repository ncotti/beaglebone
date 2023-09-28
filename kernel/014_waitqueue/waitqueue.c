#include "waitqueue.h"

/******************************************************************************
 * Static variables
******************************************************************************/

static int driver_open(struct inode *device_file, struct file *instance);
static int driver_close(struct inode *device_file, struct file *instance);
static ssize_t driver_write(struct file *file, const char *user_buffer, size_t count, loff_t *offs);
static int thread_function(void* arg);

static dev_t device_number;
static struct class *device_class;
static struct cdev my_device;
static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = driver_open,
    .release = driver_close,
    .write = driver_write,
};

static struct task_struct *kthread1, *kthread2;

static long int watch_var = 0;      // Variable written to the device
static int arg_t1 = 1, arg_t2 = 2;  // Arguments for threads

// Declare wait queues. There are two versions of wait queue functions.
// Normal ones "wait_event()" and "wake_up()" are NOT interruptible, meaning
// that the process will block even after an attemp to remove the LKM!
// It's recommended to use the "interruptible" versions.
DECLARE_WAIT_QUEUE_HEAD(waitqueue1);
DECLARE_WAIT_QUEUE_HEAD(waitqueue2);

/******************************************************************************
 * Init and exit
******************************************************************************/

/// @brief This function is called when the module is loaded into the kernel.
static int __init my_module_init(void) {
    int retval = -1;
    printk("Hello, Kernel!\n");

    // Allocate device number (MAJOR and MINOR)
    if ((retval = alloc_chrdev_region(&device_number, MINOR_NUMBER, NUMBER_OF_DEVICES, DEVICE_NAME)) != 0) {
        printk("Couldn't allocate device number\n");
        goto chrdev_error;
    }
    printk("Device number allocated successfully! Major: %d, Minor: %d\n",
        MAJOR(device_number), MINOR(device_number));

    // Create device class
    if ((device_class = class_create(THIS_MODULE, DEVICE_CLASS_NAME)) == NULL) {
        printk("Device class couldn't be created\n");
        retval = -1;
        goto class_error;
    }

    // Create device file
    if (device_create(device_class, NULL, device_number, NULL, DEVICE_NAME) == NULL) {
        printk("Couldn't create device file\n");
        retval = -1;
        goto device_error;
    }

    // Initializing and registering device file
    cdev_init(&my_device, &fops);
    if ((retval = cdev_add(&my_device, device_number, NUMBER_OF_DEVICES)) != 0 ) {
        printk("Registering of device to kernel failed\n");
        goto cdev_error;
    }

    if ((kthread1 = kthread_run(thread_function, &arg_t1, "Thread_1")) == NULL) {
        printk("Couldn't create first thread\n");
        retval = -1;
        goto cdev_error;
    }

    if ((kthread2 = kthread_run(thread_function, &arg_t2, "Thread_2")) == NULL) {
        printk("Couldn't create second thread\n");
        retval = -1;
        goto kthread_error;
    }

    return 0;

    kthread_error: kthread_stop(kthread1);
    cdev_error: device_destroy(device_class, device_number);
    device_error: class_destroy(device_class);
    class_error: unregister_chrdev(device_number, DEVICE_NAME);
    chrdev_error: return retval;
}

/// @brief This function is called when the module is removed from the kernel.
static void __exit my_module_exit(void) {
    printk("Goodbye, Kernel\n");
    kthread_stop(kthread1);
    kthread_stop(kthread2);
    device_destroy(device_class, device_number);
    class_destroy(device_class);
    unregister_chrdev(device_number, DEVICE_NAME);
}

// Set entrypoint and exit point for the kernel module.
module_init(my_module_init);
module_exit(my_module_exit);

/******************************************************************************
 * File operations
******************************************************************************/

/// @brief This function is called when the device is opened
static int driver_open(struct inode *device_file, struct file *instance) {
    printk("Device was opened!\n");
    return 0;
}

/// @brief This function is called when the device is opened
static int driver_close(struct inode *device_file, struct file *instance) {
    printk("Device was closed!\n");
    return 0;
}

/// @brief Write data to buffer
static ssize_t driver_write(struct file *file, const char *user_buffer, size_t count, loff_t *offs) {
    int to_copy, not_copied, delta;
    char buffer[16];

    memset(buffer, 0, sizeof(buffer));
    // Get amount of data to copy
    to_copy = min(count, sizeof(buffer));

    // Copy data to user, return bytes that hasn't copied
    not_copied = copy_from_user(buffer, user_buffer, to_copy);

    // Calculate data
    delta = to_copy - not_copied;

    // Convert string to long
    if(kstrtol(buffer, BASE_10, &watch_var) == -EINVAL) {
        printk("Error converting input!\n");
    }
    printk("Watch var is now %ld\n", watch_var);

    // Wait queue's condition is only evaluated after a wake_up() call.
    wake_up_interruptible(&waitqueue1);
    wake_up_interruptible(&waitqueue2);
    return delta;
}

/******************************************************************************
 * Thread function
******************************************************************************/

static int thread_function(void* arg) {
    int thread_number = *(int*) arg;
    switch(thread_number) {
        case 1:
            // Wait forever until condition is true. Condition is only evaluated
            // after a call to wake_up(&waitqueue1)
            while(!kthread_should_stop()) {
                wait_event_interruptible(waitqueue1, watch_var == WAKE_UP_THREAD1 || kthread_should_stop());
                printk("Thread %d. User input was %d\n", thread_number, WAKE_UP_THREAD1);
                watch_var = 0;
            }
        break;
        case 2:
            // Wait for certain timeout until condition is true. On timeout, returns 0.
            // Condition is only evaluated after a call to wake_up(&waitqueue2)
            while (!kthread_should_stop()) {
                if (wait_event_interruptible_timeout(waitqueue2,
                        watch_var == WAKE_UP_THREAD2 || kthread_should_stop(),
                        msecs_to_jiffies(5000)) == 0) {
                    printk("Thread %d. Timeout reached!", thread_number);
                } else {
                    printk("Thread %d. User input was %d\n", thread_number, WAKE_UP_THREAD2);
                    watch_var = 0;
                }
            }
        break;
        default: break;
    }
    printk("Thread %d exiting!\n", thread_number);
    return 0;
}
