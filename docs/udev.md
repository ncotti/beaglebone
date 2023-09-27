# Udev rules

Udev rules are a response from the user space, to new devices created by the kernel space. Udev rules can be found in the directory `/etc/udev/rules.d`. The rules start with a number, such as `66-my-udev.rules`, and are processed in lexical order, meaning that the lower the number, the higher the priority.

All *udev* rules are separated in two:
1. The "match" part, in which we define the conditions for the rule to be applied, using a series of keys separated by a comma.
2. The "action" part, in which we perform some kind of action, when the conditions are met.

This document will have three parts. First, we will start by defining the possible events that can trigger an "udev rule". Then, the actions that can be taken. And finally, how to do some testing.

## Finding your device

To act upon a device, we first need to know our device. Let's take for example an USB mouse. How can you access its information?

If you go the `/dev/*` directory, it's not listed there. 

Devices are always listed in the linux SYSFS. For example, my mouse right now is in this directory:

```txt
/sys/devices/pci0000:00/0000:00:14.0/usb1/1-1/1-1:1.0/0003:1C4F:0048.0003/input/input14
```

How do I know that it's there? Well, the directory is created dinamically by the kernel. But we can see the event in the kernel logs:

```bash
$ sudo dmesg | tail -10
[28927.148900] usb 1-1: USB disconnect, device number 15
[28930.561932] usb 1-1: new low-speed USB device number 16 using xhci_hcd
[28930.719774] usb 1-1: New USB device found, idVendor=1c4f, idProduct=0048, bcdDevice= 1.10
[28930.719779] usb 1-1: New USB device strings: Mfr=1, Product=2, SerialNumber=0
[28930.719781] usb 1-1: Product: Usb Mouse
[28930.719783] usb 1-1: Manufacturer: SIGMACHIP
[28930.723265] input: SIGMACHIP Usb Mouse as /devices/pci0000:00/0000:00:14.0/usb1/1-1/1-1:1.0/0003:1C4F:0048.0004/input/input15
[28930.782216] hid-generic 0003:1C4F:0048.0004: input,hidraw0: USB HID v1.10 Mouse [SIGMACHIP Usb Mouse] on usb-0000:00:14.0-1/input0

```

Now that we know where our device is, we can check its info.

## Using udevadm to know a device

`udevadm`, (udev adminstrator), allows to know all the "match keys" related to the device and parent devices.

