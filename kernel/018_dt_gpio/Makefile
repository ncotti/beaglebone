obj-m += dt_gpio.o

all:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) modules
	dtc -I dts -O dtb -o dts_overlay.dtbo dts_overlay.dts
	cp dts_overlay.dtbo /home/debian/device-tree/my_overlay.dtbo
	@echo "Restart the board to apply changes to device tree\n"
clean:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) clean
