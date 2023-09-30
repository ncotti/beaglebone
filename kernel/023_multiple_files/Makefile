# This is the name of the output LKM "multiple.ko == <LKM-name>.o"
obj-m += multiple.o

# Define include folders
EXTRA_CFLAGS := -I$(src)/inc

# Here, you define all objetc files needed to compile as <LKM-name>-objs := <object files>
# <LKM-name> SHOULD NOT BE THE SAME AS ANY OF THE OBJECT FILES
multiple-objs := src/multiple_files.o src/file1.o src/file2.o

all:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) clean