```txt
udevadm info -ap devices/pci0000:00/0000:00:14.0/usb1/1-1/1-1:1.0/0003:1C4F:0048.0004/input/input15

Udevadm info starts with the device specified by the devpath and then
walks up the chain of parent devices. It prints for every device
found, all possible attributes in the udev rules key format.
A rule to match, can be composed by the attributes of the device
and the attributes from one single parent device.

  looking at device '/devices/pci0000:00/0000:00:14.0/usb1/1-1/1-1:1.0/0003:1C4F:0048.0004/input/input15':
    KERNEL=="input15"
    SUBSYSTEM=="input"
    DRIVER==""
    ATTR{capabilities/abs}=="0"
    ATTR{capabilities/ev}=="17"
    ATTR{capabilities/ff}=="0"
    ATTR{capabilities/key}=="1f0000 0 0 0 0"
    ATTR{capabilities/led}=="0"
    ATTR{capabilities/msc}=="10"
    ATTR{capabilities/rel}=="903"
    ATTR{capabilities/snd}=="0"
    ATTR{capabilities/sw}=="0"
    ATTR{id/bustype}=="0003"
    ATTR{id/product}=="0048"
    ATTR{id/vendor}=="1c4f"
    ATTR{id/version}=="0110"
    ATTR{inhibited}=="0"
    ATTR{name}=="SIGMACHIP Usb Mouse"
    ATTR{phys}=="usb-0000:00:14.0-1/input0"
    ATTR{power/async}=="disabled"
    ATTR{power/control}=="auto"
    ATTR{power/runtime_active_kids}=="0"
    ATTR{power/runtime_active_time}=="0"
    ATTR{power/runtime_enabled}=="disabled"
    ATTR{power/runtime_status}=="unsupported"
    ATTR{power/runtime_suspended_time}=="0"
    ATTR{power/runtime_usage}=="0"
    ATTR{properties}=="0"
    ATTR{uniq}==""

  looking at parent device '/devices/pci0000:00/0000:00:14.0/usb1/1-1/1-1:1.0/0003:1C4F:0048.0004':
    KERNELS=="0003:1C4F:0048.0004"
    SUBSYSTEMS=="hid"
    DRIVERS=="hid-generic"
    ATTRS{country}=="00"
    ATTRS{power/async}=="enabled"
    ATTRS{power/control}=="auto"
    ATTRS{power/runtime_active_kids}=="0"
    ATTRS{power/runtime_active_time}=="0"
    ATTRS{power/runtime_enabled}=="disabled"
    ATTRS{power/runtime_status}=="unsupported"
    ATTRS{power/runtime_suspended_time}=="0"
    ATTRS{power/runtime_usage}=="0"

  looking at parent device '/devices/pci0000:00/0000:00:14.0/usb1/1-1/1-1:1.0':
    KERNELS=="1-1:1.0"
    SUBSYSTEMS=="usb"
    DRIVERS=="usbhid"
    ATTRS{authorized}=="1"
    ATTRS{bAlternateSetting}==" 0"
    ATTRS{bInterfaceClass}=="03"
    ATTRS{bInterfaceNumber}=="00"
    ATTRS{bInterfaceProtocol}=="02"
    ATTRS{bInterfaceSubClass}=="01"
    ATTRS{bNumEndpoints}=="01"
    ATTRS{physical_location/dock}=="no"
    ATTRS{physical_location/horizontal_position}=="left"
    ATTRS{physical_location/lid}=="no"
    ATTRS{physical_location/panel}=="back"
    ATTRS{physical_location/vertical_position}=="center"
    ATTRS{power/async}=="enabled"
    ATTRS{power/runtime_active_kids}=="0"
    ATTRS{power/runtime_enabled}=="enabled"
    ATTRS{power/runtime_status}=="suspended"
    ATTRS{power/runtime_usage}=="0"
    ATTRS{supports_autosuspend}=="1"

  looking at parent device '/devices/pci0000:00/0000:00:14.0/usb1/1-1':
    KERNELS=="1-1"
    SUBSYSTEMS=="usb"
    DRIVERS=="usb"
    ATTRS{authorized}=="1"
    ATTRS{avoid_reset_quirk}=="0"
    ATTRS{bConfigurationValue}=="1"
    ATTRS{bDeviceClass}=="00"
    ATTRS{bDeviceProtocol}=="00"
    ATTRS{bDeviceSubClass}=="00"
    ATTRS{bMaxPacketSize0}=="8"
    ATTRS{bMaxPower}=="98mA"
    ATTRS{bNumConfigurations}=="1"
    ATTRS{bNumInterfaces}==" 1"
    ATTRS{bcdDevice}=="0110"
    ATTRS{bmAttributes}=="a0"
    ATTRS{busnum}=="1"
    ATTRS{configuration}==""
    ATTRS{devnum}=="16"
    ATTRS{devpath}=="1"
    ATTRS{idProduct}=="0048"
    ATTRS{idVendor}=="1c4f"
    ATTRS{ltm_capable}=="no"
    ATTRS{manufacturer}=="SIGMACHIP"
    ATTRS{maxchild}=="0"
    ATTRS{physical_location/dock}=="no"
    ATTRS{physical_location/horizontal_position}=="left"
    ATTRS{physical_location/lid}=="no"
    ATTRS{physical_location/panel}=="back"
    ATTRS{physical_location/vertical_position}=="center"
    ATTRS{power/active_duration}=="198320"
    ATTRS{power/async}=="enabled"
    ATTRS{power/autosuspend}=="2"
    ATTRS{power/autosuspend_delay_ms}=="2000"
    ATTRS{power/connected_duration}=="198320"
    ATTRS{power/control}=="on"
    ATTRS{power/level}=="on"
    ATTRS{power/persist}=="1"
    ATTRS{power/runtime_active_kids}=="0"
    ATTRS{power/runtime_active_time}=="198040"
    ATTRS{power/runtime_enabled}=="forbidden"
    ATTRS{power/runtime_status}=="active"
    ATTRS{power/runtime_suspended_time}=="0"
    ATTRS{power/runtime_usage}=="1"
    ATTRS{power/wakeup}=="disabled"
    ATTRS{power/wakeup_abort_count}==""
    ATTRS{power/wakeup_active}==""
    ATTRS{power/wakeup_active_count}==""
    ATTRS{power/wakeup_count}==""
    ATTRS{power/wakeup_expire_count}==""
    ATTRS{power/wakeup_last_time_ms}==""
    ATTRS{power/wakeup_max_time_ms}==""
    ATTRS{power/wakeup_total_time_ms}==""
    ATTRS{product}=="Usb Mouse"
    ATTRS{quirks}=="0x0"
    ATTRS{removable}=="removable"
    ATTRS{rx_lanes}=="1"
    ATTRS{speed}=="1.5"
    ATTRS{tx_lanes}=="1"
    ATTRS{urbnum}=="4814"
    ATTRS{version}==" 1.10"

  looking at parent device '/devices/pci0000:00/0000:00:14.0/usb1':
    KERNELS=="usb1"
    SUBSYSTEMS=="usb"
    DRIVERS=="usb"
    ATTRS{authorized}=="1"
    ATTRS{authorized_default}=="1"
    ATTRS{avoid_reset_quirk}=="0"
    ATTRS{bConfigurationValue}=="1"
    ATTRS{bDeviceClass}=="09"
    ATTRS{bDeviceProtocol}=="01"
    ATTRS{bDeviceSubClass}=="00"
    ATTRS{bMaxPacketSize0}=="64"
    ATTRS{bMaxPower}=="0mA"
    ATTRS{bNumConfigurations}=="1"
    ATTRS{bNumInterfaces}==" 1"
    ATTRS{bcdDevice}=="0602"
    ATTRS{bmAttributes}=="e0"
    ATTRS{busnum}=="1"
    ATTRS{configuration}==""
    ATTRS{devnum}=="1"
    ATTRS{devpath}=="0"
    ATTRS{idProduct}=="0002"
    ATTRS{idVendor}=="1d6b"
    ATTRS{interface_authorized_default}=="1"
    ATTRS{ltm_capable}=="no"
    ATTRS{manufacturer}=="Linux 6.2.0-33-generic xhci-hcd"
    ATTRS{maxchild}=="12"
    ATTRS{power/active_duration}=="29127508"
    ATTRS{power/async}=="enabled"
    ATTRS{power/autosuspend}=="0"
    ATTRS{power/autosuspend_delay_ms}=="0"
    ATTRS{power/connected_duration}=="29127508"
    ATTRS{power/control}=="auto"
    ATTRS{power/level}=="auto"
    ATTRS{power/runtime_active_kids}=="4"
    ATTRS{power/runtime_active_time}=="29127507"
    ATTRS{power/runtime_enabled}=="enabled"
    ATTRS{power/runtime_status}=="active"
    ATTRS{power/runtime_suspended_time}=="0"
    ATTRS{power/runtime_usage}=="0"
    ATTRS{power/wakeup}=="disabled"
    ATTRS{power/wakeup_abort_count}==""
    ATTRS{power/wakeup_active}==""
    ATTRS{power/wakeup_active_count}==""
    ATTRS{power/wakeup_count}==""
    ATTRS{power/wakeup_expire_count}==""
    ATTRS{power/wakeup_last_time_ms}==""
    ATTRS{power/wakeup_max_time_ms}==""
    ATTRS{power/wakeup_total_time_ms}==""
    ATTRS{product}=="xHCI Host Controller"
    ATTRS{quirks}=="0x0"
    ATTRS{removable}=="unknown"
    ATTRS{rx_lanes}=="1"
    ATTRS{serial}=="0000:00:14.0"
    ATTRS{speed}=="480"
    ATTRS{tx_lanes}=="1"
    ATTRS{urbnum}=="200"
    ATTRS{version}==" 2.00"

  looking at parent device '/devices/pci0000:00/0000:00:14.0':
    KERNELS=="0000:00:14.0"
    SUBSYSTEMS=="pci"
    DRIVERS=="xhci_hcd"
    ATTRS{ari_enabled}=="0"
    ATTRS{broken_parity_status}=="0"
    ATTRS{class}=="0x0c0330"
    ATTRS{consistent_dma_mask_bits}=="64"
    ATTRS{d3cold_allowed}=="1"
    ATTRS{dbc}=="disabled"
    ATTRS{device}=="0x9d2f"
    ATTRS{dma_mask_bits}=="64"
    ATTRS{driver_override}=="(null)"
    ATTRS{enable}=="1"
    ATTRS{irq}=="124"
    ATTRS{local_cpulist}=="0-3"
    ATTRS{local_cpus}=="f"
    ATTRS{msi_bus}=="1"
    ATTRS{msi_irqs/124}=="msi"
    ATTRS{numa_node}=="-1"
    ATTRS{power/async}=="enabled"
    ATTRS{power/control}=="auto"
    ATTRS{power/runtime_active_kids}=="2"
    ATTRS{power/runtime_active_time}=="29128335"
    ATTRS{power/runtime_enabled}=="enabled"
    ATTRS{power/runtime_status}=="active"
    ATTRS{power/runtime_suspended_time}=="0"
    ATTRS{power/runtime_usage}=="0"
    ATTRS{power/wakeup}=="enabled"
    ATTRS{power/wakeup_abort_count}=="0"
    ATTRS{power/wakeup_active}=="0"
    ATTRS{power/wakeup_active_count}=="0"
    ATTRS{power/wakeup_count}=="0"
    ATTRS{power/wakeup_expire_count}=="0"
    ATTRS{power/wakeup_last_time_ms}=="0"
    ATTRS{power/wakeup_max_time_ms}=="0"
    ATTRS{power/wakeup_total_time_ms}=="0"
    ATTRS{power_state}=="D0"
    ATTRS{revision}=="0x21"
    ATTRS{subsystem_device}=="0x3844"
    ATTRS{subsystem_vendor}=="0x17aa"
    ATTRS{vendor}=="0x8086"

  looking at parent device '/devices/pci0000:00':
    KERNELS=="pci0000:00"
    SUBSYSTEMS==""
    DRIVERS==""
    ATTRS{power/async}=="enabled"
    ATTRS{power/control}=="auto"
    ATTRS{power/runtime_active_kids}=="10"
    ATTRS{power/runtime_active_time}=="0"
    ATTRS{power/runtime_enabled}=="disabled"
    ATTRS{power/runtime_status}=="unsupported"
    ATTRS{power/runtime_suspended_time}=="0"
    ATTRS{power/runtime_usage}=="0"
    ATTRS{waiting_for_supplier}=="0"
```

## Using udev to know a device, 2

```
# Shows udev and kernel actions, with device numbers
udevadm monitor
```

When reading from a new device, in the directory /dev, use:
```bash
udevadm info -an <device>
```
## Udev match keys

"If all match keys match against their values, the rule gets applied and the assigment keys get the specified values assigned".

Most "match keys" are specified singular (without "S") to refer to the target device, or plurar (with "S"), to refer to any parent device. Therefore, a device has `ATTR{}`, and their parents have `ATTRS{}`.

```udev
ACTION=="add" | "remove" | "change", \
ATTR{<attr_name>}=="value", \
```

## Sample rules

Let's define a simple rule.

After making a rule, run:

```
udevadm control --reload
```

You can test a device by using `udevadm test`. You can see that the "run" commands are prepared to execute, but they won't actually be executed.

```
udevadm test <device_path>
```


## Bibliography
[Page 1](https://linuxconfig.org/tutorial-on-how-to-write-basic-udev-rules-in-linux)

Man page for `udev`.
Man page for `udevadm`