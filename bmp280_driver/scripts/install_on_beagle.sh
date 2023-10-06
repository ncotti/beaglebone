#!/bin/bash

if [ $# -lt 1 ]; then
    echo "No arguments. Using default path."
    install_path="/home/debian/driver"
else
    install_path="$1"
fi

if [ "$EUID" -ne 0 ]; then
    echo "Please run as root"
    exit 1
fi

cd "${install_path}" || exit
make clean
make compile

### If the module is not inserted
if [ ! -e /proc/device-tree/chosen/overlays/cotti_i2c2 ]; then
    ### Copy udev rule
    cp ${install_path}/scripts/99-cotti-devices.rules /etc/udev/rules.d

    ### Modify uEnv.txt to allow for modules, and add mine
    if ! grep -q '^[^#]*enable_uboot_overlays=1' /boot/uEnv.txt; then
        echo "enable_uboot_overlays=1" >> "/boot/uEnv.txt"
        echo "Adding line \"enable_uboot_overlays=1\" to file \"/boot/uEnv.txt\""
    fi
    if ! grep -q '^[^#]*uboot_overlay_addr4' /boot/uEnv.txt; then
        echo "uboot_overlay_addr4=/lib/firmware/cotti_i2c2.dtbo" >> "/boot/uEnv.txt"
        echo "Adding line \"uboot_overlay_addr4=/lib/firmware/cotti_i2c2.dtbo\" to file \"/boot/uEnv.txt\""
    fi
    echo "Restart the board to apply changes to the device tree"
fi



