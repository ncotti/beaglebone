#!/bin/bash

for file in *; do
    sshpass -p temppwd scp -r "${file}" "debian@192.168.7.2:/home/debian/bmp280_driver/01_dt_bmp280/"
done
