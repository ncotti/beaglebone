#!/bin/bash
sshpass -p temppwd ssh debian@192.168.7.2

# Copy file from host computer to beaglebone
# sshpass -p temppwd scp vivado.log debian@192.168.7.2:/home/debian/hola.txt

# GDB
# gdbserver --multi localhost:2159

# Copy file from beaglebone to computer
# Make sure that you have installed "openssh-server"
# sudo apt install openssh-server
# scp dtb.dtb cotti@192.168.7.1:/home/cotti/dtb.dtb
