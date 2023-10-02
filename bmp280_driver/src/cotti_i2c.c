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
static inline u8 prv_is_resetting(void);

static inline u32 prv_is_busy(void);
static inline u32 prv_transmit_data_ready(void);
static inline u32 prv_receive_data_ready(void);
static inline u32 prv_access_ready(void);

static void* i2c_ptr = NULL;
static void* clk_ptr = NULL;

DECLARE_COMPLETION(comp_read);
DECLARE_COMPLETION(comp_write);

static u8 g_read = 0;
static u8 g_write = 0;

void cotti_i2c_test_irq(void) {
    printk("TEST1: reading ID\n");
    cotti_i2c_read(0xD0);
    printk("Success\n");

    printk("TEST2: reading ID\n");
    cotti_i2c_read(0xD0);
    printk("Success\n");

    printk("TEST3: reading ID\n");
    cotti_i2c_read(0xD0);
    printk("Success\n");
}

irqreturn_t cotti_i2c_isr(int irq_number, void *dev_id) {
    u16 irq;
    u16 enabled = ioread16(i2c_ptr + I2C_REG_IRQENABLE_SET);

    while(enabled & (irq = ioread16(i2c_ptr + I2C_REG_IRQSTATUS))) {
        //printk("IRQ: 0x%x\n", irq);
        if (irq & I2C_IRQ_AL) {
            printk("AL\n");
        } if (irq & I2C_IRQ_ARDY) {
            printk("ARDY\n");
        } if (irq & I2C_IRQ_NACK) {
            printk("NACK\n");
        } if (irq & I2C_IRQ_RRDY) {
            printk("RRDY\n");
            g_read = prv_read();
            complete(&comp_read);
        } if (irq & I2C_IRQ_XRDY) {
            printk("XRDY\n");
            prv_write(g_write);
            complete(&comp_write);
        }
        iowrite16(irq, i2c_ptr + I2C_REG_IRQSTATUS);
        //printk("IRQ after handled: 0x%x\n", ioread16(i2c_ptr + I2C_REG_IRQSTATUS));
        //printk("IRQ RAW after handler: 0x%x\n", ioread16(i2c_ptr + I2C_REG_IRQSTATUS_RAW));
    }
    //printk("Exiting IRQ\n");
    return IRQ_HANDLED;
}

int cotti_i2c_reset(void) {
    u8 i = 0;
    // Set reset
    prv_clear_bit(i2c_ptr + I2C_REG_CON, I2C_BIT_ENABLE);
    iowrite32(I2C_BIT_RESET, i2c_ptr + I2C_REG_SYSC);
    iowrite32(I2C_BIT_ENABLE, i2c_ptr + I2C_REG_CON);   // If not enabled, the reset flag is not set
    i = 0;
    while (prv_is_resetting()) {
        msleep(1);
        if (i++ == 10) {
            printk(KERN_ERR "Timeout on I2C reset\n");
            return -1;
        }
    };
    iowrite32(0x00, i2c_ptr + I2C_REG_CON);

    // For simplicity, no idle and clocks always on TODO check
    //iowrite32(I2C_BIT_CLKACTIVITY | I2C_BIT_IDLEMODE | I2C_BIT_WAKEUP | I2C_BIT_AUTOIDLE, i2c_ptr + I2C_REG_SYSC);
    iowrite32(I2C_BIT_CLKACTIVITY | I2C_BIT_IDLEMODE, i2c_ptr + I2C_REG_SYSC);

    // Configure clock
    iowrite8(I2C_PSC_VALUE, i2c_ptr + I2C_REG_PSC);
    iowrite8(I2C_SCLL_VALUE, i2c_ptr + I2C_REG_SCLL);
    iowrite8(I2C_SCLH_VALUE, i2c_ptr + I2C_REG_SCLH);

    // Enable I2C device
    iowrite32(I2C_BIT_ENABLE | I2C_BIT_MASTER_MODE | I2C_BIT_TX, i2c_ptr + I2C_REG_CON);

    // Configure Slave address
    iowrite32(I2C_SLAVE_ADDRESS, i2c_ptr + I2C_REG_SA);

    // Enable all WE interrupts to stop freezing in IRQ
    iowrite16(0x7fff, i2c_ptr + I2C_REG_WE);

    // Enable interrupts
    iowrite16(I2C_IRQ_XRDY | I2C_IRQ_RRDY | I2C_IRQ_NACK |
        I2C_IRQ_AL, i2c_ptr + I2C_REG_IRQENABLE_SET);

    return 0;
}

