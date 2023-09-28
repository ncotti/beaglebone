# The Linux Device Tree: how to describe hardware

How does the Linux Kernel know that we have a LED, or that the GPIO60 is in the port P9_12 of our board? Formerly, this information was added as a file in the Linux kernel, meaning that there were a lot of different versions for each specific hardware. To enhance modularity, the Linux Device Tree was invented as a way to describe the hardware and be able to utilize it.

Hardware devices all behave very similar. An UART, a GPIO, or a I2C bus all have roughly the same functionality. The differences appear in the memory address of the registers, and their distribution. The Linux Device Tree does not replace drivers, hardware drivers still need to be written; however, it allows specifying that a certain device exists, and is available for the Linux OS to be used.

The <span style="color:#0070c0">Device Tree Source (DTS)</span> is a human-readable file `.dts` which contains the description of every device and their startup conditions (for example, set the PIN 1_28 as a GPIO60 output, low state).

The <span style="color:#0070c0">Device Tree Source Includes</span> `.dtsi` are like headers files for the DTS files. DTS files tend to have the board descriptions, while the DTSI files have more specific and individual configurations (SoC or specific peripherals).

The <span style="color:#0070c0">Flattened Device Tree</span> (FTD) or the <span style="color:#0070c0">Device Tree Blob</span> (DTB) is the binary file obtained after "compiling" the DTS files. Unlike normal compiling, this process is reversible: a DTS file can compiled into a DTB file and vice versa.

The Device Tree Compiler (DTC) is used for such operation. It can be, however, quite challenging to modify and already existing DTB file, ensuring that you have the precise versions of the DTS files. Since the Linux image comes with the `.dtb` files, we can decompile them, modify them, and recompile them.

```bash
# You need to have installed "device-tree-compiler"
dtc -I <input_format> -O <output_format> -o <output_file> <input_file>
dtc -I <dtb|dts> -O <dtb|dts> -o <output_file> <input_file>
```

## Example

In the following example, I will modify the DTB files for the U-Boot in the BeagleBone Black to change the default behavior of a LED.

First, let's check the name of the current DTB file used to boot. Inside the directory `/proc/` we can find information about the currently running processes, including the hardware devices.

```bash
$ cat /proc/device-tree/chosen/base_dtb
am335x-boneblack-uboot-univ.dts
```

Now, we go to the directory where all the dtbs files are, and modify that one:
```bash
cd /home/debian
cp /boot/dtbs/4.19.94-ti-r42/am335x-boneblack.dtb /home/debian/am335x-boneblack.dtb_backup
dtc -I dtb -O dts -o am335x-boneblack-uboot-univ.dts am335x-boneblack-uboot-univ.dtb_backup
nano am335x-boneblack.dts
```

While looking at the generating file, search for the leds' definitions, and change one of the leds:

```dts
leds {
	pinctrl-names = "default";
	pinctrl-0 = < 0x237 >;
	compatible = "gpio-leds";

	led2 {
		label = "beaglebone:green:usr0";
		gpios = < 0x5a 0x15 0x00 >;
		linux,default-trigger = "heartbeat";
		default-state = "off";
    };

	led3 {
		label = "beaglebone:green:usr1";
		gpios = < 0x5a 0x16 0x00 >;
		linux,default-trigger = "mmc0";
		default-state = "off";
	};

	led4 {
		label = "beaglebone:green:usr2";
		gpios = < 0x5a 0x17 0x00 >;
		linux,default-trigger = "cpu0";
		default-state = "off";
	};

	led5 {
		label = "beaglebone:green:usr3";
		gpios = < 0x5a 0x18 0x00 >;
		linux,default-trigger = "mmc1";  // Change to "heartbeat"
		default-state = "off";
	};
};
```

```bash
sudo dtc -I dts -O dtb -o /boot/dtbs/4.19.94-ti-r42/am335x-boneblack-uboot-univ.dtb am335x-boneblack-uboot-univ.dts
```

After rebooting, it's possible to see the led doing the heartbeat pattern, and we can verify that the property was correctly modified in the `/proc` directory:

```bash
$ cat /proc/device-tree/leds/led5/linux,default-trigger
heartbeat
```

## Device tree overlays

The old ways preferred to modify directly the device tree. The new ways use device tree overlays.

The main device tree ".dts" is loaded by U-Boot. You can check the current one being used with:

```bash
$ cat /proc/device-tree/chosen/base_dtb
am335x-boneblack-uboot-univ.dts
```

Inside the `/boot/uEnv.txt` file, we can instruct the u-Boot the path to our overlay:

```bash
$ cat /boot/uEnv.txt
###U-Boot Overlays###
###Documentation: http://elinux.org/Beagleboard:BeagleBoneBlack_Debian#U-Boot_Overlays
###Master Enable
enable_uboot_overlays=1
###
###Overide capes with eeprom
#uboot_overlay_addr0=<file0>.dtbo
#uboot_overlay_addr1=<file1>.dtbo
#uboot_overlay_addr2=<file2>.dtbo
#uboot_overlay_addr3=<file3>.dtbo
###
###Additional custom capes
#uboot_overlay_addr4=<file4>.dtbo
#uboot_overlay_addr5=<file5>.dtbo
#uboot_overlay_addr6=<file6>.dtbo
#uboot_overlay_addr7=<file7>.dtbo
###
###Custom Cape
#dtb_overlay=<file8>.dtbo
```

After rebooting, the overlay should be added. You can check it


## Bibliography

[Device tree for dummies video](https://www.youtube.com/watch?v=m_NyYEBxfn8&ab_channel=TheLinuxFoundation)

[Device tree for dummies presentation](https://events.static.linuxfound.org/sites/events/files/slides/petazzoni-device-tree-dummies.pdf)

[U-Boot_Overlays](https://elinux.org/Beagleboard:BeagleBoneBlack_Debian#U-Boot_Overlays)

[Good stuff](https://takeofftechnical.com/beaglebone-black-led-control/)

https://elinux.org/Device_Tree_Usage
