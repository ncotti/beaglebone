#include "cotti_i2c.h"

/******************************************************************************
 * Static variables
******************************************************************************/

// Pointers to memory mapped registers of the CPU
static void* i2c_ptr = NULL;
static void* clk_ptr = NULL;
static void* control_module_ptr = NULL;

// Used to signal from the IRQ that a data was received from the i2c bus
DECLARE_COMPLETION(comp_read);

// Used to signal from the IRQ that all data was written on the bus
DECLARE_WAIT_QUEUE_HEAD(wq_write);

// Used to signal that the "stop" was issued on the I2C bus. Operation completed
DECLARE_COMPLETION(comp_stop);

// Used to enable only one process to read or write to the bus
DEFINE_MUTEX(lock_bus);

// Buffer for read and written values, as well as length of transaction
static u8 g_read = 0;
static u8 g_write[2] = {0,0};   // Utmost value is written first
static u16 g_len = 0;

/******************************************************************************
 * Static functions' prototypes
******************************************************************************/

static inline void prv_set_bit(void* addr, u32 bit) {
    iowrite32(ioread32(addr) | bit, addr);
}
static inline void prv_clear_bit(void* addr, u32 bit) {
    iowrite32(ioread32(addr) & ~bit, addr);
}
static inline u8 prv_is_resetting(void) {
    return ! (ioread8(i2c_ptr + I2C_REG_SYSS) & 1);
}
static inline void prv_write(u8 value) {
    iowrite8(value, i2c_ptr + I2C_REG_DATA);
}
static inline u8 prv_read(void) {
    return ioread8(i2c_ptr + I2C_REG_DATA);
}
static inline u8 prv_is_irq_triggered(u16 irq) {
    return ioread16(i2c_ptr + I2C_REG_IRQSTATUS) & irq;
}
static inline void prv_wakeup(void) {
    iowrite32(CLOCK_I2C2_ENABLE, clk_ptr + CLOCK_REG_I2C2);
}

/// @brief Set the amount of bytes to read or write to the i2c bus, excluding
///  the slave address
static inline void prv_set_count(u16 count) {
    g_len = (count <= 2) ? count : 2;
    iowrite16(count, i2c_ptr + I2C_REG_CNT);
}

static int prv_reset_i2c(void);
static int prv_wait_for_bus_busy(void);

/******************************************************************************
 * Functions
******************************************************************************/

/// @brief Handler for the I2C IRQ.
irqreturn_t cotti_i2c_isr(int irq_number, void *dev_id) {
    u16 irq;
    u16 enabled = ioread16(i2c_ptr + I2C_REG_IRQENABLE_SET);

    while(enabled & (irq = ioread16(i2c_ptr + I2C_REG_IRQSTATUS))) {
        if (irq & I2C_IRQ_AL) {
            printk("AL\n");
        } if (irq & I2C_IRQ_ARDY) {
            complete(&comp_stop);
        } if (irq & I2C_IRQ_NACK) {
            printk("NACK\n");
        } if (irq & I2C_IRQ_RRDY) {
            g_len--;
            g_read = prv_read();
            complete(&comp_read);
        } if (irq & I2C_IRQ_XRDY) {
            g_len--;
            prv_write(g_write[g_len]);
            wake_up_interruptible(&wq_write);
        }
        iowrite16(irq, i2c_ptr + I2C_REG_IRQSTATUS);
    }
    return IRQ_HANDLED;
}

/// @brief Initialize the I2C2 bus, with pins P9.21 and P9.22.
int cotti_i2c_init(void) {
    if((clk_ptr = ioremap(CLOCK_BASE_ADDRESS, CLOCK_SIZE)) == NULL) {
        printk(KERN_ERR "Couldn't configure Clock Manager Peripheral\n");
        return -1;
    }
    if((control_module_ptr = ioremap(CONTROL_MODULE_BASE_ADDRESS, CONTROL_MODULE_SIZE)) == NULL) {
        printk(KERN_ERR "Couldn't configure I2C2\n");
        goto clock_error;
    }
    if((i2c_ptr = ioremap(I2C2_BASE_ADDRESS, I2C2_SIZE)) == NULL) {
        printk(KERN_ERR "Couldn't configure I2C2\n");
        goto pin_mux_error;
    }

    // Configure P9.21 and P9.22 pinmux as I2C
    iowrite32(CONTROL_MODULE_PINMUX_P9_21_I2C, control_module_ptr + CONTROL_MODULE_REG_P9_21);
    iowrite32(CONTROL_MODULE_PINMUX_P9_22_I2C, control_module_ptr + CONTROL_MODULE_REG_P9_22);

    prv_wakeup();

    if(prv_reset_i2c() != 0) {
        goto i2c_error;
    }

    // Smart idle
    iowrite32(I2C_BIT_CLKACTIVITY | I2C_BIT_IDLEMODE | I2C_BIT_WAKEUP |
        I2C_BIT_AUTOIDLE, i2c_ptr + I2C_REG_SYSC);

    // Configure clock
    iowrite8(I2C_PSC_VALUE, i2c_ptr + I2C_REG_PSC);
    iowrite8(I2C_SCLL_VALUE, i2c_ptr + I2C_REG_SCLL);
    iowrite8(I2C_SCLH_VALUE, i2c_ptr + I2C_REG_SCLH);

    // Enable I2C device
    iowrite32(I2C_BIT_ENABLE | I2C_BIT_MASTER_MODE | I2C_BIT_TX,
        i2c_ptr + I2C_REG_CON);

    // Configure Slave address
    iowrite32(I2C_SLAVE_ADDRESS, i2c_ptr + I2C_REG_SA);

    // Enable interrupts
    iowrite16(I2C_IRQ_XRDY | I2C_IRQ_RRDY | I2C_IRQ_NACK | I2C_IRQ_ARDY |
        I2C_IRQ_AL, i2c_ptr + I2C_REG_IRQENABLE_SET);

    printk("I2C successfully configured!\n");
    return 0;

    i2c_error: iounmap(i2c_ptr);
    pin_mux_error: iounmap(control_module_ptr);
    clock_error: iounmap(clk_ptr);
    i2c_ptr = NULL;
    clk_ptr = NULL;
    return -1;
}

