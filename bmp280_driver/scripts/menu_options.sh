#!/bin/bash

### Compile and install all code on the beaglebone
function compile() {
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
    sshpass -p "${password}" ssh -q "${user}@${ip}" \
        "echo ${password} | sudo -S ${install_path}scripts/install_on_beagle.sh ${install_path}"
}

function install_module() {
    sshpass -p "${password}" ssh -q "${user}@${ip}" \
        "echo ${password} | sudo -S insmod ${install_path}${module_name}.ko"
}

function remove_module() {
    sshpass -p "${password}" ssh -q "${user}@${ip}" \
        "echo ${password} | sudo -S rmmod ${module_name}"
}

function read_temperature() {
    sshpass -p "${password}" ssh -q "${user}@${ip}" \
        "echo ${password} | head -n 1 /dev/temp-sensor"
}
