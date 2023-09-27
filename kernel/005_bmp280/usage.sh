#!/bin/bash

## compile kernel module
# make

## Prepare GPIO pins:
# config-pin P9.17 i2c
# config-pin P9.18 i2c
# i2cdetect -y -r 1

## Install module
# sudo insmod bmp280.ko

## Read temperature
# head -n 1 /dev/bmp280_driver

## See "printk()" statements
# dmesg | tail

## Remove module
# sudo rmmod bmp280
