#pragma once

#include <map>
#include <mutex>
#include <sys/types.h>
#include <sys/socket.h>
#include "stdafx.h"

class agent
{
public:
	agent();
	~agent();

	bool inti();

	bool  dowork(tcpHandle* tcphandle,const std::string& firstSPack/*发送给云端的首包*/,
		const std::string& firstAPack/*以送给本地服务端的首包*/,const char* sip, int sport,const char* aip,int aport);
private:

	bool sendSocket(SOCKET s, const char* data, int len);

	bool copySocket(SOCKET s1,SOCKET s2);

	bool closeSocket(SOCKET s);
	bool sendHandlePack(SOCKET s, tcpHandle* tcphandle);
	SOCKET getsockete(const char* ip, int port);

};
