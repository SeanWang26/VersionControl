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
#include <dirent.h> 
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
		/* ��׼�����ض��� */  
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
		/* ��׼�����ض��� */  
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
		 /* ��׼����ض��� */  
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

	fprintf(stderr, "File NO %d, �汾��< 0. 1. 1. 10 >, 2012-07-30 17:29\n", fileNums++);

       return 0;
}


#define MAXFILE 65535

//extern "C"
//{


//g++

int main(int argc,char **argv)  
{  
	struct sigaction act;  
	time_t now;  
	int memory;  
	int count = 0;
	//int theCount = 0;
	time_t t1,t2;

	//��Ҫʱ�ĳɴ������л�ȡ

	int bIsDaemon = 1;
	char programeName[] = "lnvrserver";
	char exePrograme[] = "./lnvrserver -g";
	char proDir[] = "/home/Release";
	char daemonLogDir[] = "/home/DaemonLog";
	int redirectErr = 1;
	int redirectOut = 1; 
	int redirectIn =  1;


	if(bIsDaemon)
	{
		printf("�ػ�����\n");
		fprintf(stderr,"�ػ�����\n");
		/* �������˳� */  
		if(fork()!=0) exit(1);	
		 
		/* ����һ���µĻ����� */  
		if(setsid()<0)exit(1);	
		 
		/* �����ź�SIGHUP */  
		act.sa_handler=SIG_IGN;  
		sigemptyset(&act.sa_mask);	
		act.sa_flags=0;  
		if(sigaction(SIGHUP,&act,NULL)==-1)exit(1);  
		 
		/* �ӽ����˳�,�����û�п����ն��� */  
		if(fork()!=0) exit(1);	
		 
		if(chdir("/")==-1)exit(1);	

	}
	else
	{
		printf("�����ػ�����, only comm program\n");
		fprintf(stderr,"�����ػ�����, only comm program\n");
	}

	if(chdir(daemonLogDir)==-1)
	{
		mkdir(daemonLogDir,0777);
	}
	
	RedirectLog(daemonLogDir, redirectErr, redirectOut, redirectIn);

	time(&now);  
	fprintf(stderr,"����ʱ��: Time %s, Restart lnvrserver times %d\n",ctime(&now), count);  
	
	while(1)  
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
				
				//����ǳ�ʱ������, ��;�˳�ʱ, ��������
				if(t2 - t1 > 120)
				{
					fprintf(stderr,"### t2 - t1 > 120 second, RedirectLog, Please check the next file ->\n");  
					RedirectLog(daemonLogDir, redirectErr, redirectOut, redirectIn);
					continue;
				}
				else
				{
					//��ֹ���̫Ƶ��
					//����ʱ��̫��ʱ, ��sleep	
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
	}  
	exit(0);  
} 

//}
 
