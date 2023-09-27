#!/bin/bash

## Install module with default values
# sudo insmod parameters.ko

## Install module changing the parameter's values
# sudo insmod parameters.ko param_number=5 param_string="hello"

## Check available parameters pressing "double tab", or with:
# modinfo parameters.ko

## See messages with
# dmesg | tail

## Remove module
# sudo rmmod parameters
