#!/bin/bash

## This file should be run on the host! Copy all files to the beagle

for file in *; do
    sshpass -p temppwd scp "${file}" "debian@192.168.7.2:/home/debian/device-tree/04_target_path/${file}"
done
