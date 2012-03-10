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

	unsigned char *fp;

	int size = 320*240;
	unsigned char pix[4] = {0x00, 0xFF, 0xFF, 0xFF};   //yellow color
	char pix_c[4] = {0xFF, 0x00, 0xFF, 0x00};  //cyan color

	printf("mmap\n");

	fd = open("/dev/cdata", O_RDWR);
	//ioctl(fd, IOCTL_CLEAR, &number);
	//mmap(0, 1024, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);

	fp = (unsigned char *)mmap(0, size*4, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);

	for (i=0; i<size; i++)
	{
		*(fp+(4*i)) = pix[0];
		*(fp+(4*i+1)) = pix[1];
		*(fp+(4*i+2)) = pix[2];
		*(fp+(4*i+3)) = pix[3];
	}

	printf("finish the painting\n");
	sleep(45);

	//write(fd, pix_c, 4);

	close(fd);
}
