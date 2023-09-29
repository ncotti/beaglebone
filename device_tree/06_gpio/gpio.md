# Deconstructing the Device Tree the hard way

So, we want to now do some meaningful modifications to the already existing device tree of the beaglebone black (file is in the parent directory).

Especifically, we want to modify the GPIO60 (P9_12). So let's see how it is defined, and what we can do.

## Default behaviour

Before starting, let's look at the computer on boot, and see how the default configuration responds:

```bash
debian@BeagleBone:/sys/class/gpio$ ls -l gpio60
lrwxrwxrwx 1 root root 0 Jan  1  2000 gpio60 -> ../../devices/platform/ocp/48000000.interconnect/48000000.interconnect:segment@0/4804c000.target-module/4804c000.gpio/gpiochip1/gpio/gpio60

debian@BeagleBone:/sys/class/gpio/gpio60$ ls
active_low  device  direction  edge  label  power  subsystem  uevent  value

debian@BeagleBone:/sys/class/gpio/gpio60$ cat direction 
in

debian@BeagleBone:/sys/class/gpio/gpio60$ cat value
1
```

So, we can deduce the following:

* The GPIO 60 is associated with the "gpiochip1", which should be the gpio controller.
* By default, it's configured as an input with pull-up resistor.

## Diving deep into the device tree file

If you start searching for keywords in the base decompiled DTS file, you will find several sections related to the pin P9_12.

The first one to mention is this one (Note: "string1\0string2" is the same as "string1", "string2"):

```dts
&{/ocp/P9_12_pinmux} {
    compatible = "bone-pinmux-helper";
    status = "okay";
    pinctrl-names = "default\0gpio\0gpio_pu\0gpio_pd";
    pinctrl-0 = <0x14b>;
    pinctrl-1 = <0x14c>;
    pinctrl-2 = <0x14d>;
    pinctrl-3 = <0x14e>;
}
```

This node is calling the driver `bone-pinmux-helper`, and it's associating four keywords to this GPIO.
We can see this exact behavior in action if we use the `config-pin` tool from the BeagleBone:

```bash
$ config-pin -l P9_12
Available modes for P9_12 are: default gpio gpio_pu gpio_pd

debian@BeagleBone:~$ config-pin -q P9_12
Current mode for P9_12 is:     default
```

The next step is to look at the phandles that point to `pinctrl-0, pinctrl-1` and so on.

```dts
&am33xx_pinmux {
    pinmux_P9_12_default_pin {
    pinctrl-single,pins = <0x78 0x37>;
    phandle = <0x14b>;
    };

    pinmux_P9_12_gpio_pin {
        pinctrl-single,pins = <0x78 0x2f>;
        phandle = <0x14c>;
    };

    pinmux_P9_12_gpio_pu_pin {
        pinctrl-single,pins = <0x78 0x37>;
        phandle = <0x14d>;
    };

    pinmux_P9_12_gpio_pd_pin {
        pinctrl-single,pins = <0x78 0x27>;
        phandle = <0x14e>;
    };
}
```

Here, we are basically defining a single property called `pinctrl-single,pins = <PIN_ID mode>`; where the first element identifies the pin (0x78 == P9_12), and the second value sets the operation mode. If you see at the source code from their [Github](https://github.com/beagleboard/BeagleBoard-DeviceTrees/tree/v5.10.x-ti-unified/src/arm):

```dts
/* P9_12 (ZCZ ball U18) gpmc_be1n (gpio1_28) */
BONE_PIN(P9_12, default, P9_12(PIN_OUTPUT_PULLUP | INPUT_EN | MUX_MODE7))
BONE_PIN(P9_12, gpio, P9_12(PIN_OUTPUT | INPUT_EN | MUX_MODE7))
BONE_PIN(P9_12, gpio_pu, P9_12(PIN_OUTPUT_PULLUP | INPUT_EN | MUX_MODE7))
BONE_PIN(P9_12, gpio_pd, P9_12(PIN_OUTPUT_PULLDOWN | INPUT_EN | MUX_MODE7))
```

Next step, we have this node. This node doesn't serve any specific purpose, all gpios are atached to this. I believe that is used to set default values for every GPIO. This configuration gets overriden by any other node.

```dts
&{/ocp/cape-universal/P9_12} {
    gpio-name = "P9_12";
    gpio = <0x42 0x1c 0x00>;
    input;
    dir-changeable;
};
```

Of special interest is the `gpio = <0x42 0x1c 0x00>;` property. After digging the original source code, we cac find that it means the following:

```dts
gpio = <&gpio1 28 GPIO_ACTIVE_HIGH>
gpio = <0x42 0x1c 0x00>;
```

Where `&gpio` is the phandle for the gpio controller, `28` is the pin number (we are using GPIO60, which is Port 1, pin 28).

So now, let's check this GPIO controller shall we?

```dts
&gpio1 {
    compatible = "ti,omap4-gpio";
    gpio-ranges = <0x20 0x00 0x00 0x08 0x20 0x08 0x5a 0x04 0x20 0x0c 0x0c 0x10 0x20 0x1c 0x1e 0x04>;
    gpio-controller;
    #gpio-cells = <0x02>;
    interrupt-controller;
    #interrupt-cells = <0x02>;
    reg = <0x00 0x1000>;
    interrupts = <0x62>;
    gpio-line-names = "P8_25 [mmc1_dat0]\0[mmc1_dat1]\0P8_5 [mmc1_dat2]\0P8_6 [mmc1_dat3]\0P8_23 [mmc1_dat4]\0P8_22 [mmc1_dat5]\0P8_3 [mmc1_dat6]\0P8_4 [mmc1_dat7]\0NC\0NC\0NC\0NC\0P8_12\0P8_11\0P8_16\0P8_15\0P9_15A\0P9_23\0P9_14 [ehrpwm1a]\0P9_16 [ehrpwm1b]\0[emmc rst]\0[usr0 led]\0[usr1 led]\0[usr2 led]\0[usr3 led]\0[hdmi irq]\0[usb vbus oc]\0[hdmi audio]\0P9_12\0P8_26\0P8_21 [emmc]\0P8_20 [emmc]";
    phandle = <0x42>;
};
```

We can corroborate that its phandle is <0x42>, and that in the "gpio-line-names" it is included P9_12.
