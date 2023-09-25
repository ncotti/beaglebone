#include <linux/init.h>     // Always include these two headers
#include <linux/module.h>

// Module description
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nicolas G. Cotti");
MODULE_DESCRIPTION("A hello world LKM");

/// @brief This function is called when the module is loaded into the kernel.
static int __init my_module_init(void) {
    printk("Hello, Kernel!\n");
    return 0;
}

/// @brief This function is called when the module is removed from the kernel.
static void __exit my_module_exit(void) {
    printk("Goodbye, Kernel\n");
}

// Set entrypoint and exit point for the kernel module.
module_init(my_module_init);
module_exit(my_module_exit);
