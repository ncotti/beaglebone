#ifndef SEND_SIGNAL_H
#define SEND_SIGNAL_H

#include <linux/module.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/poll.h>
#include <linux/wait.h>
#include <linux/jiffies.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/kthread.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nicolas G. Cotti");
MODULE_DESCRIPTION(
    "Creates a device. Writing to that device the value will wake up two threads. "
    "The threads then will check for a condition to be true, and, if that is "
    "the case, will finish execution."
);

#define DEVICE_NAME "waitqueue_driver"
#define DEVICE_CLASS_NAME "cotti"
#define MINOR_NUMBER 0
#define NUMBER_OF_DEVICES 1

#define WAKE_UP_THREAD1 11  // Value needed to wake up thread 1
#define WAKE_UP_THREAD2 22  // Value needed to wake up thread 2
#define BASE_10 10

#endif // SEND_SIGNAL_H
