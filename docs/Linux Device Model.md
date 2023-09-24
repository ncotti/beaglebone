# Hardware Devices in Linux

In this document, we will take a look at the SYSFS  `/sys`, a virtual filesystem that allows managing devices as if they were files. The BeagleBone Black will be used as an example.

## Hardware devices are also files

"Everything in Linux is a file", and hardware devices are no exception. Inside the BBB, under the directory `/sys/class`, you can find most of them (for example, gpio, pwm, i2c, dma, etc):

```bash
$ ls /sys/class
ata_device   devfreq-event   hidraw       mem           pwm           sound       ubi
ata_link     dma             hwmon        misc          regulator     spi_master  udc
ata_port     drm             i2c-adapter  mmc_host      remoteproc    spi_slave   uio
backlight    drm_dp_aux_dev  i2c-dev      mtd           rfkill        spidev      usb_role
bdi          extcon          input        net           rtc           thermal     vc
block        firmware        iommu        phy           scsi_device   tpm         vtconsole
bsg          gnss            leds         power_supply  scsi_disk     tpmrm       watchdog
devcoredump  gpio            mbox         pps           scsi_generic  tty
devfreq      graphics        mdio_bus     ptp           scsi_host     typec
```

All devices are listed in what's called the **sysfs** (system file system). As a first approach, let's modify the LEDS on the board. First, go to the led device directory:

```bash
$ cd /sys/class/leds/beaglebone\:green\:usr3
$ ls
brightness  device  max_brightness  power  subsystem  trigger  uevent
```

As you can see, all properties are listed as files inside this directory. Let's configure the led to turn on and off:

```bash
# Here, the element enclosed with "[]" is the actual trigger mode
$ cat trigger
none rfkill-any rfkill-none kbd-scrolllock kbd-numlock kbd-capslock kbd-kanalock kbd-shiftlock kbd-altgrlock kbd-ctrllock kbd-altlock kbd-shiftllock kbd-shiftrlock kbd-ctrlllock kbd-ctrlrlock mmc0 [mmc1] usb-gadget usb-host timer oneshot disk-activity disk-read disk-write ide-disk mtd nand-disk heartbeat backlight gpio cpu cpu0 activity default-on panic netdev

# Change the trigger mode to "none"
$ echo none > trigger
$ cat trigger
[none] rfkill-any rfkill-none kbd-scrolllock kbd-numlock kbd-capslock kbd-kanalock kbd-shiftlock kbd-altgrlock kbd-ctrllock kbd-altlock kbd-shiftllock kbd-shiftrlock kbd-ctrlllock kbd-ctrlrlock mmc0 mmc1 usb-gadget usb-host timer oneshot disk-activity disk-read disk-write ide-disk mtd nand-disk heartbeat backlight gpio cpu cpu0 activity default-on panic netdev

# Turn on and off
$ cat brightness
0
$ echo 1 > brightness   # turn on
$ echo 0 > brightness   # turn off
```

Next, let's configure the led to make a periodic heartbeat:

```bash
$ echo timer > trigger
# Now, new files appeared: delay_on and delay_off. These are only accessible by the root user and group
$ ls -l
-rw-rw-r-- 1 root gpio 4096 Sep  3 19:45 brightness
-rw-r--r-- 1 root root 4096 Sep  3 19:47 delay_off
-rw-r--r-- 1 root root 4096 Sep  3 19:47 delay_on
lrwxrwxrwx 1 root gpio    0 Sep  3 18:13 device -> ../../../leds
-r--r--r-- 1 root gpio 4096 Sep  3 18:13 max_brightness
drwxrwxr-x 2 root gpio    0 Sep  3 18:13 power
lrwxrwxrwx 1 root gpio    0 Sep  3 18:13 subsystem -> ../../../../../class/leds
-rw-rw-r-- 1 root gpio 4096 Sep  3 19:47 trigger
-rw-rw-r-- 1 root gpio 4096 Sep  3 18:13 uevent

$ sudo -i
$ cd /sys/class/leds/beaglebone\:green\:usr3
# 500msec on, 500msec off
$ cat delay_on delay_off
500
500
# 500msec on, 100msec off
$ echo 100 > delay_off
```

Of course, the SYSFS isn't all that there is to manage hardware devices. Let's look for example at the interface for gpios. If you pay attention, you can see that there is no way to modify the "pull-up" or "pull-down" resistor. Besides, the pin is configured as an input, but we want to be able to set a pin's default behavior at boot.

```bash
$ ls /sys/class/gpio/gpio60
active_low  device  direction  edge  label  power  subsystem  uevent  value
$ cat direction label
in
P9_12
```

## The Linux Device Tree: how to describe hardware

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
