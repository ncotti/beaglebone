#include "cotti_i2c.h"

/******************************************************************************
 * Static functions' prototypes
******************************************************************************/

static inline void prv_set_bit(void* addr, u32 bit) {
    iowrite32(ioread32(addr) | bit, addr);
}
static inline void prv_clear_bit(void* addr, u32 bit) {
    iowrite32(ioread32(addr) & ~bit, addr);
}

static inline void prv_start(void);
static inline void prv_stop(void);
static inline void prv_write(u8 value);
static inline u8 prv_read(void);
static inline void prv_tx_mode(void);
static inline void prv_rx_mode(void);
static inline void prv_clear_all_irq(void);
static inline u8 prv_is_resetting(void);

static inline u32 prv_is_busy(void);
static inline u32 prv_transmit_data_ready(void);
static inline u32 prv_receive_data_ready(void);
static inline u32 prv_access_ready(void);

static void* i2c_ptr = NULL;
static void* clk_ptr = NULL;

irqreturn_t cotti_i2c_isr(int irq, void *dev_id) {
    printk("On IRQ!!!\n");
    return IRQ_HANDLED;
}

/// @brief Initialize the I2C2 bus, with pins P9.21 and P9.22.
int cotti_i2c_init(void) {
    u8 i = 0;
    //I2C2 Clock Manager Peripheral (CM_PER) configuration
    if((clk_ptr = ioremap(CLOCK_BASE_ADDRESS, CLOCK_SIZE)) == NULL) {
        printk(KERN_ERR "Couldn't configure Clock Manager Peripheral\n");
        return -1;
    }
    iowrite32(CLOCK_I2C2_ENABLE, clk_ptr + CLOCK_REG_I2C2);

    //I2C2 configuration
    if((i2c_ptr = ioremap(I2C2_BASE_ADDRESS, I2C2_SIZE)) == NULL) {
        printk(KERN_ERR "Couldn't configure I2C2\n");
        goto clock_error;
    }
    printk("I2C version: 0x%x, 0x%x\n",
        ioread32(i2c_ptr + I2C_REG_REVNB_HI),
        ioread32(i2c_ptr + I2C_REG_REVNB_LO));

    // Set reset
    prv_clear_bit(i2c_ptr + I2C_REG_CON, I2C_BIT_ENABLE);
    iowrite32(I2C_BIT_RESET, i2c_ptr + I2C_REG_SYSC);
    iowrite32(I2C_BIT_ENABLE, i2c_ptr + I2C_REG_CON);   // If not enabled, the reset flag is not set
    i = 0;
    while (prv_is_resetting()) {
        msleep(1);
        if (i++ == 10) {
            printk(KERN_ERR "Timeout on I2C reset\n");
            goto i2c_error;
        }
    };
    iowrite32(0x00, i2c_ptr + I2C_REG_CON);

    // For simplicity, no idle and clocks always on TODO check
    iowrite32(I2C_BIT_CLKACTIVITY | I2C_BIT_IDLEMODE, i2c_ptr + I2C_REG_SYSC);

    // Configure clock
    iowrite8(I2C_PSC_VALUE, i2c_ptr + I2C_REG_PSC);
    iowrite8(I2C_SCLL_VALUE, i2c_ptr + I2C_REG_SCLL);
    iowrite8(I2C_SCLH_VALUE, i2c_ptr + I2C_REG_SCLH);

    // Enable I2C device
    iowrite32(I2C_BIT_ENABLE | I2C_BIT_MASTER_MODE | I2C_BIT_TX, i2c_ptr + I2C_REG_CON);

    // Configure Slave address
    iowrite32(I2C_SLAVE_ADDRESS, i2c_ptr + I2C_REG_SA);

    // Enable interrupts
    iowrite16(I2C_IRQ_XRDY | I2C_IRQ_RRDY | I2C_IRQ_ARDY | I2C_IRQ_NACK |
        I2C_IRQ_AL, i2c_ptr + I2C_REG_IRQENABLE_SET);

    printk("I2C successfully configured!\n");
    return 0;

    i2c_error: iounmap(i2c_ptr);
    clock_error: iounmap(clk_ptr);
    i2c_ptr = NULL;
    clk_ptr = NULL;
    return -1;
}

/// @brief Deinitialize the I2C2 bus.
void cotti_i2c_deinit(void) {
    if (clk_ptr != NULL) {
        iounmap(clk_ptr);
    }
    if (i2c_ptr != NULL) {
        iounmap(i2c_ptr);
    }
}

/// @brief Write a value to the I2C bus.
/// @param value Value to be written
/// @param address Address of the device register
void cotti_i2c_write(u8 value, u8 address) {
    u8 i;
    printk("Writing address: 0x%x with value 0x%x", address, value);
    msleep(1);

    // poll the bus busy
    if (prv_is_busy()) {
        printk("Bus is busy. Can't read\n");
        prv_stop();
        return;
    }

    // Amount of bytes to transfer
    iowrite32(2, i2c_ptr + I2C_REG_CNT);

    // Clear fifo buffer
    prv_set_bit(i2c_ptr + I2C_REG_BUF, I2C_BIT_RXFIFO_CLR | I2C_BIT_TXFIFO_CLR);

    iowrite32(I2C_BIT_ENABLE | I2C_BIT_MASTER_MODE | I2C_BIT_START |
        I2C_BIT_TX | I2C_BIT_STOP, i2c_ptr + I2C_REG_CON);

    // poll transmit data ready
    if (!prv_transmit_data_ready()) {
        printk("Transmission busy.\n");
        prv_stop();
        return;
    }
    prv_set_bit(i2c_ptr + I2C_REG_IRQSTATUS, I2C_IRQ_XRDY);
    prv_write(address);

    i = 0;
    while (!prv_transmit_data_ready()) {
        printk("Waiting for data ready\n");
        msleep(1);
        if (i++ == 3) {
            printk("ERROR on second byte transfer\n");
            return;
        }
    }
    prv_set_bit(i2c_ptr + I2C_REG_IRQSTATUS, I2C_IRQ_XRDY);
    prv_write(value);
}

