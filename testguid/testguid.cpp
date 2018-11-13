// testguid.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include <windows.h>
#include <Nb30.h>
#include <time.h>

#include "openssl/md5.h"

#pragma comment(lib, "netapi32.lib")
#pragma comment(lib, "libeay32.lib")
#pragma comment(lib, "ssleay32.lib")

bool GetMAC(char*  lpszMAC)
{
	char mac[200] = { 0 };
	NCB ncb;

	typedef struct _ASTAT_
	{
		ADAPTER_STATUS   adapt;
		NAME_BUFFER   NameBuff[30];
	}ASTAT, *PASTAT;

	ASTAT Adapter;

	typedef struct _LANA_ENUM
	{
		UCHAR   length;
		UCHAR   lana[MAX_LANA];
	}LANA_ENUM;

	LANA_ENUM lana_enum;
	UCHAR uRetCode;
	memset(&ncb, 0, sizeof(ncb));
	memset(&lana_enum, 0, sizeof(lana_enum));
	ncb.ncb_command = NCBENUM;
	ncb.ncb_buffer = (unsigned char *)&lana_enum;
	ncb.ncb_length = sizeof(LANA_ENUM);
	uRetCode = Netbios(&ncb);
	if (uRetCode != NRC_GOODRET)
		return false;

	for (int lana = 0; lana < lana_enum.length; lana++)
	{
		ncb.ncb_command = NCBRESET;
		ncb.ncb_lana_num = lana_enum.lana[lana];
		uRetCode = Netbios(&ncb);
		if (uRetCode == NRC_GOODRET)
			break;
	}

	if (uRetCode != NRC_GOODRET)
		return false;

	memset(&ncb, 0, sizeof(ncb));
	ncb.ncb_command = NCBASTAT;
	ncb.ncb_lana_num = lana_enum.lana[0];
	strcpy((char*)ncb.ncb_callname, "*");
	ncb.ncb_buffer = (unsigned char *)&Adapter;
	ncb.ncb_length = sizeof(Adapter);
	uRetCode = Netbios(&ncb);

	if (uRetCode != NRC_GOODRET)
		return false;

	sprintf(mac, "%02X%02X%02X%02X%02X%02X",
		Adapter.adapt.adapter_address[0],
		Adapter.adapt.adapter_address[1],
		Adapter.adapt.adapter_address[2],
		Adapter.adapt.adapter_address[3],
		Adapter.adapt.adapter_address[4],
		Adapter.adapt.adapter_address[5]);
	strcpy(lpszMAC, mac);

	return true;
}

void createguid(char guid[33])
{
	srand(GetTickCount());

	static volatile long ndx = rand();
	static char macbuff[64] = { 0 };
	static const char hex[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

	MD5_CTX     ctx;
	FILETIME    ft;
	unsigned    char md5[33];
	char        buff[128];

	InterlockedAdd(&ndx, 1);

	if (macbuff[0] == 0) 
		if (!GetMAC(macbuff)) memset(macbuff, 0, sizeof(macbuff));

	GetSystemTimeAsFileTime(&ft);
	int len = sprintf_s(buff, sizeof(buff)-1, "%u%u%s%d", ft.dwHighDateTime, ft.dwLowDateTime, macbuff, ndx);
	
	MD5_Init(&ctx);
	MD5_Update(&ctx, buff,len);
	MD5_Final(md5, &ctx);
	
	for (int i = 0; i < 16; i++){
		guid[i*2]   = hex[md5[i] >> 4];
		guid[i*2+1] = hex[md5[i]&4];
	}
	guid[32] = '\0';
}


int _tmain(int argc, _TCHAR* argv[])
{
	char guid[64];

	for (int i = 0; i < 1000; i++){
		createguid(guid);
		printf("%s\n", guid);
	}
	
	system("pause");

	return 0;
}

