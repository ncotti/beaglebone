#ifndef KMALLOC_H
#define KMALLOC_H

#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/string.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nicolas G. Cotti");
MODULE_DESCRIPTION("Dynamic memory management with kmalloc() and kfree()");

struct driver_data {
    u8 version;
    char text[64];
};

#endif // KMALLOC_H