#include <stdio.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#define I2C_SLAVE_ADDRESS 0x76

int i2c_test(void);