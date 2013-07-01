#include "CreateUDPSocket.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>


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

void* UninDomainSocketServer(void *p)
{
	int fd = create_multicast_socket(0,60001,0);
	socket_join_group(fd, "224.0.0.88");
	int i;
	while(1)
	{
		//SendUdpResult(fd, "GetServciePort:\r\n", (struct sockaddr*)&dest_addr, dest_len);
		//int sens = sendto(fd, "messgess1\n", 11, 0, (struct sockaddr*)&dest_addr, dest_len);
		//printf("send %d\n", sens);
		//sens = sendto(fd, "messgess11\n", 12, 0, (struct sockaddr*)&dest_addr, dest_len);
		//printf("SendUdpResult \n");
		
		struct sockaddr_in address;
		socklen_t address_len = sizeof(struct sockaddr_in);
		printf("ReadUdpCmd %d\n", i);

		recvfrom(fd, &i, 4, 0, (struct sockaddr *)&address, &address_len);

		printf("ReadUdpCmd %d\n", i);

		//sleep(2);
	}

}


int main(int argc,char **argv) 
{
	/*int fd = create_broadcast_socket(40000);
	if(fd<0)
	{
		printf("bad fd\n");
	}

	MAKE_SOCKADDR_IN(dest_addr,INADDR_BROADCAST,htons(40111))
	socklen_t dest_len = sizeof(dest_addr);
	while(1)
	{
		SendUdpResult(fd, "GetServciePort:\r\n", (struct sockaddr*)&dest_addr, dest_len);
		//int sens = sendto(fd, "messgess1\n", 11, 0, (struct sockaddr*)&dest_addr, dest_len);
		//printf("send %d\n", sens);
		//sens = sendto(fd, "messgess11\n", 12, 0, (struct sockaddr*)&dest_addr, dest_len);
		printf("SendUdpResult \n");

		char* res = ReadUdpCmd(fd, NULL, 0, (struct sockaddr *)&dest_addr, &dest_len);

		printf("ReadUdpCmd %s\n", res);

		sleep(2);
	}
*/

	pthread_t tid = 0;
	pthread_create(&tid, NULL, UninDomainSocketServer, NULL);

	struct sockaddr_in Multi_addr;//多播地址
	Multi_addr.sin_family=AF_INET;
	Multi_addr.sin_port=htons(60001);//多播端口
	Multi_addr.sin_addr.s_addr=inet_addr("224.0.0.88");//多播地址
	int fd = create_udp_socket(5234);

	socklen_t len=sizeof(struct sockaddr_in);
	int i=0;
	while(1)
	{
		sendto(fd, &i, 4, 0, (struct sockaddr*)&Multi_addr, len);
		printf("send\n");
		sleep(2);
	}

	sleep(300000);


	
	return 0;
}













































