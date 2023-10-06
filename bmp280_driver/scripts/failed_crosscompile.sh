#!/bin/bash

###############################################################################
# Failed cross compilation
###############################################################################

### Check if user has the linux headers installed on this computer
# if [ ! -d "${kernel}" ]; then
#     echo "Couldn't find the linux headers for target Beaglebone \"${kernel}\""
#     read -N 1 -r -p "Do you want to install them [y/n] ? >> " input
#     echo ""
#     if [ "${input}" = "y" ] || [ "${input}" = "Y" ]; then
#         echo "Installing Linux headers..."
#         git clone --branch "${kernel}" --depth 1 https://github.com/beagleboard/linux.git "${kernel}"
#         # cd "${kernel}"
#         # yes "" | make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- oldconfig && yes "" | make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- prepare
#         echo "Linux headers installed successfully!"
#     else
#         echo "Aborting installation"
#     fi
# fi
