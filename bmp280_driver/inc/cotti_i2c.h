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

int cotti_i2c_init(void);
void cotti_i2c_deinit(void);
void cotti_i2c_write(u8 value, u8 address);
u8 cotti_i2c_read(u8 address);
irqreturn_t cotti_i2c_isr(int irq, void *devid);
void cotti_i2c_test_irq(void);
int cotti_i2c_reset(void);
void cotti_i2c_wakeup(void);

// Clock Manager Peripheral (CM_PER)
#define CLOCK_BASE_ADDRESS 0x44E00000
#define CLOCK_SIZE 0x400
#define CLOCK_REG_I2C2  0x44
#define CLOCK_I2C2_ENABLE 0x02

// I2C2
#define I2C2_BASE_ADDRESS   0x4819C000
#define I2C2_SIZE           0x1000
#define I2C_SLAVE_ADDRESS   0x76

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

#define I2C_BIT_ENABLE          (1 << 15)
#define I2C_BIT_MASTER_MODE     (1 << 10)
#define I2C_BIT_TX              (1 << 9)
#define I2C_BIT_RX              (1 << 9)
#define I2C_BIT_STOP            (1 << 1)
#define I2C_BIT_START           (1 << 0)

// #define I2C_BIT_AUTOIDLE        (1 << 0)
// #define I2C_BIT_RESET           (1 << 1)
// #define I2C_BIT_WAKEUP          (1 << 2)
// #define I2C_BIT_IDLEMODE        (2 << 3)
// #define I2C_BIT_CLKACTIVITY     (2 << 8)

#define I2C_BIT_AUTOIDLE        (1 << 0)
#define I2C_BIT_RESET           (1 << 1)
#define I2C_BIT_WAKEUP          (1 << 2)
#define I2C_BIT_IDLEMODE        (1 << 3)
#define I2C_BIT_CLKACTIVITY     (3 << 8)

// I2C_CLOCK should be around 12MHZ. If the system clock is at 48MHz (PER_CLKOUTM2 / 4), "fsck",
// and the clock is obtained as:
//      I2C_clock = System_clock / (PSC + 1)
// Then, the PSC should be 3. ==> I2C_clock = 48 MHz / 4 = 12 MHz
#define I2C_PSC_VALUE           3

// The serial clock (SCL) needs to have 100kbps ==> t_low = t_high = 5useg
// According to SCLL:
//  t_low = (SCLL + 7) / I2C_clock  ==> SCLL = t_low * I2C_clock - 7  ==> SCLL = 53
// According to SCLH:
//  t_high = (SCLH + 5) / I2C_clock ==> SCLH = t_high * I2C_clock - 5 ==> SCLH = 55
// However, it was found empirically that if SCLL = SCLH = 53, then:
//  t_low = 5.25us; t_high = 4.75us; and you have 100kbps exactly.
#define I2C_SCLL_VALUE          53
#define I2C_SCLH_VALUE          53

#define I2C_IRQ_XRDY    (1 << 4)
#define I2C_IRQ_RRDY    (1 << 3)
#define I2C_IRQ_ARDY    (1 << 2)
#define I2C_IRQ_NACK    (1 << 1)
#define I2C_IRQ_AL      (1 << 0)

#define I2C_BIT_RXFIFO_CLR (1 << 14)
#define I2C_BIT_TXFIFO_CLR (1 << 6)

#endif // COTTI_I2C
