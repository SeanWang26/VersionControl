#include "RemoteCtrlService.h"

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>

#include <unistd.h>
#include <fcntl.h>

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
#include <arpa/inet.h>

#include "CreateUDPSocket.h"

#include "UdpCmd.h"

#define TASK_LIB "./libremotetask.so"

#define SERVICE_BASE_PORT   4011
#define PROBE_BASE_PORT 40111

static int _listenfd = -1;
static int _broadcastfd = -1;
//static int _ctrlfd = -1;
static int _pid = -1;

unsigned int _port = SERVICE_BASE_PORT; 
const static unsigned int _udpport = PROBE_BASE_PORT; 

void * _taskHead = NULL;

extern void* GetServciePort(char** cmdarglist);

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

static void* ReadUdpCmd(int fd, char* buffer, int len, struct sockaddr *address, socklen_t* address_len)
{
	//struct sockaddr address;
	//socklen_t address_len = sizeof(struct sockaddr);
	uint8_t header[2];
	ssize_t rvsize = recvfrom(fd, header, 2, MSG_PEEK, address, address_len);
	
	struct sockaddr_in* address_ = (struct sockaddr_in*)&address;
	char* ip = inet_ntoa(address_->sin_addr);
	unsigned int port = ntohs(address_->sin_port);
	//show sss.sss.sss.sss
	printf("from %s:%d\n", ip, port);
	
	if(rvsize<2)
	{
		printf("recvfrom error %d, %s\n", errno, strerror(errno));
		exit(1);
	}
	size_t cmdlen = header[1] << 8 | header[0];
	if(cmdlen>65535)
	{
		printf("cmdlen>65535\n");
		exit(1);
	}
	char *rvbuf = (char*)malloc(2+cmdlen+1);//header+cmdstring+"\0"
	rvsize = recvfrom(fd, rvbuf, 2+cmdlen, 0, address, address_len);
	if(rvsize<=2)
	{
		printf("recvfrom rvsize %zu error %d, %s\n", rvsize, errno, strerror(errno));
		exit(1);
	}
	assert((2+cmdlen)==rvsize);
	rvbuf[2+cmdlen] = 0;

	return rvbuf+2;//not good........................................to do
}


static void* HandleCmd(int fd, char *cmdstr, struct sockaddr* fromaddr, socklen_t addlen)
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

	void *cmdrsp = NULL;
	const char *dlpath = TASK_LIB;
	void * dl = dlopen(dlpath, RTLD_NOW | RTLD_GLOBAL);
	void* (*remoteservice)(char** arg);
	remoteservice = NULL;
	if (dl == NULL) {
		fprintf(stderr, "try open %s failed : %s\n",dlpath, dlerror());
		cmdrsp = strdup("error:message=can load "TASK_LIB"\n");
		return cmdrsp;
	}

	remoteservice = dlsym(dl, cmdarg_list[0]);
	if(remoteservice)
		cmdrsp = remoteservice(cmdarg_list);
	else
		cmdrsp = strdup("error:message=can't find task %s\n");

	dlclose(dl);
	return cmdrsp;
}
static void* HandleUdpCmd(int fd, char *cmdstr, struct sockaddr* fromaddr, socklen_t addlen)
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

	void *cmdrsp = NULL;
	if(strncmp("GetServciePort", cmdarg_list[0], 14)==0)
	{
		cmdrsp = GetServciePort(cmdarg_list);
	}
	else
	{
		const char *dlpath = TASK_LIB;
		void * dl = dlopen(dlpath, RTLD_NOW | RTLD_GLOBAL);
		void* (*remoteservice)(char** arg);
		remoteservice = NULL;
		if (dl == NULL) {
			fprintf(stderr, "try open %s failed : %s\n",dlpath, dlerror());
			cmdrsp = strdup("error:message=can load "TASK_LIB"\n");
			return cmdrsp;
		}

		remoteservice = dlsym(dl, cmdarg_list[0]);
		if(remoteservice)
			cmdrsp = remoteservice(cmdarg_list);
		else
			cmdrsp = strdup("error:message=can't find task %s\n");

		dlclose(dl);		
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

static int SendUdpResult(int fd, const char* result, struct sockaddr* fromaddr, socklen_t addlen)
{
	uint16_t reslen = strlen(result);
	char *tobuf=0;
	tobuf = malloc(2+reslen+1);
	memcpy(tobuf, &reslen, sizeof(uint16_t));
	memcpy(tobuf+2, result, reslen+1);
	tobuf[2+reslen] = 0;
	return sendto(fd, tobuf, 2+reslen+1, 0, fromaddr, addlen);
}


static int InitSocket()
{	
	if((_listenfd = socket(AF_INET,SOCK_STREAM,0))<0)
	{
		return -1;
	}

	fcntl(_listenfd, F_SETFD, 1);
	
	int bReuseaddr = 1;
	int ret = setsockopt(_listenfd,SOL_SOCKET,SO_REUSEADDR,&bReuseaddr,sizeof(int));
	if(ret<0)
	{
		return ret;
	}
	struct sockaddr_in addr;
	memset(&addr,0,sizeof(addr));
	addr.sin_family =AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	do{
		addr.sin_port = htons(_port);
		if(bind(_listenfd, (struct sockaddr *)&addr, sizeof(addr))==0){
			break;
		}

		++_port;
		if(_port > (SERVICE_BASE_PORT+100)){
			close(_listenfd);
			_listenfd=-1;
			return -1;
		}
	}while(1);
	
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
			printf("[process_get_status]precess %d quit with %d in %d\n", pid, status, getpid());
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
		signal(SIGCHLD, SIG_DFL);
		close(_listenfd);
		printf("fork new pid=%d, fd=%d\n", pid, fd);

		sleep(2);
		
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
			
			void* result = HandleCmd(fd, _recvBuf, NULL, 0);

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
		_pid=pid;
		close(fd);
		//sleep(2);
	}
	else if(pid==-1)
	{
		printf("CreateClientProcess fork error %d\n", pid);
	}

	return pid;
}

