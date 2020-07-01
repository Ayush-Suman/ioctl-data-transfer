#include <linux/ioctl.h>
#define IO_MAGIC 238
#define IOCTL_READ_DATA _IOR(IO_MAGIC, 0, char*)
#define IOCTL_WRITE_DATA _IOR(IO_MAGIC, 1, char*)
