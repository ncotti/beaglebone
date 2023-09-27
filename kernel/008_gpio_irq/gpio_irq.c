#include "gpio_irq.h"

/******************************************************************************
 * Static variables
******************************************************************************/

// Contains pin number of interrupt controller to which GPIO 17 is mapped to
static unsigned int irq_number;

static irq_handler_t gpio_irq_handler(unsigned int irq, void *dev_id, struct pt_regs *regs);

/******************************************************************************
 * Init and exit
******************************************************************************/

/// @brief This function is called when the module is loaded into the kernel.
static int __init my_module_init(void) {
    int retval;
    printk("Hello, Kernel!\n");

    // Button initialization
    if ((retval = gpio_request(BUTTON_PIN, BUTTON_NAME)) != 0) {
        printk("Couldn't allocate GPIO %d\n", BUTTON_PIN);
        goto gpio_request_error;
    }
    if (( retval = gpio_direction_input(BUTTON_PIN)) != 0) {
        printk("Couldn't set GPIO %d to input\n", BUTTON_PIN);
        goto button_error;
    }

    // Setup IRQ
    irq_number = gpio_to_irq(BUTTON_PIN);
    if ( (retval = request_irq(irq_number, (irq_handler_t) gpio_irq_handler, IRQF_TRIGGER_RISING, IRQ_NAME, NULL)) != 0) {
        printk("Couldn't request irq number %d\n", irq_number);
        goto button_error;
    }

    printk("GPIO %d is mapped to IRQ %d\n", BUTTON_PIN, irq_number);
    return 0;

    button_error: gpio_free(BUTTON_PIN);
    gpio_request_error: return retval;
}

/// @brief This function is called when the module is removed from the kernel.
static void __exit my_module_exit(void) {
    printk("Goodbye, Kernel\n");
    free_irq(irq_number, NULL);
    gpio_free(BUTTON_PIN);
}

// Set entrypoint and exit point for the kernel module.
module_init(my_module_init);
module_exit(my_module_exit);

/******************************************************************************
 * IRQ handler
******************************************************************************/

static irq_handler_t gpio_irq_handler(unsigned int irq, void *dev_id, struct pt_regs *regs ) {
    printk("gpio_irq: Interrupt triggered!\n");
    return (irq_handler_t) IRQ_HANDLED;
}