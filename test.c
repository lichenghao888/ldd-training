#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>

#include "cdata_ioctl.h"

void main()
{
    int fd;
    int size = (320*120);

    printf("Test code - IOCTL\n");
    fd = open("/dev/cdata", O_RDWR);
    ioctl(fd, IOCTL_CLEAR, &size);
    //write(fd, "123", 0);

    close(fd);
}
