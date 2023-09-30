#!/bin/bash

for file in *; do
    sshpass -p temppwd scp -r "${file}" "debian@192.168.7.2:/home/debian/kernel/023_multiple_files/"
done
