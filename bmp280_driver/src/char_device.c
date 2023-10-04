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

static char* device_name = NULL;
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
/// @param name Name of the device. It'll be seen as "/dev/<name>"
/// @return "0"on success, non zero error code on error.
int char_device_create(const char *name) {
    int retval = -1;

    // Get own copy of name
    if ((device_name = kmalloc(strlen(name) + 1, GFP_KERNEL)) == NULL) {
        printk(ERROR("Out of memory for char device.\n"));
        goto name_error;
    }
    strcpy(device_name, name);

    // Allocate device number (MAJOR and MINOR)
    if ((retval = alloc_chrdev_region(&device_number, MINOR_NUMBER, NUMBER_OF_DEVICES, device_name)) != 0) {
        printk(ERROR("Couldn't allocate device number.\n"));
        goto kmalloc_error;
    }

    // Create device class (This name will be seen as SUBSYSTEM=="DEVICE_CLASS_NAME"
    // for udev, and a folder with the same will be created inside "/sys/class/DEVICE_CLASS_NAME" )
    if ((device_class = class_create(THIS_MODULE, DEVICE_CLASS_NAME)) == NULL) {
        printk(ERROR("Device class couldn't be created.\n"));
        retval = -1;
        goto chrdev_error;
    }

    // Create device file (/sys/class/<DEVICE_CLASS_NAME>/<name>)
    if (device_create(device_class, NULL, device_number, NULL, device_name) == NULL) {
        printk(ERROR("Couldn't create device file.\n"));
        retval = -1;
        goto class_error;
    }

    // Initializing and registering device file
    cdev_init(&my_device, &fops);
    if ((retval = cdev_add(&my_device, device_number, NUMBER_OF_DEVICES)) != 0 ) {
        printk(ERROR("Registering of device to kernel failed.\n"));
        goto device_error;
    }

    printk(INFO("Device /dev/%s created successfully. Major: %d, Minor: %d.\n"),
        device_name, MAJOR(device_number), MINOR(device_number));
    return 0;

    device_error: device_destroy(device_class, device_number);
    class_error: class_destroy(device_class);
    chrdev_error: unregister_chrdev(device_number, device_name);
    kmalloc_error: kfree(device_name);
    name_error: return retval;
}

/// @brief Remove previously created device.
void char_device_remove(void) {
    device_destroy(device_class, device_number);
    class_destroy(device_class);
    unregister_chrdev(device_number, device_name);
    kfree(device_name);
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
    printk(INFO("Temperature of the device: %s.\n"), out_string);

    // Copy data to user, return bytes that hasn't copied
    not_copied = copy_to_user(user_buffer, out_string, to_copy);

    // Calculate data
    delta = to_copy - not_copied;
    return delta;
}
