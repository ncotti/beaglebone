#include "kmalloc.h"

/******************************************************************************
 * Static variables
******************************************************************************/

u32 *ptr1;
struct driver_data *ptr2;

/******************************************************************************
 * Init and exit
******************************************************************************/

/// @brief This function is called when the module is loaded into the kernel.
static int __init my_module_init(void) {
    printk("Hello, Kernel!\n");

    // Kmalloc works like kmalloc(<size>, <flag>). Returns NULL on error.
    if ((ptr1 = kmalloc(sizeof(u32), GFP_KERNEL)) == NULL) {
        printk("Out of memory\n");
        goto ptr1_kmalloc_error;
    }
    printk("Always clean memory with memset! *ptr1: 0x%x\n", *ptr1);
    *ptr1 = 0xA5A5A5A5;
    printk("*ptr1: 0x%x\n", *ptr1);
    kfree(ptr1);

    // Kzalloc initializes the memory to all zeros.
    if ((ptr1 = kzalloc(sizeof(u32), GFP_KERNEL)) == NULL) {
        printk("Out of memory\n");
        goto ptr1_kmalloc_error;
    }
    printk("Should be zero now. *ptr1: 0x%x\n", *ptr1);
    *ptr1 = 0xA5A5A5A5;
    printk("*ptr1: 0x%x\n", *ptr1);
    kfree(ptr1);

    if ((ptr2 = kzalloc(sizeof(struct driver_data), GFP_KERNEL)) == NULL) {
        printk("Out of memory\n");
        goto ptr1_kmalloc_error;
    }
    ptr2->version = 123;
    strcpy(ptr2->text, "Copied string");
    printk("ptr2->version: %d\n", ptr2->version);
    printk("ptr2->text: %s\n", ptr2->text);

    return 0;
    ptr1_kmalloc_error: return -1;
}

/// @brief This function is called when the module is removed from the kernel.
static void __exit my_module_exit(void) {
    printk("Goodbye, Kernel\n");
    printk("ptr2->version: %d\n", ptr2->version);
    printk("ptr2->text: %s\n", ptr2->text);
    kfree(ptr2);
}

// Set entrypoint and exit point for the kernel module.
module_init(my_module_init);
module_exit(my_module_exit);
