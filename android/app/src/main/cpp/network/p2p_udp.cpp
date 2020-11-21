//
// Created by xiaoma on 2020/2/27.
//

#include <endian.h>
#include <arpa/inet.h>
#include <audioPlayer.h>
#include <unistd.h>
#include <netdb.h>
#include "p2p_udp.h"
#include "publicbase/baseFunc.h"
#include "cryptopp/rc4.h."
#include "publicbase/md5.h"
#include "syncSendMgr.h"
#define  sendtoserverblock 100

extern  audioPlayer m_audioPlayer;

p2p_udp::p2p_udp():
        m_quit(false)
{

}


p2p_udp::~p2p_udp()
{
}
bool p2p_udp::connect(bool isShowMsg) {


    LOGIN_STATUS p_ls;
    int p_loginIndex=-1;
    int p_p2p =0;
    for (int i = 0; i < m_sips.size(); ++i) {
        p_ls=  p2p_start(m_sips[i].c_str(),m_sport,m_uname.c_str(),"",0,p_p2p);
        if(p_ls==LOGIN_STATUS::L_LOGIN)
        {
            p_loginIndex=i;
            break;
        }
        p_p2p=0;
    }

    if(p_loginIndex==-1){
        if(isShowMsg) {
            ntl::bridgeToWeb(("func=setLoginStatus&s=2&err=" + loginStatus2Str(p_ls)).c_str());
        }
        return false;
    }

    arqInit();
    p_ls=sendHelloPack();
    if(p_ls!= LOGIN_STATUS::L_LOGIN){
        if(isShowMsg) {
            ntl::bridgeToWeb(("func=setLoginStatus&s=2&err=" + loginStatus2Str(p_ls)).c_str());
        }
        return false;
    }
    if(isShowMsg) {
        ntl::bridgeToWeb("func=setLoginStatus&s=3");
        ntl::bridgeToJava("func=loginEnd");
    }
    sendStateSignal(m_gameStatus);
    return true;
}
bool p2p_udp::udp_makeServerSafeLink()
{
    std::string p_psw = e2eeMgr::getRandPsw();
    std::string p_str = "func=makeSafeLink&psw=" + p_psw;
    std::string p_ret;
    p2p_sendDataToServer(server_rsa, &p_str[0], p_str.length(), p_ret, 1000);
    return p_ret == "ok";
}

LOGIN_STATUS p2p_udp::clogin(int& p2p, const char* uname)
{
    char loginData[256] = {};
    sprintf(loginData, "func=cloginv3&uname=%s&sid=%u&ret=1&p2p=%d&", uname, m_sid, p2p);
    std::string p_str = loginData + getPublicKey();
    std::string p_ret;
    if (!p2p_sendDataToServer(server_head::server_rc4, &p_str[0], p_str.length(), p_ret, 1000))
    {
        return L_SYS_ERROR;
    }
    int p_status = webbase::post_i(p_ret, "status");
    if (p_status != L_LOGIN)
    {
        return (LOGIN_STATUS)p_status;
    }
    m_turnID = m_sid;
    std::string  p_cip;
    int p_cport;
    p_cip = webbase::post(p_ret, "cip", false);
    p_cport = webbase::post_i(p_ret, "cport");
    m_isP2P = p2p;
    m_sendBuf.resize(1024);
    m_sendBuf[0] = server_repeat;
    memcpy(&m_sendBuf[1], &m_turnID, 4);
    if (!p2p || !makeP2PC(p_cip, p_cport))
    {
        m_isP2P = false;
        p_cip = m_sip;
        p_cport = m_sport;
    }
    return udp_connect(p_cip.c_str(), p_cport) ? L_LOGIN : L_SYS_ERROR;
}

