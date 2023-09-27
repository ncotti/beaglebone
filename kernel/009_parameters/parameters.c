#include "parameters.h"

/******************************************************************************
 * Static variables
******************************************************************************/

static unsigned int param_number = 0;
static char *param_string = "default";

// Arguments are:
//  * Variable to use a parameter (gpio_number)
//  * Type of the variable (uint)
//  * Permissions
module_param(param_number, uint, S_IRUGO);
module_param(param_string, charp, S_IRUGO);

// Description of the parameters
MODULE_PARM_DESC(param_number, "Numerical parameter");
MODULE_PARM_DESC(param_string, "String parameter");

/******************************************************************************
 * Init and exit
******************************************************************************/

/// @brief This function is called when the module is loaded into the kernel.
static int __init my_module_init(void) {
    printk("Hello, Kernel!\n");
    printk("gpio_number: %u\n", param_number);
    printk("device_name: %s\n", param_string);
    return 0;
}

/// @brief This function is called when the module is removed from the kernel.
static void __exit my_module_exit(void) {
    printk("Goodbye, Kernel\n");
}

// Set entrypoint and exit point for the kernel module.
module_init(my_module_init);
module_exit(my_module_exit);
