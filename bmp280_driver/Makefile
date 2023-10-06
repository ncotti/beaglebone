# Don't use this Makefile directly.
# Please, run the installation script "install.sh"

obj-m += temp_sensor.o
EXTRA_CFLAGS := -I$(src)/inc

temp_sensor-objs := src/driver.o src/char_device.o src/bmp280.o src/cotti_i2c.o

compile:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) modules
	dtc -I dts -O dtb -o cotti_i2c2.dtbo cotti_i2c2.dts
	cp cotti_i2c2.dtbo /lib/firmware/cotti_i2c2.dtbo

clean:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) clean
