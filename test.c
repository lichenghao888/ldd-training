#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>

void main()
{
   int fd;
    fd = open("/dev/cdata4", O_RDWR);
    fd = open("/dev/cdata4", O_RDWR);
sleep(10);
close(fd);
close(fd);
}
