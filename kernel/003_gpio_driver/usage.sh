#!/bin/bash

## compile kernel module
# make

## Install module
# sudo insmod gpio_driver.ko

## See "printk()" statements (put in other terminal)
# tail -f /var/log/kern.log
# dmesg | tail

## Turn on or off gpio60
# sudo chmod 666 /dev/gpio_driver
# echo 0 > /dev/gpio_driver
# echo 1 > /dev/gpio_driver

# Read button state
# head -n 1 /dev/gpio_driver

## Remove module
# sudo rmmod gpio_driver
