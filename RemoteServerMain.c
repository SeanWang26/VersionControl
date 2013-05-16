#include "RemoteCtrlService.h"
#include "getopt.h"
#include <stdio.h>
#include <dlfcn.h>

int main(int argc,char **argv)  
{
	/*const char *dlpath = "./libremoteservice.so";
	void * dl = dlopen(dlpath, RTLD_NOW | RTLD_GLOBAL);
	int (*remoteservice)(int);
	remoteservice = NULL;
	if (dl == NULL) {
		fprintf(stderr, "try open %s failed : %s\n",dlpath, dlerror());
	}
	else
	{
		printf("try open %s success\n", dlpath);
	}
	
	remoteservice = dlsym(dl, "RemoteCtrlServiceOpen");
	if(remoteservice)
		remoteservice(1);*/

	RemoteCtrlServiceOpen(1);
	//RemoteCtrlServiceClose();

	return 0;
}






