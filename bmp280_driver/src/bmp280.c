#include "bmp280.h"

/******************************************************************************
 * Static variables
******************************************************************************/

static s32 dig_T1, dig_T2, dig_T3;  // Calibration values.
static u32 slave_address = 0;
static bmp280_unit g_unit = CELSIUS;

/******************************************************************************
 * Functions
******************************************************************************/

/// @brief Initialize the temperature sensor BMP280
/// @param pdev Platform device of the I2C bus attached to the sensor in the
///  device tree.
/// @return "0" if the sensor is found and configured, "-1" on error.
int bmp280_init(struct platform_device *pdev) {
    int retval = -1;
    struct device_node *temp_sensor_node = NULL;

    // Get slave address
    if ((temp_sensor_node = of_get_child_by_name(pdev->dev.of_node, DT_I2C_CHILD_NAME)) == NULL) {
        printk(ERROR("Couldn't get temperature sensor node.\n"));
        retval = -1;
        goto pdev_error;
    }
    if ((retval = of_property_read_u32(temp_sensor_node, DT_PROPERTY_REG, &slave_address)) != 0) {
        printk(ERROR("Couldn't get temperature sensor slave address.\n"));
        goto temp_sensor_node_error;
    }

    if (!bmp280_is_connected()) {
        retval = -1;
        goto temp_sensor_node_error;
    }

    // Read and store calibration values
    dig_T1 = cotti_i2c_read(BMP280_REG_DIG_T1_MSB, slave_address) << 8 |
             cotti_i2c_read(BMP280_REG_DIG_T1_LSB, slave_address);
    dig_T2 = cotti_i2c_read(BMP280_REG_DIG_T2_MSB, slave_address) << 8 |
             cotti_i2c_read(BMP280_REG_DIG_T2_LSB, slave_address);
    dig_T3 = cotti_i2c_read(BMP280_REG_DIG_T3_MSB, slave_address) << 8 |
             cotti_i2c_read(BMP280_REG_DIG_T3_LSB, slave_address);

    bmp280_set_frequency(FREQ_1);
    bmp280_set_mode(NORMAL);

    printk(INFO("BMP280 configured.\n"));
    return 0;

    temp_sensor_node_error: of_node_put(temp_sensor_node);
    pdev_error: return retval;
}

/// @brief Deinitialize the temperature sensor
void bmp280_deinit(void) {
    bmp280_set_mode(SLEEP);
}

/// @brief Read current temperature from BMP280 sensor. The temperature unit
///  is dependant of the last call to bmp280_set_unit() (defaults to °C).
/// @return Temperature as "5280", which equals "52.80 °C", or BMP280_ERROR
s32 bmp280_read_temperature(void) {
    s32 raw_temp, d1, d2, d3, var1, var2, temp;

    // Read Temperature
	if ((d1 = cotti_i2c_read(BMP280_REG_TEMP_MSB, slave_address)) == -1 ||
	    (d2 = cotti_i2c_read(BMP280_REG_TEMP_LSB, slave_address)) == -1 ||
	    (d3 = cotti_i2c_read(BMP280_REG_TEMP_XLSB, slave_address)) == -1) {
            return BMP280_ERROR;
    }
	raw_temp = ((d1<<16) | (d2<<8) | d3) >> 4;

	// Calculate temperature in °C
	var1 = ((((raw_temp >> 3) - (dig_T1 << 1))) * (dig_T2)) >> 11;
	var2 = (((((raw_temp >> 4) - (dig_T1)) * ((raw_temp >> 4) - (dig_T1))) >> 12) * (dig_T3)) >> 14;
	temp = ((var1 + var2) *5 +128) >> 8;

    switch(g_unit) {
        case CELSIUS: break;
        case FAHRENHEIT:
            temp = temp * 9 / 5 + 3200;
        break;
        case KELVIN:
            temp += 27315;
        break;
        default: break; // Celsius by default
    }
    return temp;
}

/// @brief Change operation mode. This function disables pressure measurement and
///  enables temperature reading, so it should be called if the device is reset.
int bmp280_set_mode(bmp280_mode mode) {
    return cotti_i2c_write(mode | BMP280_TEMP_OVERSAMPLING_X1, BMP280_REG_CTRL_MEAS, slave_address);
}

/// @brief Change operation frequency
int bmp280_set_frequency(bmp280_freq freq) {
    return cotti_i2c_write(freq << 5, BMP280_REG_CONFIG, slave_address);
}

/// @brief Change temperature unit to show
void bmp280_set_unit(bmp280_unit unit) {
    g_unit = unit;
}

/// @brief Check if the sensor is responding.
/// @return "1" if connected, "0" if not detected
int bmp280_is_connected(void) {
    u8 id;
    // Read ID, check that the device is working
    if ((id = cotti_i2c_read(BMP280_REG_ID, slave_address)) == -1) {
        printk(ERROR("BMP280 not responding.\n"));
        return 0;
    }
    if (id != BMP280_ID) {
        printk(ERROR("BMP280 ID mismatch. I2C communication is not working correctly. "
            "Expected value: 0x%x, value read: 0x%x\n"), BMP280_ID, id);
        return 0;
    }
    return 1;
}
