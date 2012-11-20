#include "RemoteCtrlService.h"

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>

#include <sys/socket.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <pthread.h>

#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "Base64.h"
#include "des.h"




extern const char* CheckKey(char *key);
extern const char* GetLicence(char* cmdstr);
extern const char* DoUpdate(char *cmdstr);


static pthread_t pid = 0;
static int _listenfd = -1;
static int _ctrlfd = -1;
const static unsigned int _port = 4011; 
void * _taskHead = NULL;

static int Readn(int fd, void* buffer, int len)
{
	for (;;) {
		int err = recv(fd , buffer, len, MSG_WAITALL);
		if (err < 0) {
			assert(errno != EAGAIN);
			if (errno == EINTR)
				continue;
			break;
		}
		return err;
	}
	return -1;
}

static int SendCmd(int fd, const char* buffer)
{
	assert(NULL != buffer);
	
	uint16_t head = strlen(buffer)+1;

	int err = send(fd, &head, sizeof(head), 0);
	if(err<0)
	{
		return err;
	}
	
	send(fd, buffer, head, 0);
	if(err<0)
	{
		return err;
	}

	return err;
}

static int ReadCmd(int fd, char* buffer, int len)
{
	uint8_t header[2];

	int err = 0;
	err = Readn(fd, header, 2);

	printf("ReadCmd 1 err=%d\n", err);
	if (err <= 0)
		return err;
	
	size_t cmdlen = header[1] << 8 | header[0];
	printf("ReadCmd 2 cmdlen=%u\n", (uint32_t)cmdlen);
	if (cmdlen>0) {
		//assert(len > cmdlen);

		err = Readn(fd, buffer, cmdlen);
		if (err <= 0)
			return err;

		buffer[cmdlen-1]='\0';
		printf("%s\n",buffer);
	}

	return err;
}

static const char* HandleCmd(int fd, char *cmdstr)
{
	printf("PaserCmd:%d \n%s\n", (int)strlen(cmdstr), cmdstr);
	
	assert(strlen(cmdstr) >= 2);
	assert(cmdstr[strlen(cmdstr)-2]=='\r');
	assert(cmdstr[strlen(cmdstr)-1]=='\n');

	char cmd[strlen(cmdstr)+1];
	strncpy(cmd, cmdstr, strlen(cmdstr)+1);
	char *curcmdentry = cmd;

	char* cmdarg_list[256] = {0};
	int i=0;
	while(NULL != (cmdarg_list[i++] = strsep(&curcmdentry, ":\r\n\t")));

	const char *cmdrsp = NULL;
	if(cmdarg_list[0]==NULL)
	{
		printf("error:no cmd\n");
		cmdrsp = strdup("error:no cmd\n");		
	}
	else if(0==strcmp(cmdarg_list[0], "checkkey"))
	{
		cmdrsp = CheckKey(cmdstr);
	}
	else if(0==strcmp(cmdarg_list[0], "getlisence"))
	{
		cmdrsp = GetLicence(cmdstr);
	}
	else if(0==strcmp(cmdarg_list[0], "update"))
	{
		cmdrsp = DoUpdate(cmdstr);
	}
	else
	{
		printf("error:unkown cmd\n");
		cmdrsp = strdup("error:unkown cmd\n");
	}

	return cmdrsp;
}

static int EncryptResult(const char* result_in, char **result_out, int *result_out_len)
{
	if(NULL==result_in) 
	{
		*result_out=0;
		*result_out_len=0;
	}
	else
	{
		static const uint8_t cbc_key[] = {
			0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
			0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef, 0x01,
			0x45, 0x67, 0x89, 0xab, 0xcd, 0xef, 0x01, 0x23
		};
		struct AVDES d;
		uint8_t vi[8] = {0x12, 0x34, 0x56, 0x78, 0x90, 0xab, 0xcd, 0xef};
		//(*(uint64_t *)(vi) = (0x1234567890abcdefULL));
		int res = av_des_init(&d, cbc_key, 192, 0);
		if(res!=0)
			return -1;
		*result_out_len = strlen(result_in);
		av_des_crypt(&d, (uint8_t *)result_in, (const uint8_t *)result_in, strlen(result_in), vi, 0);
		*result_out = (char*)result_in;

		printf("cihper:");
		int i;
		for(i=0; i<*result_out_len; ++i)
		{
			printf("0x%2x  ", result_in[i]);
		}
		printf("\n");

		uint8_t tbuf[1024];
		uint8_t vi2[8] = {0x12, 0x34, 0x56, 0x78, 0x90, 0xab, 0xcd, 0xef};
		av_des_crypt(&d, tbuf,  result_in, *result_out_len, vi2, 1);
		printf("plain:");
		for(i=0; i<*result_out_len; ++i)
		{
			printf("0x%2x  ", tbuf[i]);
		}
		printf("\n");


		//*result_out = strdup(result_in);
		//*result_out_len = strlen(result_in);
	}

	return 0;
}

