#include <linux/init.h>
#include <linux/module.h>
#include <asm/io.h> // ioremap | iounmap

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nicolas G. Cotti");
MODULE_DESCRIPTION("Reading a memory mapped register from the CPU datasheet");

// Base physical address
#define MAC0_LO 0x44E10630
#define SIZE_MAC 0x08
