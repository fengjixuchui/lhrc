//
// Created by Administrator on 2020/1/16.
//

#include <string>
#include <publicbase/baseFunc.h>
#include <thread>
#include "audioPlayer.h"
audioPlayer::audioPlayer() {
    m_en= nullptr;
    m_engineSL= nullptr;
    m_pcmQue= nullptr;
    m_playListMaxNum=14;
    m_sampleRate=16000;
    m_channels=2;
    m_isInit=false;
    m_isFirstRecv=true;
    for (int i = 0; i < m_playListMaxNum; ++i) {
        strPtr p_str= std::make_shared<std::string>();
        p_str->reserve(2048);
        m_strPtrList.push(p_str);
    }
}
audioPlayer::~audioPlayer() {

}

bool audioPlayer::init() {
    if(m_isInit)
    {
        return true;
    }
    if (!deCodeInit()) {
        return false;
    }
    return slesInit();
    //std::thread(&audioPlayer::slesInit,this).join();
   // return true;
}

bool audioPlayer::pushData(const char *data, int len) {
    strPtr p_ptr=std::make_shared<std::string>(data,len);
    m_workThread.commit(std::bind(&audioPlayer::pushDataThread,this,p_ptr));
    return true;
}

bool audioPlayer::clean() {
    std::unique_lock<std::mutex> lock{m_listLock};
    for (int i = 0; i <m_playList.size(); ++i) {
        m_strPtrList.push(std::move(m_playList.front()));
        m_playList.pop();
    }
    return true;
}
bool audioPlayer::slesInit() {
    SLresult re;
    re = slCreateEngine(&m_engineSL,0,0,0,0,0);
    if(re != SL_RESULT_SUCCESS) return false;
    re = (*m_engineSL)->Realize(m_engineSL,SL_BOOLEAN_FALSE);
    if(re != SL_RESULT_SUCCESS) return false;
    re = (*m_engineSL)->GetInterface(m_engineSL,SL_IID_ENGINE,&m_en);
    if (re != SL_RESULT_SUCCESS) return false;
    const SLInterfaceID ids2[] = {SL_IID_ENVIRONMENTALREVERB};
    const SLboolean req2[] = {SL_BOOLEAN_FALSE};

    SLObjectItf mix = NULL;
    re = (*m_en)->CreateOutputMix(m_en,&mix,1,ids2,req2);
    if (re != SL_RESULT_SUCCESS) return false;


    //b 实例化
    re = (*mix)->Realize(mix,SL_BOOLEAN_FALSE);
    if (re != SL_RESULT_SUCCESS)return false;
    //输出
    SLDataLocator_OutputMix outmix ={SL_DATALOCATOR_OUTPUTMIX,mix};
    SLDataSink audioSink= {&outmix,0};

    SLDataLocator_AndroidSimpleBufferQueue que = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,2};

    //音频格式配置
    SLDataFormat_PCM pcm = {
            SL_DATAFORMAT_PCM,
            2,//通道数
            SL_SAMPLINGRATE_16,//采样率
            SL_PCMSAMPLEFORMAT_FIXED_16, // bitsPerSample
            SL_PCMSAMPLEFORMAT_FIXED_16,// containerSize
            SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,
           /* SL_BYTEORDER_BIGENDIAN*/SL_BYTEORDER_LITTLEENDIAN  //字节序,小端 ()
    };

    //播放器使用的结构体
    SLDataSource ds = {&que,&pcm};

    SLObjectItf  player = NULL;
    SLPlayItf playerInterface = NULL;

    const SLInterfaceID ids1[] = {SL_IID_BUFFERQUEUE, SL_IID_EFFECTSEND, SL_IID_VOLUME};
    const SLboolean req1[] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};

    //sizeof(ids)/sizeof(SLInterfaceID) 参数个数
    re = (*m_en)->CreateAudioPlayer(m_en,&player,&ds,&audioSink, sizeof(ids1)/sizeof(SLInterfaceID),ids1,req1);
    if (re != SL_RESULT_SUCCESS)return false;

    //实例化
    re = (*player)->Realize(player,SL_BOOLEAN_FALSE);
    if (re != SL_RESULT_SUCCESS)return false;
    //获取接口
    re = (*player)->GetInterface(player,SL_IID_PLAY,&playerInterface);
    if (re != SL_RESULT_SUCCESS)
        return false;

    //获取缓冲队列接口
    re =(*player)->GetInterface(player,SL_IID_BUFFERQUEUE,&m_pcmQue);
    if (re != SL_RESULT_SUCCESS) {
        m_pcmQue=nullptr ;
        return false;
    }

   re= (*m_pcmQue)->RegisterCallback(m_pcmQue,pcmCallBack,this);
    if (re != SL_RESULT_SUCCESS)
    {
        m_pcmQue=nullptr ;
        return false;
    }

    re= (*playerInterface)->SetPlayState(playerInterface,SL_PLAYSTATE_PLAYING);

    (*m_pcmQue)->Enqueue(m_pcmQue, "", 1);
    if (re != SL_RESULT_SUCCESS)
    {
        m_pcmQue=nullptr ;
        return false;
    }
    m_isInit=true;
    return true;
}


