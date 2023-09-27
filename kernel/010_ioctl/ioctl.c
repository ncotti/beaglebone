#include "ioctl.h"

/******************************************************************************
 * Static function's prototypes
******************************************************************************/

static int driver_close(struct inode *device_file, struct file *instance);
static int driver_open(struct inode *device_file, struct file *instance);
static long int driver_ioctl(struct file *file, unsigned cmd, unsigned long arg);

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
    .unlocked_ioctl = driver_ioctl,
};

// All values will be multiplied by this number
static int multiplier = 1;

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

/// @brief This function is called when the device is closed
static int driver_close(struct inode *device_file, struct file *instance) {
    printk("Device was closed!\n");
    return 0;
}

/// @brief This functions is called when the C function "ioctl()" is called from
///  user space.
static long int driver_ioctl(struct file *file, unsigned cmd, unsigned long arg) {
    int answer;
    switch(cmd) {
        case IOCTL_CMD_RESET:
            multiplier = 1;
            printk("Multiplier set to default value %d\n", multiplier);
        break;

        case IOCTL_CMD_MULTIPLY:
            if (copy_from_user(&answer, (int *) arg, sizeof(answer)) != 0) {
                printk("Error copying data from user\n");
                return -1;
            } else {
                answer *= multiplier;
                if (copy_to_user((int*) arg, &answer, sizeof(answer)) != 0) {
                    printk("Error copying data to user\n");
                    return -1;
                }
                printk("Value of multiplication copied: %d\n", answer);
            }
        break;
        case IOCTL_CMD_READ:
            if(copy_to_user((int*) arg, &multiplier, sizeof(multiplier)) != 0) {
                printk("Error copying data to user\n");
                return -1;
            } else {
                printk("The value of the multiplier is: %d\n", multiplier);
            }
        break;
        case IOCTL_CMD_WRITE:
            if(copy_from_user(&multiplier, (int*) arg , sizeof(multiplier)) != 0) {
                printk("Error copying data from user\n");
                return -1;
            } else {
                printk("Update multiplier to: %d\n", multiplier);
            }
        break;
    }
    return 0;
}

