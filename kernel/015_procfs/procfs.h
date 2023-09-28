#ifndef PROCFS_H
#define PROCFS_H

#include <linux/init.h>
#include <linux/module.h>
#include <linux/proc_fs.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nicolas G. Cotti");
MODULE_DESCRIPTION("Proc filesystem (procfs) is a virtual filesystem mounted on the /proc dir. "
    "This module creates a file under /prc/FOLDER_NAME/FILE_NAME, and defines "
    "read() and write() operations for that file."
);

#define RETURNED_ON_READ "Hello from a procfs file!\n"

#define FOLDER_NAME "proc_folder"
#define FILE_NAME   "proc_file"

/* If NULL, the folder will be placed in /proc/FOLDER_NAME. Otherwise, it will be put on
 * the path specified FOLDER_PATH/FOLDER_NAME */
#define FOLDER_PATH NULL

#endif // PROCFS_H
