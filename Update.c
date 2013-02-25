#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h> 
#include <string.h>
#include <stdint.h>
#include<sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include<sys/stat.h>
#include<fcntl.h>

#include "our_md5.h"

void* DoUpdate(char** cmdarglist)
{
	char msg[64] = "successful";
	char* rspcmd = NULL;
	
	char* CmdMain=NULL;
	char* ArgName=NULL; char* ArgSize=NULL; char* ArgMd5=NULL; char* ArgKey=NULL;
	char* ArgVer=NULL; char* ArgIp=NULL; char* ArgPort=NULL;

	int i=0;
	char *curcmdentry=0;
	printf("1212121111111\n");
	
	//update:name=update.tar.gz:size=178:md5=ae7bc45b1dae2d2a1a4287cef1949db8:key=111111111111111111:ver=12.3:ip=172.16.140.53:port=12345
	while(NULL != (curcmdentry = cmdarglist[i++])){
		if(strncmp("update", curcmdentry, 5)==0){
			printf("a\n");
			CmdMain = curcmdentry;
		}
		else if(strncmp("name=", curcmdentry, 5)==0){
			printf("b\n");
			ArgName = curcmdentry+5;
		}
		else if(strncmp("size=", curcmdentry, 5)==0){
			printf("c\n");
			ArgSize = curcmdentry+5;
		}
		else if(strncmp("md5=", curcmdentry, 4)==0){
			printf("d\n");
			ArgMd5 = curcmdentry+4;
		}		
		else if(strncmp("key=", curcmdentry, 4)==0){
			printf("e\n");
			ArgKey = curcmdentry+4;
		}	
		else if(strncmp("ver=", curcmdentry, 4)==0){
			printf("f\n");
			ArgVer = curcmdentry+4;
		}
		else if(strncmp("ip=", curcmdentry, 3)==0){
			printf("g\n");
			ArgIp = curcmdentry+3;
		}
		else if(strncmp("port=", curcmdentry, 5)==0){
			ArgPort = curcmdentry+5;
		}
	}
	
	printf("121212111111---------1\n");

	printf("%s:name=%s:size=%s:md5=%s:key=%s:ver=%s:ip=%s:port=%s\n"
		, CmdMain, ArgName, ArgSize, ArgMd5, ArgKey, ArgVer, ArgIp, ArgPort);

	int res = 0;
	char NamePath[strlen(ArgName)];
	sprintf(NamePath, "./%s", ArgName);

	int filefd = -1;

	int connfd = socket(AF_INET,SOCK_STREAM,0);
	if(connfd<0)
	{
		sprintf(msg, "socket error"); 
		res = -1;
		goto end;
	}
	
	printf("%s:name=%s:size=%s:md5=%s:key=%s:ver=%s:ip=%s:port=%s\n"
		, CmdMain, ArgName, ArgSize, ArgMd5, ArgKey, ArgVer, ArgIp, ArgPort);
	
	struct sockaddr_in addr;
	memset(&addr,0,sizeof(addr));
	addr.sin_family =AF_INET;
	uint16_t up= atoi(ArgPort);
	printf("up=%d\n", up);
	addr.sin_port = htons(up);
	addr.sin_addr.s_addr = inet_addr(ArgIp);

	if(connect(connfd, (struct sockaddr *)&addr, sizeof(struct sockaddr))==-1)
	{
		printf("errno=%d\n", errno);
		sprintf(msg, "connect error%d", errno); 
		res = -2;
		goto end;
	}

	printf("file download connected!\n");
	
	char filebuff[1024];
	int rsize = 0;
	
	filefd = open(NamePath, O_TRUNC|O_WRONLY|O_CREAT);	
	if(filefd<0)
	{
		sprintf(msg, "create file %s error", NamePath);
		res = -3;
		goto end;
	}

	while((rsize=read(connfd, filebuff, 1024))>0)
	{
		if(write(filefd, filebuff, rsize)<0)
		{
			sprintf(msg, "write file %s errno%d", NamePath, errno);
			res = -4;
		}
	}

	char* mdstring = our_MD5File(NamePath, NULL);
	if(mdstring==NULL || strcmp(mdstring, ArgMd5)!=0)
	{
		printf("md5 check error! src=%s, des=%s \n", ArgMd5, mdstring);

		if(mdstring) 
			free(mdstring);

		//unlink(NamePath);//delete it

		res = -5;
		sprintf(msg, "md5 check failed 2");
		goto end;
	}
	
	printf("mdstring=%s\n", mdstring);

	if(mdstring) free(mdstring);
	
	//Backup old version??????
	//system();

	

	system("tar xvf update.tar.gz");
	system("cp update/* ./ -r -f");
	system("dos2unix update.sh");
	system("bash update.sh");

	printf("file download _exit!\n");

end:
	printf("111");
	if(connfd>0)
		close(connfd);

	if(filefd>0)
		close(filefd);
	
	rspcmd = (char*)malloc(256);
	sprintf(rspcmd, "update:result=%d:message=%s\r\n", res, msg);
	printf("%s\n", rspcmd);
	return rspcmd;
	
}

