#!/bin/bash

# Enables internet connection.
# This script is run at boot, added to the crontables with:
# sudo crontab -e
#   @reboot /home/debian/boot/network.sh

# Known issue: "/sbin/route" command is overwritten by the network manager after
# the boot process is completed.

sudo /sbin/route add default gw 192.168.7.1
sudo sh -c "echo \"nameserver 8.8.8.8\" >> /etc/resolv.conf"
