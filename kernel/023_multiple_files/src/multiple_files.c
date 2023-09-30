#include "multiple_files.h"

/// @brief This function is called when the module is loaded into the kernel.
static int __init my_module_init(void) {
    printk("Hello, from multiple_files.c!\n");
    file1_function();
    file2_function();
    return 0;
}

/// @brief This function is called when the module is removed from the kernel.
static void __exit my_module_exit(void) {
    printk("Goodbye, Kernel\n");
}

// Set entrypoint and exit point for the kernel module.
module_init(my_module_init);
module_exit(my_module_exit);
