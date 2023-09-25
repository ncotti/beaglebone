#!/bin/bash

## P9.17 == SCL; P9.18 == SDA
## Configure pins as i2c
config-pin P9.17 i2c
config-pin P9.18 i2c

# Device slave address on 0x76
i2cdetect -y -r 1

# Dumps all registers
i2cdump -y 1 0x76 b

# Read a single register. At address 0xD0, the chip ID should return 0x58.
i2cget -y 1 0x76 0xD0
