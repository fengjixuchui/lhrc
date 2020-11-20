//
// Created by admin on 2020/11/20.
//

#ifndef ROCKER_MASTER_SYNCSENDMGR_H
#define ROCKER_MASTER_SYNCSENDMGR_H
#include <atomic>
#include <memory>
#include <string>
#include <mutex>
#include <map>
#include <publicbase/baseFunc.h>


class syncSendMgr
{
public:

    enum PACK_MARK
    {
        SEND_WAIT = 0,		// 同步發送
        SEND_WAIT_RET = 1,	// 回應同步發送
        SEND_NOT_WAIT,
    };

    syncSendMgr();
    virtual ~syncSendMgr();

    bool send(const void* user,const char* data, int len, std::string& ret, int timeout);
    bool send(const void* user, const char* data, int len);

    bool sendRecv(const void* user, uint32_t id, const char* data, int len);

    bool onPushData(const void* user,const char* data, int len);
private:
    virtual bool syncOnRecv(const void* user,const char* data, int len,uint32_t retid);
    virtual bool syncOnRecv(const void* user,const char* data, int len);
    virtual bool syncOnSend(const void* user,const char* data, int len)=0;

    bool setSyncData(uint32_t id, const char* data, int len);
    bool setSyncMap(sendVarPtr& ptr);
    bool delSyncData(uint32_t index);



    uint32_t						m_sendIndex;
    std::mutex						m_syncSendLock;
    std::map<uint32_t , sendVarPtr>	m_syncSendMap;
};


#endif //ROCKER_MASTER_SYNCSENDMGR_H
