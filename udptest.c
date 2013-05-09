#include "CreateUDPSocket.h"
#include <stdio.h>
#include <unistd.h>

int main(int argc,char **argv) 
{
	int fd = create_broadcast_socket(40000);
	if(fd<0)
	{
		printf("bad fd\n");
	}

	MAKE_SOCKADDR_IN(dest_addr,INADDR_BROADCAST,htons(4012))
	int dest_len = sizeof(dest_addr);
	while(1)
	{
		int sens = sendto(fd, "messgess1\n", 11, 0, (struct sockaddr*)&dest_addr, dest_len);
		printf("send %d\n", sens);
		sens = sendto(fd, "messgess11\n", 12, 0, (struct sockaddr*)&dest_addr, dest_len);
		printf("send2 %d\n", sens);
		sleep(2);
	}
	
	return 0;
}













































