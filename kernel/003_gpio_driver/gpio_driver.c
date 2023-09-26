#include "gpio_driver.h"

/******************************************************************************
 * Static function's prototypes
******************************************************************************/

static int driver_close(struct inode *device_file, struct file *instance);
static int driver_open(struct inode *device_file, struct file *instance);
static ssize_t driver_read(struct file *file, char *user_buffer, size_t count, loff_t *offs);
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
    .read = driver_read,
    .write = driver_write,
};

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

    // Led initialization
    if ((retval = gpio_request(LED_PIN, LED_NAME)) != 0) {
        printk("Couldn't allocate GPIO %d\n", LED_PIN);
        goto cdev_error;
    }
    if (( retval = gpio_direction_output(LED_PIN, LED_RESET_STATE)) != 0) {
        printk("Couldn't set GPIO %d to output\n", LED_PIN);
        goto led_error;
    }

    // Button initialization
    if ((retval = gpio_request(BUTTON_PIN, BUTTON_NAME)) != 0) {
        printk("Couldn't allocate GPIO %d\n", BUTTON_PIN);
        goto led_error;
    }
    if (( retval = gpio_direction_input(BUTTON_PIN)) != 0) {
        printk("Couldn't set GPIO %d to input\n", BUTTON_PIN);
        goto button_error;
    }

    return 0;

    button_error: gpio_free(BUTTON_PIN);
    led_error: gpio_free(LED_PIN);
    cdev_error: device_destroy(device_class, device_number);
    device_error: class_destroy(device_class);
    class_error: unregister_chrdev(device_number, DEVICE_NAME);
    chrdev_error: return retval;
}

/// @brief This function is called when the module is removed from the kernel
static void __exit my_module_exit(void) {
    gpio_set_value(LED_PIN, LED_RESET_STATE);
    gpio_free(BUTTON_PIN);
    gpio_free(LED_PIN);
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

/// @brief When read, it will return "0" or "1", depending of the value of the button
static ssize_t driver_read(struct file *file, char *user_buffer, size_t count, loff_t *offs) {
    int to_copy, not_copied, delta;
    char tmp[3] = " \n";

    // Get amount of data to copy
    to_copy = min(count, sizeof(tmp));

    // Read value of button
    printk("Value of button %d\n", gpio_get_value(BUTTON_PIN));
    tmp[0] = gpio_get_value(BUTTON_PIN) + '0';

    // Copy data to user, return bytes that hasn't copied
    not_copied = copy_to_user(user_buffer, tmp, to_copy);

    // Calculate data
    delta = to_copy - not_copied;
    return delta;
}

/// @brief When written "0" or "1", it will turn on the GPIO.
static ssize_t driver_write(struct file *file, const char *user_buffer, size_t count, loff_t *offs) {
    int to_copy, not_copied, delta;
    char value;

    // Get amount of data to copy
    to_copy = min(count, sizeof(value));

    // Copy data to user, return bytes that hasn't copied
    not_copied = copy_from_user(&value, user_buffer, to_copy);

    switch(value) {
        case '0':
            gpio_set_value(LED_PIN, 0);
        break;
        case '1':
            gpio_set_value(LED_PIN, 1);
        break;
        default:
            printk("Invalid input\n");
        break;
    }

    // Calculate data
    delta = to_copy - not_copied;
    return delta;
}

