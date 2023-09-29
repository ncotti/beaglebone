# Phandles

The concept of phandles and labels requires a thorough explanation.

A **phandle** is a u32 number, which is used as basically a pointer. You can reference another element in the device tree using that u32 value.

There are various ways to set and get the phandle value.

## Using u32 values

```dts
// Interrupt controller
pic {
    phandle = <1>;  // This node can be referenced using the literal u32 value "1".
};

uart@ff00 {
    interrupt-parent = <1>; // The uart has a reference to the interrupt controller.
};
```

## Using labels

Labels do not refer to the property `label = "my_label"` that some nodes may have. A label is a fantasy name used to describe a node.

```dts
[label:] node-name[@unit-address] {
    [properties definitions]
    [child nodes]
};
```

Instead of hardcoding literal values for the "phandle", you can reference a node path or phandle with the label. The compiler will replace all labels with an unique u32 number:

Before compilation to DTB:

```dts
// We are using the label "PIC_LABEL"
PIC_LABEL: pic {
};

uart@ff00 {
    interrupt-parent = <&PIC_LABEL>; // Using label for "phandle"
    interrupt-parent2 = <&{/pic}>;   // You can use the literal path aswell
};
```

After compilation and decompilation:

```dts
// Interrupt controller
pic {
    phandle = <1>;
};

uart@ff00 {
    interrupt-parent = <1>;
    interrupt-parent2 = <1>;
    interrupt-path = "/pic";
};

// All labels are stored here, with their corresponding path
__symbols__ {
    PIC_LABEL = "/pic";
};
```

This is important to know, because you might see several `phandle = <0x42>` statements, which most likely were replaced for labels.

## Using aliases and \_\_symbols\_\_

We have a problem. If we are writing an overlay for an already compiled DTB file, then we won't be able to access the labels, we will need to explicitly use the phandle u32 values. Fortunately, that's not the case.

After decompiling a DTB file, the DTS will generate a node called `__symbols__ {}`. Inside that node there will be a list of properties, where each entry corresponds to a previously existing label.

Besides, the user might create the node called `aliases {}`, which is an explicit intention from the user to preserve label names.

You can use any value form aliases or __symbols__ as such:

Decompiled DTB file:

```dts

// User defined aliases
aliases {
    my_alias = "/ocp/interconnect@44c00000/segment@200000/target-module@b000/i2c@0";
};

__symbols__ {
    pic = "/ocp/interconnect@44c00000/segment@200000/target-module@0/prcm@0/per-cm@0";
};
```

```dts
// Using "my_alias" as insertion point for module
&my_alias {
    // Using "pic" as phandle value.
    interrupt-parent = <&pic>;
}
```

Note: You can retrieve the path from the alias, and use it as a property value with: `path = &alias;`. However, that isn't possible from an overlay. Property values need to be defined at compile time, and the overlay is not compiled together with the base DTS file.

