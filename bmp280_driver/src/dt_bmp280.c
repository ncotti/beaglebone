#include "dt_bmp280.h"

/******************************************************************************
 * Static variables
******************************************************************************/

static int dt_probe(struct platform_device *pdev);
static int dt_remove(struct platform_device *pdev);

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

static int irq;

/******************************************************************************
 * Init and exit
******************************************************************************/

/// @brief This function is called when the module is loaded into the kernel.
static int __init my_module_init(void) {
    printk("Hello, Kernel!\n");

    // Register driver
    if(platform_driver_register(&my_driver) != 0) {
        printk("Couldn't load driver\n");
        return -1;
    }
    return 0;
}

/// @brief This function is called when the module is removed from the kernel.
static void __exit my_module_exit(void) {
    printk("Goodbye, Kernel\n");
    platform_driver_unregister(&my_driver);
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
        printk("Device property \"label\" not found\n");
        return -1;
    } else if (!device_property_present(dev, DT_PROPERTY_MY_VALUE)) {
        printk("Device property \"label\" not found\n");
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
    printk("Label: %s\nMy value: %d\n", label, my_value);

    if ((irq = platform_get_irq(pdev, 0)) < 0) {
        printk("Couldn't get IRQ\n");
        return irq;
    }

    // if ((ret = request_irq (irq, (irq_handler_t) cotti_i2c_isr ,IRQF_TRIGGER_RISING, pdev->name, NULL)) < 0) {
    //     printk("Couldn't get IRQ from platform\n");
    //     return ret;
    // }

    printk("IRQ %d configured!\n", irq);

    if (char_device_create() != 0) {
        return -1;
    }
    if (cotti_i2c_init() != 0) {
        return -1;
    }
    // if (bmp280_init() != 0) {
    //     return -1;
    // }

    printk("Values read: %x %x %x %x %x %x %x %x %x %x\n", cotti_i2c_read(0xD0),
    cotti_i2c_read(0x89), cotti_i2c_read(0x8a), cotti_i2c_read(0x8b),
    cotti_i2c_read(0x8c), cotti_i2c_read(0x8d), cotti_i2c_read(0x8e),
    cotti_i2c_read(0x8f), cotti_i2c_read(0x90), cotti_i2c_read(0x91));
    return 0;
}

static int dt_remove(struct platform_device *pdev) {
    //free_irq(irq, NULL);
    char_device_remove();
    cotti_i2c_deinit();
    printk("Now I am in the remove function\n");
    return 0;
}
