#ifndef _CDATA_IOCTL_H_
#define _CDATA_IOCTL_H_

#define <linux/ioctl.h>

#define DEV_IOCTL_ID    0xCC

#define IOCTL_CLEAR     _IOW(DEV_IOCTL_ID, 1, int)

#define IOCTL_RED       _IO(DEV_IOCTL_ID, 2)
#define IOCTL_GREEN     _IO(DEV_IOCTL_ID, 3)
#define IOCTL_BLUE      _IO(DEV_IOCTL_ID, 4)
#define IOCTL_WHITE     _IO(DEV_IOCTL_ID, 5)
#define IOCTL_BLACK     _IO(DEV_IOCTL_ID, 6)

#endif
