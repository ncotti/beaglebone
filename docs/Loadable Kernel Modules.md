# LKM: Loadable Kernel Modules

A LKM is a mechanism for adding code to, or removing code from, the Linux kernel a run time.

The user memory space is isolated from the kernel space. The user can access the kernel by means of <mark style="background: #FFB86CA6;">system calls</mark>. For example, *printf()* is a system call which accesses the standard output of the device. Therefore, writing a kernel module entails quite a challenge, because:
* You won't have access to any C standard library function, or header.
* Code doesn't execute sequentially. A kernel module is initialized, and then uses an event-driven logic to handle request (for example, the device /dev/stdout is a LKM that only operates when that file is written).
* It's really hard to write and debug. Because it runs at kernel level, you can't execute it freely from the user space. Besides, the OS can't do any cleanup or control of the memory resources that are taken. EXTREME CAUTION must be taken to ensure that resources are freed correctly.
* A single error in a LKM can corrupt the entire OS.

![Kernel and user spaces](Images/Pasted%20image%2020230924144256.png)

## Linux headers

In the directory `/usr/src` or in `/lib/modules` (softlink) you can see all the Linux Kernel's headers that are available.  In addition, you can check your current kernel version with:

```bash
$ uname -r
6.2.0-33-generic
```

Let's say something really important:

"A kernel module will only work if it was compiled using the headers for your kernel version."

A Linux kernel should be recompiled every time the kernel is updated, with matching header files. Cross-compiling Linux kernel proves to be challenging. Therefore, all the following instructions assume that they are being run on the target device.

```bash
sudo apt install linux-headers-$(uname -r)
```