static char* EncryptResultToString(char* result, int result_len)
{
	if(NULL==result || 0==result_len) return result;

	char* lisencebase64 = base64Encode(result, result_len);
	printf("base64:%s, len=%zu\n", lisencebase64, strlen(lisencebase64));
	free(result);

	return lisencebase64;
}

static int SendResult(int fd, char* result)
{
	assert(fd>0);

	if(NULL==result) return 0;

	int err = SendCmd(fd, result); 

	free(result);

	return err;
}

static int InitSocket()
{
	if((_listenfd = socket(AF_INET,SOCK_STREAM,0))<0)
	{
		return -1;
	}
	
	int bReuseaddr = 1;
	int ret = setsockopt(_listenfd,SOL_SOCKET,SO_REUSEADDR,&bReuseaddr,sizeof(int));
	if(ret<0)
	{
		return ret;
	}
	struct sockaddr_in addr;
	memset(&addr,0,sizeof(addr));
	addr.sin_family =AF_INET;
	addr.sin_port = htons(_port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	if((ret=bind(_listenfd, (struct sockaddr *)&addr, sizeof(addr)))<0)
	{
		return ret;
	}
	
	if((ret=listen(_listenfd, 1))<0)
	{
		return ret;
	}

	return _listenfd;
}

void acthandler(int num)
{
	printf("num=%d\n", num);

	if(SIGCHLD==num)
	{
		int exitcode=0;
		pid_t pid = wait(&exitcode);
		printf("precess %d quit with %d, _ctrlfd=%d\n", pid, exitcode, _ctrlfd);
		close(_ctrlfd);
		_ctrlfd = -1;
	}	
}

static int CreateClientProcess(int fd)
{
	int pid=0;
	pid=fork();
	if(pid==0)
	{
		printf("fork new pid=%d, _ctrlfd=%d\n", pid, _ctrlfd);
		//close(_listenfd);
		char *_recvBuf = 0;
		int RECV_BUFF_LEN = 1024;
		while((_recvBuf = (char *)malloc(RECV_BUFF_LEN))==NULL)
		{
			sleep(1);
		}
		
		while(1)
		{
			int err = ReadCmd(fd, _recvBuf, RECV_BUFF_LEN);
			if(err <= 0)
			{
				printf("ReadCmd connect close %d\n", fd);
				break;
			}
			
			const char* result = HandleCmd(fd, _recvBuf);

			char *result_out=NULL;
			int result_out_len=0;
			EncryptResult(result, &result_out, &result_out_len);

			char *result2 = EncryptResultToString(result_out, result_out_len);

			err = SendResult(fd, result2);
			if(err <= 0)
			{
				printf("SendResult error\n");
				break;
			}
		}

		if(_recvBuf) free(_recvBuf);
		
		shutdown(fd, SHUT_RD);

		exit(0);
		printf("fork new end%d\n", pid);

	}
	else if(pid>0)
	{
		printf("CreateClientProcess fork main %d\n", pid);
		_ctrlfd = fd;
		sleep(2);
	}
	else if(pid==-1)
	{
		printf("CreateClientProcess fork error %d\n", pid);
	}

	return pid;
}

static int LoopSocket()
{
	assert(_listenfd!=-1);
	
	fd_set _fdset;

	struct timeval tv = {10,0};
	
	struct sigaction act;
	act.sa_handler=acthandler; 
	sigemptyset(&act.sa_mask);
	act.sa_flags=0;  
	if(sigaction(SIGCHLD,&act,NULL)==-1)exit(1);  
	printf("LoopSocket num=%d\n", SIGCHLD);

	while(1)
	{
		FD_ZERO(&_fdset);
		FD_SET(_listenfd, &_fdset);

		tv.tv_sec = 60000;
		tv.tv_usec = 0;

		printf("while 60000\n");
		
		//assume _ctrlfd always big than _listenfd, it should be!
		if(-1==select(1+_listenfd , &_fdset, NULL, NULL, &tv))
		{
			if(EINTR==errno)
			{
				continue;
			}
			else
			{
				printf("[loop_socket]select errno=%d;\n", errno);
				break;
			}
		}
		

		if(FD_ISSET(_listenfd, &_fdset))
		{
			struct sockaddr_in addr;
			socklen_t addr_len = sizeof(struct sockaddr_in);	

			int tfd = accept(_listenfd, (struct sockaddr *)&addr, &addr_len);
			if(tfd>0)
			{
				if(_ctrlfd == -1)
				{
					CreateClientProcess(tfd);					
				}
				else
				{
					printf("server be occupied by %d\n", _ctrlfd);
					SendCmd(tfd, "error:server be occupied\n");
					shutdown(tfd, 0);
				}
			}
		}
	
	}

	return 0;
}

static void* RemoteCtrlServer(void *p)
{
	InitSocket();
	LoopSocket();

	return 0;
}

static int Start() 
{
	pthread_create(&pid, NULL, RemoteCtrlServer, NULL);
	
	pthread_join(pid, NULL);

	return 0;	
}
static int Stop()
{
	pthread_cancel(pid);

	return 0;
}

#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>

int RemoteCtrlServiceOpen()
{
	/*int fd = open("/dev/sde", O_RDWR);
	if(fd<0)
	{
		printf("open error %d\n", fd);
		return 0;
	}

	char dd[512]={0};
	int err=0;
	for(;;)
	{
		err = write(fd, dd,  512);
		if(err<0)
		{
			printf("write error %d\n", errno);
			return 0;
		}
		else
		{
			printf("write %d\n", err);
		}
	}*/


	
	/*static const uint8_t cbc_key[] = {
		0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
		0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef, 0x01,
		0x45, 0x67, 0x89, 0xab, 0xcd, 0xef, 0x01, 0x23
	};
	
	struct AVDES d;
	int res = av_des_init(&d, cbc_key, 192, 0);
	int i;
	for(i=0; i<3; ++i)
	{
		int j; 
		for(j=0; j<3; ++j)
		{
			printf("0x%lld ", d.round_keys[i][j]);
		}
		printf("\n");
	}
	printf("triple_des=%d\n", d.triple_des);
	
	int j;
	char buf[] = "12345678901234567890123456789012345678901234567890"
				"12345678901234567890123456789012345678912345678900"
				"12345678901234567890123456789012345678901234567890"
				"12345678901234567890123456789012345678901234567890"
				"12345678901234567890123456789012345678901234567890"
				"12345678901234567890123456789012345678901234567890"
				"12345678901234567890123456789012345678901234567890"
				"12345678901234567890123456789012345678901234567890";
	printf("plain1:");
	for(j=0; j<sizeof(buf); ++j)
	{
		printf("%d ", buf[j]);
	}
	printf("\n");
	
	uint8_t vi[8];
	(*(uint64_t *)(vi) = (0x1234567890abcdefULL));

	printf("\n");
	av_des_crypt(&d, buf, buf, sizeof(buf)-1, vi, 0);

	printf("cipher:");
	for(j=0; j<sizeof(buf)-1; ++j)
	{
		printf("%d ", buf[j]);
	}
	printf("\n");

	printf("vi1:");
	for(j=0; j<8; ++j)
	{
		printf("%x ", vi[j]);
	}
	printf("\n");
	(*(uint64_t *)(vi) = (0x1234567890abcdefULL));
	av_des_crypt(&d, buf, buf, sizeof(buf)-1, vi, 1);

	printf("plain2:");
	for(j=0; j<sizeof(buf); ++j)
	{
		printf("%d ", buf[j]);
	}
	printf("\n");
	printf("vi2:");
	for(j=0; j<8; ++j)
	{
		printf("%x ", vi[j]);
	}	
	printf("\n");
	//maintest();
	//maintest2();*/
	return Start();
}

int RemoteCtrlServiceClose()
{
	return Stop();
}



