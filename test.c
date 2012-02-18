#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>

#include "cdata_ioctl.h"

void main()
{
    int fd, i;
    int size = 200; //(320*240);
    char pix[4] = {0x00, 0xFF, 0xFF, 0xFF};

    printf("Test code - IOCTL\n");
    fd = open("/dev/cdata", O_RDWR);
    //ioctl(fd, IOCTL_CLEAR, &size);
    //write(fd, "123", 0);
     
    //for (i=0;i<size;i++) {
    while(1) {
         write(fd, pix, 4);
      }

    close(fd);
}
