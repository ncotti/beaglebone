#include "cotti_i2c.h"

/******************************************************************************
 * Static variables
******************************************************************************/

// Pointers to memory mapped registers of the CPU
static void __iomem *i2c_ptr = NULL;
static void __iomem *clk_ptr = NULL;
static void __iomem *control_module_ptr = NULL;

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
static u8 g_write[2] = {0,0};   // Upper value is written first
static u16 g_len = 0;

static int g_irq;               // IRQ number, saved for deinitialization
static u32 g_clk_reg_offset;    // Memory offset from base clock address

/******************************************************************************
 * Static functions' prototypes
******************************************************************************/

static inline u32 prv_is_clk_disabled(void) {
    return ioread32(clk_ptr + g_clk_reg_offset) & (CLK_IDLEST);
}
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

static irqreturn_t prv_isr(int irq_number, void *dev_id);
static void prv_set_count(u16 count);
static void prv_wakeup(void);
static int prv_reset_i2c(void);
static int prv_wait_for_bus_busy(void);

/******************************************************************************
 * Functions
******************************************************************************/

/// @brief Initialize the I2C2 bus, with pins P9.21 and P9.22.
int cotti_i2c_init(struct platform_device *pdev) {
    struct device *i2c_dev = &pdev->dev;
    struct device_node *clk_node = NULL;
    struct device_node *pinmux_node = NULL;
    struct clk *fclk;
    u32 pins[4];
    u32 scll, sclh, psc, bit_rate, fclk_rate, int_clk_rate;
    int retval = -1;

    // Check that parent device exists (should be target-module@9c000)
    if (i2c_dev->parent == NULL) {
        printk(ERROR("I2C device doesn't have a parent.\n"));
        retval = -1;
        goto pdev_error;
    }

    // Check for device properties
    if (!device_property_present(i2c_dev, DT_PROPERTY_PINMUX_PHANDLE)       ||
        !device_property_present(i2c_dev, DT_PROPERTY_PINS)                 ||
        !device_property_present(i2c_dev->parent, DT_PROPERTY_CLK_PHANDLE)  ||
        !device_property_present(i2c_dev, DT_PROPERTY_CLK_REG_OFFSET)       ||
        !device_property_present(i2c_dev, DT_PROPERTY_CLK_FREQ)             ||
        !device_property_present(i2c_dev, DT_PROPERTY_INT_CLK_FREQ)) {
        printk(ERROR("Device properties for i2c device not found.\n"));
        retval = -1;
        goto pdev_error;
    }

    // Read device properties
    if ((retval = device_property_read_u32(i2c_dev, DT_PROPERTY_CLK_REG_OFFSET, &g_clk_reg_offset)) != 0 ||
        (retval = device_property_read_u32(i2c_dev, DT_PROPERTY_CLK_FREQ, &bit_rate))               != 0 ||
        (retval = device_property_read_u32(i2c_dev, DT_PROPERTY_INT_CLK_FREQ, &int_clk_rate))       != 0 ||
        (retval = device_property_read_u32_array(i2c_dev, DT_PROPERTY_PINS, pins, 4))!= 0) {
        printk(ERROR("Couldn't read properties.\n"));
        goto pdev_error;
    }

    // Get clock node
    if ((clk_node = of_parse_phandle((*i2c_dev->parent).of_node, DT_PROPERTY_CLK_PHANDLE, 0)) == NULL ) {
        printk(ERROR("Couldn't get clock node from phandle.\n"));
        retval = -1;
        goto pdev_error;
    }

    // Get pinmux node
    if ((pinmux_node = of_parse_phandle(pdev->dev.of_node, DT_PROPERTY_PINMUX_PHANDLE, 0)) == NULL ) {
        printk(ERROR("Couldn't get pinmux node from phandle.\n"));
        retval = -1;
        goto clk_node_error;
    }

    // Get clock frequency
    if ((fclk = clk_get(i2c_dev, "fck")) == NULL) {
        printk(ERROR("Couldn't get fclk.\n"));
        retval = -1;
        goto pinmux_node_error;
    }
    fclk_rate = clk_get_rate(fclk);
    clk_put(fclk);

    // Get io addresses
    if ((clk_ptr = of_iomap(clk_node, 0)) == NULL) {
        retval = -1;
        goto pinmux_node_error;
    } if ((control_module_ptr = of_iomap(pinmux_node, 0)) == NULL) {
        retval = -1;
        goto clk_ptr_error;
    } if ((i2c_ptr = devm_platform_ioremap_resource(pdev, 0)) == NULL) {
        retval = -1;
        goto control_module_ptr_error;
    }

    // Configure P9.21 and P9.22 pinmux as I2C
    iowrite32(pins[1], control_module_ptr + pins[0]);
    iowrite32(pins[3], control_module_ptr + pins[2]);

    prv_wakeup();

    if(prv_reset_i2c() != 0) {
        goto i2c_ptr_error;
    }

    // No idle, clocks always active, wakeup enable
    iowrite32(I2C_BIT_CLKACTIVITY | I2C_BIT_NOIDLE | I2C_BIT_WAKEUP,
        i2c_ptr + I2C_REG_SYSC);

    // Configure internal I2C clock and SCL.
    psc  = fclk_rate / int_clk_rate - 1;
    scll = int_clk_rate / (bit_rate * 2) - 7;
    sclh = int_clk_rate / (bit_rate * 2) - 5;
    iowrite8((u8) psc, i2c_ptr + I2C_REG_PSC);
    iowrite8((u8) scll, i2c_ptr + I2C_REG_SCLL);
    iowrite8((u8) sclh, i2c_ptr + I2C_REG_SCLH);

    // Enable I2C device
    iowrite32(I2C_BIT_ENABLE | I2C_BIT_MASTER_MODE | I2C_BIT_TX,
        i2c_ptr + I2C_REG_CON);

    // Configure Slave address
    iowrite32(I2C_SLAVE_ADDRESS, i2c_ptr + I2C_REG_SA);

    // Enable interrupts
    iowrite16(I2C_IRQ_XRDY | I2C_IRQ_RRDY | I2C_IRQ_NACK | I2C_IRQ_ARDY |
        I2C_IRQ_AL, i2c_ptr + I2C_REG_IRQENABLE_SET);

    if ((g_irq = platform_get_irq(pdev, 0)) < 0) {
        printk(ERROR("Couldn't get I2C IRQ number.\n"));
        retval = g_irq;
        goto i2c_ptr_error;
    }
    if ((retval = request_irq(g_irq, (irq_handler_t) prv_isr, IRQF_TRIGGER_RISING, pdev->name, NULL)) < 0) {
        printk(ERROR("Couldn't request I2C IRQ.\n"));
        goto i2c_ptr_error;
    }

    of_node_put(pinmux_node);
    of_node_put(clk_node);
    printk(INFO("I2C successfully configured.\n"));
    return 0;

    i2c_ptr_error: iounmap(i2c_ptr);
    control_module_ptr_error: iounmap(control_module_ptr);
    clk_ptr_error: iounmap(clk_ptr);
    pinmux_node_error: of_node_put(pinmux_node);
    clk_node_error: of_node_put(clk_node);
    pdev_error: i2c_ptr = NULL; clk_ptr = NULL; control_module_ptr = NULL;
    return retval;
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
    free_irq(g_irq, NULL);
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
        printk(WARNING("Timeout reached on i2c write.\n"));
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
        printk(WARNING("Timeout reached on i2c read, while writing the address.\n"));
        reinit_completion(&comp_stop);
        mutex_unlock(&lock_bus);
        return -1;
    }

    // Read one byte
    prv_set_count(1);
    iowrite32(I2C_BIT_ENABLE | I2C_BIT_MASTER_MODE | I2C_BIT_START |
        I2C_BIT_STOP, i2c_ptr + I2C_REG_CON);

    if (wait_for_completion_timeout(&comp_read, msecs_to_jiffies(TIMEOUT_READ_WRITE)) == 0 ||
        wait_for_completion_timeout(&comp_stop, msecs_to_jiffies(TIMEOUT_READ_WRITE)) == 0) {
        printk(WARNING("Timeout reached on i2c read operation.\n"));
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

/// @brief Handler for the I2C IRQ.
static irqreturn_t prv_isr(int irq_number, void *dev_id) {
    u16 irq;
    u16 enabled = ioread16(i2c_ptr + I2C_REG_IRQENABLE_SET);

    while(enabled & (irq = ioread16(i2c_ptr + I2C_REG_IRQSTATUS))) {
        if (irq & I2C_IRQ_AL) {
            printk(WARNING("IRQ I2C AL: Arbitration lost.\n"));
        } if (irq & I2C_IRQ_ARDY) {
            complete(&comp_stop);
        } if (irq & I2C_IRQ_NACK) {
            printk(WARNING("IRQ I2C NACK: Not acknowledge.\n"));
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

/// @brief Wakeup the I2C2 clock. The OS might put the I2C clock to sleep, so
///  re-enable the clock just in case.
static void prv_wakeup(void) {
    iowrite32(CLK_I2C2_ENABLE, clk_ptr + g_clk_reg_offset);
    while(prv_is_clk_disabled());
}

/// @brief Wait until teh bus is freed.
/// @return "0" if the bus is free. "-1" on timeout.
static int prv_wait_for_bus_busy(void) {
    u8 i = 0;
    while(prv_is_irq_triggered(I2C_IRQ_BB) || mutex_is_locked(&lock_bus)) {
        msleep(1);
        if (i++ == 100) {
            printk(WARNING("I2C bus is busy.\n"));
            return -1;
        }
    }
    return 0;
}

/// @brief Reset the I2C2 registers to power up state.
/// @return "0" on success, "-1" on error
static int prv_reset_i2c(void) {
    u8 i = 0;
    prv_clear_bit(i2c_ptr + I2C_REG_CON, I2C_BIT_ENABLE);
    iowrite32(I2C_BIT_RESET, i2c_ptr + I2C_REG_SYSC);
    iowrite32(I2C_BIT_ENABLE, i2c_ptr + I2C_REG_CON);   // If not enabled, the reset flag is not set
    while (prv_is_resetting()) {
        msleep(1);
        if (i++ == 10) {
            printk(ERROR("Timeout on I2C reset.\n"));
            return -1;
        }
    };
    iowrite32(0x00, i2c_ptr + I2C_REG_CON);
    return 0;
}

/// @brief Set the amount of bytes to read or write to the i2c bus, excluding
///  the slave address
static void prv_set_count(u16 count) {
    g_len = (count <= 2) ? count : 2;
    iowrite16(count, i2c_ptr + I2C_REG_CNT);
}