LOGIN_STATUS p2p_udp::slogin(int& p2p, const char* uname)
{
    char loginData[256] = {};
    sprintf(loginData, "func=sloginv3&uname=%s&p2p=%d", uname, p2p);
    std::string p_ret;
    if (!p2p_sendDataToServer(server_head::server_rc4, loginData, strlen(loginData), p_ret, 2000))
    {
        return L_SYS_ERROR;
    }
    int p_status = webbase::post_i(p_ret, "status");
    if (p_status != L_LOGIN)
    {
        return (LOGIN_STATUS)p_status;
    }
    m_turnID = webbase::post_ui(p_ret, "cid");
    if (m_turnID == 0)
    {
        return L_SYS_ERROR;
    }
    std::string  p_cip;
    int p_cport;
    p_cip = webbase::post(p_ret, "cip", false);
    p_cport = webbase::post_i(p_ret, "cport");
    m_rsaPublicKey = "seed="+ntl::getlastValue(p_ret,"seed=");
    m_sendBuf.resize(1024);
    m_sendBuf[0] = server_repeat;
    ntl::reverseWord(m_turnID);
    memcpy(&m_sendBuf[1], &m_turnID, 4);

    if (!p2p || !makeP2PS(p_cip, p_cport))
    {
        p2p = false;
        p_cip = m_sip;
        p_cport = m_sport;
    }
    m_isP2P = p2p;
    return udp_connect(p_cip.c_str(), p_cport) ? L_LOGIN : L_SYS_ERROR;
}

std::string p2p_udp::getRsaPublicKey()
{
    return m_rsaPublicKey;
}

bool p2p_udp::makeP2PS( std::string& ip, int& port)
{

    std::string p_data;
    m_p2pCode = ntl::getRandStr(20,30);
    std::string p_rsaKey = getRsaPublicKey();
    std::string p_seed = webbase::post(p_rsaKey, "seed");
    p_rsaKey = ntl::getlastValue(p_rsaKey, "publickey=");

    p_data.resize(1);
    p_data[0] = server_head::server_p2pS;
    p_data+= e2eeMgr::encode(m_p2pCode.c_str(), m_p2pCode.length(), p_seed, p_rsaKey);
    m_p2pport = 0;
    m_p2pEvent.notify_all();
    int p_nport = port;
    struct timespec tm;
    tm.tv_sec = 0;
    tm.tv_nsec = 200 * 1000 * 1000;
    nanosleep(&tm, nullptr);
    udp_sendData(ip.c_str(), p_nport, &p_data[0], p_data.length());
    int p_selfport = udp_getLocalPort();
    for (size_t i = 0; i < 2; i++)
    {
        p_nport = port - 5;
        for (size_t i = 0; i < 10; i++)
        {
            p_nport++;
            if (!udp_sendData(ip.c_str(), p_nport, &p_data[0], p_data.length()))
            {
                return false;
            }
        }
    }
    std::cv_status p_waitStatus;
    {
        std::unique_lock<std::mutex> lock{m_syncSendLock};
        p_waitStatus = m_p2pEvent.wait_for(lock, std::chrono::milliseconds(1000));
    }
    if (m_p2pport != 0)
    {
        port = m_p2pport;
        return true;
    }
    return false;
}

bool p2p_udp::makeP2PC(std::string& ip, int& port) {
    std::string p_data;
    p_data.resize(8);
    p_data[0] = server_head::server_p2pC;
    int p_nport = port;
    m_p2pport = 0;
    m_p2pEvent.notify_all();
    udp_sendData(ip.c_str(), p_nport, &p_data[0], p_data.length());
    for (size_t i = 0; i < 2; i++) {
        p_nport = port - 5;
        for (size_t i = 0; i < 10; i++) {
            if (!udp_sendData(ip.c_str(), p_nport++, &p_data[0], p_data.length())) {
                return false;
            }
        }
    }
    std::cv_status p_waitStatus;
    {
        std::unique_lock<std::mutex> lock{m_syncSendLock};
        p_waitStatus = m_p2pEvent.wait_for(lock, std::chrono::milliseconds(800));
    }
    if (p_waitStatus == std::cv_status::no_timeout && m_p2pport !=0)
    {
        port = m_p2pport;
        return true;
    }
    return false;
}

