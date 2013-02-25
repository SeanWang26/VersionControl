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
#include "getopt.h"
#include <dlfcn.h>

#include <sys/types.h>

#define MAXFILE 65535


extern int Daemon2();
extern int RemoteCtrlServiceOpen(int);
extern int RedirectLog(char *pLogDir, int redirectErr, int redirectOut, int redirectIn) ;

char Help[] =
"Usage: \n"
"lnvrDaemon.out -d num\n" 
"	if num...\n"         
"lnvrDaemon.out -r num\n" 
"	if num...\n"         
"lnvrDaemon.out -i num\n" 
"	if num...\n"        
"lnvrDaemon.out -h\n" 
"	show help\n";


int main(int argc,char **argv)  
{  

	//int memory;  
	//int count = 0;
	//int theCount = 0;
	//time_t t1,t2;

	//必要时改成从配置中获取

	//默认不以守护运行
	int bIsDaemon = 0;

	int bRedirect = 1;

	int bOpenRemoteCtrl = 1;
	//
	int RestartInterval = 10; 
	char *c;

	//char programeName[] = "lnvrserver";
	//char exePrograme[] = "./lnvrserver -g";
	//char proDir[] = "/home/Release";

	int option_index=-1;
	int opt=-1;
	char const *shortopt = "d:r:i:o:h";
	struct option long_options[] = {
		{"daemon", required_argument, 0, 'b'},
		{"redirect", required_argument, 0, 'r'},
		{"interval", required_argument, 0, 'i'},
		{"remote", required_argument, 0, 'o'},
		{"help", 0, 0, 'h'}
	};

	time_t now;
	time(&now);
	printf("launch time %s\n", ctime(&now)); 

	while ((opt=getopt_long(argc, argv,
					shortopt, long_options,
					&option_index)) != -1) 
	{
		switch(opt)
		{
			case 'd':
				printf("Daemon\n");
				if(*optarg=='0')
					bIsDaemon = 0;
			break;
			case 'r':
				printf("redirect\n");
				if(*optarg=='0')
					bRedirect = 0;
			break;
			case 'i':
				printf("interval %s\n", optarg);
				RestartInterval = strtol(optarg, &c, 10);
				if(RestartInterval<1 || *c)
				{
					printf("-i the value is invalid\n");
					exit(0);
				}
				
			break;
			case 'h':
				printf("%s", Help);
				break;
			case ':':
			case '?':
				printf("use --help %d\n", opt);
				exit(0);
		}
	}

#if 1
	if(bIsDaemon)
	{
		printf("running in daemon mode\n");
		Daemon2();
	}
	else
	{
		printf("not running in daemon mode\n");
	}

	//should not do rederect here
	//if(bRedirect)
	//{
	//	RedirectLog(daemonLogDir, redirectErr, redirectOut, redirectIn);
	//}	
	//else 
	//{
	//	printf("no Redirect");
	//}
		
	if(bOpenRemoteCtrl)
	{	
		const char *dlpath = "./libremoteservice.so";
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
			remoteservice(1);
	}
		//RemoteCtrlServiceOpen(1);

	//app monitor
	/*while(0) 
	{  
		if(chdir(proDir)==-1)
		{
			fprintf(stderr,"Error: no the dir %s  \n",proDir);  
			while(1)
			{
				if(chdir(proDir)==-1)
				{
					sleep(5);
				}
				else
				{
					break;
				}
			}
		}
		else
		{
			if(!access(programeName,0))
			{
				count++;
				t1 = time(NULL);
				system(exePrograme);
				time(&now);
				memory=sysconf(_SC_AVPHYS_PAGES);  
				fprintf(stderr,"Time %s, memory[%d], Restart %s times %d\n",
									ctime(&now), memory, programeName, count);  

				t2 = time(NULL);
				
				//如果是长时间运行, 中途退出时, 马上重启
				if(t2 - t1 > 120)
				{
					fprintf(stderr,"### t2 - t1 > 120 second, RedirectLog, Please check the next file ->\n");  
					RedirectLog(daemonLogDir, redirectErr, redirectOut, redirectIn);
					continue;
				}
				else
				{
					//防止输出太频繁
					//运行时间太短时, 则sleep	
					sleep(2);
				}

				fflush(stdout);
				
			}
			else
			{
				fprintf(stderr,"Error: no the file %s \n", programeName);  
				while(1)
				{
					if(access(programeName,0))
					{
						sleep(5);
					}
					else
					{
						break;
					}
				}
			}
		}
	} */ 
#endif
	
	exit(0);  
} 
 
