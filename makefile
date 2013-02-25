CC = gcc -ggdb3 

CCC = g++ -ggdb3 

#OBJS = lnvrDaemon.o RemoteCtrlService.o Update.o

SRC = lnvrDaemon.c LogCfg.c Daemon.c 

#RemoteCtrlService.c Update.c GetLicence.c CheckKey.c KillProcess.c our_md5.c our_md5hl.c Base64.c strDup.c GetHardwareInfo.c

TSRC = RemoteServerMain.c RemoteCtrlService.c Update.c GetLicence.c CheckKey.c KillProcess.c our_md5.c our_md5hl.c Base64.c strDup.c GetHardwareInfo.c

all : lnvrDaemon.out RemoteServer.out libremoteservice.so libremotetask.so

lnvrDaemon.out : $(SRC)
	$(CC) -O2  -Wall -o $@ $^ -lrt -lpthread -ldl

RemoteServer.out : $(TSRC)
	$(CC) -O2 -Wall -o $@ $^ -lrt -lpthread -ldl

libremoteservice.so : RemoteCtrlService.c RemoteCtrlService.h Update.c GetLicence.c CheckKey.c KillProcess.c our_md5.c our_md5hl.c Base64.c strDup.c GetHardwareInfo.c
	$(CC) -fPIC --shared -O2 $^ -o $@

libremotetask.so : CheckKey.c CheckKey.c KillProcess.c GetLicence.c Update.c
	$(CC) -fPIC --shared -O2 $^ -o $@

.PHONY : all

clean:
	rm -f *.o
	rm -f *.out
	rm -r *.so