bool p2p_udp::p2pOnRecvS(const sockaddr_in& addr, const char* data, int len)
{
    std::string p_data(data + 1, len - 1);
    p_data = e2eeMgr::rsaDecode(p_data);
    if (p_data == "")
    {
        return true;
    }
    std::string p_ret = "p2psuccess";
    rc4::encrypt(p_ret.c_str(), p_ret.length(), p_data.c_str(), p_data.length());
    p_ret = char(server_head::server_p2pC_ret) + p_ret;
    udp_sendData(inet_ntoa(addr.sin_addr), ntohs(addr.sin_port), &p_ret[0], p_ret.length());
    m_p2pport = ntohs(addr.sin_port);
    m_p2pEvent.notify_all();
    return true;
}

bool p2p_udp::p2pOnRecvC(const sockaddr_in& addr, const char* data, int len)
{
    if (m_p2pCode.empty())
    {
        return true;
    }
    rc4::encrypt(data + 1, len - 1,m_p2pCode.c_str(),m_p2pCode.length());
    std::string p_data(data + 1, len - 1);
    if (p_data == "p2psuccess")
    {
        m_p2pport = ntohs(addr.sin_port);
        m_p2pEvent.notify_all();
        return true;
    }
    return true;
}


bool p2p_udp::p2p_sendDataToServer(server_head encrypt, const char* data, int len, std::string& ret, int timeout)
{

    int p_sendnum = timeout / sendtoserverblock;
    if (timeout % sendtoserverblock > 0)
    {
        p_sendnum++;
    }
    std::string p_data;
    p_data.resize(len + 6);
    sendVarPtr p_sendVar;
    setSyncMap(p_sendVar);


    p_data[0] = encrypt;
    p_data[1] = syncSendMgr::SEND_WAIT;
    memcpy(&p_data[2], &(p_sendVar->clientId), 4);
    memcpy(&p_data[6], data, len);

    if (encrypt == server_rsa)
    {
        std::string p_temp = e2eeMgr::rsaEndcodeToServer(&p_data[1], p_data.length()-1);
        p_data.resize(1);
        p_data.append(p_temp);
    }

    else if (encrypt == server_rc4)
    {
        if (!e2eeMgr::rc4Encode(&p_data[1], p_data.length() - 1))
        {
            return false;
        }
    }
    uint32_t p_singleRet;
    std::cv_status p_waitStatus;
    for (size_t i = 0; i < p_sendnum; i++)
    {
        if (!udp_sendData(m_sip.c_str(),m_sport, &p_data[0], p_data.length()))
        {
            return false;
        }

        {
            std::unique_lock<std::mutex> lock{p_sendVar->eventLock};
            p_waitStatus=p_sendVar->event.wait_for(lock,std::chrono::milliseconds(sendtoserverblock));
        }

        if(p_waitStatus==std::cv_status::timeout)
        {
            continue;
        } else  if (p_sendVar->retBin)
        {
            ret = *(p_sendVar->retBin);
            delSyncData(p_sendVar->clientId);
            return true;
        }
    }
    delSyncData(p_sendVar->clientId);
    return false;
}



LOGIN_STATUS p2p_udp::p2p_start(const char * sip, int sport, const char * uname, const char * psw, uint32_t sid, int p2p)
{
    m_loginStatus = L_NOMER;

    m_sip = sip;
    m_sport = sport;
    if (!udp_start())
    {
        return L_SYS_ERROR;
    }
    if(!udp_makeServerSafeLink())
    {
        return L_NOT_CONNECT_SERVER;
    }

    m_sid = sid;

    if (sid != 0)//clogin
    {
        e2eeMgr::init();
        return clogin(p2p, uname);
    }
    return slogin(p2p, uname);
}

