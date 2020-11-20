#include "stdafx.h"
#include "proxyClient.h"
#include <thread>
#include <unistd.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <netdb.h>
proxyClient::proxyClient()
	:m_isquit(true),
	m_server(-1)
{

}

proxyClient::~proxyClient()
{
}

bool proxyClient::init(const char* host, const char* ip, const char* mark, bool connect, int lport)
{

	if (!m_isquit)
	{
		return false;
	}
	m_isLogin = false;
	m_isquit = false;
	m_isconnect = connect;

	m_serverip =ip;
	m_serverHost = host;
	m_barid = mark;
	m_serverPort = 8301;
	if ((m_serverip == "" && m_serverHost == "") || m_barid == "" || m_barid.length() > 32 || m_serverPort == 0)
	{
		return false;
	}
	m_serverAgent.inti();
	makeFirstPack();
	m_pack.reserve(1024 * 65);
	std::thread(&proxyClient::workThread, this).detach();
	return true;
}

bool proxyClient::unInit()
{
	return false;
}

bool proxyClient::setConnect(bool connect)
{
	return false;
}

bool proxyClient::workThread()
{
	int p_num = rand() % 100;//防止外面机器大批量重启都集中在了同一时间打点
	int p_desNum = 120;
	std::string p_sip;



#ifndef WIN32
	sigset_t signal_mask;
	sigemptyset(&signal_mask);
	sigaddset(&signal_mask, SIGPIPE);
	int rc = pthread_sigmask(SIG_BLOCK, &signal_mask, NULL);
	if (rc != 0) {
		printf("block sigpipe error\n");
	}
#endif

	while (true)
	{
		if (!m_isconnect)
		{
			sleep(1);
			continue;
		}
		if (readPack())
		{
			continue;
		}
		if (connectThread(getsip().c_str(), 8301) && sendHandleShark())
		{
			continue;
		}
		closeClient();
		sleep(5);
	}
	m_isquit = true;
	return true;
}

bool proxyClient::connectThread(const char* ip ,int port )
{
	if (m_server != -1)
	{
		close(m_server);
	}
	m_server = socket(AF_INET, SOCK_STREAM, 0);
	if (m_server == -1)
	{
		return false;
	}
	int p_buffSize = 32 * 1024;
	int optLen = sizeof(p_buffSize);
	setsockopt(m_server, SOL_SOCKET, SO_RCVBUF, (char*)&p_buffSize, optLen);
	setsockopt(m_server, SOL_SOCKET, SO_SNDBUF, (char*)&p_buffSize, optLen);

	p_buffSize = 1;
	setsockopt(m_server, SOL_SOCKET, SO_REUSEADDR, (char*)&p_buffSize, optLen);

	sockaddr_in saddr = {};
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(port);
	saddr.sin_addr.s_addr = inet_addr(ip);
	struct timeval timeo = { };
	socklen_t len = sizeof(timeo);
	timeo.tv_usec = 700 * 1000;
	setsockopt(m_server, SOL_SOCKET, SO_SNDTIMEO, &timeo, len);
	if (::connect(m_server, (struct  sockaddr*)&saddr, sizeof(struct sockaddr)) != 0)
	{
		close(m_server);
		return 0;
	}
	timeo.tv_sec = 0;
	timeo.tv_usec = 0;
	setsockopt(m_server, SOL_SOCKET, SO_SNDTIMEO, &timeo, len);
	return true;
}

bool proxyClient::onloginRet(const std::string& data)
{
	m_isLogin = true;
	std::string p_str = data.substr(tcp_handel_len);
	m_privetip = ntl_post(p_str, "sip");
	return true;
}

bool proxyClient::onHttpHandle(const std::string& data)
{
	tcpHandle* p_tcpHandle = (tcpHandle*)&data[0];
	CONNID p_aid = 0;
	std::string p_str = data.substr(tcp_handel_len);
	std::string p_func = ntl_post(p_str, "func");
	if (p_func == "test")
	{
		sendHttpMsg(p_tcpHandle->httpId,200,"ok");
		return true;
	}
	sendHttpMsg(p_tcpHandle->httpId, 404, "not func");
	return true;
}

bool proxyClient::onHandle(const std::string& data)
{
	tcpHandle* p_tcpHandle = (tcpHandle*)&data[0];
	CONNID p_aid = 0;
	std::string p_str = data.substr(tcp_handel_len);

	std::string p_aip = ntl_post(p_str,"sip");
	int p_aport =atoi(ntl_post(p_str ,"sport").c_str());
	if (p_aip == "" || p_aport == 0)
	{
		sendFirstPack(p_tcpHandle->httpId, E_PARAM, "aip or aport error");
		return true;
	}
	p_str = "{\"err\":0,\"msg\":\"sucess\"}";
	uint32_t p_len = p_str.length();
	std::string p_buff;
	p_buff.resize(p_len + 4);
	memcpy(&p_buff[4], &p_str[0], p_len);
	p_len = htonl(p_len);
	memcpy(&p_buff[0], &p_len, 4);
	if (!m_serverAgent.dowork(p_tcpHandle, p_buff, "", m_privetip.c_str(), 8305, p_aip.c_str(), p_aport))
	{
		sendFirstPack(p_tcpHandle->httpId, E_CONNECT_FAILED, "make agent failed");
	}
	return true;
	
}


