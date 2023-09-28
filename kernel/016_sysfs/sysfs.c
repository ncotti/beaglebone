#include "sysfs.h"

/******************************************************************************
 * Static variables
******************************************************************************/

static ssize_t read_sysfs_file(struct kobject *folder, struct kobj_attribute *file, char *buffer);
static ssize_t write_sysfs_file(struct kobject *arg_folder, struct kobj_attribute *arg_file, const char *buffer, size_t count);

// Represents the folder
static struct kobject *folder;

// Represents the file. Name of the file is "sys_file", but don't use "" (quote marks).
static struct kobj_attribute file = __ATTR(sys_file, PERMISSIONS, read_sysfs_file, write_sysfs_file);

/******************************************************************************
 * Init and exit
******************************************************************************/

/// @brief This function is called when the module is loaded into the kernel.
static int __init my_module_init(void) {
    printk("Hello, Kernel!\n");

    /* Create folder
     * kernel_kobj is defined from the kernel as "the parent" for the directory.
     * k_obj points to the folder /sys/kernel, therefore, The file will be
     * loaded on /sys/kernel/FOLDER_NAME/sys_file */
    if ((folder = kobject_create_and_add(FOLDER_NAME, kernel_kobj)) == NULL) {
        printk("Couldn't create sysfs folder %s\n", FOLDER_NAME);
        goto folder_error;
    }

    // Create file
    if (sysfs_create_file(folder, &file.attr) != 0) {
        printk("Couldn't create sysfs file\n");
        goto file_error;
    }

    printk("File created: /sys/kernel/%s/%s\n", folder->name, file.attr.name);
    return 0;

    file_error: kobject_put(folder);
    folder_error: return -1;
}

/// @brief This function is called when the module is removed from the kernel.
static void __exit my_module_exit(void) {
    printk("Goodbye, Kernel\n");
    sysfs_remove_file(folder, &file.attr);
    kobject_put(folder);
}

// Set entrypoint and exit point for the kernel module.
module_init(my_module_init);
module_exit(my_module_exit);

/******************************************************************************
 * File operations
******************************************************************************/

/// @brief Read callback for the file
static ssize_t read_sysfs_file(struct kobject *arg_folder, struct kobj_attribute *arg_file, char *buffer) {
    return sprintf(buffer, "You have read from /sys/kernel/%s/%s\n", arg_folder->name, arg_file->attr.name);
}

/// @brief Write to the file
static ssize_t write_sysfs_file(struct kobject *arg_folder, struct kobj_attribute *arg_file, const char *buffer, size_t count) {
    printk("You wrote %s to /sys/kernel/%s/%s\n", buffer, arg_folder->name, arg_file->attr.name);
    return count;
}