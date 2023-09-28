#ifndef SYSFS_H
#define SYSFS_H

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kobject.h>
#include <linux/string.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nicolas G. Cotti");
MODULE_DESCRIPTION("Creating a folder and file in sysfs");

#define FOLDER_NAME "sysfs_folder"

// Permissions can't be "0666", it throws error.
#define PERMISSIONS 0660

#endif // SYSFS_H
