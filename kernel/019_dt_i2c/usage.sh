#!/bin/bash

## Inside the makefile, the line to compile the DTS file was added.
## Make sure to edit this line in your /boot/uEnv.txt:

# uboot_overlay_addr4=/home/debian/device-tree/my_overlay.dtbo

## Reboot to apply changes to Device tree
# sudo reboot

## After that, your device should appear in folder /proc/device-tree/my_device
