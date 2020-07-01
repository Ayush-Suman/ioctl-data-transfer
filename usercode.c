#include "ioc.h"
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <stdio.h>

int main(){
int f_desc;
f_desc = open("/dev/CharDe", 0);
if(f_desc<0){
	printf("Cant open file\n");
	return 0;
}

char data[5];

sprintf(data, "%d", rand());
data[4] = '\0';
printf("Data to be sent to driver through ioctl %s\n", data);
ioctl(f_desc, IOCTL_WRITE_DATA, data);
char incoming_data[5];
ioctl(f_desc, IOCTL_READ_DATA, incoming_data);
printf("Data that came from kernel through ioctl %s\n", incoming_data);
return 0;
}




