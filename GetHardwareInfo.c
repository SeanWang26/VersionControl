#include <linux/hdreg.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>

//buf length equal to 20 is ok
int GetDiskSerialNumber(char* buf, size_t max)
{
	struct hd_driveid hid;

	int fd = open ("/dev/sda", O_RDONLY);
	if(fd==-1)
	{		
		printf("SerialNumber:22\n");
		return 0;
	}

	if(ioctl(fd, HDIO_GET_IDENTITY, &hid)<0)
	{
		printf("SerialNumber:22\n");
		close(fd);
		return 0;
	}

	snprintf (buf, max, "%s", hid.serial_no);

	//printf("SerialNumber:%s\n", buf);

	return strlen(buf);
}















































