#include "stdafx.h"
#include "clientAgent.h"
#include <thread>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
agent::agent()
{

}

agent::~agent()
{
	
}

bool agent::inti()
{
	return false;
}

bool agent::dowork(tcpHandle* tcphandle, const std::string& firstSPack/*发送给云端的首包*/,
    const std::string& firstAPack/*以送给本地服务端的首包*/, const char* sip, int sport, const char* aip, int aport)
{
	SOCKET p_asocket = getsockete(aip, aport);
	if (p_asocket == 0)
	{
		return false;
	}
	SOCKET p_ssocket = getsockete(sip, sport);
	if (p_ssocket == 0)
	{
		closeSocket(p_asocket);
		return false;
	}

    if (!sendHandlePack(p_ssocket, tcphandle)
        || !sendSocket(p_ssocket,&firstSPack[0],firstSPack.length())
        || !sendSocket(p_asocket, &firstAPack[0], firstAPack.length()))
    {
        closeSocket(p_ssocket);
        closeSocket(p_asocket);
        return false;
    }
    std::thread(&agent::copySocket, this, p_ssocket, p_asocket).detach();
    std::thread(&agent::copySocket, this, p_asocket, p_ssocket).detach();
    return true;
}



bool agent::sendSocket(SOCKET s, const char* data, int len)
{
	int p_i = 0;
	int p_sendLen = 0;
	while (len > 0)
	{
		p_i = ::send(s, data + p_sendLen, len, 0);
		p_sendLen += p_i;
		if (p_i <1)
		{
			return false;
		}
		len -= p_i;
	}
	return true;
}

bool agent::copySocket(SOCKET cid, SOCKET cid2)
{
    int p_bufLen = 1024 * 16;
    char* p_buf = new char[p_bufLen];
	ssize_t p_readLen = 0;
    int p_sendLen = 0;
    int p_rlen = 0;
    bool error = 0;
	ssize_t p_i = 0;
    while ((p_readLen = ::recv(cid, p_buf, p_bufLen, 0)) > 0)
    {
        p_sendLen = 0;
        while (p_sendLen < p_readLen)
        {
            p_i = ::send(cid2, p_buf + p_sendLen, p_readLen - p_sendLen, 0);
            p_sendLen += p_i;
            if (p_i < 1)
            {
                break;
            }
        }
    }
    delete p_buf;
    shutdown(cid2, SHUT_WR);
    close(cid2);
    return 0;
}

bool agent::closeSocket(SOCKET s)
{
    shutdown(s, SHUT_WR);
    close(s);
	return true;
}

bool agent::sendHandlePack(SOCKET s,tcpHandle* tcphandle)
{
    //Json::Value p_value;
    //p_value["mark"] = "1";
    //p_value["httpid"] = tcphandle->httpId;
    //p_value["cid"] = tcphandle->checkid;

    std::string p_str = "{\"mark\":\"1\",\"httpid\":"+ ntl_ntoT(tcphandle->httpId) +",\"cid\":" + ntl_ntoT(tcphandle->checkid) + "}";
    u_short p_mark = htons(1);
	int c = sizeof(p_mark);
    u_long	p_len = htonl(p_str.length() + sizeof(p_mark)); 
    std::string p_pack;
    p_pack.resize(4 + 2 + p_str.length());
    memcpy(&p_pack[0], &p_len, sizeof(p_len));
    memcpy(&p_pack[4], &p_mark, sizeof(p_mark));
    memcpy(&p_pack[6], p_str.c_str(), p_str.length());
    return sendSocket(s, &p_pack[0], p_pack.length());
}

SOCKET agent::getsockete(const char* ip, int port)
{
	SOCKET  p_ret= socket(AF_INET, SOCK_STREAM, 0);
	if (p_ret == -1)
	{
		return 0;
	}
	int p_buffSize = 256 * 1024;
	int optLen = sizeof(p_buffSize);
	setsockopt(p_ret, SOL_SOCKET, SO_RCVBUF, (char*)&p_buffSize, optLen);
	setsockopt(p_ret, SOL_SOCKET, SO_SNDBUF, (char*)&p_buffSize, optLen);

	p_buffSize = 1;
	setsockopt(p_ret, SOL_SOCKET, SO_REUSEADDR, (char*)&p_buffSize, optLen);

	sockaddr_in saddr = {};
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(port);
	saddr.sin_addr.s_addr = inet_addr(ip);
	
	struct timeval timeo = { };
	socklen_t len = sizeof(timeo);
	timeo.tv_usec = 700 * 1000;
	setsockopt(p_ret, SOL_SOCKET, SO_SNDTIMEO, &timeo, len);
	if (::connect(p_ret, (struct  sockaddr*)&saddr, sizeof(struct sockaddr)) != 0)
	{
		close(p_ret);
		return 0;
	}
	timeo.tv_sec =0;
	timeo.tv_usec = 0;
	setsockopt(p_ret, SOL_SOCKET, SO_SNDTIMEO, &timeo, len);
	return p_ret;
}