bool p2p_udp::p2p_stop()
{
    m_quit=true;
    udphelp::udp_stop();
    udpArqHelp::close();
    return true;
}
bool p2p_udp::p2p_OnClose() {

    return true;
}

bool p2p_udp::sendStateSignal(p2p_udp::SIGNAL_STATUS signal) {
    m_gameStatus=signal;
    char p_str[64]={};
    sprintf(p_str,"fun=setState&signal=%d",signal);
    Elog("%s\r\n",p_str);
    sendData(p_str,strlen(p_str));
    return true;
}
int p2p_udp::getPing() {
    return m_networkDTime.load();
}

bool p2p_udp::udp_OnRecv(const sockaddr_in & addr, const char * data, int len)
{
    if (len < 6)
    {
        return false;
    }
    switch (data[0])
    {
        case server_head::server_ret:return onServerRet(addr, data, len);
        case server_head::server_repeat: return recvData(data + 5, len - 5);
        case server_head::server_p2pS: return p2pOnRecvS(addr,data, len);
        case server_head::server_p2pC_ret: return p2pOnRecvC(addr, data, len);
        default:
            break;
    }
    return true;
}

bool p2p_udp::onServerRet(const sockaddr_in& addr, const char* data,int len)
{
    rc4Encode(data+1, len - 1);
    uint32_t * p_retid = (uint32_t *)(data+1);
    return setSyncData(*p_retid, data+5,len -5);
}


bool p2p_udp::keepConnect() {
    std::string p_data="fun=checkQuality";
    std::string p_ret;
    /*
    char* p_shost[] = S_HOST;
    char* p_sip[] = S_IP;
    for (size_t i = 0; i < sizeof(p_sip) / sizeof(char*); i++)
    {
        hostent* p_host = gethostbyname(p_shost[i]);
        if (p_host != NULL && p_host->h_length == 4 && p_host->h_addr_list[0])
        {
            m_sips.push_back(inet_ntoa(*((struct in_addr*)p_host->h_addr_list[0])));
        }
        else
        {
            m_sips.push_back(p_sip[i]);
        }
    }
*/
    srand(ntl::timeGetTime()+rand());
    m_sips.push_back("129.204.163.30");
    std::random_shuffle(m_sips.begin(), m_sips.end());
    if(!connect(true)){
        return false;
    }
    uint32_t  p_time=0;
    while (!m_quit){
        p_time=ntl::timeGetTime();
        if(!sendDataR(p_data.c_str(),p_data.length(),p_ret,5000))
        {
            m_networkDTime=-1;
            if(!connect(false)){
                sleep(5);
            }
            //Elog("udpclose");
        } else{
            m_networkDTime=ntl::timeGetTime()-p_time;
            Elog("udpnetworkDTime[%d]",m_networkDTime.load());
        }
        sleep(1);
    }
    return true;
}

bool p2p_udp::OnRecv(const char* data, int len){
    static std::string p_data;
    int p_nlen = len > 512 ? 512 : len;
    if (p_data.length() != p_nlen) {
        p_data.resize(p_nlen);
    }
    memcpy(&p_data[0], data, p_nlen);
    std::string p_fun = webbase::post(p_data, "fun");

    if(p_fun=="OnVoice")
    {
        m_audioPlayer.pushData(data,len);
    }   else if(p_fun=="OnImg")
    {
        OnImg(data,len,0);
    }else if(p_fun=="handShark")
    {

    }
    return true;
}
bool p2p_udp::OnRecv(const char* data, int len,uint32_t retid)
{
    static std::string p_data;
    int p_nlen = len > 512 ? 512 : len;
    if (p_data.length() != p_nlen) {
        p_data.resize(p_nlen);
    }
    memcpy(&p_data[0], data, p_nlen);
    std::string p_fun = webbase::post(p_data, "fun");
    Elog("onFunc[%s][%d]",p_fun.c_str(),retid);
    if(p_fun=="OnImg")
    {
        OnImg(data,len,retid);
    }
    return true;
}
bool p2p_udp::OnImg(const char* data,int datalen,uint32_t retid)
{
    strPtr p_ptr=std::make_shared<std::string>(data,datalen);
    mRenderer.requestFunc(std::bind(&deskTopDraw::OnImg,&g_deskTopDraw,p_ptr,retid));
    return true;
}

