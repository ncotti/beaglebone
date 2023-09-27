#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <signal.h>

#include "ioctl_types.h"

void signal_handler(int sig);

int main(void) {
	int fd;
	signal(SIGUSR1, signal_handler);
	printf("PID: %d\n", getpid());

	// Open the device file
	if((fd = open("/dev/signal_driver", O_RDONLY)) == -1) {
		perror("Could not open the device file\n");
		return -1;
	}

	// Register app for LKM
	if(ioctl(fd, IO_REGISTER_USER_APP)) {
		perror("Error registering app");
		close(fd);
		return -1;
	}

	printf("Waiting for signal...\n");
	while(1) {
        sleep(1);
    }
    close(fd);
	return 0;
}

void signal_handler(int sig) {
	printf("Signal number: %d. Button was pressed!\n", sig);
}
