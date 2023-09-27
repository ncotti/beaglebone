#include "char_device.h"

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

static char buffer[255];
static int buffer_pointer;

/******************************************************************************
 * Init and exit
******************************************************************************/

/// @brief This function is called when the module is loaded into the kernel.
static int __init my_module_init(void) {
    int retval;
    printk("Hello, Kernel!\n");

    // Allocate device number (MAJOR and MINOR)
    if ((retval = alloc_chrdev_region(&device_number, MINOR_NUMBER, NUMBER_OF_DEVICES, DEVICE_NAME)) != 0) {
        printk("Couldn't allocate device number\n");
        goto chrdev_error;
    }
    printk("Device number allocated successfully! Major: %d, Minor: %d\n",
        MAJOR(device_number), MINOR(device_number));

    // Create device class (This name will be seen as SUBSYSTEM=="DEVICE_CLASS_NAME"
    // for udev, and a folder with the same will be created inside "/sys/class/DEVICE_CLASS_NAME" )
    if ((device_class = class_create(THIS_MODULE, DEVICE_CLASS_NAME)) == NULL) {
        printk("Device class couldn't be created\n");
        retval = -1;
        goto class_error;
    }

    // Create device file (/sys/class/DEVICE_CLASS_NAME/DEVICE_NAME)
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
    return 0;

    cdev_error: device_destroy(device_class, device_number);
    device_error: class_destroy(device_class);
    class_error: unregister_chrdev(device_number, DEVICE_NAME);
    chrdev_error: return retval;
}

/// @brief This function is called when the module is removed from the kernel
static void __exit my_module_exit(void) {
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

/// @brief Read data out of the buffer
static ssize_t driver_read(struct file *file, char *user_buffer, size_t count, loff_t *offs) {
    int to_copy, not_copied, delta;

    // Get amount of data to copy
    to_copy = min(count, buffer_pointer);

    // Copy data to user, return bytes that hasn't copied
    not_copied = copy_to_user(user_buffer, buffer, to_copy);

    // Calculate data
    delta = to_copy - not_copied;
    return delta;
}

/// @brief Write data to buffer
static ssize_t driver_write(struct file *file, const char *user_buffer, size_t count, loff_t *offs) {
    int to_copy, not_copied, delta;

    // Get amount of data to copy
    to_copy = min(count, sizeof(buffer));

    // Copy data to user, return bytes that hasn't copied
    not_copied = copy_from_user(buffer, user_buffer, to_copy);
    buffer_pointer = to_copy;

    // Calculate data
    delta = to_copy - not_copied;
    return delta;
}

