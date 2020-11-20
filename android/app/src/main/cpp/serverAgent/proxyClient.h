#pragma once
#include "stdafx.h"
#include <atomic>
#include "clientAgent.h"

class proxyClient
{
public:
	proxyClient();
	~proxyClient();
	bool init(const char* host, const char* ip, const char* mark, bool connect, int lport);
	bool unInit();
	bool setConnect(bool connect);
private:
	bool		workThread();
	bool		connectThread(const char* ip, int port);

	bool		onloginRet(const std::string& data);//登录回包
	bool		onHttpHandle(const std::string& data);
	bool		onHandle(const std::string& data);

	bool		sendFirstPack(uint64_t httpid, FIRST_PACK_NUM type, const char* msg);
	bool		sendHttpMsg(uint64_t httpid, int httpcode, const char* msg);

	bool		makeFirstPack();

	bool		closeClient();

	bool		sendHandleShark();

	bool		onRecv(const std::string& pack);
	bool		sendData(const char* data,int len);

	bool		readPack();

	
	std::string		getsip();
	

	int					m_serverPort;
	bool				m_isLogin;
	std::atomic<bool>	m_isquit;
	std::atomic<bool>	m_isconnect;
	std::string			m_serverip; //负载均衡ip
	std::string			m_privetip;//真正的公网ip
	std::string			m_barid;
	std::string			m_ver;
	std::string			m_serverHost;
	std::string			m_firstPack;

	agent				m_serverAgent;
	std::string			m_pack;
	SOCKET				m_server;


};

