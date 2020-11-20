//
// Created by Administrator on 2019/12/30.
//
#include <jni.h>

#include <sys/types.h>
#include <linux/if_ether.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <cstring>
#include <thread>
#include <unistd.h>
#include "udphelplib.h"
#include <errno.h>
udphelp::udphelp() {
    m_socket = 0;
    m_sendaddr.sin_family = AF_INET;
    m_sendaddr.sin_port = 0;
}
udphelp::~udphelp() {
    udp_stop();
}

bool udphelp::udp_start(int port){
    Elog("udpinit");
    udp_stop();
    m_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (m_socket < 0) {
        return false;
    }
    sockaddr_in addr_serv = {};
    memset(&addr_serv, 0, sizeof(addr_serv));
    addr_serv.sin_family = AF_INET;
    addr_serv.sin_addr.s_addr = inet_addr("0.0.0.0");
    addr_serv.sin_port = htons(0);
    if (::bind(m_socket, (struct sockaddr *) &addr_serv, sizeof(sockaddr_in)) < 0) {
        return false;
    }
    m_recvThread=std::thread(&udphelp::udpRecvThread, this);
    m_recvThread.detach();
    return true;
}
bool udphelp::udp_connect(const char *ip, int remotePort) {
    Elog("udp_connect[%s][%d][%d]",ip,remotePort,m_socket.load());
    sockaddr_in addr_serv={};
    memset(&addr_serv, 0, sizeof(addr_serv));
    addr_serv.sin_addr.s_addr = inet_addr(ip);
    addr_serv.sin_port = htons(remotePort);
    addr_serv.sin_family=AF_INET;
    if( ::connect(m_socket.load(), (struct sockaddr *) &addr_serv, sizeof(addr_serv)) >= 0)
    {
        m_sendaddr.sin_port=addr_serv.sin_port;
        return true;
    }
    udp_stop();
    return false;
}

bool udphelp::udp_stop(){

    int p_s = m_socket.exchange(0);
    m_sendaddr.sin_port = 0;
    if (p_s && p_s != 0)
    {
        shutdown(p_s,SHUT_RDWR);
        ::close(p_s);
        Elog("udpc[%d]",p_s);
    }
    if(m_recvThread.joinable())
    {
        m_recvThread.join();
    }
    return true;
}

int udphelp::udp_getLocalPort()
{
    if (!m_socket)
    {
        return 0;
    }
    sockaddr_in   p_addr={};
    int32_t   p_iLen = sizeof(p_addr);
    getsockname(m_socket, (struct   sockaddr   *)&p_addr, (socklen_t*)&p_iLen);
    return ntohs( p_addr.sin_port);
}

bool udphelp::udpRecvThread() {
    std::string p_recvBuff;
    p_recvBuff.resize(4096);
    sockaddr_in p_addr = {};
    int32_t p_addrSize = sizeof(p_addr);
    int p_rlen = 0;
    while (true) {
        p_rlen = recvfrom(m_socket, &p_recvBuff[0], 4096, 0, (struct sockaddr *) &p_addr, (socklen_t*)&p_addrSize);
        if (p_rlen <1) {
            Elog("udprecvfromError[%s][%d]",strerror(errno),m_socket.load());
            break;
        }
        udp_OnRecv(p_addr,&p_recvBuff[0], p_rlen);
    }
    Elog("udpRecvThreadEnd");
    udp_stop();
    return true;
}

bool udphelp::udp_sendData(const char * ip, int remotePort, const char * data, int datalen)
{
    if (m_sendaddr.sin_port)
    {
        return udp_sendData(data,datalen);
    }
    Elog("udpudp_sendData[%s][%d]",ip,remotePort);
    sockaddr_in p_addr = {};
    p_addr.sin_family = AF_INET;
    p_addr.sin_addr.s_addr = inet_addr(ip);
    p_addr.sin_port = htons(remotePort);
    return sendto(m_socket, data, datalen, 0, (struct sockaddr *) &p_addr, sizeof(p_addr)) != -1;
}

bool udphelp::udp_sendData(const char * data, int datalen)
{
    if (!m_sendaddr.sin_port)
    {
        return false;
    }
    if( send(m_socket, data, datalen, 0) > 0)
    {
        return true;
    }
    return false;
}

extern "C"
JNIEXPORT bool
JNICALL
Java_com_example_lb_lbrocker_udphelp_udphelp_init(
        JNIEnv *env,
        jobject /* this */,jstring ip,int port) {
    std::string p_bip= ntl::jstringToChar(env,ip);
    //m_udphelp.init(p_bip.c_str(),port);
    return true;
}
