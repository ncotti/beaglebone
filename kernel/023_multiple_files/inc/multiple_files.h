#ifndef MULTIPLE_FILES_H
#define MULTIPLE_FILES_H

#include <linux/init.h>
#include <linux/module.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nicolas G. Cotti");
MODULE_DESCRIPTION("An example LKM built from multiple source files, "
"and with a header directory");

#include "file1.h"
#include "file2.h"

#endif // MULTIPLE_FILES_H
