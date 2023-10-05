#include "driver.h"

/******************************************************************************
 * Static functions' prototypes
******************************************************************************/

static int driver_probe(struct platform_device *pdev);
static int driver_remove(struct platform_device *pdev);

/******************************************************************************
 * Static variables
******************************************************************************/

// Used to identify a device in the device tree. Matches "compatible" property
static struct of_device_id my_driver_ids[] = {
    {.compatible = DT_COMPATIBLE}, { /*Don't remove*/ }
};
MODULE_DEVICE_TABLE(of, my_driver_ids);

// Platform driver, with "probe" and "remove" functions when a matching device
// is found
static struct platform_driver my_driver = {
    .probe = driver_probe,
    .remove = driver_remove,
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
    printk(INFO("Initializing LKM.\n"));

    // Register driver
    if(platform_driver_register(&my_driver) != 0) {
        printk(ERROR("Couldn't load driver.\n"));
        return -1;
    }
    return 0;
}

/// @brief This function is called when the module is removed from the kernel.
static void __exit my_module_exit(void) {
    printk(INFO("Removing LKM.\n"));
    platform_driver_unregister(&my_driver);
}

// Set entrypoint and exit point for the kernel module.
module_init(my_module_init);
module_exit(my_module_exit);

/******************************************************************************
 * Device tree functions
******************************************************************************/

/// @brief This function is called when a device matches the "compatible"
///  property in the device tree.
/// @param pdev Reference to the device tree.
/// @return "0" on success, not "0" on error.
static int driver_probe(struct platform_device *pdev) {
    int retval = -1;

    printk(INFO("Installing driver for %s.\n"), pdev->name);
    if ((retval = cotti_i2c_init(pdev)) != 0) {
        goto base_error;
    }
    if ((retval = bmp280_init(pdev)) != 0) {
        goto i2c_error;
    }
    if ((retval = char_device_create(DEVICE_NAME)) != 0) {
        goto bmp280_error;
    }
    return 0;

    bmp280_error: bmp280_deinit();
    i2c_error: cotti_i2c_deinit();
    base_error: return retval;
}

/// @brief This function is called when the driver is removed, for every device
///  that matched the "compatible" property in the device tree.
/// @param pdev Reference to the device tree.
/// @return "0" on success, not "0" on error.
static int driver_remove(struct platform_device *pdev) {
    printk(INFO("Removing driver for %s.\n"), pdev->name);
    char_device_remove();
    bmp280_deinit();
    cotti_i2c_deinit();
    return 0;
}
