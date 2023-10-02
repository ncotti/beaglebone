#include "cotti_i2c.h"

/******************************************************************************
 * Static functions' prototypes
******************************************************************************/

static inline void prv_start(void);
static inline void prv_stop(void);
static inline void prv_write(u8 value);
static inline u8 prv_read(void);
static inline void prv_set_dcount(u16 count);
static inline void prv_tx_mode(void);
static inline void prv_rx_mode(void);
static inline void prv_clear_all_irq(void);
static inline u8 prv_is_resetting(void);

static inline u32 prv_is_busy(void);
static inline u32 prv_transmit_data_ready(void);
static inline u32 prv_receive_data_ready(void);
static inline u32 prv_access_ready(void);

static void* i2c_ptr;

irqreturn_t cotti_i2c_isr(int irq, void *dev_id) {
    printk("On IRQ!!!\n");
    return IRQ_HANDLED;
}

/// @brief Initialize the I2C2 bus, with pins P9.21 and P9.22.
int cotti_i2c_init(void) {
    void * ptr;

    //I2C2 Clock Manager Peripheral (CM_PER) configuration
    ptr = ioremap(CLOCK_BASE_ADDRESS, CLOCK_SIZE);
    if(ptr == NULL) {
        printk(KERN_ERR "Couldn't configure Clock Manager Peripheral\n");
        return -1;
    }
    iowrite32(CLOCK_I2C2_ENABLE, ptr + CLOCK_I2C2_OFFSET);
    iounmap(ptr);

    //I2C2 configuration
    i2c_ptr = ioremap(I2C2_BASE_ADDRESS, I2C2_SIZE);
    if(i2c_ptr == NULL) {
        printk(KERN_ERR "Couldn't configure I2C2\n");
        return -1;
    }
    printk("I2C version: 0x%x, 0x%x\n", ioread32(i2c_ptr + I2C_REG_REVNB_HI), ioread32(i2c_ptr + I2C_REG_REVNB_LO));


    if (prv_is_resetting()) {
        printk("Shouldn't be resetting at the start!\n");
        return -1;
    }
    // Disable I2C
    iowrite32(ioread32(i2c_ptr + I2C_REG_CON) & ~I2C_BIT_ENABLE, i2c_ptr + I2C_REG_CON);

    // // Set reset
    // iowrite32(I2C_SYSC_VALUE, i2c_ptr + I2C_REG_SYSC);
    // iowrite32(I2C_BIT_ENABLE, i2c_ptr + I2C_REG_CON);   // If not enabled, the reset flag is not set
    // if (!prv_is_resetting()) {
    //     printk("Should be resetting!");
    //     return -1;
    // }
    // do {
    //     printk("Resetting...\n");
    //     msleep(100);
    // } while (prv_is_resetting());

    iowrite32(0x00, i2c_ptr + I2C_REG_CON); // Disable
    printk("Reset complete!\n");

    // Configure clock
    iowrite8(I2C_PSC_VALUE, i2c_ptr + I2C_REG_PSC);
    iowrite8(I2C_SCLL_VALUE, i2c_ptr + I2C_REG_SCLL);
    iowrite8(I2C_SCLH_VALUE, i2c_ptr + I2C_REG_SCLH);

    // Configure Slave address
    iowrite32(I2C_SLAVE_ADDRESS, i2c_ptr + I2C_REG_SA);

    // Enable interrupts

    iowrite16(I2C_IRQ_XRDY | I2C_IRQ_RRDY | I2C_IRQ_ARDY | I2C_IRQ_NACK |
        I2C_IRQ_AL, i2c_ptr + I2C_REG_IRQENABLE_SET);

    iowrite32(I2C_CON_VALUE, i2c_ptr + I2C_REG_CON);
    prv_stop();
    printk("I2C successfully configured!\n");
    return 0;
}

/// @brief Deinitialize the I2C2 bus.
void cotti_i2c_deinit(void) {
    iounmap(i2c_ptr);
}

/// @brief Write a value to the I2C bus.
/// @param value Value to be written
/// @param address Address of the device register
void cotti_i2c_write(u8 value, u8 address) {
    prv_tx_mode();
    prv_set_dcount(2);
    prv_write(address);
    prv_write(value);
    prv_start();
    msleep(100);
    prv_stop();
}

/// @brief Read a value from the I2C bus.
/// @param address Register address of the device
/// @return Value read.
u8 cotti_i2c_read(u8 address) {
    u8 read;
    u32 w;
    printk("Reading address: 0x%x\n", address);

    // poll the bus busy
    if (prv_is_busy()) {
        printk("Bus is busy. Can't read\n");
        prv_stop();
        return -1;
    }

    // Amount of bytes to transfer
    prv_set_dcount(1);

    // Clear fifo buffer
    iowrite32(ioread32(i2c_ptr + I2C_REG_BUF) | I2C_BIT_RXFIFO_CLR | I2C_BIT_TXFIFO_CLR, i2c_ptr + I2C_REG_BUF);

    w = I2C_BIT_ENABLE | I2C_BIT_MASTER_MODE | I2C_BIT_START | I2C_BIT_TX | I2C_BIT_STOP;
    iowrite32(w, i2c_ptr + I2C_REG_CON);

    // poll transmit data ready
    if (!prv_transmit_data_ready()) {
        printk("Transmission busy.\n");
        prv_stop();
        return -1;
    }
    prv_write(address);
    msleep(10);

    prv_set_dcount(1);

    w = I2C_BIT_ENABLE | I2C_BIT_MASTER_MODE | I2C_BIT_START | I2C_BIT_STOP;
    iowrite32(w, i2c_ptr + I2C_REG_CON);

    //prv_start();  // TODO this one makes writing the 10 values possible
    msleep(10);
    if (!prv_receive_data_ready()) {
        printk("Can't read.\n");
        prv_stop();
        return -1;
    }
    //prv_stop();
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
    u32 buffer;
    buffer = ioread32(i2c_ptr + I2C_REG_IRQSTATUS_RAW);
    buffer |= (1 << 4); // Write XRDY. Set transmission on going. Clear IRQ
    iowrite32(buffer, i2c_ptr + I2C_REG_IRQSTATUS_RAW);
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

/// @brief Sets the DCOUNT register. Controls the number of bytes of the data
///  payload (excluding the slave address). Triggers ARDY IRQ when finished.
/// @param count Amount of bytes to transfer, until ARDY IRQ.
static inline void prv_set_dcount(u16 count) {
    iowrite16(count, i2c_ptr + I2C_REG_CNT);
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