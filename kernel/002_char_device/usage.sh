#!/bin/bash

## compile kernel module
# make

## Install module
# sudo insmod char_device.ko

## See "printk()" statements (put in other terminal)
# tail -f /var/log/kern.log
# dmesg | tail

## Write and read from the device
# sudo chmod 666 /dev/echo_driver
# echo "testing" > /dev/echo_driver
# head -n 1 /dev/echo_driver

## Remove module
# sudo rmmod char_device
