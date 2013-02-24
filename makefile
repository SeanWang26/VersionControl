CC = gcc -ggdb3 

CCC = g++ -ggdb3 

#OBJS = lnvrDaemon.o RemoteCtrlService.o dlink.o Update.o

SRC = lnvrDaemon.c LogCfg.c Daemon.c 

#RemoteCtrlService.c dlink.c Update.c GetLicence.c CheckKey.c KillProcess.c our_md5.c our_md5hl.c Base64.c strDup.c GetHardwareInfo.c

TSRC = RemoteServerMain.c RemoteCtrlService.c dlink.c Update.c GetLicence.c CheckKey.c KillProcess.c our_md5.c our_md5hl.c Base64.c strDup.c GetHardwareInfo.c

all : lnvrDaemon.out RemoteServer.out libremoteservice.so

lnvrDaemon.out : $(SRC)
	$(CC) -O2  -Wall -o $@ $^ liblua.a libcrypto.a -lrt -lpthread -ldl

RemoteServer.out : $(TSRC)
	$(CC) -O2 -Wall -o $@ $^ liblua.a libcrypto.a -lrt -lpthread -ldl


libremoteservice.so : RemoteCtrlService.c RemoteCtrlService.h dlink.c Update.c GetLicence.c CheckKey.c KillProcess.c our_md5.c our_md5hl.c Base64.c strDup.c GetHardwareInfo.c
	$(CC) -fPIC --shared -O2 $^ -o $@

#%.o: %.c
#	$(CC) -O2 -c $< -lrt -lpthread

.PHONY : all

clean:
	rm -f *.o
	rm -f *.out
	rm -r *.so























