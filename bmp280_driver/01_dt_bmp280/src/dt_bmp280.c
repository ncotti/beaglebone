#include "dt_bmp280.h"

/******************************************************************************
 * Static variables
******************************************************************************/

static int i2c_probe(struct i2c_client *client, const struct i2c_device_id *id);
static int i2c_remove(struct i2c_client *client);

// Used to identify a device in the device tree. Matches "compatible" property
static struct of_device_id my_driver_ids[] = {
    {
        .compatible = DT_COMPATIBLE,
    }, { /*Don't remove*/ },
};
MODULE_DEVICE_TABLE(of, my_driver_ids);

// Used to identify i2c devices from the device tree
static struct i2c_device_id i2c_ids[] = {
    {DT_CHILD_DEVICE_NAME, 0},
    {},
};
MODULE_DEVICE_TABLE(i2c, i2c_ids);

// Instead of using a platform driver, we are defining an "i2c_driver".
// The platform driver is a generic description of a device in the device-tree,
// the i2c driver is specifically designed to be used for i2c devices.
static struct i2c_driver my_driver = {
    .probe = i2c_probe,
    .remove = i2c_remove,
    .id_table = i2c_ids,
    .driver = {
        .name = DRIVER_NAME,
        .of_match_table = my_driver_ids,
    },
};



/******************************************************************************
 * Init and exit
******************************************************************************/

// This will create the init and exit function automatically
module_i2c_driver(my_driver);

/******************************************************************************
 * I2C from device tree functions
******************************************************************************/

/// @brief Called when an i2c device matches this driver
static int i2c_probe(struct i2c_client *client, const struct i2c_device_id *id) {
    printk(KERN_INFO "Now I am in the probe function\n");

    if (char_device_create() != 0) {
        return -1;
    }
    if (bmp280_init(client) != 0) {
        return -1;
    }
    return 0;
}

static int i2c_remove(struct i2c_client *client) {
    char_device_remove();
    return 0;
}
