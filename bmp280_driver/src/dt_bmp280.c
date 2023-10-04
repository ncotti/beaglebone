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
    if (cotti_i2c_init(pdev) != 0) {
        return -1;
    }
    if (bmp280_init() != 0) {
        return -1;
    }
    if (char_device_create() != 0) {
        return -1;
    }

    return 0;

    // TODO if dt_probe fails, need to unitialize here

}

static int dt_remove(struct platform_device *pdev) {
    printk("Now I am in the remove function\n");
    char_device_remove();
    cotti_i2c_deinit();
    return 0;
}