bool p2p_udp::OnSendData(const char *data, int len) {
    std::lock_guard<std::mutex> p_lock(m_syncSendLock);
    if (m_sendBuf.length() < len + 5) {
        m_sendBuf.resize(len + 6);
    }
    memcpy(&m_sendBuf[5], data, len);
    return udp_sendData(&m_sendBuf[0], len + 5);
}

std::string p2p_udp::loginStatus2Str(LOGIN_STATUS status) {
    switch (status){
        case L_PSW_ERROE:
            return "密码错误";
        case L_TIMEOUT:
            return "连接超时";
        case L_SYS_ERROR:
            return "系统出错";
        case L_NO_USER:
            return "用户不存在";
        case L_NOMER:
            return "通信异常";
        case L_NOT_CONNECT_SERVER:
            return "无法连接到服务器";
    }
return "";
}

bool p2p_udp::init(const char *sip, int sport, const char *uname, const char *psw) {
    m_sip=sip;
    m_psw=psw;
    m_sport=sport;
    m_uname=uname;
    std::thread(&p2p_udp::keepConnect,this).detach();
    return true;
}
LOGIN_STATUS p2p_udp::sendHelloPack()
{
    std::string p_ret;
    std::string p_networkPsw = ntl::getRandStr(20, 24);
    std::string p_str = "uname=" + m_uname + "&psw=" + ntl::md5(m_psw) + "&rc4psw=" + p_networkPsw;
    std::string p_rsaKey = getRsaPublicKey();
    std::string p_seed = webbase::post(p_rsaKey, "seed");
    p_rsaKey = ntl::getlastValue(p_rsaKey, "publickey=");
    setNetworkPsw("");
    if (p_rsaKey.empty())
    {
        p_rsaKey = "";
    }
    p_str =  e2eeMgr::encode(p_str.c_str(), p_str.length(), p_seed.c_str(), p_rsaKey);
    p_str = "fun=shello&data=" + p_str;
    sendDataR(&p_str[0], p_str.length(), p_ret, 2000);
    if (p_ret.empty())
    {
        return L_PSW_ERROE;
    }
    if (p_ret == "psw_error")
    {
        return L_PSW_ERROE;
    }
    setNetworkPsw(p_networkPsw);
    if (p_ret == "ok")
    {
        return L_LOGIN;
    }
    return L_SYS_ERROR;
}


bool p2p_udp::setSyncData(uint32_t id, const char* data ,int len)
{
    if (len <1 )
    {
        return false;
    }
    std::lock_guard<std::mutex>p_lock(m_syncSendLock);
    auto p_it = m_syncSendMap.find(id);
    if (p_it == m_syncSendMap.end())
    {
        return false;
    }
    p_it->second->retBin = std::make_shared<std::string>(data, len);
    p_it->second->event.notify_all();
    return true;
}

bool p2p_udp::setSyncMap(sendVarPtr& ptr)
{
    std::lock_guard<std::mutex>p_lock(m_syncSendLock);
    ptr = std::make_shared<sendVar>();
    ptr->clientId = m_sendIndex++;
    m_syncSendMap[ptr->clientId] = ptr;
    return true;
}

bool p2p_udp::delSyncData(uint32_t index)
{
    std::lock_guard<std::mutex>p_lock(m_syncSendLock);
    auto p_it = m_syncSendMap.find(index);
    if (p_it == m_syncSendMap.end())
    {
        return false;
    }
    m_syncSendMap.erase(p_it);
    return true;
}

