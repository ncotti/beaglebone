#include "bmp280.h"

/******************************************************************************
 * Static function's prototypes
******************************************************************************/

static int driver_close(struct inode *device_file, struct file *instance);
static int driver_open(struct inode *device_file, struct file *instance);
static ssize_t driver_read(struct file *file, char *user_buffer, size_t count, loff_t *offs);
s32 read_temperature(void);

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
};

static struct i2c_adapter *bmp280_i2c_adapter = NULL;
static struct i2c_client *bmp280_i2c_client = NULL;

static const struct i2c_device_id bmp_id[] = {
    { SLAVE_DEVICE_NAME, 0},
    {},
};

static struct i2c_driver bmp280_driver = {
    .driver = {
        .name = SLAVE_DEVICE_NAME,
        .owner = THIS_MODULE,
    },
};

static struct i2c_board_info bmp280_i2c_board_info = {
    I2C_BOARD_INFO(SLAVE_DEVICE_NAME, BMP280_SLAVE_ADDRESS),
};

static s32 dig_T1, dig_T2, dig_T3;

/******************************************************************************
 * Init and exit
******************************************************************************/

/// @brief This function is called when the module is loaded into the kernel.
static int __init my_module_init(void) {
    int retval = -1;
    u8 id;
    printk("Hello, Kernel!\n");

    // Allocate device number
    if ((retval = alloc_chrdev_region(&device_number, MINOR_NUMBER, NUMBER_OF_DEVICES, DEVICE_NAME)) != 0) {
        printk("Couldn't allocate device number\n");
        goto chrdev_error;
    }
    printk("Device number allocated successfully! Major: %d, Minor: %d\n",
        MAJOR(device_number), MINOR(device_number));

    // Create device class
    if ((device_class = class_create(THIS_MODULE, DEVICE_CLASS_NAME)) == NULL) {
        printk("Device class couldn't be created\n");
        retval = -1;
        goto class_error;
    }

    // Create device file
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

    // I2C device
    if ((bmp280_i2c_adapter = i2c_get_adapter(I2C_BUS_NUMBER)) != NULL) {
        if ((bmp280_i2c_client = i2c_new_client_device(bmp280_i2c_adapter, &bmp280_i2c_board_info)) != NULL ) {
            if(i2c_add_driver(&bmp280_driver) != -1) {
                retval = 0;
            } else {
                printk("Can't add driver... \n");
            }
        }
        i2c_put_adapter(bmp280_i2c_adapter);
    }
    printk("BMP280 Driver added\n");

    // Read ID
    id = i2c_smbus_read_byte_data(bmp280_i2c_client, ADDRESS_ID);
    printk("ID: 0x%x\n", id);

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

/// @brief This function is called when the module is removed from the kernel
static void __exit my_module_exit(void) {
    i2c_unregister_device(bmp280_i2c_client);
	i2c_del_driver(&bmp280_driver);
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

/// @brief When read, return temperature
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
 * BMP280 functions
******************************************************************************/

/// @brief Read current temperature from BMP280 sensor
/// @return temperature
s32 read_temperature(void) {
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
