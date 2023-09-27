#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <string.h>
#include <poll.h>

int main() {
	int fd;
	struct pollfd my_poll;

	/* Open the device file */
	if ((fd = open("/dev/poll_driver", O_RDONLY)) == -1) {
        perror("Could not open device file");
		return -1;
    }
	memset(&my_poll, 0, sizeof(my_poll));
	my_poll.fd = fd;            // Device to wait for
	my_poll.events = POLLIN;    // Event to wait for

	/* Wait for Signal */
	printf("Wait for signal...\n");
	poll(&my_poll, 1, -1);
	printf("Button was pressed!\n");
	return 0;
}