obj-m += reading_device_tree.o

all:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) modules
	dtc -I dts -O dtb -o dts_overlay.dtbo dts_overlay.dts
	cp dts_overlay.dtbo /home/debian/device-tree/my_overlay.dtbo
clean:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) clean