/// @brief Deinitialize the I2C2 bus.
void cotti_i2c_deinit(void) {
    if (clk_ptr != NULL) {
        iounmap(clk_ptr);
    } if (control_module_ptr != NULL) {
        iounmap(control_module_ptr);
    } if (i2c_ptr != NULL) {
        iounmap(i2c_ptr);
    }
}

/// @brief Write a value to the I2C bus.
/// @param value Value to be written
/// @param address Address of the device register
int cotti_i2c_write(u8 value, u8 address) {
    int retval = 0;
    prv_wakeup();

    if(prv_wait_for_bus_busy() != 0) {
        return -1;
    }
    mutex_lock(&lock_bus);

    // Amount of bytes to transfer
    prv_set_count(2);
    g_write[1] = address;
    g_write[0] = value;

    // Clear fifo buffer
    prv_set_bit(i2c_ptr + I2C_REG_BUF, I2C_BIT_RXFIFO_CLR | I2C_BIT_TXFIFO_CLR);

    iowrite32(I2C_BIT_ENABLE | I2C_BIT_MASTER_MODE | I2C_BIT_START |
        I2C_BIT_TX | I2C_BIT_STOP, i2c_ptr + I2C_REG_CON);

    if (wait_event_interruptible_timeout(wq_write, !g_len, msecs_to_jiffies(TIMEOUT_READ_WRITE)) == 0 ||
        wait_for_completion_timeout(&comp_stop, msecs_to_jiffies(TIMEOUT_READ_WRITE)) == 0 ) {
        printk("Timeout reached on i2c write\n");
        retval = -1;
    }
    reinit_completion(&comp_stop);
    mutex_unlock(&lock_bus);
    return retval;
}

/// @brief Read a value from the I2C bus.
/// @param address Register address of the device
/// @return Value read.
int cotti_i2c_read(u8 address) {
    prv_wakeup();
    msleep(1);  // TODO, delay need for I2C wakeup

    if (prv_wait_for_bus_busy() != 0) {
        return -1;
    }
    mutex_lock(&lock_bus);

    // Amount of bytes to transfer
    prv_set_count(1);
    g_write[0] = address;

    // Clear fifo buffer
    prv_set_bit(i2c_ptr + I2C_REG_BUF, I2C_BIT_RXFIFO_CLR | I2C_BIT_TXFIFO_CLR);

    // Enable TX
    iowrite32(I2C_BIT_ENABLE | I2C_BIT_MASTER_MODE | I2C_BIT_START |
        I2C_BIT_TX | I2C_BIT_STOP, i2c_ptr + I2C_REG_CON);

    // Wait until transmit data
    if (wait_event_interruptible_timeout(wq_write, !g_len, msecs_to_jiffies(TIMEOUT_READ_WRITE)) == 0 ||
        wait_for_completion_timeout(&comp_stop, msecs_to_jiffies(TIMEOUT_READ_WRITE)) == 0) {
        printk("Timeout reached on i2c read register address' write\n");
        reinit_completion(&comp_stop);
        mutex_unlock(&lock_bus);
        return -1;
    }

    // Set RX 1 byte
    prv_set_count(1);

    iowrite32(I2C_BIT_ENABLE | I2C_BIT_MASTER_MODE | I2C_BIT_START |
        I2C_BIT_STOP, i2c_ptr + I2C_REG_CON);

    if (wait_for_completion_timeout(&comp_read, msecs_to_jiffies(TIMEOUT_READ_WRITE)) == 0 ||
        wait_for_completion_timeout(&comp_stop, msecs_to_jiffies(TIMEOUT_READ_WRITE)) == 0) {
        printk("Timeout reached on i2c read operation\n");
        reinit_completion(&comp_read);
        reinit_completion(&comp_stop);
        mutex_unlock(&lock_bus);
        return -1;
    }
    reinit_completion(&comp_read);
    mutex_unlock(&lock_bus);
    return g_read;
}

/******************************************************************************
 * I2C private operations
******************************************************************************/

/// @brief Wait until teh bus is freed.
/// @return "0" if the bus is free. "-1" on timeout.
static int prv_wait_for_bus_busy(void) {
    u8 i = 0;
    while(prv_is_irq_triggered(I2C_IRQ_BB) || mutex_is_locked(&lock_bus)) {
        msleep(1);
        if (i++ == 100) {
            printk("Bus is busy\n");
            return -1;
        }
    }
    return 0;
}

static int prv_reset_i2c(void) {
    u8 i = 0;
    prv_clear_bit(i2c_ptr + I2C_REG_CON, I2C_BIT_ENABLE);
    iowrite32(I2C_BIT_RESET, i2c_ptr + I2C_REG_SYSC);
    iowrite32(I2C_BIT_ENABLE, i2c_ptr + I2C_REG_CON);   // If not enabled, the reset flag is not set
    while (prv_is_resetting()) {
        msleep(1);
        if (i++ == 10) {
            printk(KERN_ERR "Timeout on I2C reset\n");
            return -1;
        }
    };
    iowrite32(0x00, i2c_ptr + I2C_REG_CON);
    return 0;
}

