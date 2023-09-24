# The embedded programmer IDE

In this document, I will go through all steps needed to get a simple programming environment going for an embedded device using Linux.

## Installing an image to the Beaglebone black

Go to the beaglebone Page and install the latest image. Warning: dont't install the eMMC image. Go to the notes of the isntallation and download the one for booting from the SD card.

## Communicate with the board

The first step is to have a reliable connection to the board. If it's a simple bare-metal board, you'll have the debugger and flasher interfaces, such as STLink, JTAG, SWD (Serial Wire Debug), etc. If this is the case, please check my [embedded turorial repo](https://github.com/ncotti/tutorials).

If your board has Linux installed, the first step is to get a SSH connection to the board. Flashing the Linux Kernel and getting this connection should be specified by the vendor of the board, but this process is generally reduced to flashing the Linux Image, and plugging the board through USB to the desktop computer to provide both power and LAN connection.

Internet connection will be required as well, as some Linux packages will need to be installed on the board. If you can connect an Ethernet cable from your router to the board, and maybe an external power supply to have the board running without being physically connected to your PC, problem solved. Otherwise, follow the next guide.

### Getting internet over USB

The following section is based on [this document](https://gist.github.com/pdp7/d2711b5ff1fbb000240bd8337b859412).
If you try to access the Internet from your board, you will get:

```bash
$ ping 8.8.8.8
connect: Network is unreachable
```

First, connect your board to the computer, and run the `ifconfig` command. We want to get the name of the internet interface ("wlp2s0"), and the name of the ethernet interface with the board ("enx98f07b9de2e5"). 

```bash
$ ifconfig
wlp2s0: flags=4163<UP,BROADCAST,RUNNING,MULTICAST> mtu 1500
	inet 192.168.0.77 netmask 255.255.255.0 broadcast 192.168.0.255
	inet6 fe80::f99d:5d68:d398:61d8 prefixlen 64 scopeid 0x20<link>
	ether 10:f0:05:e9:63:f3 txqueuelen 1000 (Ethernet)
	RX packets 8850 bytes 6256151 (6.2 MB)
	RX errors 0 dropped 0 overruns 0 frame 0
	TX packets 6731 bytes 2389176 (2.3 MB)
	TX errors 0 dropped 0 overruns 0 carrier 0 collisions 0
	
enx98f07b9de2e5: flags=4163<UP,BROADCAST,RUNNING,MULTICAST> mtu 1500
	inet 192.168.7.1 netmask 255.255.255.0 broadcast 192.168.7.255
	inet6 fe80::22d8:de85:abc9:8d0c prefixlen 64 scopeid 0x20<link>
	ether 98:f0:7b:9d:e2:e5 txqueuelen 1000 (Ethernet)
	RX packets 328 bytes 38831 (38.8 KB)
	RX errors 0 dropped 0 overruns 0 frame 0
	TX packets 469 bytes 68970 (68.9 KB)
	TX errors 0 dropped 0 overruns 0 carrier 0 collisions 0
```

Then, we need to enable "ip forwarding". First, make sure that the following lines in the file `/etc/sysctl.conf`:

```txt
# Uncomment the next line to enable packet forwarding for IPv4
net.ipv4.ip_forward=1
# Uncomment the next line to enable packet forwarding for IPv6
net.ipv6.conf.all.forwarding=1
```

Then, you should run the following script (this script might have to be run every time the computer reboots).

```bash
internet="wlp2s0"
bbb_ether="enx98f07b9de2e5"
sudo iptables -t nat -F; sudo iptables -t mangle -F; sudo iptables -F; sudo iptables -X
sudo iptables --table nat --append POSTROUTING --out-interface "${internet}" -j MASQUERADE
sudo iptables --append FORWARD --in-interface "${bbb_ether}" -j ACCEPT
sudo sh -c "echo 1 > /proc/sys/net/ipv4/ip_forward"
```

All done from the computer, now we need to configure the internet from the board. Run on the BeagleBone:

```bash
sudo route add default gw 192.168.7.1
sudo sh -c "echo \"nameserver 8.8.8.8\" >> /etc/resolv.conf"
```

## Cross-compiling

The first step is to compile for the specific architecture that you are going to use. You may have an Intel processor, but your target is using RISC-V or ARM. Therefore, you need to compile from your computer, code that will be executed on the other device.

### Installing the toolchain with Linaro

You can get all the cross-compilation tools and libraries from the [Linaro webpage](https://www.linaro.org/downloads/). Download the files for the architecture you want to compile to, and add the compilation binaries to your PATH variable. After that, the cross-compilation tools work exactly as the normal ones, but have the architecture name prepended:

```bash
echo "export PATH=${PATH}:<path_to_linaro>/bin/"
arm-linux-gnueabihf-gcc -v
```

We can check that, in fact, different libraries and headers are used for compilation with the following command:

```bash
$ echo | gcc -xc -E -v -
#include "..." search starts here:
#include <...> search starts here:
 /usr/lib/gcc/x86_64-linux-gnu/11/include
 /usr/local/include
 /usr/include/x86_64-linux-gnu
 /usr/include
LIBRARY_PATH=/usr/lib/gcc/x86_64-linux-gnu/11/:/usr/lib/gcc/x86_64-linux-gnu/11/../../../x86_64-linux-gnu/:/usr/lib/gcc/x86_64-linux-gnu/11/../../../../lib/:/lib/x86_64-linux-gnu/:/lib/../lib/:/usr/lib/x86_64-linux-gnu/:/usr/lib/../lib/:/usr/lib/gcc/x86_64-linux-gnu/11/../../../:/lib/:/usr/lib/

$ echo | arm-linux-gnueabihf-gcc -xc -E -v -
#include "..." search starts here:
#include <...> search starts here:
 /home/cotti/.gcc-linaro/bin/../lib/gcc/arm-linux-gnueabihf/7.5.0/include
 /home/cotti/.gcc-linaro/bin/../lib/gcc/arm-linux-gnueabihf/7.5.0/include-fixed
 /home/cotti/.gcc-linaro/bin/../lib/gcc/arm-linux-gnueabihf/7.5.0/../../../../arm-linux-gnueabihf/include
 /home/cotti/.gcc-linaro/bin/../arm-linux-gnueabihf/libc/usr/include
LIBRARY_PATH=/home/cotti/.gcc-linaro/bin/../lib/gcc/arm-linux-gnueabihf/7.5.0/:/home/cotti/.gcc-linaro/bin/../lib/gcc/arm-linux-gnueabihf/:/home/cotti/.gcc-linaro/bin/../lib/gcc/:/home/cotti/.gcc-linaro/bin/../lib/gcc/arm-linux-gnueabihf/7.5.0/../../../../arm-linux-gnueabihf/lib/:/home/cotti/.gcc-linaro/bin/../arm-linux-gnueabihf/libc/lib/:/home/cotti/.gcc-linaro/bin/../arm-linux-gnueabihf/libc/usr/lib/
```

As a side note, if you want to compile for a Linux system, you shouldn't use the linker "ld" for linking. Instead, call directly "gcc", because it will handle all the library paths and default memory allocation needed for the Linux OS. 

Specially, it will tag the dynamic libraries needed at runtime (or static libraries if you the gcc flag `-static` while linking) . This is important because the binary file will be executed on the embedded device, and you will need to have installed those libraries both on your desktop computer to be able to compile; and on your device to execute it. The path to those libraries is not hardcoded. If you look at the ".elf" file headers, you will see that all libraries are listed in the "dynamic section":

```txt
Dynamic Section:
  NEEDED               libc.so.6
```

When executed, these can be found with the variable `LD_LIBRARY_PATH` set in the Linux device (`LD_LIBRARY_PATH` is not exported to child shells, so you can't see its value from the terminal).

### Binary installation of the toolchain

**This methodology is not recommended**. To compile for another architecture rather than your own, you first need to add the [multi-arch support from Debian](https://wiki.debian.org/Multiarch/HOWTO)

```
$ dpkg --print-architecture
amd64
$ sudo dpkg --add-architecture armhf
$ dpkg --print-foreign-architectures
i386 armhf
```

Now, you need to add access to the binaries' repositories for that architecture. For example, you have installed in your computer the standard C library for amd64, but you need the one for armhf. Go to the file `/etc/apt/sources.list`, and do the following:

* Add `[arch=amd64]` in all the current binary repositories.
* Copy the following repositories at the end of the file, which contain the binaries for the new foreign architecture (in this example, is for arch=armhf, and using Ubuntu jammy):

```txt
deb [arch=armhf] http://ports.ubuntu.com/ jammy main restricted
deb [arch=armhf] http://ports.ubuntu.com/ jammy-updates main restricted
deb [arch=armhf] http://ports.ubuntu.com/ jammy universe
deb [arch=armhf] http://ports.ubuntu.com/ jammy-updates universe
deb [arch=armhf] http://ports.ubuntu.com/ jammy multiverse
deb [arch=armhf] http://ports.ubuntu.com/ jammy-updates multiverse
deb [arch=armhf] http://ports.ubuntu.com/ jammy-backports main restricted universe multiverse
```

After saving the file, let's update the apt repositories, and install the toolchain (notice the ":armhf" at the end of the packaged):

```bash
sudo apt update
sudo apt install crossbuild-essential-armhf # Install crosscompiler
sudo apt install libudev-dev:armhf # Libraries for armhf
sudo apt install libstdc++6:armhf
```

## Running scripts on boot

There are several ways of implementing boot scripts.

* `rc.local` file.
* `systemd` services.
* `init.d`
* `crontab` tables.

I will only explain the last one.
### Crontables

`crontab` is a Linux command that allows to schedule commands to run on a regular basis. You can put files inside any of the following folders to execute a script hourly, daily, weekly or monthly:
* `/etc/cron.hourly`
* `/etc/cron.daily`
* `/etc/cron.weekly`
* `/etc/cron.monthly`

However, since the BeagleBone doesn't have a RTC, the board might be turned off for a week, and miss the execution of most of it's programmed scripts. To solve this, check the `anacron` command.

To create a script that executes on boot, you should modify the cron tables directly. There are two separate list, the user tables, and the root user tables, depending if you execute the command with root privileges or not.

**Important!** If you call scripts, make sure to use full paths. The "route" command does not have effect, probably another process takes control of the internet connection after this script executes.
 
```bash
$ sudo crontab -e
#Add this to the file
@reboot /sbin/route add default gw 192.168.7.1; echo "nameserver 8.8.8.8" >> /etc/resolv.conf
```

## Debugging the program on the board

Now, you should have your binary file `.elf` on your computer. We want to execute and debug it on the device. For this to work, you should install the ssh and gdb servers as:

```bash
sudo apt install gdbserver openssh-server
```

Then, execute the gdb server specifying the port (the IP is localhost because the gdb server is running on the board, which has it's own IP).

```bash
$ gdbserver --multi localhost:2159
Listening on port 2159
```

Use the `scp` program to copy only the `.elf` file to any path on the board. After that, execute `gdb-multiarch` from the desktop PC.

```bash 
# Copy binary file
elf_file=bin/exe.elf
gdb_script=debug.gdb
sshpass -p temppwd scp ${elf_file} debian@192.168.7.2:/home/debian/exe.elf
gdb-multiarch -q $${gdb_script} "${elf_file}"
```

The `debug.gdb` file connects to the board (which has the gdbserver running on port 2159), sets the remote executable file to the address where we copied the file, and then runs the program normally.

We had to specify the binary from the desktop PC at launch for gdb to read all the symbols and know where the source files in the desktop PC are for debugging. In addition, we are telling to gdb that the same binary is being executed on a remote device, and the address in that device. 

```gdb
set architecture arm
target extended-remote 192.168.7.2:2159
set remote exec-file /home/debian/exe.elf
lay src
b main
run
```


## References

* [Debian Multiarch Wiki](https://wiki.debian.org/Multiarch/HOWTO)
* [Internet Over USB](https://gist.github.com/pdp7/d2711b5ff1fbb000240bd8337b859412)