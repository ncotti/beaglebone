# Analyzing the basic device tree overlay example

In this document, I will emphasize the core concepts of the minimal device tree overlay, from which I will build upon on the next examples.

The first line is the DTS version. The second line marks the file as an OVERLAY, instead of a raw device tree file.

```dts
/dts-v1/;   // Version
/plugin/;   // OVERLAY
```

The `/` here is very important. This line marks the beginning of the root directory for the device tree. Every element from a device tree can be referenced using a path notation like "/first_node/second_node"

```dts
// Root directory
/ {
};
```

In the root directory, we should list all the properties that should match with the base DTS file where we want to put the overlay. If these properties don't match, then the overlay won't be loaded. The meaning of the properties will be explained in the next section.

```dts
/ {
    compatible = "ti,am335x-bone-black", "ti,am335x-bone", "ti,am33xx";
    model = "TI AM335x BeagleBone Black";
    #address-cells = <1>;
    #size-cells = <0>;
};
```

Next, you should specify the "fragment" of the device tree that you want to insert. The name "fragment" is standard, and "@0" is used to identify several fragments.

The property `target-path = "/";` tells that this fragment should be inserted in the root directory of the base device tree.

Later, the `__overlay__{};` child marks the beginning of the inserted code.

```dts
    fragment@0 {
        target-path = "/";
        __overlay__ {
            // Your device to be inserted
        };
    };
```

## Running the example

To run the example, execute the `copy.sh` command, that will copy all files to the beaglebone, and then execute the `install.sh` there.

The root directory of the device tree gets mapped to `/ -> /proc/device-tree`, so you should see your new device in the directory `/proc/device-tree/hello_device`, and you should be able to read the properties as files inside that directory:

```bash
debian@BeagleBone:/proc/device-tree/hello_device$ ls
compatible  label  name  status
debian@BeagleBone:/proc/device-tree/hello_device$ cat label
Hello
```
