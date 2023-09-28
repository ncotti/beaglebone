#!/bin/bash

## This file should be run on the BeagleBone!

# Device name, without extension
dts_name="hello"

# Overlay name set in the /boot/uEnv.txt file
overlay=/home/debian/device-tree/my_overlay.dtbo

if ! dtc -I dts -O dtb -o "${dts_name}.dtbo" "${dts_name}.dts"; then
    exit 0
fi

cp "${dts_name}.dtbo" "${overlay}"
sudo reboot
