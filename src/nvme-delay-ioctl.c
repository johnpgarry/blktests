// SPDX-License-Identifier: GPL-3.0+

/*
 * Simple test to issue an ioctl some time after opening a cdev.
 * This is to test lifetime of cdev, and whether we can handle
 * removing the cdev while we have active fds.
 */
#define _GNU_SOURCE
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <inttypes.h>
#include <sys/ioctl.h>
#include <linux/types.h>

#ifndef _LINUX_NVME_IOCTL_H
#define _LINUX_NVME_IOCTL_H
#define NVME_IOCTL_ID		_IO('N', 0x40)
#endif /* _UAPI_LINUX_NVME_IOCTL_H */

int main(int argc, char **argv)
{
	int fd, fd1;
	int count;

	int delay_seconds = 2;

	if (argc < 2) {
		fprintf(stderr, "usage: %s /dev/ngXnX", argv[0]);
		return EINVAL;
	}

	fd = open(argv[1], O_RDONLY);
	if (fd < 0)
		return fd;

	/*
	 * Steps:
	 * a. Try to open device until disallowed/gone
	 * b. sleep to allow nvme-subsystem be torn down
	 * c. issue the ioctl on original fd
	 */
	count = 0;
	for (;;) {
		fd1 = open(argv[1], O_RDONLY);
		usleep(500000);
		if (fd1 < 0)
			break;
		close(fd1);
		count++;
		if (count > 10) {
			fprintf(stderr, "%s still present", argv[0]);
			return EINVAL;
		}
	}
	sleep(delay_seconds);
	ioctl(fd, NVME_IOCTL_ID);
	close(fd);

	return 0;
}
