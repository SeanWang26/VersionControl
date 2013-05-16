#include <string.h>
#include <stdio.h>

extern int _port;

void* GetServciePort(char** cmdarglist)
{
	char *rspcmd = (char*)malloc(256);
	sprintf(rspcmd, "GetServciePort:Port=%d\r\n", _port);
	return rspcmd;
}

























