#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/types.h>
#include <linux/jiffies.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/interrupt.h>
#include <linux/pm_runtime.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/of_platform.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>
#include <linux/uaccess.h>
#include <linux/pinctrl/consumer.h>


//registros driver
#define CM_PER 0x44E00000
#define CM_PER_SIZE 1024
#define CM_PER_I2C2_CLK_OFFSET 0x44
#define I2C2 0x4819C000
#define I2C2_MODULE_SIZE 4096
#define I2C2_REV 0x0

static int funcion_probe(struct platform_device * pdev);
static int funcion_remove(struct platform_device * pdev);
static irqreturn_t Mi_handler_interrupt(int irq);


static struct{
    int irq;
    void *cm_per;
    void *i2c2;
    void *pointer3;
}Host_Controller;

static const struct of_device_id Mis_Dispositivos_Compatibles [] = {
    { .compatible = "Mi-Apellido,Mi-Host-1234Version1"},
    {},
};

MODULE_DEVICE_TABLE(of,Mis_Dispositivos_Compatibles);

static struct platform_driver Mi_I2C_Host_Controller = {
        .probe = funcion_probe,
        .remove = funcion_remove,
        .driver = {
            .name = "Host_Controller",
            .of_match_table = of_match_ptr(Mis_Dispositivos_Compatibles),
        },
};

static int funcion_probe(struct platform_device * pdev)
{
        static int Request_result;

        printk(KERN_ALERT "Ingreso a PROBE\n");

        Host_Controller.irq = platform_get_irq(pdev,0);
        Request_result = request_irq ( Host_Controller.irq,(irq_handler_t)Mi_handler_interrupt,IRQF_TRIGGER_RISING,pdev->name,NULL );

        if(Request_result < 0){
            printk(KERN_ALERT "Mi_IRQ ERROR \n");
            return -1;
        }

        //I2C clock domain configuration
        Host_Controller.cm_per = ioremap (CM_PER,CM_PER_SIZE);
        if(Host_Controller.cm_per == NULL)
		{
            printk(KERN_ALERT "CM PER ERROR \n");
            return -1;
        }
        iowrite32 (0x02, Host_Controller.cm_per + CM_PER_I2C2_CLK_OFFSET);
        printk(KERN_ALERT "Entro a PROBE luego de escribir el CM PER \n");

/*
        //i2c2
        Host_Controller.i2c2 = ioremap (I2C2,I2C2_MODULE_SIZE);
        if(Host_Controller.i2c2 == NULL){
            printk(KERN_ALERT "I2C2 PER ERROR \n");
            return -1;
        }
        printk(KERN_ALERT "Llego a escribir I2C2 - Revision I2C2 %d\n",ioread32(Host_Controller.i2c2 + I2C2_REV));
*/
        return 0;
}


static irqreturn_t Mi_handler_interrupt(int irq){
    return IRQ_HANDLED;
}

static int funcion_remove(struct platform_device * pdev){
	printk(KERN_ALERT "\n  Entro a REMOVE \n");
	//free_irq(Host_Controller.irq,NULL);
	iounmap(Host_Controller.cm_per);
	iounmap(Host_Controller.i2c2);
	return 0;
}


static int Host_Device_Initialization(void) {
  platform_driver_register(&Mi_I2C_Host_Controller);
  return 0;
}

static void Host_Device_Exit(void) {
  platform_driver_unregister(&Mi_I2C_Host_Controller);
}

//Macros de instalacion y desinstalacion
module_init(Host_Device_Initialization);
module_exit(Host_Device_Exit);
