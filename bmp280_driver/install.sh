#!/bin/bash

user="debian"                           # Beaglebone user
password="temppwd"                      # Beagleboen sudo password
ip="192.168.7.2"                        # Beaglebone IP
install_path="/home/${user}/driver/"    # Installation path for the driver

## Get kernel version from the board.
if ! kernel=$(sshpass -p "${password}" ssh -q "${user}@${ip}" "/bin/uname -r"); then
    echo "Couldn't connect to BeagleBone board through SSH"
    echo "If your device is connected, try running the following line on you shell client:"
    # key fingerprint probably missing
    echo "    ssh ${user}@${ip} \"/bin/uname -r\""
    exit 1
fi

echo "Compiling on the BeagleBone Black with kernel: ${kernel}"

# Copy all files to the BeagleBone
for file in *; do
    sshpass -p "${password}" scp -q -r "${file}" "${user}@${ip}:${install_path}"
done

# Execute on beaglebone
    sshpass -p "${password}" ssh "${user}@${ip}" \
        "echo ${password} | sudo -S ${install_path}scripts/install_on_beagle.sh ${install_path}"

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
