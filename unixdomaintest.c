#include "UnixDomainSockets.h"
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>


void* UninDomainSocketServer(void *p)
{
	int fd = serv_listen("wyx.socket");
	if(fd<0)
	{
		printf("serv_listen error\n");
	}
		
	uid_t uidptr;

	int cfd = serv_accept(fd, &uidptr);
	while(1)
	{
		char buffer[256];
		int err = recv(cfd , buffer, 256, MSG_WAITALL);
		if(err<0)
		{
			printf("recv error\n");
		}
		printf("%s\n", buffer);
	}
}

int main(int argc,char **argv) 
{
	pthread_t tid = 0;
	pthread_create(&tid, NULL, UninDomainSocketServer, NULL);

	sleep(3);

	int fd = cli_conn("wyx.socket");
	if(fd<0)
	{
		printf("cli_conn error\n");
	}
	
	while(1)
	{
		char buffer[256] = "1111 send !!!!\n";
		int err = send(fd, buffer, 256, 0);
		if(err<0)
		{
			printf("send error\n");
		}
		sleep(2);
	}

	return 0;
}




























