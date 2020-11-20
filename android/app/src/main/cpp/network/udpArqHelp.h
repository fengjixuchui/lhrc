#pragma once

#include <mutex>
#include <atomic>
#include <map>
#include <memory>
#include "publicbase/baseFunc.h"
#include "syncSendMgr.h"
extern "C" {
#include "ikcp.h"
}
class udpArqHelp: protected syncSendMgr
{
public:


	udpArqHelp();
	virtual ~udpArqHelp();

	bool arqInit();
	bool sendData(const char* data, int len);
	bool sendDataR(const char * data, int length, std::string & ret, int timeout);
	bool recvData(const char* data,int len);
	bool sendRecvData(uint32_t id, const char* data, int len);
	bool close();
	bool setNetworkPsw(const std::string& psw);
	virtual bool OnarqHelpClose();
	virtual bool OnRecv(const char* data, int len) = 0;
	virtual bool OnRecv(const char* data, int len,uint32_t retid) = 0;
	virtual bool OnSendData(const char* data, int len)=0;
private:


    bool syncOnRecv(const void* user,const char* data, int len,uint32_t retid);
    bool syncOnRecv(const void* user,const char* data, int len);
    bool syncOnSend(const void* user,const char* data, int len);


	bool OnKCPData(const char* data,unsigned int len);

	bool check(int t);
	static int	 output(const char *buf, int len, ikcpcb *kcp, void *user);

	std::atomic<int>			m_quitNum;
	std::atomic<bool>			m_isQuit;

	std::recursive_mutex		m_sendLock;
	std::string					m_recvBuff;
    std::string					m_sendBuff;
	std::string					m_networkPsw;
	ikcpcb*						m_ikcp;

};

