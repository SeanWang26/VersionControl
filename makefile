CC = gcc -ggdb3 

CCC = g++ -ggdb3 

OBJS = lnvrDaemon.o RemoteCtrlService.o dlink.o Update.o

SRC = lnvrDaemon.c RemoteCtrlService.c dlink.c Update.c GetLicence.c CheckKey.c our_md5.c our_md5hl.c Base64.c strDup.c GetHardwareInfo.c des.c

TSRC = RemoteServerMain.c RemoteCtrlService.c dlink.c Update.c GetLicence.c CheckKey.c our_md5.c our_md5hl.c Base64.c strDup.c GetHardwareInfo.c des.c

all : lnvrDaemon.out RemoteServer.out

lnvrDaemon.out : $(SRC)
	$(CC) -Wall -o $@ $^ liblua.a libcrypto.a -lrt -lpthread

RemoteServer.out : $(TSRC)
	$(CC) -O2 -Wall -o $@ $^ liblua.a libcrypto.a -lrt -lpthread

#%.o: %.c
#	$(CC) -O2 -c $< -lrt -lpthread

.PHONY : all

clean:
	rm -f *.o
	rm -f *.out























