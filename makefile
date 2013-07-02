CC = gcc 

CCC = g++ -ggdb3 

#OBJS = lnvrDaemon.o RemoteCtrlService.o Update.o

SRC = lnvrDaemon.c LogCfg.c Daemon.c 

#RemoteCtrlService.c Update.c GetLicence.c CheckKey.c KillProcess.c our_md5.c our_md5hl.c Base64.c strDup.c GetHardwareInfo.c

#TSRC = RemoteServerMain.c RemoteCtrlService.c Update.c GetLicence.c CheckKey.c KillProcess.c our_md5.c our_md5hl.c Base64.c strDup.c GetHardwareInfo.c

all : libremotetask.so libremoteservice.so lnvrDaemon.out RemoteServer.out udptest.out UnixDomainClient.out UnixDomainServer.out

lnvrDaemon.out : $(SRC)
	$(CC) -O2  -Wall -o $@ $^ -lrt -lpthread -ldl

RemoteServer.out : RemoteServerMain.c
	$(CC) -O2 -Wall -o $@ $^ -L. -lremoteservice -lrt -lpthread -ldl -Wl,-rpath,.

libremoteservice.so : RemoteCtrlService.c RemoteCtrlService.h CreateUDPSocket.c UnixDomainSockets.c GetServciePort.c our_md5.c our_md5hl.c Base64.c strDup.c
	$(CC) -fPIC -Wall --shared -O2 $^ -o $@

libremotetask.so : CheckKey.c CheckKey.c KillProcess.c GetLicence.c Update.c
	$(CC) -fPIC -Wall --shared -O2 $^ -o $@

udptest.out : udptest.c CreateUDPSocket.c
	$(CC) -O2 -Wall -o $@ $^ -L./ -lrt -lpthread -ldl

UnixDomainClient.out : UnixDomainClient.c UnixDomainSockets.c
	$(CC) -O2 -g -Wall -o $@ $^ -L./ -lrt -lpthread -ldl

UnixDomainServer.out : UnixDomainServer.c UnixDomainSockets.c
	$(CC) -O2 -g -Wall -o $@ $^ -L./ -lrt -lpthread -ldl

.PHONY : all

clean:
	rm -f *.o
	rm -f *.out
	rm -r *.so



#Update.c GetLicence.c CheckKey.c KillProcess.c  GetHardwareInfo.c



















