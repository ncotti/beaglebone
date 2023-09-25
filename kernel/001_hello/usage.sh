#!/bin/bash

## Install module
# sudo insmod hello.ko

## See "printk()" statements (put in other terminal)
# tail -f /var/log/kern.log
# dmesg | tail

## See information about the moduke
# modinfo hello.ko
# lsmod | grep hello
# cd /sys/module/hello

## Remove module
# sudo rmmod hello