/// @brief Initialize the I2C2 bus, with pins P9.21 and P9.22.
int cotti_i2c_init(void) {

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

    if (cotti_i2c_reset() != 0) {
        goto i2c_error;
    }

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
    i = 0;
    while(prv_is_busy()) {
        msleep(1);
        if (i++ == 3) {
            printk("Bus is busy. Can't write\n");
            return;
        }
    }

    // Amount of bytes to transfer
    iowrite32(2, i2c_ptr + I2C_REG_CNT);

    // Clear fifo buffer
    prv_set_bit(i2c_ptr + I2C_REG_BUF, I2C_BIT_RXFIFO_CLR | I2C_BIT_TXFIFO_CLR);

    g_write = address;
    iowrite32(I2C_BIT_ENABLE | I2C_BIT_MASTER_MODE | I2C_BIT_START |
        I2C_BIT_TX | I2C_BIT_STOP, i2c_ptr + I2C_REG_CON);

    // poll transmit data ready
    if (wait_for_completion_timeout(&comp_write, msecs_to_jiffies(100)) == 0){
        printk("Timeout reached!\n");
        return;
    }
    reinit_completion(&comp_write);
    g_write = value;
    if (wait_for_completion_timeout(&comp_write, msecs_to_jiffies(100)) == 0){
        printk("Timeout reached!\n");
        return;
    }
    reinit_completion(&comp_write);
}

/// @brief Read a value from the I2C bus.
/// @param address Register address of the device
/// @return Value read.
u8 cotti_i2c_read(u8 address) {
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

    g_write = address;
    // Enable TX
    iowrite32(I2C_BIT_ENABLE | I2C_BIT_MASTER_MODE | I2C_BIT_START |
        I2C_BIT_TX | I2C_BIT_STOP, i2c_ptr + I2C_REG_CON);

    // Wait until transmit data
    if (wait_for_completion_timeout(&comp_write, msecs_to_jiffies(100)) == 0){
        printk("Timeout reached!\n");
        return -1;
    }
    reinit_completion(&comp_write);
    msleep(10);

    // Set RX 1 byte
    iowrite32(1, i2c_ptr + I2C_REG_CNT);
    iowrite32(I2C_BIT_ENABLE | I2C_BIT_MASTER_MODE | I2C_BIT_START |
        I2C_BIT_STOP, i2c_ptr + I2C_REG_CON);

    if (wait_for_completion_timeout(&comp_read, msecs_to_jiffies(100)) == 0){
        printk("Timeout reached!\n");
        return -1;
    }
    reinit_completion(&comp_read);
    return g_read;
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


/// @brief BB IRQ (bus busy)
static inline u32 prv_is_busy(void) {
    return ioread32(i2c_ptr + I2C_REG_IRQSTATUS) & (1 << 12);
}

/// @brief XRDY (Transmit Data Ready IRQ)
static inline u32 prv_transmit_data_ready(void) {
    return ioread32(i2c_ptr + I2C_REG_IRQSTATUS) & (1 << 4);
}

/// @brief RRDY (Receive Data ready IRQ)
static inline u32 prv_receive_data_ready(void) {
    return ioread32(i2c_ptr + I2C_REG_IRQSTATUS) & (1 << 3);
}

static inline u32 prv_access_ready(void) {
    return ioread32(i2c_ptr + I2C_REG_IRQSTATUS) & (1 << 2);
}

static inline u8 prv_is_resetting(void) {
    return ! (ioread8(i2c_ptr + I2C_REG_SYSS) & 1);
}

/*
/*
 * Prepare controller for a transaction and call omap_i2c_xfer_msg
 * to do the work during IRQ processing.
static int
omap_i2c_xfer(struct i2c_adapter *adap, struct i2c_msg msgs[], int num)
{
	struct omap_i2c_dev *dev = i2c_get_adapdata(adap);
	int i;
	int r;

	pm_runtime_get_sync(dev->dev);

	r = omap_i2c_wait_for_bb(dev);
	if (r < 0)
		goto out;

	if (dev->set_mpu_wkup_lat != NULL)
		dev->set_mpu_wkup_lat(dev->dev, dev->latency);

	for (i = 0; i < num; i++) {
		r = omap_i2c_xfer_msg(adap, &msgs[i], (i == (num - 1)));
		if (r != 0)
			break;
	}

	if (dev->set_mpu_wkup_lat != NULL)
		dev->set_mpu_wkup_lat(dev->dev, -1);

	if (r == 0)
		r = num;

	omap_i2c_wait_for_bb(dev);
out:
	pm_runtime_put(dev->dev);
	return r;
}
*/