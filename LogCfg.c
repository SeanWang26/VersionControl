#include <stdio.h>
#include <stdlib.h>
#include <error.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mount.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h> 
#include <stddef.h> 
#include <sys/types.h> 
#include <signal.h> 
#include <assert.h>


static int fileNums = 0;


int RedirectLog(char *pLogDir, int redirectErr, int redirectOut, int redirectIn)  
{
	struct tm *p;
	int error,in,out;  
	time_t stime = time(NULL);
	char lnvrErrorName[255];
	char lnvrOutName[255];
	char lnvrInName[255];	

	if(chdir(pLogDir)==-1)
	{
		mkdir(pLogDir,0777);
	}
	
	p = localtime(&stime);
	sprintf(lnvrErrorName, "%s/lnvrdaemonerror%04d%02d%02d_%02d%02d%02d", pLogDir, (1900+p->tm_year),
					(p->tm_mon + 1),p->tm_mday,p->tm_hour, p->tm_min, p->tm_sec);

	sprintf(lnvrOutName, "%s/lnvrdaemonout%04d%02d%02d_%02d%02d%02d", pLogDir,  (1900+p->tm_year),
					(p->tm_mon + 1),p->tm_mday,p->tm_hour, p->tm_min, p->tm_sec);

	sprintf(lnvrInName, "%s/lnvrdaemonin%04d%02d%02d_%02d%02d%02d", pLogDir, (1900+p->tm_year),
					(p->tm_mon + 1),p->tm_mday,p->tm_hour, p->tm_min, p->tm_sec);

#if 1
	if(redirectErr)
	{
		/* 标准错误重定向 */  
		error=open(lnvrErrorName,O_WRONLY|O_CREAT,0600);  
		if(error < 0)
		{
			assert(0);
		}
		else
		{
			dup2(error,STDERR_FILENO);	
			close(error);  
		}
	}

	if(redirectIn)
	{
		/* 标准输入重定向 */  
		in=open(lnvrInName,O_RDONLY|O_CREAT,0600);	
		if(in < 0)
		{
			assert(0);
		}
		else
		{
			if(dup2(in,STDIN_FILENO)==-1)perror("in");	
			close(in);	
		}
	}
	 
	if(redirectOut)
	{
		 /* 标准输出重定向 */  
		 out=open(lnvrOutName,O_WRONLY|O_CREAT,0600);  
		 if(out < 0)
		 {
			assert(0);
		 }
		 else
		 {
			 if(dup2(out,STDOUT_FILENO)==-1)perror("out");	
			 close(out);  
		 }
	}

#endif

	fprintf(stderr, "File NO %d, 版本号< 0. 1. 1. 10 >, 2012-07-30 17:29\n", fileNums++);

       return 0;
}



















































