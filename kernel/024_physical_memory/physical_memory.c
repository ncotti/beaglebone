#include "physical_memory.h"

/// @brief This function is called when the module is loaded into the kernel.
static int __init my_module_init(void) {
    u8 mac5, mac4, mac3, mac2, mac1, mac0;
    void * ptr;

    printk("Hello, Kernel!\n");

    if ((ptr = ioremap(MAC0_LO, SIZE_MAC)) == NULL) {
        printk(KERN_ERR "Couldn't get MAC address memory\n");
        return -1;
    }
    mac4 = ioread8(ptr);
    mac5 = ioread8(ptr + 1);
    mac0 = ioread8(ptr + 4);
    mac1 = ioread8(ptr + 5);
    mac2 = ioread8(ptr + 6);
    mac3 = ioread8(ptr + 7);

    printk(KERN_INFO "Mac address: %x:%x:%x:%x:%x:%x\n",
        mac0, mac1, mac2, mac3, mac4, mac5);

    iounmap(ptr);
    return 0;
}

/// @brief This function is called when the module is removed from the kernel.
static void __exit my_module_exit(void) {
    printk("Goodbye, Kernel\n");
}

// Set entrypoint and exit point for the kernel module.
module_init(my_module_init);
module_exit(my_module_exit);
