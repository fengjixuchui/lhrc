//
// Created by admin on 2020/11/20.
//

#include "syncSendMgr.h"



syncSendMgr::syncSendMgr():m_sendIndex(0)
{

}

syncSendMgr::~syncSendMgr()
{
    std::lock_guard<std::mutex>p_lock(m_syncSendLock);
    m_syncSendMap.clear();
}


bool syncSendMgr::send(const void* user, const char* data, int len, std::string& ret, int timeout)
{

    stringPtr p_data;
    p_data=ntl::makeStringPtr(len+5);
    char* p_addr =&(*p_data)[0];
    sendVarPtr p_sendVar;
    setSyncMap(p_sendVar);
    p_addr[0] = SEND_WAIT;
    memcpy(p_addr+5, data, len);
    memcpy(p_addr+1, &(p_sendVar->clientId), 4);
    if (!syncOnSend(user, p_addr, len+5))
    {
        delSyncData(p_sendVar->clientId);
        return false;
    }

    std::unique_lock<std::mutex> p_lock( p_sendVar->eventLock );
    std::cv_status p_waitStatus=p_sendVar->event.wait_for(p_lock, std::chrono::milliseconds(timeout));
    delSyncData(p_sendVar->clientId);
    if (p_waitStatus == std::cv_status::no_timeout && p_sendVar->retBin)
    {
        ret.clear();
        ret = std::move(*(p_sendVar->retBin));
        return true;
    }
    return false;
}

bool syncSendMgr::send(const void* user, const char* data, int len)
{
    stringPtr p_data;
    p_data = ntl::makeStringPtr(len + 1);
    char* p_addr = &(*p_data)[0];
    p_addr[0] = SEND_NOT_WAIT;
    memcpy(p_addr+1, data, len);
    return syncOnSend(user, p_addr, len+1);
}

bool syncSendMgr::sendRecv(const void* user, uint32_t id, const char* data, int len)
{
    if (len == 0)
    {
        return false;
    }
    stringPtr p_data;
    p_data = ntl::makeStringPtr(len + 5);
    char* p_addr = &(*p_data)[0];
    p_addr[0] = SEND_WAIT_RET;
    memcpy(p_addr+5, data, len);
    memcpy(p_addr+1, &id, 4);
    return syncOnSend(user, p_addr, len+5);
}

bool syncSendMgr::onPushData(const void* user,const char* data, int len)
{
    if (len < 2)
    {
        return false;
    }
    uint32_t* p_recvid;
    switch (data[0])
    {
        case SEND_WAIT:
        {
            if (len < 5)
            {
                return false;
            }
            p_recvid = (uint32_t*)(data + 1);
            return syncOnRecv(user, data + 5, len - 5, *p_recvid);
        }
        case SEND_WAIT_RET:
        {
            if (len < 5)
            {
                return false;
            }
            p_recvid = (uint32_t*)(data + 1);;
            setSyncData(*p_recvid, data + 5, len - 5);
            return true;
        }
        case SEND_NOT_WAIT :return syncOnRecv(user, data+1, len-1);
    }
    return true;
}

bool syncSendMgr::syncOnRecv(const void* user, const char* data, int len, uint32_t retid)
{
    return true;
}

bool syncSendMgr::syncOnRecv(const void* user, const char* data, int len)
{
    return true;
}

bool syncSendMgr::setSyncData(uint32_t id, const char* data, int len)
{
    std::lock_guard<std::mutex>p_lock(m_syncSendLock);
    auto p_it = m_syncSendMap.find(id);
    if (p_it == m_syncSendMap.end())
    {
        return false;
    }

    if (p_it->second->retBin)
    {
        p_it->second->retBin->resize(len);
        memcpy(&((*(p_it->second->retBin))[0]), data, len);
    }
    else
    {
        p_it->second->retBin = std::make_shared<std::string>(data, len);
    }

    p_it->second->event.notify_all();
    return true;
}

bool syncSendMgr::setSyncMap(sendVarPtr& ptr)
{
    ptr=std::make_shared<sendVar>();
    std::lock_guard<std::mutex>p_lock(m_syncSendLock);
    ptr->clientId = m_sendIndex++;
    m_syncSendMap.insert({ptr->clientId,ptr});
    return true;
}

bool syncSendMgr::delSyncData(uint32_t index)
{
    std::lock_guard<std::mutex>p_lock(m_syncSendLock);
    auto p_it = m_syncSendMap.find(index);
    if (p_it == m_syncSendMap.end())
    {
        return false;
    }
    p_it->second->event.notify_all();
    m_syncSendMap.erase(p_it);
    return true;
}