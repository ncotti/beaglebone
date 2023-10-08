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

### Enable direct access to internet from the BeagleBone Black
### This script was made using the following guide:
### https://gist.github.com/pdp7/d2711b5ff1fbb000240bd8337b859412
function internet_over_usb() {
    #echo "Please, run \"$ ifconfig\" on other terminal and complete the following info."
    #read -rp "Name of the internet interface: " internet
    #read -rp "Name of the BeagleBone ethernet interface: " bbb_ether
    internet="wlp2s0"
    bbb_ether="enx98f07b9de2e5"

    sudo iptables -t nat -F; sudo iptables -t mangle -F; sudo iptables -F; sudo iptables -X
    sudo iptables --table nat --append POSTROUTING --out-interface "${internet}" -j MASQUERADE
    sudo iptables --append FORWARD --in-interface "${bbb_ether}" -j ACCEPT
    sudo sh -c "echo 1 > /proc/sys/net/ipv4/ip_forward"

    echo "All ready. On the BeagleBone, run:"
    echo "sudo route add default gw 192.168.7.1"

    sshpass -p "${password}" ssh -q "${user}@${ip}" \
        "echo ${password} | sudo -S /sbin/route add default gw 192.168.7.1"

    nameserver="nameserver 8.8.8.8"
    sshpass -p "${password}" ssh -q "${user}@${ip}" \
        "echo ${password} | sudo -S sh -c 'echo ${nameserver} >> /etc/resolv.conf'"
}

function interactive_terminal() {
    sshpass -p temppwd ssh debian@192.168.7.2
}
