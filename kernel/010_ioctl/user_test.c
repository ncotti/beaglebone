#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "ioctl_types.h"

/* Copy this file into the BeagleBone, and execute it after the module
*  was installed. */

int main(void) {
	int answer, multiplier;
    int fd;

    if ((fd = open("/dev/ioctl_driver", O_WRONLY)) == -1) {
        perror("open");
        return -1;
    }

	ioctl(fd, IOCTL_CMD_READ, &multiplier);
	printf("Default multiplier should be one: %d\n", multiplier);

    multiplier = 3;
    ioctl(fd, IOCTL_CMD_WRITE, &multiplier);
    ioctl(fd, IOCTL_CMD_READ, &multiplier);
    printf("Multiplier should be three: %d\n", multiplier);

    answer = 5;
    ioctl(fd, IOCTL_CMD_MULTIPLY, &answer);
    printf("Answer should be fifteen: %d\n", answer);

    answer = 5;
    ioctl(fd, IOCTL_CMD_RESET);
    ioctl(fd, IOCTL_CMD_MULTIPLY, &answer);
    printf("Answer should be five: %d\n", answer);

	close(fd);
	return 0;
}
