#ifndef IOCTL_TYPES_H
#define IOCTL_TYPES_H

// This variable can be any arbitrary number.
#define MAGIC_NUMBER 'c'

// With the macros "_IO, _IOW, _IOR, _IORW", you can define an unique ioctl identifier.
// The first argument is the magic number, the second arguments is an unique identifier,
// and the third argument is the type of the arguments passed (in fact, the size of
// the type is used). Arguments are always pointers, because they must be read or written
// from user space to kernel space
#define IOCTL_CMD_MULTIPLY  _IOWR(MAGIC_NUMBER,  1, int*)
#define IOCTL_CMD_RESET     _IO(MAGIC_NUMBER,  2)
#define IOCTL_CMD_READ      _IOR(MAGIC_NUMBER, 3, int*)
#define IOCTL_CMD_WRITE     _IOW(MAGIC_NUMBER, 4, int*)

#endif // IOCTL_TYPES_H