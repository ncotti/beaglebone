#!/bin/bash

host_computer="cotti@192.168.7.1"
host_path="/home/cotti/Desktop/TD3/beaglebone/device_tree/dtb.dtb"

dtb_file="/boot/dtbs/$(uname -r)/$(head -n 1 /proc/device-tree/chosen/base_dtb)"
dtb_file=${dtb_file//".dts"/".dtb"}

scp ${dtb_file} ${host_computer}:${host_path}
