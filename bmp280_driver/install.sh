#!/bin/bash

export user="debian"                           # Beaglebone user
export password="temppwd"                      # Beagleboen sudo password
export ip="192.168.7.2"                        # Beaglebone IP
export install_path="/home/${user}/driver/"    # Installation path for the driver
export module_name="temp_sensor"

# shellcheck source=scripts/menu_options.sh
source scripts/menu_options.sh

echo "What to do?"
echo "1. Compile kernel module"
echo "2. Install kernel module"
echo "3. Remove kernel module"
echo "4. Read temperature"
echo "5. Give internet to the BBB"
echo "6. Open an interative terminal on the beaglebone"
echo "q. Quit"

read -p "Select option: " -r -n 1 option
echo ""

case $option in
1) {
    compile
};;
2) {
    install_module
};;
3) {
    remove_module
};;
4) {
    read_temperature
};;
5) {
    internet_over_usb
};;
6) {
    interactive_terminal
};;
*) {
    echo "Leaving"
    exit 0
};;
esac
