#include "procfs.h"

/******************************************************************************
 * Static variables
******************************************************************************/

static ssize_t driver_read(struct file *file, char *user_buffer, size_t count, loff_t *offs);
static ssize_t driver_write(struct file *file, const char *user_buffer, size_t count, loff_t *offs);

// The folder and the file created in the procfs, /proc/<proc_folder>/<proc_file>
static struct proc_dir_entry *proc_folder, *proc_file;

/* This "proc_ops" struct is very similar to the "file_operations" one used
 * for character devices. Holds pointers to functions to be called when a
 * certain system call its called upon the file. */
static struct proc_ops fops = {
    .proc_read = driver_read,
    .proc_write = driver_write,
};


/******************************************************************************
 * Init and exit
******************************************************************************/

/// @brief This function is called when the module is loaded into the kernel.
static int __init my_module_init(void) {
    printk("Hello, Kernel!\n");

    // Create folder
    if ((proc_folder = proc_mkdir(FOLDER_NAME, FOLDER_PATH)) == NULL ) {
        printk("Couldn't create proc directory: /proc/%s\n", FOLDER_NAME);
        goto folder_error;
    }

    // Create file
    if ((proc_file = proc_create(FILE_NAME, 0666, proc_folder, &fops)) == NULL ) {
        printk("Couldn't create file: /proc/%s/%s\n", FOLDER_NAME, FILE_NAME);
        goto file_error;
    }
    return 0;

    file_error: proc_remove(proc_folder);
    folder_error: return -1;
}

/// @brief This function is called when the module is removed from the kernel.
static void __exit my_module_exit(void) {
    proc_remove(proc_file);
    proc_remove(proc_folder);
    printk("Goodbye, Kernel\n");
}

// Set entrypoint and exit point for the kernel module.
module_init(my_module_init);
module_exit(my_module_exit);

/******************************************************************************
 * Proc filesystem functions
******************************************************************************/

/// @brief When read, the file from procfs will return a fixed string
static ssize_t driver_read(struct file *file, char *user_buffer, size_t count, loff_t *offs) {
    char text[] = RETURNED_ON_READ;
    int to_copy, not_copied, delta;

    // Get amount of data to copy
    to_copy = min(count, sizeof(text));

    // Copy data to user, return bytes that hasn't copied
    not_copied = copy_to_user(user_buffer, text, to_copy);

    // Calculate data
    delta = to_copy - not_copied;
    return delta;
}

/// @brief When written, it will echo the response in the kernel logs
static ssize_t driver_write(struct file *file, const char *user_buffer, size_t count, loff_t *offs) {
    char text[255];
    int to_copy, not_copied, delta;

    // Clear text
    memset(text, 0, sizeof(text));

    // Get amount of data to copy
    to_copy = min(count, sizeof(text));

    // Copy data to user, return bytes that hasn't copied
    not_copied = copy_from_user(text, user_buffer, to_copy);
    printk("You have written: \"%s\"\n", text);

    // Calculate data
    delta = to_copy - not_copied;
    return delta;
}