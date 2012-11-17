#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include<time.h>
#include "des.h"
#include "Base64.h"

extern int GetDiskSerialNumber(char* buf, size_t max);
static const uint8_t cbc_key[] = {
    0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
    0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef, 0x01,
    0x45, 0x67, 0x89, 0xab, 0xcd, 0xef, 0x01, 0x23
};

const char* GetLicence(char* cmdarglist[])
{	
	struct AVDES d;
	uint64_t key[3];
	uint64_t ct;
	uint64_t data;

	char edsstr[] = "des test string haha\n";

	//maintest();

	/*data = AV_RB64(edsstr);

	key[0] = rand64(); key[1] = rand64(); key[2] = rand64();
	data = rand64();
	av_des_init(&d, key, 192, 0);
	av_des_crypt(&d, &ct, &data, 1, NULL, 0);
	av_des_init(&d, key, 192, 1);
	av_des_crypt(&d, &ct, &ct, 1, NULL, 1);
	if (ct != data) {
		printf("Test 2 failed\n");
		return 1;
	}*/


	char *reschar = malloc(1024);//not good
	//1.add cmd head
	sprintf(reschar, "%s", "getlisence");

	//2.add disk number serial
	char DiskSerialNumber[20];
	int n = GetDiskSerialNumber(DiskSerialNumber, 20);
	if(n)
	sprintf(reschar, "%s:disksn=%s", reschar, DiskSerialNumber);

	//3.add time  "2001.12.1 20-12-39" to avoid ':'
	time_t timep;
	struct tm *p;
	time(&timep);
	p=localtime(&timep);
	
	sprintf(reschar, "%s:systime=%02d.%02d.%02d %02d-%02d-%02d", reschar
		, (1900+p->tm_year), (1+p->tm_mon), p->tm_mday, 
		p->tm_hour, p->tm_min, p->tm_sec);


	printf("GetLicence:%s\n", reschar);
	//unsigned resultSize=0;
	//char* dele = base64Decode(lisencebase64, &resultSize, 0);
	//if(resultSize>0)
	//printf("dele:%s\n", dele);
	
	return reschar;
}



































