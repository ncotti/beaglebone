#include "pwm_driver.h"

/******************************************************************************
 * Static function's prototypes
******************************************************************************/

static int driver_close(struct inode *device_file, struct file *instance);
static int driver_open(struct inode *device_file, struct file *instance);
static ssize_t driver_write(struct file *file, const char *user_buffer, size_t count, loff_t *offs);

/******************************************************************************
 * Static variables
******************************************************************************/

static dev_t device_number;
static struct class *device_class;
static struct cdev my_device;
static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = driver_open,
    .release = driver_close,
    .write = driver_write,
};

struct pwm_device *pwm0 = NULL;
u32 pwm_on_time = PWM_DEFAULT_DUTY_CYCLE*PWM_PERIOD;

/******************************************************************************
 * Init and exit
******************************************************************************/

/// @brief This function is called when the module is loaded into the kernel.
static int __init my_module_init(void) {
    int retval;
    printk("Hello, Kernel!\n");

    // Allocate device number
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

    // PWM initialization
    if ((pwm0 = pwm_request(PWM_NUMBER, PWM_NAME)) != 0) {
        printk("Couldn't get PWM%d\n", PWM_NUMBER);
        goto cdev_error;
    }
    pwm_config(pwm0, pwm_on_time, PWM_PERIOD);
    pwm_enable(pwm0);

    return 0;

    cdev_error: device_destroy(device_class, device_number);
    device_error: class_destroy(device_class);
    class_error: unregister_chrdev(device_number, DEVICE_NAME);
    chrdev_error: return retval;
}

/// @brief This function is called when the module is removed from the kernel
static void __exit my_module_exit(void) {
    pwm_disable(pwm0);
    pwm_free(pwm0);
    cdev_del(&my_device);
    device_destroy(device_class, device_number);
    class_destroy(device_class);
    unregister_chrdev(device_number, DEVICE_NAME);
    printk("Goodbye, Kernel\n");
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


/// @brief When written, "0" to "100", it will change the duty cycle of the PWM signal.
static ssize_t driver_write(struct file *file, const char *user_buffer, size_t count, loff_t *offs) {
    int to_copy, not_copied, delta;
    char duty_cycle[4] = "000";
    int duty_cycle_number;

    // Get amount of data to copy
    to_copy = min(count, sizeof(duty_cycle));

    // Copy data from user, return bytes that hasn't copied
    not_copied = copy_from_user(&duty_cycle[sizeof(duty_cycle) - to_copy], user_buffer, to_copy);

    // Set PWM duty cycle. Value should be from "0" to "100"
    if (duty_cycle[0] > '1' || duty_cycle[0] < '0' ||
        duty_cycle[1] > '9' || duty_cycle[1] < '0' ||
        duty_cycle[2] > '9' || duty_cycle[2] < '0' ) {
        printk("Invalid value!\n");
    } else {
        duty_cycle_number = duty_cycle[0]*100 + duty_cycle[1]*10 + duty_cycle[2] - 3*'0';
        printk("Setting duty cycle to %d%%\n", duty_cycle_number);
        pwm_config(pwm0, PWM_PERIOD*duty_cycle_number/100, PWM_PERIOD);
    }

    // Calculate data
    delta = to_copy - not_copied;
    return delta;
}