bool proxyClient::sendFirstPack(uint64_t httpid, FIRST_PACK_NUM type, const char* msg)
{
	std::string p_str;
	p_str = "{\"err\":" + ntl_ntoT(type) + ",\"msg\":\"" + msg + "\"}";
	tcpHandle p_handle = {};
	p_handle.type = send_to_httpid;
	p_handle.httpId = httpid;

	std::string p_buf;
	p_buf.resize(8);
	int32_t p_len = tcp_handel_len + p_str.length();

	memcpy(&p_buf[0], &p_len, 4);
	p_len = htonl(p_str.length());
	memcpy(&p_buf[4], &p_len, 4);
	p_buf.append((char*)&p_handle, tcp_handel_len);
	p_buf.append(p_str);
	return sendData(&p_buf[0], p_buf.length());
}

bool proxyClient::sendHttpMsg(uint64_t httpid, int httpcode, const char* msg)
{
	int p_len = strlen(msg);
	std::string p_str = "HTTP/1.1 " + ntl_ntoT(httpcode) + " OK\r\nServer: ubantu\r\nContent-Type: application/json\r\nContent-Length:" + ntl_ntoT(p_len) + "\r\n\r\n";
	p_str += msg;

	tcpHandle p_handle = {};
	p_handle.type = send_to_httpid;
	p_handle.httpId = httpid;

	std::string p_buf;
	p_buf.resize(4);
	 p_len = tcp_handel_len + p_str.length();
	int c = sizeof(int);
	memcpy(&p_buf[0], &p_len, 4);
	p_buf.append((char*)&p_handle, tcp_handel_len);
	p_buf.append(p_str);
	sendData(&p_buf[0], p_buf.length());
	return true;
}


bool proxyClient::makeFirstPack()
{
	std::string p_str = "{\"mark\":\"" + m_barid + "\"}";
	u_short p_mark = htons(1);
	u_long	p_len =p_str.length() + sizeof(u_short)+4;
	m_firstPack.resize(p_len);
	p_len = htonl(p_len);
	memcpy(&m_firstPack[0], &p_len, sizeof(p_len));
	memcpy(&m_firstPack[4], &p_mark, sizeof(p_mark));
	memcpy(&m_firstPack[6], p_str.c_str(), p_str.length());
	return true;
}

bool proxyClient::closeClient()
{
	if (m_server == -1)
	{
		return true;
	}
	close(m_server);
	m_server = -1;
	return true;
}

bool proxyClient::sendHandleShark()
{
	std::string p_buf;
	std::string p_pram = "&ver=" + m_ver;
	unsigned short p_pramLen = p_pram.length();
	p_buf.resize(tcp_handel_len + 2 +4);
	tcpHandle* p_handle = (tcpHandle*)&p_buf[4];
	p_handle->type = send_login_v3;
	uint p_len = tcp_handel_len + 2 + p_pramLen;
	//p_len = htonl(p_len);

	int c = tcp_handel_len;
	//int  p_len =tcp_handel_len + 2 + p_pramLen;
	memcpy(&p_buf[0], &p_len, 4);
	memcpy(p_handle->barid, m_barid.c_str(), m_barid.length());
	memcpy(&p_buf[tcp_handel_len+4], &p_pramLen, 2);
	p_buf.append(p_pram);
	return sendData(&p_buf[0], p_buf.length());
}

bool proxyClient::onRecv(const std::string& pack)
{
	if (pack.length() < tcp_handel_len)
	{
		return true;
	}
	switch (pack[0])
	{
	case recv_login:return onloginRet(pack);
	case send_handle:return onHandle(pack);
	case send_http_handle:return onHttpHandle(pack);
	default:
		break;
	}
	return true;
}

bool proxyClient::sendData(const char* data, int len)
{
	int p_sendLen = 0;
	ssize_t p_i = 0;
	while (len > 0)
	{
		p_i = ::send(m_server, data + p_sendLen, len, 0);
		if (p_i == -1)
		{
			return false;
		}
		len -= p_i;
		p_sendLen += p_i;
	}
	return true;
}

bool proxyClient::readPack()
{
	int p_packLen = sizeof(uint32_t);
	uint32_t	p_len = 0;
	int p_readlen = 0;
	ssize_t  p_i = 0;
	while (p_packLen > 0)
	{
		p_i = ::recv(m_server, ((char*)&p_len) + p_readlen, p_packLen, 0);
		if (p_i < 1)
		{
			return false;
		}
		p_packLen -= p_i;
		p_readlen += p_i;
	}
	//p_len = ntohl(p_len);
	if (p_len > 65536 || p_len < 1)
	{
		return false;
	}
	m_pack.resize(p_len);
	p_readlen = 0;
	while (p_len > 0)
	{
		p_i = ::recv(m_server, &m_pack[p_readlen], p_len, 0);
		if (p_i < 1)
		{
			return false;
		}
		p_len -= p_i;
		p_readlen += p_i;
	}
	if (onRecv(m_pack))
	{
		return true;
	}
	return false;
}

std::string proxyClient::getsip()
{
	if (m_serverHost == "")
	{
		return m_serverip;
	}
	//return m_serverHost;
	
	hostent* p_host = gethostbyname(m_serverHost.c_str());
	if (p_host != NULL && p_host->h_length == 4 && p_host->h_addr_list[0])
	{
		return inet_ntoa(*((struct in_addr*)p_host->h_addr_list[0]));
	}
	return m_serverip;
}
