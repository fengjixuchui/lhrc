//
// Created by Administrator on 2020/1/16.
//

#ifndef ROCKER_MASTER_AUDIOPLAYER_H
#define ROCKER_MASTER_AUDIOPLAYER_H
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include <queue>
#include <string>
#include <mutex>
#include <memory>
#include "opus/opus.h"
#include "publicbase/baseFunc.h"
#include "publicbase/threadPool.h"
class audioPlayer {
public:

    audioPlayer();
    ~audioPlayer();
    bool init();
    bool clean();
    bool pushData(const char* data,int len);

private:
    bool pushDataThread(strPtr str);
    bool slesInit();
    bool deCodeInit();
    bool deCode(const char* data,int len);
    bool popData(strPtr& ret);
    static  void pcmCallBack(SLAndroidSimpleBufferQueueItf bf, void*contex);
    void pcmCallBackThread(SLAndroidSimpleBufferQueueItf bf, void*contex);
    OpusDecoder*m_decode;
    int			m_sampleRate;
    int			m_channels;
    int         m_playListMaxNum;
    std::string m_decodeBuf;
    std::mutex  m_listLock;
    std::condition_variable m_listSignal;
    std::queue<strPtr> m_playList;
    std::queue<strPtr> m_strPtrList;
    threadpool  m_workThread;
    //threadpool  m_getDataThread;
    SLEngineItf m_en;
    SLObjectItf m_engineSL;
    SLAndroidSimpleBufferQueueItf m_pcmQue;
    bool m_isInit;
    bool m_isFirstRecv;
};


#endif //ROCKER_MASTER_AUDIOPLAYER_H
