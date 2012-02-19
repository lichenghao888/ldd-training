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
    int fd, i;
    pid_t pid;

    int size = 320*240;
    char pix[4] = {0x00, 0xFF, 0xFF, 0xFF};   //yellow color
    char pix_c[4] = {0xFF, 0x00, 0xFF, 0x00};  //cyan color

    printf("Test code - entrance\n");

    pid = fork();

    fd = open("/dev/cdata", O_RDWR);


    if (pid == 0)
     {
        sleep(1);
	while(1) 
	{
		write(fd, pix, 4);
	}
     }
    else
     {
        sleep(3);
	while(1) 
	{
		write(fd, pix_c, 4);
	}        
     }

    close(fd);
}
