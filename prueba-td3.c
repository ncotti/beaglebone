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


static struct {
  struct class *devclass;
  struct device *dev;
  struct cdev chardev;
  dev_t devtype;

  unsigned int freq;
  void __iomem *base;
  int irq;
} state;

static irqreturn_t driver_isr(int irq, void *devid) {
  return IRQ_HANDLED;
}

static int driver_open(struct inode *inode, struct file *file) {
  return 0;
}

static ssize_t driver_read(struct file *file, char __user *buffer,
                           size_t size, loff_t *offset) {

  return 0;
}

static int driver_release(struct inode *inode,
                          struct file *file) {
  return 0;
}

static const struct file_operations driver_file_op = {
  .owner = THIS_MODULE,
  .open = driver_open,
  .read = driver_read,
  .release = driver_release
};

#define BASE_MINOR 0
#define MINOR_COUNT 1
#define DEVICE_PARENT NULL
#define DEVICE_NAME "prueba-td3"
#define DEVICE_CLASS_NAME "prueba-td3"

static int driver_probe(struct platform_device *pdev) {
  int status = 0;
  struct resource *mem = NULL;

  dev_info(&pdev->dev, "Initializing driver controller\n");

  if ((state.irq = platform_get_irq(pdev, 0)) < 0) {
    return state.irq;
  }

  mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
  state.base = devm_ioremap_resource(&pdev->dev, mem);
  if (IS_ERR(state.base)) {
    return PTR_ERR(state.base);
  }

  if ((status = of_property_read_u32(pdev->dev.of_node,
                                     "clock-frequency",
                                     &state.freq)) != 0) {
    state.freq = 100000; // default to 100000 Hz
  }

  if ((status = devm_request_irq(&pdev->dev, state.irq, driver_isr,
                                 IRQF_NO_SUSPEND, pdev->name, NULL)) != 0) {
    return status;
  }

  // Lo que es especifico al bus y al sensor que esta en el bus.

  if ((status = alloc_chrdev_region(
          &state.devtype,
          BASE_MINOR,
          MINOR_COUNT,
          DEVICE_NAME)) != 0) {
    dev_err(&pdev->dev, "failed to allocate char device region\n");
    return status;
  }

  state.devclass = class_create(
      THIS_MODULE, DEVICE_CLASS_NAME);
  if (IS_ERR(state.devclass)) {
    dev_err(&pdev->dev, "failed to create char device class\n");
    unregister_chrdev_region(
        state.devtype, MINOR_COUNT);
    return PTR_ERR(state.devclass);
  }

  state.dev = device_create(
      state.devclass, DEVICE_PARENT,
      state.devtype, NULL, DEVICE_NAME);
  if (IS_ERR(state.dev)) {
    dev_err(&pdev->dev, "failed to create char device [%d]\n", status);
    class_destroy(state.devclass);
    unregister_chrdev_region(
        state.devtype, MINOR_COUNT);
    return PTR_ERR(state.dev);
  }

  cdev_init(&state.chardev, &driver_file_op);

  if ((status = cdev_add(&state.chardev, state.devtype,
                         MINOR_COUNT)) != 0) {
    dev_err(&pdev->dev, "failed to add char device\n");
    device_destroy(state.devclass, state.devtype);
    class_destroy(state.devclass);
    unregister_chrdev_region(
        state.devtype, MINOR_COUNT);
    return status;
  }

  dev_info(&pdev->dev, "Driver initialized\n");

  return 0;
}

static int driver_remove(struct platform_device *pdev) {
  dev_info(&pdev->dev, "Removing driver\n");
  cdev_del(&state.chardev);
  device_destroy(state.devclass, state.devtype);
  class_destroy(state.devclass);
  unregister_chrdev_region(
      state.devtype, MINOR_COUNT);
  dev_info(&pdev->dev, "Driver removed\n");
  return 0;
}

static const struct of_device_id driver_of_match[] = {
  { .compatible = "Mi-Apellido,Mi-Host-1234Version1" },
  { },
};

MODULE_DEVICE_TABLE(of, driver_of_match);

static struct platform_driver driver = {
  .probe = driver_probe,
  .remove = driver_remove,
  .driver = {
    .name = "prueba-td3",
    .of_match_table = of_match_ptr(driver_of_match),
  },
};

static int __init init_driver(void) {
  return platform_driver_register(&driver);
}
module_init(init_driver);

static void __exit exit_driver(void) {
  platform_driver_unregister(&driver);
}
module_exit(exit_driver);

