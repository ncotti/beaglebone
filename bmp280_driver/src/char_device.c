#include "char_device.h"

/******************************************************************************
 * Static functions' prototypes
******************************************************************************/

static int char_device_open(struct inode *device_file, struct file *instance);
static int char_device_close(struct inode *device_file, struct file *instance);
static ssize_t char_device_write(struct file *file, const char *user_buffer, size_t count, loff_t *offs);
static ssize_t char_device_read(struct file *file, char *user_buffer, size_t count, loff_t *offs);

/******************************************************************************
 * Static variables
******************************************************************************/

static dev_t device_number;
static struct class *device_class;
static struct cdev my_device;
static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = char_device_open,
    .release = char_device_close,
    .write = char_device_write,
    .read = char_device_read,
};

/******************************************************************************
 * Char device control
******************************************************************************/

/// @brief Creates the char device
/// @return "0"on success, non zero error code on error.
int char_device_create(void) {
    int retval = -1;

    // Allocate device number (MAJOR and MINOR)
    if ((retval = alloc_chrdev_region(&device_number, MINOR_NUMBER, NUMBER_OF_DEVICES, DEVICE_NAME)) != 0) {
        printk(KERN_ERR "Couldn't allocate device number\n");
        goto chrdev_error;
    }

    // Create device class (This name will be seen as SUBSYSTEM=="DEVICE_CLASS_NAME"
    // for udev, and a folder with the same will be created inside "/sys/class/DEVICE_CLASS_NAME" )
    if ((device_class = class_create(THIS_MODULE, DEVICE_CLASS_NAME)) == NULL) {
        printk(KERN_ERR "Device class couldn't be created\n");
        retval = -1;
        goto class_error;
    }

    // Create device file (/sys/class/DEVICE_CLASS_NAME/DEVICE_NAME)
    if (device_create(device_class, NULL, device_number, NULL, DEVICE_NAME) == NULL) {
        printk(KERN_ERR "Couldn't create device file\n");
        retval = -1;
        goto device_error;
    }

    // Initializing and registering device file
    cdev_init(&my_device, &fops);
    if ((retval = cdev_add(&my_device, device_number, NUMBER_OF_DEVICES)) != 0 ) {
        printk(KERN_ERR "Registering of device to kernel failed\n");
        goto cdev_error;
    }

    printk(KERN_INFO "Device /dev/%s created successfully.\n", DEVICE_NAME);
    printk(KERN_INFO "Major: %d, Minor: %d\n", MAJOR(device_number), MINOR(device_number));
    return 0;

    cdev_error: device_destroy(device_class, device_number);
    device_error: class_destroy(device_class);
    class_error: unregister_chrdev(device_number, DEVICE_NAME);
    chrdev_error: return retval;
}

/// @brief Remove previously created device.
void char_device_remove(void) {
    device_destroy(device_class, device_number);
    class_destroy(device_class);
    unregister_chrdev(device_number, DEVICE_NAME);
}

/******************************************************************************
 * File operations
******************************************************************************/

/// @brief This function is called when the device is opened
static int char_device_open(struct inode *device_file, struct file *instance) {
    return 0;
}

/// @brief This function is called when the device is opened
static int char_device_close(struct inode *device_file, struct file *instance) {
    return 0;
}

/// @brief Does nothing
static ssize_t char_device_write(struct file *file, const char *user_buffer, size_t count, loff_t *offs) {
    return 0;
}

/// @brief Read data out of the buffer
static ssize_t char_device_read(struct file *file, char *user_buffer, size_t count, loff_t *offs) {
    int to_copy, not_copied, delta;
    char out_string[50];
    int temperature;

    // Get amount of data to copy
    to_copy = min(count, sizeof(out_string));

    // Get temperature
    temperature = bmp280_read_temperature();
    snprintf(out_string, sizeof(out_string), "%d.%d\n", temperature/100, temperature%100);
    printk("Temperature of the device: %s\n", out_string);

    // Copy data to user, return bytes that hasn't copied
    not_copied = copy_to_user(user_buffer, out_string, to_copy);

    // Calculate data
    delta = to_copy - not_copied;
    return delta;
}
