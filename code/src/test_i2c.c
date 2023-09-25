#include "test_i2c.h"

int i2c_test(void) {
    FILE* fd;
    char id = 0xD0;
    if ((fd = fopen("/dev/i2c-1", "r+")) == NULL) {
        perror("Couldn't open device");
        return -1;
    } else if (ioctl(fileno(fd), I2C_SLAVE, I2C_SLAVE_ADDRESS) == -1 ) {
        perror("Not device found on slave address");
        return -1;
    } else if (fwrite(&id, 1, 1, fd) == 0) {
        perror("write");
        return -1;
    } else if (fread(&id, 1, 1, fd) == 0) {
        perror("read");
        return -1;
    }

    printf("The i2c id is: 0x%x\n", id);
    fclose(fd);
    return 0;
}