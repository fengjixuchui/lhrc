
#include "udpArqHelp.h"
#include <thread>
#include <unistd.h>
#include <mutex>
#include <rc4.h>


udpArqHelp::udpArqHelp()
{
	m_ikcp = nullptr;
	m_quitNum = 0;
}

udpArqHelp::~udpArqHelp()
{
	close();
}

bool udpArqHelp::arqInit()
{
	close();
	m_ikcp = ikcp_create(1,this);
	if (m_ikcp == NULL)
	{
		return false;
	}
	m_isQuit = false;

	ikcp_nodelay(m_ikcp, 1, 20, 3, 1);
	m_ikcp->rx_minrto = 10;
	m_ikcp->fastresend = 1;
	::ikcp_wndsize(m_ikcp, 1270, 1280);
	::ikcp_setmtu(m_ikcp, 1394);
	m_ikcp->output = udpArqHelp::output;
	m_recvBuff.resize(1024 * 1024);
	m_quitNum++;
	std::thread(&udpArqHelp::check, this,20*1000*1000).detach();
	return true;
}


bool udpArqHelp::sendDataR(const char *data, int len, std::string &ret, int timeout) {
	return syncSendMgr::send(nullptr,data, len, ret, timeout);
}

bool udpArqHelp::setNetworkPsw(const std::string& psw)
{
    std::lock_guard<std::recursive_mutex>p_lock(m_sendLock);
    m_networkPsw = psw;
    return true;
}


bool udpArqHelp::syncOnRecv(const void* user,const char* data, int len, uint32_t retid)
{
	return OnRecv(data, len, retid);
}

bool udpArqHelp::syncOnRecv(const void* user,const char* data, int len)
{
	return OnRecv(data,len);
}

bool udpArqHelp::syncOnSend(const void* user,const char* data, int len)
{
	std::lock_guard<std::recursive_mutex>p_lock(m_sendLock);
	if (!m_ikcp)
	{
		return false;
	}
	if (!m_networkPsw.empty())
	{
		if (m_sendBuff.length() < len)
		{
			m_sendBuff.resize(len);
		}
		memcpy(&m_sendBuff[0], data, len);
		rc4::encrypt(&m_sendBuff[0], maxrc4(len), m_networkPsw.c_str(), m_networkPsw.length());
		if (ikcp_send(m_ikcp, &m_sendBuff[0], len) != 0)
		{
			return false;
		}
	}
	else
	{
		if (ikcp_send(m_ikcp, data, len) != 0)
		{
			return false;
		}
	}

	::ikcp_flush(m_ikcp);
	return true;
}

bool udpArqHelp::sendData(const char * data, int len)
{
	return syncSendMgr::send(nullptr, data, len);
}

bool udpArqHelp::sendRecvData(uint32_t id, const char *data, int len) {
	return syncSendMgr::sendRecv(nullptr, id, data, len);
}
bool udpArqHelp::recvData(const char * data, int len)
{
	std::lock_guard<std::recursive_mutex>p_lock(m_sendLock);
	if (!m_ikcp)
	{
		return false;
	}
	if (ikcp_input(m_ikcp, data, len) != 0)
	{
		return false;
	}
	while (true)
	{
		int iRead = ::ikcp_recv(m_ikcp, &m_recvBuff[0], m_recvBuff.length());
		if (iRead >= 5)
		{
			OnKCPData(&m_recvBuff[0], iRead);
		}
		else if (iRead == -3)
		{
			return false;
		}
		else
			break;
	}
	::ikcp_flush(m_ikcp);
	return true;
}

bool udpArqHelp::close()
{
	m_isQuit = true;
	struct timespec tm;
	tm.tv_sec=0;
	tm.tv_nsec=10*1000*1000;
	for (size_t i = 0; i < 1000; i++)
	{
		if (m_quitNum == 0)
		{
			break;
		}
		nanosleep (&tm, nullptr);
	}
    {
        std::lock_guard<std::recursive_mutex> p_lock(m_sendLock);
        {
            if (m_ikcp) {
                ::ikcp_release(m_ikcp);
                m_ikcp = NULL;
            }
        }
    }
	return true;
}

bool udpArqHelp::OnarqHelpClose()
{
	return true;
}


bool udpArqHelp::OnKCPData(const char* data ,unsigned int len)
{
	if (!m_networkPsw.empty())
	{
		rc4::encrypt(data, maxrc4(len), m_networkPsw.c_str(), m_networkPsw.length());
	}
	return syncSendMgr::onPushData(nullptr,data,len);
}

bool udpArqHelp::check(int t)
{
	struct timespec tm;
	tm.tv_sec=0;
	tm.tv_nsec=t;
	while (!m_isQuit)
	{
		nanosleep(&tm, nullptr);

		m_sendLock.lock();
		if (m_ikcp)
		{
			ikcp_update(m_ikcp, ntl::timeGetTime());
		}
		m_sendLock.unlock();
	}
	m_quitNum--;
	OnarqHelpClose();
	return true;
}

int udpArqHelp::output(const char * buf, int len, ikcpcb * kcp, void * user)
{
	udpArqHelp* p_arqPtr = (udpArqHelp*)user;
	if (!p_arqPtr->OnSendData(buf, len))
	{
		return 1;
	}
	return 0;
}
