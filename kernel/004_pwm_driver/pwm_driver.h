#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>    // dev_t
#include <linux/fs.h>       // chrdev
#include <linux/cdev.h>     // cdev
#include <linux/uaccess.h>  // copy_from_user, copy_to_user
#include <linux/pwm.h>      // pwm_device

#define DEVICE_NAME "pwm_driver"
#define DEVICE_CLASS_NAME "my_module_class"
#define MINOR_NUMBER 0
#define NUMBER_OF_DEVICES 1
#define PWM_NUMBER 0    // Which PWM you want to use, might be "0", "1", "2", etc
#define PWM_NAME "my_pwm"
#define PWM_PERIOD 1000000000 // 1 second, in nanoseconds
#define PWM_DEFAULT_DUTY_CYCLE 0.5

// Module description
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nicolas G. Cotti");
MODULE_DESCRIPTION("A simple pwm driver, using pin P9.22");

/*NOTE: this driver is not operational. I don't have the right access to the PWM,
 further reading is required to map the "pwmchip" to the correct GPIO pin */