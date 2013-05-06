#ifndef CREATEUDPSOCKET_H
#define CREATEUDPSOCKET_H

#include <netinet/in.h>

#define RTPUDPV4TRANS_MAXPACKSIZE							65535

#define MAKE_SOCKADDR_IN(var,adr,prt) /*adr,prt must be in network order*/\
    struct sockaddr_in var;\
    var.sin_family = AF_INET;\
    var.sin_addr.s_addr = (adr);\
    var.sin_port = (prt);


int make_socket_nonblocking(int sock);


int create_udp_socket(unsigned short int port);


int create_broadcast_socket(unsigned int port);
int enable_broadcast(int fd);

int create_multicast_socket(char *multi_addr, unsigned int port, char* interface_ip);
int socket_join_group(int fd, unsigned int group_addr, unsigned int interface_addr);
int socket_leave_group(int fd, unsigned int group_addr, unsigned int interface_addr);


#endif
