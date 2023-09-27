#include "send_signal.h"

/******************************************************************************
 * Static variables
******************************************************************************/

static irq_handler_t gpio_irq_handler(unsigned int irq, void *dev_id, struct pt_regs *regs);
static int driver_open(struct inode *device_file, struct file *instance);
static int driver_close(struct inode *device_file, struct file *instance);
static long int driver_ioctl(struct file *file , unsigned cmd, unsigned long arg);

static dev_t device_number;
static struct class *device_class;
static struct cdev my_device;
static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = driver_open,
    .release = driver_close,
    .unlocked_ioctl = driver_ioctl,
};

// Information about the process is stored here
static struct task_struct *task = NULL;

static unsigned int irq_number;

/******************************************************************************
 * Init and exit
******************************************************************************/

/// @brief This function is called when the module is loaded into the kernel.
static int __init my_module_init(void) {
    int retval;
    printk("Hello, Kernel!\n");

    // Allocate device number (MAJOR and MINOR)
    if ((retval = alloc_chrdev_region(&device_number, MINOR_NUMBER, NUMBER_OF_DEVICES, DEVICE_NAME)) != 0) {
        printk("Couldn't allocate device number\n");
        goto chrdev_error;
    }
    printk("Device number allocated successfully! Major: %d, Minor: %d\n",
        MAJOR(device_number), MINOR(device_number));

    // Create device class
    if ((device_class = class_create(THIS_MODULE, DEVICE_CLASS_NAME)) == NULL) {
        printk("Device class couldn't be created\n");
        retval = -1;
        goto class_error;
    }

    // Create device file
    if (device_create(device_class, NULL, device_number, NULL, DEVICE_NAME) == NULL) {
        printk("Couldn't create device file\n");
        retval = -1;
        goto device_error;
    }

    // Initializing and registering device file
    cdev_init(&my_device, &fops);
    if ((retval = cdev_add(&my_device, device_number, NUMBER_OF_DEVICES)) != 0 ) {
        printk("Registering of device to kernel failed\n");
        goto cdev_error;
    }

    // Button initialization
    if ((retval = gpio_request(BUTTON_PIN, BUTTON_NAME)) != 0) {
        printk("Couldn't allocate GPIO %d\n", BUTTON_PIN);
        goto cdev_error;
    }
    if (( retval = gpio_direction_input(BUTTON_PIN)) != 0) {
        printk("Couldn't set GPIO %d to input\n", BUTTON_PIN);
        goto button_error;
    }
    if (gpio_set_debounce(BUTTON_PIN, DEBOUNCE_TIME) != 0) {
        // The debounce feature might exists, but normally just return ENOSYS
        printk("Couldn't set debounce. Continuing...\n");
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
    cdev_error: device_destroy(device_class, device_number);
    device_error: class_destroy(device_class);
    class_error: unregister_chrdev(device_number, DEVICE_NAME);
    chrdev_error: return retval;
}

/// @brief This function is called when the module is removed from the kernel.
static void __exit my_module_exit(void) {
    printk("Goodbye, Kernel\n");
    free_irq(irq_number, NULL);
    gpio_free(BUTTON_PIN);
    device_destroy(device_class, device_number);
    class_destroy(device_class);
    unregister_chrdev(device_number, DEVICE_NAME);
}

// Set entrypoint and exit point for the kernel module.
module_init(my_module_init);
module_exit(my_module_exit);

/******************************************************************************
 * IRQ handler
******************************************************************************/

/// @brief Handler for the GPIO IRQ
static irq_handler_t gpio_irq_handler(unsigned int irq, void *dev_id, struct pt_regs *regs ) {
    struct siginfo info;
    printk("Interrupt triggered!\n");
    if (task != NULL) {
        memset(&info, 0, sizeof(info));
        info.si_signo = SIGUSR1;
        info.si_code = SI_QUEUE;

        // Send signal to "task", which should have been registered with the ioctl() call.
        if (send_sig_info(SIGUSR1, (struct kernel_siginfo *) &info, task) != 0) {
            printk("Error sending signal\n");
        } else {
            printk("Signal sended to %d\n", task->pid);
        }
    }
    return (irq_handler_t) IRQ_HANDLED;
}

/// @brief This function is called when the device is opened
static int driver_open(struct inode *device_file, struct file *instance) {
    printk("Device was opened!\n");
    return 0;
}

/// @brief This function is called when the device is opened
static int driver_close(struct inode *device_file, struct file *instance) {
    printk("Device was closed!\n");
    task = NULL;
    return 0;
}

/// @brief It's used to get the PID of the user program, and send signals to that.
static long int driver_ioctl(struct file *file , unsigned cmd, unsigned long arg) {
    if (cmd == IO_REGISTER_USER_APP) {
        task = get_current();
        printk("Registered user space app with PID: %d\n", task->pid);
    } else {
        printk("Unknown ioctl command\n");
        return -1;
    }
    return 0;
}