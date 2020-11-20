//
// Created by xiaoma on 2020/2/27.
//

#ifndef ROCKER_MASTER_P2P_UDP_H
#define ROCKER_MASTER_P2P_UDP_H

#include "udphelplib.h"
#include <atomic>
#include "deskTopDraw.h"
#include "e2eeMgr.h"

#define S_HOST {"kefuproxy1.110route.cn","kefuproxy2.110route.cn","kefuproxy3.110route.cn"}
#define S_IP {"106.15.103.97","47.107.149.9","182.92.76.127"}

typedef enum server_head
{
    server_rsa,
    server_rc4,
    server_repeat,
    server_ret,
    server_p2pS,
    server_p2pC,
    server_p2pC_ret
};

extern "C" {
#include "ikcp.h"
}
enum LOGIN_STATUS
{
    L_NOMER = 0,
    L_LOGIN = 1,
    L_PSW_ERROE = 2,
    L_TIMEOUT = 3,
    L_SYS_ERROR = 4,
    L_NO_USER,
    L_NOT_CONNECT_SERVER
};

class p2p_udp:
        public udphelp,
        public udpArqHelp,
        public e2eeMgr
{
public:

    enum SIGNAL_STATUS
    {
        START=0,
        STOP=1
    };

    p2p_udp();
    virtual ~p2p_udp();

    bool init(const char* sip, int sport,const char* uname, const char* psw);
    bool sendStateSignal(SIGNAL_STATUS signal);
    int  getPing();

private:

    bool connect(bool isShowMsg);
    bool p2p_sendDataToServer(server_head encrypt,const char* data, int len,std::string& ret ,int timeout);
    bool udp_makeServerSafeLink();//确保本机到中转服务器安全
    LOGIN_STATUS clogin(int& p2p, const char* uname);
    LOGIN_STATUS slogin(int& p2p, const char* uname);
    bool makeP2PS(std::string& ip ,int&  port);
    bool makeP2PC(std::string& ip, int& port);
    bool p2pOnRecvS(const sockaddr_in& addr, const char* data, int len);//收到s端之p2p包
    bool p2pOnRecvC(const sockaddr_in& addr, const char* data, int len); //收到c端之p2p包
    bool onServerRet(const sockaddr_in& addr, const char* data,int len);

    LOGIN_STATUS p2p_start(const char* sip, int sport,const char* uname, const char* psw ,uint32_t sid,int p2p);

    bool p2p_stop();
    bool p2p_OnClose();

    bool OnRecv(const char* data, int len);
    bool OnRecv(const char* data, int len,uint32_t retid);

    bool OnImg(const char* data,int datalen,uint32_t retid);
    bool OnSendData(const char* data, int len);

    std::string getRsaPublicKey();
    std::string loginStatus2Str(LOGIN_STATUS status);
    LOGIN_STATUS sendHelloPack();

    bool keepConnect();

    bool udp_OnRecv(const sockaddr_in& addr, const char* data, int len);



    bool setSyncData(uint32_t id, const char* data, int len);
    bool setSyncMap(sendVarPtr& ptr);
    bool delSyncData(uint32_t index);


    std::atomic<LOGIN_STATUS> m_loginStatus;


    int			m_sport;
    bool        m_quit;
    bool		m_isP2P;
    uint32_t    m_sid;
    uint32_t	m_turnID;

    std::string m_uname;
    std::string m_sip;
    std::string m_sendBuf;
    std::string m_rsaPublicKey;
    std::string	m_p2pCode;
    std::string m_psw;
    SIGNAL_STATUS               m_gameStatus;
    std::atomic<uint32_t>       m_sendIndex;
    std::atomic<int>            m_networkDTime;
    std::atomic<int>	        m_p2pport;
    std::vector<std::string>	m_sips;
    std::condition_variable		m_p2pEvent;
    std::mutex			        m_syncSendLock;

    std::map<uint32_t , sendVarPtr>	m_syncSendMap;
};


#endif //ROCKER_MASTER_P2P_UDP_H
