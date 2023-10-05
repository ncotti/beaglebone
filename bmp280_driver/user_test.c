#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <string.h>
#include "inc/bmp280_types.h"

/// @brief Test basic functionality of the ioctl() command
int main(void) {
    bmp280_unit unit = CELSIUS;
    bmp280_mode mode = NORMAL;
    char temp[50];
    int fd;

    if ((fd = open("/dev/temp-sensor", O_RDWR)) == -1) {
        perror("open");
        return -1;
    }

    unit = CELSIUS;
	ioctl(fd, IOCTL_CMD_SET_UNIT, &unit);
    printf("Bytes read: %d\n", read(fd, temp, sizeof(temp)));
    temp[strlen(temp)-1] = '\0';  // Remove \n char
    printf("%s °C\n", temp);

    unit = KELVIN;
	ioctl(fd, IOCTL_CMD_SET_UNIT, &unit);
    read(fd, temp, sizeof(temp));
    temp[strlen(temp)-1] = '\0';
    printf("%s K\n", temp);

    unit = FAHRENHEIT;
	ioctl(fd, IOCTL_CMD_SET_UNIT, &unit);
    read(fd, temp, sizeof(temp));
    temp[strlen(temp)-1] = '\0';
    printf("%s °F\n", temp);

    printf("Disconnect device, and then press ENTER to continue\n");
    getchar();

    if (read(fd, temp, sizeof(temp)) == -1) {
        printf("After disconnect, the read was unsuccessful, as expected.\n");
    } else {
        printf("Device was disconnected, and something went wrong.\n");
    }

    printf("Reconnect device. You should see a normal temperature reading now, "
        "and positive bytes read. Press ENTER to continue\n");
    getchar();

    ioctl(fd, IOCTL_CMD_SET_MODE, &mode);   // Reset

    unit = CELSIUS;
	ioctl(fd, IOCTL_CMD_SET_UNIT, &unit);
    printf("Bytes read: %d\n", read(fd, temp, sizeof(temp)));
    temp[strlen(temp)-1] = '\0';
    printf("%s °C\n", temp);

	close(fd);
	return 0;
}
