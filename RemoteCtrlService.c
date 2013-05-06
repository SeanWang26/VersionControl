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
//#include "Base64.h"
//#include "openssl/des.h"
#include <dlfcn.h>

#include "CreateUDPSocket.h"


#define TASK_LIB "./libremotetask.so"

static int _listenfd = -1;
static int _broadcastfd = -1;
static int _ctrlfd = -1;
static int _pid = -1;

const static unsigned int _port = 4011; 
const static unsigned int _udpport = 4012; 

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

static void* HandleCmd(int fd, char *cmdstr)
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

	const char *dlpath = TASK_LIB;
	void * dl = dlopen(dlpath, RTLD_NOW | RTLD_GLOBAL);
	void* (*remoteservice)(char** arg);
	remoteservice = NULL;
	if (dl == NULL) {
		fprintf(stderr, "try open %s failed : %s\n",dlpath, dlerror());
	}
	else
	{
		printf("try open %s success\n", dlpath);
	}
	
	void *cmdrsp = NULL;
	if(dl)
	{
		remoteservice = dlsym(dl, cmdarg_list[0]);
		if(remoteservice)
			cmdrsp = remoteservice(cmdarg_list);
		else
			cmdrsp = strdup("error:message=can't find task\n");
	}
	else
	{
		cmdrsp = strdup("error:message=can load libremoteservice.so\n");
	}
	
	return cmdrsp;
}

