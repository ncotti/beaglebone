#include "dt_gpio.h"

/******************************************************************************
 * Static variables
******************************************************************************/

static int dt_probe(struct platform_device *pdev);
static int dt_remove(struct platform_device *pdev);
static int driver_open(struct inode *device_file, struct file *instance);
static int driver_close(struct inode *device_file, struct file *instance);
static ssize_t driver_write(struct file *file, const char *user_buffer, size_t count, loff_t *offs);

// Used to identify a device in the device tree. Matches "compatible" property
static struct of_device_id my_driver_ids[] = {
    {
        .compatible = DT_COMPATIBLE,
    }, { /*Don't remove*/ }
};
MODULE_DEVICE_TABLE(of, my_driver_ids);

// Similar to the file operations, we define functions to be called when the
// device is discovered (probe) and removed.
static struct platform_driver my_driver = {
    .probe = dt_probe,
    .remove = dt_remove,
    .driver = {
        .name = DRIVER_NAME,
        .of_match_table = my_driver_ids,
    },
};

static struct gpio_desc *my_led = NULL;

static dev_t device_number;
static struct class *device_class;
static struct cdev my_device;
static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = driver_open,
    .release = driver_close,
    .write = driver_write,
};

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

    // Register driver
    if(platform_driver_register(&my_driver) != 0) {
        printk("Couldn't load driver\n");
        retval = -1;
        goto cdev_error;
    }
    return 0;

    cdev_error: device_destroy(device_class, device_number);
    device_error: class_destroy(device_class);
    class_error: unregister_chrdev(device_number, DEVICE_NAME);
    chrdev_error: return retval;
}

/// @brief This function is called when the module is removed from the kernel.
static void __exit my_module_exit(void) {
    printk("Goodbye, Kernel\n");
    platform_driver_unregister(&my_driver);
    device_destroy(device_class, device_number);
    class_destroy(device_class);
    unregister_chrdev(device_number, DEVICE_NAME);
}

// Set entrypoint and exit point for the kernel module.
module_init(my_module_init);
module_exit(my_module_exit);


/******************************************************************************
 * Device tree functions
******************************************************************************/

static int dt_probe(struct platform_device *pdev) {
    struct device *dev = &pdev->dev;
    const char *label;
    int my_value, ret;

    // Check for device properties
    if (!device_property_present(dev, DT_PROPERTY_LABEL)) {
        printk("Device property \"%s\" not found\n", DT_PROPERTY_LABEL);
        return -1;
    } else if (!device_property_present(dev, DT_PROPERTY_MY_VALUE)) {
        printk("Device property \"%s\" not found\n", DT_PROPERTY_MY_VALUE);
        return -1;
    } else if (!device_property_present(dev, DT_PROPERTY_GPIO)) {
        printk("Device property \"%s\" not found\n", DT_PROPERTY_GPIO);
        return -1;
    }

    // Read device properties
    if ((ret = device_property_read_string(dev, DT_PROPERTY_LABEL, &label)) != 0) {
        printk("Couldn't read label\n");
        return -1;
    }
    if ((ret = device_property_read_u32(dev, DT_PROPERTY_MY_VALUE, &my_value)) != 0) {
        printk("Couldn't read my_value\n");
        return -1;
    }

    // Init GPIO
    if (IS_ERR(my_led = gpiod_get(dev, DT_PROPERTY_GPIO, GPIOD_OUT_LOW))) {
        printk("Couldn't set up the gpio\n");
        return -1 * IS_ERR(my_led);
    }

    printk("Label: %s\nMy value: %d\n", label, my_value);
    return 0;
}

static int dt_remove(struct platform_device *pdev) {
    printk("Now I am in the remove function\n");
    return 0;
}


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

/// @brief Change GPIO state. Write either "1" or "0"
static ssize_t driver_write(struct file *file, const char *user_buffer, size_t count, loff_t *offs) {
    int to_copy, not_copied, delta;
    char buffer[10];
    long int number;

    memset(buffer, 0, sizeof(buffer));

    // Get amount of data to copy
    to_copy = min(count, sizeof(buffer));

    // Copy data to user, return bytes that hasn't copied
    not_copied = copy_from_user(buffer, user_buffer, to_copy);

    // Convert string to long
    if(kstrtol(buffer, BASE_10, &number) == -EINVAL) {
        printk("Error converting input!\n");
    } else {
        switch (number) {
            case 0:
            case 1:
                gpiod_set_value(my_led, number);
                printk("Wrote %ld to led\n", number);
            default:
            break;
        }
    }

    // Calculate data
    delta = to_copy - not_copied;
    return delta;
}
