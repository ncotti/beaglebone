#include "char_device.h"

/******************************************************************************
 * Static functions' prototypes
******************************************************************************/

static int char_device_open(struct inode *device_file, struct file *instance);
static int char_device_close(struct inode *device_file, struct file *instance);
static ssize_t char_device_write(struct file *file, const char *user_buffer, size_t count, loff_t *offs);
static ssize_t char_device_read(struct file *file, char *user_buffer, size_t count, loff_t *offs);
static long int char_device_ioctl(struct file *file, unsigned cmd, unsigned long arg);

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
    .unlocked_ioctl = char_device_ioctl,
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
    if(!bmp280_is_connected()) {
        printk("Couldn't open device.\n");
        return -1;
    }
    return 0;
}

/// @brief This function is called when the device is closed
static int char_device_close(struct inode *device_file, struct file *instance) {
    return 0;
}

/// @brief Does nothing
static ssize_t char_device_write(struct file *file, const char __user *user_buffer, size_t count, loff_t *offs) {
    return 0;
}

/// @brief Read temperature. Returns a string like "50.80".
/// @return Amount of bytes read, or "-1" on error.
static ssize_t char_device_read(struct file *file, char __user *user_buffer, size_t count, loff_t *offs) {
    int to_copy, not_copied;
    char out_string[10];
    s32 temperature;

    // Get temperature
    if ((temperature = bmp280_read_temperature()) == BMP280_ERROR) {
        printk(ERROR("Couldn't read temperature.\n"));
        return -1;
    } else {
        snprintf(out_string, sizeof(out_string), "%d.%02d\n", temperature/100, temperature%100);
    }

    // Get amount of data to copy
    to_copy = min(count, strlen(out_string) + 1);

    // Copy data to user, return bytes that hasn't copied
    not_copied = copy_to_user(user_buffer, out_string, to_copy);

    return to_copy - not_copied;    // Return bytes copied
}

static long int char_device_ioctl(struct file *file, unsigned cmd, unsigned long __user arg) {
    int operation;
    if (copy_from_user(&operation, (int *) arg, sizeof(operation)) != 0) {
        printk(ERROR("Couldn't copy data from user.\n"));
        return -1;
    }
    switch(cmd) {
        case IOCTL_CMD_SET_FREQ:
            if (bmp280_set_frequency((bmp280_freq) operation) != 0) {
                printk("Couldn't change frequency.\n");
                return -1;
            }
            printk(INFO("Frequency changed to %d.\n"), operation);
        break;
        case IOCTL_CMD_SET_MODE:
            if (bmp280_set_mode((bmp280_mode) operation) != 0) {
                printk(ERROR("Couldn't change mode.\n"));
                return -1;
            }
            printk(INFO("Mode changed to %d.\n"), operation);
        break;
        case IOCTL_CMD_SET_UNIT:
            bmp280_set_unit((bmp280_unit) operation);
            printk(INFO("Unit changed to %d.\n"), operation);
        break;
        default:
            printk(ERROR("Unknown ioctl command.\n"));
            return -1;
        break;
    }
    return 0;
}