bool audioPlayer::deCodeInit() {
    int err=0;
    m_decode= opus_decoder_create(m_sampleRate, m_channels, &err);
    if (err != OPUS_OK) {
        return false;
    }
    m_decodeBuf.resize(1024*8);
    return true;
}
bool audioPlayer::deCode(const char* data,int len) {
    //ret=std::make_shared<std::string>(data,len);
    //return true;


    unsigned char * p_decodePtr=(unsigned char *)&m_decodeBuf[0];
    int p_ret= opus_decode(m_decode,(const unsigned char *)data,len,(opus_int16*)p_decodePtr,m_decodeBuf.length(),0);
    if(p_ret<1)
    {
        Elog("voiceError1");
        return false;
    }

    static bool p_isFull=false;
    strPtr p_data;
    {
        std::lock_guard<std::mutex> p_lock(m_listLock);
        if(p_isFull){
          if(m_playList.size()< m_playListMaxNum-3)
          {
              p_isFull=false;
          }else{
              Elog("voiceError2[%d]",len);
              return true;
          }
        }
        if (m_playList.size() == m_playListMaxNum) {
            p_isFull=true;
            return true;
        } else {
            p_data = std::move(m_strPtrList.front());
            m_strPtrList.pop();
        }

        p_data->resize(p_ret * 4);
        memcpy(&(*p_data)[0], p_decodePtr, p_ret * 4);

        // p_data->resize(len);
        // memcpy(&(*p_data)[0],data,len);
        m_playList.push(p_data);
        m_listSignal.notify_one();
    }
    Elog("voicepush[%d]",m_playList.size());
    return true;
}
void audioPlayer::pcmCallBackThread(SLAndroidSimpleBufferQueueItf bf, void *contex) {
    audioPlayer *p_this = (audioPlayer *) contex;
    strPtr p_data;

    std::unique_lock<std::mutex> lock{p_this->m_listLock};
    if (p_this->m_playList.size() == 0) {
        Elog("voice3");
        p_this->m_listSignal.wait(lock, [p_this] {
            static int p_waitNum=3;
            static int p_waitTime=0;
            static struct timeval tpstart={}, tpend;
            gettimeofday(&tpend, NULL);
            float  timeuse = 1000000 * (tpend.tv_sec - tpstart.tv_sec) + tpend.tv_usec - tpstart.tv_usec;
            timeuse /= 1000;

            if(timeuse<1000*20)
            {
                p_waitNum++;
                if(p_waitNum>6)
                {
                    p_waitNum=6;
                }
            }else if(timeuse>1000*120)
            {
                p_waitNum=3;
            }
            tpstart.tv_sec=tpend.tv_sec;
            tpstart.tv_usec=tpend.tv_usec;
            return p_this->m_playList.size() > p_waitNum;//当上次为空时保持最少3个 防止爆音
        });
        Elog("voice4");
    }
    p_data = std::move(p_this->m_playList.front()); // 取一个 task
    p_this->m_playList.pop();
    (*bf)->Enqueue(bf, &(*p_data)[0], p_data->length());
    p_this->m_strPtrList.push(p_data);
    p_this->m_isFirstRecv=false;

    Elog("voicepushPcm[%d][%d]",p_data->length(),p_this->m_playList.size());

}
void audioPlayer::pcmCallBack(SLAndroidSimpleBufferQueueItf bf, void *contex) {
    audioPlayer *p_this = (audioPlayer *) contex;
    //p_this->m_getDataThread.commit(std::bind(&audioPlayer::pcmCallBackThread,(audioPlayer*)contex,bf,contex));
   // return;
    strPtr p_data;

    std::unique_lock<std::mutex> lock{p_this->m_listLock};
    if (p_this->m_playList.size() == 0) {
        Elog("voice3");
        p_this->m_listSignal.wait(lock, [p_this] {
            static int p_waitNum=3;
            static int p_waitTime=0;
            static struct timeval tpstart={}, tpend;
            gettimeofday(&tpend, NULL);
            float  timeuse = 1000000 * (tpend.tv_sec - tpstart.tv_sec) + tpend.tv_usec - tpstart.tv_usec;
            timeuse /= 1000;

            if(timeuse<1000*20)  {
                p_waitNum++;
                if(p_waitNum>6)
                {
                    p_waitNum=6;
                }
            }else {
                p_waitNum=3;
            }
            tpstart.tv_sec=tpend.tv_sec;
            tpstart.tv_usec=tpend.tv_usec;
            return p_this->m_playList.size() > p_waitNum;//当上次为空时保持最少3个 防止爆音
        });
        Elog("voice4");
    }
    p_data = std::move(p_this->m_playList.front()); // 取一个 task
    p_this->m_playList.pop();
    (*bf)->Enqueue(bf, &(*p_data)[0], p_data->length());
    p_this->m_strPtrList.push(p_data);
    p_this->m_isFirstRecv=false;

    Elog("voicepushPcm[%d][%d]",p_data->length(),p_this->m_playList.size());
}
bool audioPlayer::popData(strPtr &ret) {
    std::lock_guard<std::mutex>p_lock(m_listLock);
    if(m_playList.size()==0)  {
        return false;
    }
    ret = std::move(m_playList.front()); // 取一个 task
    m_playList.pop();
    return true;
}
bool audioPlayer::pushDataThread(strPtr str) {

    if (!m_pcmQue) {
        Elog("voice2");
        return false;
    }
    int p_len = webbase::post_i(*str, "len");
    int c = str->find("data=");
    if (c == -1)
    {
        return false;
    }
    c += 5;
    if (p_len + c > str->length())
    {
        Elog("voice1[%d]",p_len);
        return true;
    }
    if(!deCode(&(*str)[c],p_len))
    {
        Elog("voiceDecode failed");
        return true;
    }
    return true;
}


