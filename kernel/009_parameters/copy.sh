#!/bin/bash

for file in *; do
    sshpass -p temppwd scp "${file}" "debian@192.168.7.2:/home/debian/kernel/009_parameters/${file}"
done