/// @brief Read a value from the I2C bus.
/// @param address Register address of the device
/// @return Value read.
u8 cotti_i2c_read(u8 address) {
    u8 read;
    u8 i;
    printk("Reading address: 0x%x\n", address);

    // poll the bus busy
    i = 0;
    while(prv_is_busy()) {
        msleep(1);
        if (i++ == 3) {
            printk("Bus is busy. Can't read\n");
            return -1;
        }
    }

    // Amount of bytes to transfer
    iowrite32(1, i2c_ptr + I2C_REG_CNT);

    // Clear fifo buffer
    prv_set_bit(i2c_ptr + I2C_REG_BUF, I2C_BIT_RXFIFO_CLR | I2C_BIT_TXFIFO_CLR);

    // Enable TX
    iowrite32(I2C_BIT_ENABLE | I2C_BIT_MASTER_MODE | I2C_BIT_START |
        I2C_BIT_TX | I2C_BIT_STOP, i2c_ptr + I2C_REG_CON);

    // Wait until transmit data
    i = 0;
    while(!prv_transmit_data_ready()) {
        msleep(1);
        if (i++ == 3) {
            printk("Transmission busy.\n");
            return -1;
        }
    }
    prv_set_bit(i2c_ptr + I2C_REG_IRQSTATUS, I2C_IRQ_XRDY);
    prv_write(address);
    msleep(10);

    // Set RX 1 byte
    iowrite32(1, i2c_ptr + I2C_REG_CNT);
    iowrite32(I2C_BIT_ENABLE | I2C_BIT_MASTER_MODE | I2C_BIT_START |
        I2C_BIT_STOP, i2c_ptr + I2C_REG_CON);

    i = 0;
    while (!prv_receive_data_ready()) {
        msleep(1);
        if (i++ == 3) {
            printk("Can't read.\n");
            return -1;
        }
    }
    read = prv_read();
    return read;
}

/******************************************************************************
 * I2C private operations
******************************************************************************/

/// @brief Set I2C start condition:
///  1. SCLK=1; SDA=1
///  2. SCLK=1; SDA=0
///  3. SCLK=0; SDA=0
/// @param count Amount of bytes that will be transmitted or received until
///  the next prv_stop()
static inline void prv_start(void) {
    u32 buffer;
    buffer = ioread32(i2c_ptr + I2C_REG_CON);
    buffer |= I2C_BIT_START | I2C_BIT_STOP;
    iowrite32(buffer, i2c_ptr + I2C_REG_CON);
}

/// @brief Set I2C stop condition:
///  1. SCLK=0; SDA=0
///  2. SCLK=1; SDA=0
///  3. SCLK=1; SDA=1
static inline void prv_stop(void) {
    u32 buffer;
    buffer = ioread32(i2c_ptr + I2C_REG_CON);
    buffer |= I2C_BIT_STOP;
    iowrite32(buffer, i2c_ptr + I2C_REG_CON);
}

/// @brief Writes a byte to the I2C bus.
/// @param value Value to be written to the I2C bus.
static inline void prv_write(u8 value) {
    iowrite8(value, i2c_ptr + I2C_REG_DATA);
}

/// @brief Return the byte read from the I2C bus.
static inline u8 prv_read(void) {
    u32 buffer;
    buffer = ioread32(i2c_ptr + I2C_REG_IRQSTATUS_RAW);
    buffer |= (1 << 3); // Write RRDY. Data read. Clear IRQ
    iowrite32(buffer, i2c_ptr + I2C_REG_IRQSTATUS_RAW);
    return ioread8(i2c_ptr + I2C_REG_DATA);
}

static inline void prv_tx_mode(void) {
    u32 buffer;
    buffer = ioread32(i2c_ptr + I2C_REG_CON);
    buffer |= I2C_BIT_TX;
    iowrite32(buffer, i2c_ptr + I2C_REG_CON);
}

static inline void prv_rx_mode(void) {
    u32 buffer;
    buffer = ioread32(i2c_ptr + I2C_REG_CON);
    buffer &= ~I2C_BIT_RX;
    iowrite32(buffer, i2c_ptr + I2C_REG_CON);
}

static inline void prv_clear_all_irq(void) {
    u32 buffer;
    buffer = ioread32(i2c_ptr + I2C_REG_IRQSTATUS_RAW);
    iowrite32(buffer, i2c_ptr + I2C_REG_IRQSTATUS_RAW);
}

/// @brief BB IRQ (bus busy)
static inline u32 prv_is_busy(void) {
    return ioread32(i2c_ptr + I2C_REG_IRQSTATUS_RAW) & (1 << 12);
}

/// @brief XRDY (Transmit Data Ready IRQ)
static inline u32 prv_transmit_data_ready(void) {
    return ioread32(i2c_ptr + I2C_REG_IRQSTATUS_RAW) & (1 << 4);
}

/// @brief RRDY (Receive Data ready IRQ)
static inline u32 prv_receive_data_ready(void) {
    return ioread32(i2c_ptr + I2C_REG_IRQSTATUS_RAW) & (1 << 3);
}

static inline u32 prv_access_ready(void) {
    return ioread32(i2c_ptr + I2C_REG_IRQSTATUS_RAW) & (1 << 2);
}

static inline u8 prv_is_resetting(void) {
    return ! (ioread8(i2c_ptr + I2C_REG_SYSS) & 1);
}