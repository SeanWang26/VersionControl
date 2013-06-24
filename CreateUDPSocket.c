#include "CreateUDPSocket.h"
#include <stdio.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

//
int make_socket_nonblocking(int sock) {
  int curFlags = fcntl(sock, F_GETFL, 0);
  return fcntl(sock, F_SETFL, curFlags|O_NONBLOCK);
}

int create_udp_socket(unsigned short int port)
{
	//struct sockaddr_in adr_srvr;
	MAKE_SOCKADDR_IN(adr_srvr,INADDR_ANY,htons(port));
	int len_srvr = sizeof(adr_srvr);

	
	int fd = socket(AF_INET, SOCK_DGRAM, 0);
	if(fd==-1){
		printf("create_broadcast_socket socket error, %d, %s\n", errno, strerror(errno));
		return -1;
	}

	fcntl(fd, F_SETFD, 1);

	if(bind(fd, (struct sockaddr *)&adr_srvr, len_srvr)){
		printf("create_udp_socket socket bind, %d, %s\n", errno, strerror(errno));
		close(fd);
		return -1;
	}
		
	return fd;
}

int enable_broadcast(int fd)
{
	assert(fd>=0);

	int so_broadcast=1;
	int res = setsockopt(fd, SOL_SOCKET, SO_BROADCAST, &so_broadcast, sizeof(so_broadcast));
	if(res<0){
		printf("enable_broadcast setsockopt error, %d, %s\n", errno, strerror(errno));
		return -1;
	}

	return 0;
}
//for send INADDR_BROADCAST
int create_broadcast_socket(unsigned int port)
{
	MAKE_SOCKADDR_IN(adr_srvr,INADDR_ANY,htons(port));
	//struct sockaddr_in adr_srvr;
	int len_srvr = sizeof(adr_srvr);
	
	int fd = socket(AF_INET, SOCK_DGRAM, 0);
	if(fd==-1){
		printf("create_broadcast_socket socket error, %d, %s\n", errno, strerror(errno));
		return -1;
	}

	if(enable_broadcast(fd)==-1){
		close(fd);
		return -1;
	}

	

	if(bind(fd, (struct sockaddr *)&adr_srvr, len_srvr)){
		printf("create_broadcast_socket bind error, %d, %s\n", errno, strerror(errno));
		close(fd);
		return -1;
	}
		
	return fd;
}

int is_multicast_address(unsigned int address) {
  unsigned int addressInHostOrder = ntohl(address);
  return addressInHostOrder >  0xE00000FF &&
         addressInHostOrder <= 0xEFFFFFFF;
}


int create_multicast_socket(char *multi_addr, unsigned int port, char* interface_ip)
{

	//struct sockaddr_in adr_srvr;
	MAKE_SOCKADDR_IN(adr_srvr,INADDR_ANY,port);
	int len_srvr = sizeof(adr_srvr);
	
	int fd = socket(AF_INET, SOCK_DGRAM, 0);
	if(fd==-1){
		printf("create_broadcast_socket socket error, %d, %s\n", errno, strerror(errno));
		return -1;
	}

	

	if(bind(fd, (struct sockaddr *)&adr_srvr, len_srvr)){
		printf("create_udp_socket socket bind, %d, %s\n", errno, strerror(errno));
		close(fd);
		return -1;
	}

	// Set the sending interface for multicasts, if it's not the default:
	if (adr_srvr.sin_addr.s_addr != INADDR_ANY) {
		struct in_addr addr;
		addr.s_addr = adr_srvr.sin_addr.s_addr;

		if (setsockopt(fd, IPPROTO_IP, IP_MULTICAST_IF, (const char*)&addr, sizeof(addr)) < 0) {
			printf("create_multicast_socket IP_MULTICAST_IF, %d, %s\n", errno, strerror(errno));
			close(fd);
			return -1;
		}
	}

	return fd;
}

int socket_join_group(int fd, unsigned int group_addr, unsigned int interface_addr)
{
	if (!is_multicast_address(group_addr)) return 0; // ignore this case
	
	struct ip_mreq imr;
	imr.imr_multiaddr.s_addr = group_addr;
	imr.imr_interface.s_addr = interface_addr;//
	if (setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (const char*)&imr, sizeof (struct ip_mreq)) < 0) {
		printf("socket_join_group IP_ADD_MEMBERSHIP, %d, %s\n", errno, strerror(errno));
		return -1;
	}
	
	return 0;
}
int socket_leave_group(int fd, unsigned int group_addr, unsigned int interface_addr)
{
	if (!is_multicast_address(group_addr)) return 0; // ignore this case
	
	struct ip_mreq imr;
	imr.imr_multiaddr.s_addr = group_addr;
	imr.imr_interface.s_addr = interface_addr;
	if (setsockopt(fd, IPPROTO_IP, IP_DROP_MEMBERSHIP,(const char*)&imr, sizeof (struct ip_mreq)) < 0) {
		printf("socket_join_group IP_ADD_MEMBERSHIP, %d, %s\n", errno, strerror(errno));
		return -1;
	}
	
	return 0;
}








