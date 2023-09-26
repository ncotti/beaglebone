#!/bin/bash

## Get DTS from DTB
dtc -I dtb -O dts -o dts.dts dtb.dtb

## Reconvert from 
#dtc -I dts -O dtb -o dtb1.dtb dts.dts