/*static int EncryptResult(const char* result_in, char **result_out, int *result_out_len)
{
	if(NULL==result_in) 
	{
		*result_out=0;
		*result_out_len=0;

		return -1;
	}
	else
	{
		des_key_schedule ks,ks2,ks3;
		static unsigned char cbc_key [8]={0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef};
		static unsigned char cbc2_key[8]={0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef, 0x01};
		static unsigned char cbc3_key[8]={0x45, 0x67, 0x89, 0xab, 0xcd, 0xef, 0x01, 0x23};	
		static unsigned char cbc_iv [8]={0x12, 0x34, 0x56, 0x78, 0x90, 0xab, 0xcd, 0xef};
		
		des_cblock iv3;
		memcpy(iv3,cbc_iv,sizeof(cbc_iv));

		if (DES_set_key_checked(&cbc_key,&ks) != 0 
		 || DES_set_key_checked(&cbc2_key,&ks2) != 0 
		 || DES_set_key_checked(&cbc3_key,&ks3) != 0)
		{
			printf("Key error \n");
			return -1;
		}

		int result_in_len = strlen(result_in);
		int cbc_out_len = (result_in_len+15)/16*16+16;
		unsigned char *cbc_in = malloc(cbc_out_len);
		unsigned char *cbc_out = malloc(cbc_out_len);
		memset(cbc_in, 0, cbc_out_len);

		memcpy(cbc_in, result_in, result_in_len);

		printf("1 %d\n",cbc_out_len);
		int i, j=cbc_out_len/16;
		for(i=0; i<j; ++i)
		{
			des_ede3_cbc_encrypt(cbc_in+i*16,cbc_out+i*16,16L,ks,ks2,ks3,&iv3,
						 DES_ENCRYPT);
			printf("1\n");
		}

		free(cbc_in);
		
		*result_out_len = cbc_out_len;
		*result_out = (char*)cbc_out;

	}

	return 0;
}

static char* EncryptResultToString(char* result, int result_len)
{
	if(NULL==result || 0==result_len) return result;
	
	printf("len=%d\n", result_len);
	char* lisencebase64 = base64Encode(result, result_len);
	printf("base64:%s, len=%zu\n", lisencebase64, strlen(lisencebase64));
	free(result);

	return lisencebase64;
}
*/
static int SendResult(int fd, const char* result)
{
	assert(fd>0);

	if(NULL==result) return 0;

	/*int Encrypt=0;//no Encrypt now!!
	char *result2;
	if(Encrypt)
	{
		//char *result_out=NULL;
		//int result_out_len=0;
		//EncryptResult(result, &result_out, &result_out_len);
		//result2 = EncryptResultToString(result_out, result_out_len);
	}
	else
	{
		result2 = result;
	}*/


	int err = SendCmd(fd, result); 

	//free(result);

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

static void process_get_status(void)
{
	pid_t pid;
	int status;

	unsigned int one = 0;

	for ( ;; )
	{
		pid = waitpid(-1, &status, WNOHANG);//not block
		if (pid == 0) {
			printf("[process_get_status]WNOHANG\n");
            return;
        }
        
		if(-1==pid){
			if (errno == EINTR) {
				printf("[process_get_status]EINTR\n");
				continue;
			}

			if (errno == ECHILD && one) {
				printf("[process_get_status]ECHILD\n");
				return;
			}
		}

		one = 1;

		if(pid==_pid){
			printf("[process_get_status]precess %d quit with %d, _ctrlfd=%d\n", pid, status, _ctrlfd);
			close(_ctrlfd);
			_ctrlfd = -1;
			_pid = -1;
			break;
		}
	}
}

void acthandler(int signo)
{
	if(signo==SIGCHLD){
		process_get_status();
	}
	else
	{
		printf("[acthandler]ge unkonw signo %d\n", signo);
	}

}


static int CreateClientProcess(int fd)
{
	int pid=0;
	pid=fork();
	if(pid==0)
	{
		printf("fork new pid=%d, fd=%d\n", pid, fd);
		close(_listenfd);
		char *_recvBuf = 0;
		int RECV_BUFF_LEN = 1024;
		
		while((_recvBuf = (char *)malloc(RECV_BUFF_LEN))==NULL)
		{
			sleep(1);
		}

		printf("_recvBuf=%p\n", _recvBuf);
		
		while(1)
		{
			int err = ReadCmd(fd, _recvBuf, RECV_BUFF_LEN);
			if(err <= 0)
			{
				printf("ReadCmd connect close %d\n", fd);
				break;
			}
			
			void* result = HandleCmd(fd, _recvBuf);

			err = SendResult(fd, result);
			if(err <= 0)
			{
				printf("SendResult error\n");
				break;
			}

			free(result);
		}

		if(_recvBuf) free(_recvBuf);
		
		shutdown(fd, SHUT_RD);
		close(fd);
		exit(0);
		printf("fork new end%d\n", pid);

	}
	else if(pid>0)
	{
		printf("CreateClientProcess fork main %d\n", pid);
		_ctrlfd = fd;
		//sleep(2);
	}
	else if(pid==-1)
	{
		printf("CreateClientProcess fork error %d\n", pid);
	}

	return pid;
}

int handleudp(char *rvbuf)
{


	return 0;
}


static int LoopSocket()
{
	assert(_listenfd!=-1);
	
	fd_set _fdset;

	struct timeval tv = {100,0};
	
	struct sigaction act;
	act.sa_handler=acthandler; 
	sigemptyset(&act.sa_mask);//empty it
	act.sa_flags=0;
	if(sigaction(SIGCHLD,&act,NULL)==-1){
		printf("sigaction error %d\n", errno);////not good...............???
		return -1;
	}  
	printf("LoopSocket num=%d\n", SIGCHLD);

	while(1)
	{
		int maxfd=0;
		FD_ZERO(&_fdset);
		if(_listenfd>0)FD_SET(_listenfd, &_fdset);
		if(_broadcastfd>0)FD_SET(_broadcastfd, &_fdset);
		maxfd = (_broadcastfd>_listenfd)?_broadcastfd:_listenfd;
		
		tv.tv_sec = 60000;
		tv.tv_usec = 0;

		printf("while 6000\n");
		
		//assume _ctrlfd always big than _listenfd, it should be!
		int res = select(1+maxfd , &_fdset, NULL, NULL, &tv);
		printf("2222222222\n");
		if(-1==res)
		{
			if(EINTR==errno)
			{
				printf("select EINTR\n");
				continue;
			}
			else
			{
				printf("[loop_socket]select errno=%d;\n", errno);
				break;
			}
		}
		else if(0==res)
		{
			printf("time up\n");
		}
		else if(FD_ISSET(_listenfd, &_fdset))
		{
			struct sockaddr_in addr;
			socklen_t addr_len = sizeof(struct sockaddr_in);	

			int tfd = accept(_listenfd, (struct sockaddr *)&addr, &addr_len);
			if(tfd>0)
			{
				if(_ctrlfd == -1)
				{
					_pid = CreateClientProcess(tfd);					
				}
				else
				{
					SendResult(tfd, strdup("error:server be occupied\n"));			
					//printf("server be occupied by %d\n", _ctrlfd);
					//SendCmd(tfd, strdup("error:server be occupied\n"));
					shutdown(tfd, 0);
					close(tfd);
				}
			}
		}
		else if(FD_ISSET(_broadcastfd, &_fdset))
		{
			printf("11111111\n");
			struct sockaddr address;
			socklen_t address_len = sizeof(struct sockaddr);
			char rvbuf[1024];
			ssize_t rvsize = recvfrom(_broadcastfd, rvbuf, 1024, 0, &address, &address_len);

			printf("rvsize=%d, rvbuf=%s\n", rvsize, rvbuf);
		}
		else
		{

		}
	
	}

	return 0;
}

static void* RemoteCtrlServer(void *p)
{
	if(InitSocket()<0)
	{
		printf("listen error\n");
		return (void*)-1;
	}

	_broadcastfd = create_udp_socket(_udpport);
	if(_broadcastfd<0)
	{
		printf("init udp error\n");
	}

	LoopSocket();

	return (void*)0;
}

static int Start(int wait) 
{
	static pthread_t tid = 0;

	pthread_create(&tid, NULL, RemoteCtrlServer, NULL);
	
	if(wait)pthread_join(tid, NULL);

	return tid;	
}
static int Stop(int tid)
{
	return pthread_cancel(tid);
}

int RemoteCtrlServiceOpen(int wait)
{
	return Start(wait);
}

int RemoteCtrlServiceClose(int tid)
{
	return Stop(tid);
}



