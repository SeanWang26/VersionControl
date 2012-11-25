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



#define MAXFILE 65535

extern int Daemon2();
extern int RemoteCtrlServiceOpen(int);
extern int RedirectLog(char *pLogDir, int redirectErr, int redirectOut, int redirectIn) ;

int main(int argc,char **argv)  
{  
	  
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
		//printf("�ػ�����\n");
		Daemon2();
	}
	else
	{
		printf("�����ػ�����, only comm program\n");
		fprintf(stderr,"�����ػ�����, only comm program\n");
	}

	RedirectLog(daemonLogDir, redirectErr, redirectOut, redirectIn);

	time(&now);  
	fprintf(stderr,"����ʱ��: Time %s, Restart lnvrserver times %d\n",ctime(&now), count);  

	RemoteCtrlServiceOpen(0);
	
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
 
