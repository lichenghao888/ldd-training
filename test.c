#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>

void main()
{
    int fd;
    fd = open("/dev/cdata", O_RDWR);
    write(fd, "123", 0);

    close(fd);
}
