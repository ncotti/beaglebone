#ifndef COTTI_I2C
#define COTTI_I2C

#include <linux/init.h>
#include <linux/module.h>
#include <asm/io.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/completion.h>
#include <linux/jiffies.h>
#include <linux/mutex.h>
#include <linux/mod_devicetable.h>
#include <linux/property.h>
#include <linux/platform_device.h>
#include <linux/of_device.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>
#include <linux/of_clk.h>
#include <linux/clk.h>
#include "log.h"

int cotti_i2c_init(struct platform_device *pdev);
void cotti_i2c_deinit(void);
int cotti_i2c_write(u8 value, u8 reg_address, u8 slave_address);
int cotti_i2c_read(u8 address, u8 slave_address);

#define TIMEOUT_READ_WRITE 100  // msec

#define DT_PROPERTY_PINMUX_PHANDLE  "pinmux"
#define DT_PROPERTY_PINS            "pins"
#define DT_PROPERTY_CLK_PHANDLE     "clocks"
#define DT_PROPERTY_CLK_FREQ        "clock-frequency"
#define DT_PROPERTY_INT_CLK_FREQ    "int-clock-frequency"
#define DT_PROPERTY_CLK_REG_OFFSET  "clock-reg-offset"
#define DT_PROPERTY_BIT_RATE        "bit-rate"

// Clock Manager Peripheral (CM_PER)
#define CLK_I2C2_ENABLE         0x02
#define CLK_IDLEST              (0x3 << 16)

// Registers' offset address
#define I2C_REG_REVNB_LO        0x00
#define I2C_REG_REVNB_HI        0x04
#define I2C_REG_SYSC            0x10
#define I2C_REG_IRQSTATUS_RAW   0x24
#define I2C_REG_IRQSTATUS       0x28
#define I2C_REG_IRQENABLE_SET   0x2C
#define I2C_REG_IRQENABLE_CLR   0x30
#define I2C_REG_WE              0x34
#define I2C_REG_SYSS            0x90
#define I2C_REG_BUF             0x94
#define I2C_REG_CNT             0x98
#define I2C_REG_DATA            0x9C
#define I2C_REG_CON             0xA4
#define I2C_REG_SA              0xAC
#define I2C_REG_PSC             0xB0
#define I2C_REG_SCLL            0xB4
#define I2C_REG_SCLH            0xB8

// CON
#define I2C_BIT_ENABLE          (1 << 15)
#define I2C_BIT_MASTER_MODE     (1 << 10)
#define I2C_BIT_TX              (1 << 9)
#define I2C_BIT_RX              (1 << 9)
#define I2C_BIT_STOP            (1 << 1)
#define I2C_BIT_START           (1 << 0)

// SYSC
#define I2C_BIT_AUTOIDLE        (1 << 0)
#define I2C_BIT_RESET           (1 << 1)
#define I2C_BIT_WAKEUP          (1 << 2)    // Enable own wakeup
#define I2C_BIT_NOIDLE          (1 << 3)    // No idle
#define I2C_BIT_CLKACTIVITY     (3 << 8)    // Both clocks active

// IRQSTATUS
#define I2C_IRQ_BB              (1 << 12)
#define I2C_IRQ_XRDY            (1 << 4)
#define I2C_IRQ_RRDY            (1 << 3)
#define I2C_IRQ_ARDY            (1 << 2)
#define I2C_IRQ_NACK            (1 << 1)
#define I2C_IRQ_AL              (1 << 0)

// BUF
#define I2C_BIT_RXFIFO_CLR      (1 << 14)
#define I2C_BIT_TXFIFO_CLR      (1 << 6)

#endif // COTTI_I2C
