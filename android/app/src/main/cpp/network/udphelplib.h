//
// Created by Administrator on 2019/12/30.
//

#pragma once
#include <sys/socket.h>
#include <atomic>
#include <linux/in.h>
#include "udpArqHelp.h"

class udphelp  {
public:

    udphelp();
    virtual ~udphelp();
    bool udp_start(int port=0);
    bool udp_connect(const char * ip, int remotePort);

    bool udp_sendData(const char * ip, int remotePort, const char* data, int datalen);
    bool udp_sendData(const char* data, int datalen);


    int  udp_getLocalPort();
    bool udp_stop();
    sockaddr_in   m_sendaddr;
private:

    bool udpRecvThread();
    virtual bool udp_OnRecv(const sockaddr_in& addr, const char* data, int len)=0;
    std::atomic<int>    m_socket;
    std::thread m_recvThread;

};


