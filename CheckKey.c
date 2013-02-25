#include <stdio.h>
#include <stdlib.h>

void* CheckKey(char** cmdarglist)
{
	/*
	result:0 success
	*/
	char *reschar = malloc(1024);//
	if(reschar)
	{
		sprintf(reschar, "checkkey:result=0:message=seuccessful\r\n");
	}
	else
	{
		return NULL;
	}
			
	return reschar;
}











