static int CreateUdpClientProcess(int fd)
{
	int pid=0;

	pid=fork();
	if(pid==0)
	{
		struct sockaddr address;
		socklen_t address_len = sizeof(struct sockaddr);
		void* result = ReadUdpCmd(_broadcastfd, NULL, 0, &address, &address_len);

		result = HandleUdpCmd(_broadcastfd, result, &address, address_len);

		SendUdpResult(_broadcastfd, result, &address, address_len);

		exit(0);
	}
	else if(pid>0)
	{
		//int status;
		//int pid = waitpid(pid, &status, 0);//not block
		//printf("waitpid %d\n", pid);
	}
	else if(pid==-1)
	{
		printf("CreateUdpClientProcess fork error %d, %s\n", errno, strerror(errno));
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

	//struct timeval tv = {100,0};
	
	struct sigaction act;
	act.sa_handler=acthandler; 
	sigemptyset(&act.sa_mask);//empty it
	act.sa_flags=0;
	if(sigaction(SIGCHLD,&act,NULL)==-1){
		printf("sigaction error %d\n", errno);////not good...............???
		return -1;
	}

	while(1)
	{
		int maxfd=0;
		FD_ZERO(&_fdset);
		if(_listenfd>0)FD_SET(_listenfd, &_fdset);
		if(_broadcastfd>0)FD_SET(_broadcastfd, &_fdset);
		maxfd = (_broadcastfd>_listenfd)?_broadcastfd:_listenfd;
		
		//tv.tv_sec = 60000;
		//tv.tv_usec = 0;
		int res = select(1+maxfd , &_fdset, NULL, NULL, NULL);
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
				if(_pid == -1)
				{
					CreateClientProcess(tfd);					
				}
				else
				{
					//should handle epipe......................todo......................
					SendResult(tfd, strdup("error:server be occupied\n"));			
					//printf("server be occupied by %d\n", _ctrlfd);
					shutdown(tfd, 0);
					close(tfd);
				}
			}
		}
		else if(FD_ISSET(_broadcastfd, &_fdset))
		{	
			printf("get for upd server\n");
			

			//int pid = CreateUdpClientProcess(_broadcastfd);
			//printf("pid %d for upd server\n", pid);
		}
		else
		{}
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

	printf("servcie on port %d\n", _port);
	
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
	printf("sizeof(pthread_t) %d\n", sizeof(pthread_t));
	
	if(!wait)
	{
		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
		pthread_create(&tid, &attr, RemoteCtrlServer, NULL);
		pthread_attr_destroy(&attr);
	}
	else
	{
		pthread_create(&tid, NULL, RemoteCtrlServer, NULL);
		pthread_join(tid, NULL);
	}

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



