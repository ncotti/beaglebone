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

