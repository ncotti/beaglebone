#!/bin/bash

# Enable direct access to internet from the BeagleBone Black
# This script was made using the following guide:
# https://gist.github.com/pdp7/d2711b5ff1fbb000240bd8337b859412

set -e
#echo "Please, run \"$ ifconfig\" on other terminal and complete the following info."
#read -rp "Name of the internet interface: " internet
#read -rp "Name of the BeagleBone ethernet interface: " bbb_ether
internet="wlp2s0"
bbb_ether="enx98f07b9de2e5"

sudo iptables -t nat -F; sudo iptables -t mangle -F; sudo iptables -F; sudo iptables -X
sudo iptables --table nat --append POSTROUTING --out-interface "${internet}" -j MASQUERADE
sudo iptables --append FORWARD --in-interface "${bbb_ether}" -j ACCEPT
sudo sh -c "echo 1 > /proc/sys/net/ipv4/ip_forward"

echo "All ready. On the BeagleBone, run:"
echo "sudo route add default gw 192.168.7.1"
