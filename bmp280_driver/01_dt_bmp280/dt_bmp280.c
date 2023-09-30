#include "dt_bmp280.h"

/******************************************************************************
 * Static variables
******************************************************************************/

static int driver_open(struct inode *device_file, struct file *instance);
static int driver_close(struct inode *device_file, struct file *instance);
static ssize_t driver_write(struct file *file, const char *user_buffer, size_t count, loff_t *offs);
static ssize_t driver_read(struct file *file, char *user_buffer, size_t count, loff_t *offs);
static int i2c_probe(struct i2c_client *client, const struct i2c_device_id *id);
static int i2c_remove(struct i2c_client *client);
static s32 read_temperature(void);

// Character device
static dev_t device_number;
static struct class *device_class;
static struct cdev my_device;
static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = driver_open,
    .release = driver_close,
    .write = driver_write,
    .read = driver_read,
};

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

// i2c client
static struct i2c_client *bmp280_i2c_client;
static s32 dig_T1, dig_T2, dig_T3;

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
    int retval;
    u8 bmp280_id;
    printk("Now I am in the probe function\n");

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

    // Checking that the device slave address matches (could be many i2c devices on the same bus)
    if(client->addr != I2C_SLAVE_ADDRESS) {
        printk("Device doesn't have the right slave address. Expected: %d, got: %d\n",
            I2C_SLAVE_ADDRESS, client->addr);
            goto cdev_error;
    }
    bmp280_i2c_client = client;

    // Read ID
    bmp280_id = i2c_smbus_read_byte_data(bmp280_i2c_client, ADDRESS_ID);
    printk("ID: 0x%x\n", bmp280_id);

    // Read calibration values
    dig_T1 = i2c_smbus_read_word_data(bmp280_i2c_client, 0x88);
    dig_T2 = i2c_smbus_read_word_data(bmp280_i2c_client, 0x8a);
    dig_T3 = i2c_smbus_read_word_data(bmp280_i2c_client, 0x8c);

    if(dig_T2 > 32767)
		dig_T2 -= 65536;

	if(dig_T3 > 32767)
		dig_T3 -= 65536;

    // Initialize device
    i2c_smbus_write_byte_data(bmp280_i2c_client, 0xf5, 0x5<<5);
    i2c_smbus_write_byte_data(bmp280_i2c_client, 0xf4, (5<<5) | (5<<2) | (3<<0) );

    return 0;

    cdev_error: device_destroy(device_class, device_number);
    device_error: class_destroy(device_class);
    class_error: unregister_chrdev(device_number, DEVICE_NAME);
    chrdev_error: return retval;
}

static int i2c_remove(struct i2c_client *client) {
    printk("Now I am in the remove function\n");
    device_destroy(device_class, device_number);
    class_destroy(device_class);
    unregister_chrdev(device_number, DEVICE_NAME);
    return 0;
}

/******************************************************************************
 * File operations
******************************************************************************/

/// @brief This function is called when the device is opened
static int driver_open(struct inode *device_file, struct file *instance) {
    return 0;
}

/// @brief This function is called when the device is opened
static int driver_close(struct inode *device_file, struct file *instance) {
    return 0;
}

/// @brief Does nothing
static ssize_t driver_write(struct file *file, const char *user_buffer, size_t count, loff_t *offs) {
    return 0;
}

/// @brief Read data out of the buffer
static ssize_t driver_read(struct file *file, char *user_buffer, size_t count, loff_t *offs) {
    int to_copy, not_copied, delta;
    char out_string[20];
    int temperature;

    // Get amount of data to copy
    to_copy = min(count, sizeof(out_string));

    // Get temperature
    temperature = read_temperature();
    snprintf(out_string, sizeof(out_string), "%d.%d\n", temperature/100, temperature%100);
    printk("Temperature of the device: %s\n", out_string);

    // Copy data to user, return bytes that hasn't copied
    not_copied = copy_to_user(user_buffer, out_string, to_copy);

    // Calculate data
    delta = to_copy - not_copied;
    return delta;
}

/******************************************************************************
 * Managing the BMP280
******************************************************************************/

/// @brief Read current temperature from BMP280 sensor
/// @return temperature
static s32 read_temperature(void) {
    int var1, var2;
    s32 raw_temp;
    s32 d1, d2, d3;

    // Read Temperature
	d1 = i2c_smbus_read_byte_data(bmp280_i2c_client, 0xFA);
	d2 = i2c_smbus_read_byte_data(bmp280_i2c_client, 0xFB);
	d3 = i2c_smbus_read_byte_data(bmp280_i2c_client, 0xFC);
	raw_temp = ((d1<<16) | (d2<<8) | d3) >> 4;

	// Calculate temperature in degree
	var1 = ((((raw_temp >> 3) - (dig_T1 << 1))) * (dig_T2)) >> 11;
	var2 = (((((raw_temp >> 4) - (dig_T1)) * ((raw_temp >> 4) - (dig_T1))) >> 12) * (dig_T3)) >> 14;
	return ((var1 + var2) *5 +128) >> 8;
}
