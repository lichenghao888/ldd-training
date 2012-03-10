#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <stdlib.h>


#include "cdata_ioctl.h"

//start to try the re-entrance code

void main(void)
{
	int fd, i, number = 10000;
	pid_t pid;

	int size = 320*240;
	char pix[4] = {0x00, 0xFF, 0xFF, 0xFF};   //yellow color
	char pix_c[4] = {0xFF, 0x00, 0xFF, 0x00};  //cyan color

	printf("mmap\n");

	fd = open("/dev/cdata", O_RDWR);
	ioctl(fd, IOCTL_CLEAR, &number);

	mmap(0, 1024, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	sleep(15);

	//write(fd, pix_c, 4);

	close(fd);
}
