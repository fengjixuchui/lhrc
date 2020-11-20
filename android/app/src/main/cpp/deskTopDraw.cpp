//
// Created by Administrator on 2019/12/31.
//

#include "deskTopDraw.h"
#include "publicbase/def.h"
#include "publicbase/baseFunc.h"
#include "network/udphelplib.h"
#include <sys/time.h>
#include <unistd.h>
#include <gl/GLbase.h>
#include <cstring>
#include <cstdio>
#include <network/p2p_udp.h>

extern JavaVM* g_vm;
jobject g_imgThis;
jmethodID g_imgMid;

extern int g_screenWidth;
extern int g_screenHeight;
extern p2p_udp m_udphelp;
#define  scaleX 300
#define  scaleY 300
deskTopDraw::deskTopDraw() {
    m_oldw=0;
    m_oldh=0;

    m_scalePoint=-1;
    m_softwareShow=true;
    return;
}
bool deskTopDraw::init() {
    std::function<int()> p_func= std::bind( &deskTopDraw::draw,this);
    mRenderer.requestRegDrawFunc(p_func);
    m_yuv.setRc(m_showRC);
    m_imgBuf.reserve(1024*1024);
    return true;
}
int  deskTopDraw::draw(){
    m_yuv.draw();
    return 1;
}
bool deskTopDraw::reSetImg() {
    m_oldw=0;
    return true;
}
bool deskTopDraw::sendRecvImg(uint32_t retid) {
    m_udphelp.sendRecvData(retid,"1",1);
    return true;
}
bool deskTopDraw::requestIDR() {

}

bool deskTopDraw::mediaShow(const char *data, int len) {

    JNIEnv* p_env = nullptr;
    g_vm->AttachCurrentThread(&p_env, NULL);
    jbyteArray jbarray = p_env->NewByteArray(len);
    p_env->SetByteArrayRegion(jbarray, 0, len, (jbyte *) data);
    p_env->CallVoidMethod(g_imgThis, g_imgMid, jbarray,len);
    p_env->DeleteLocalRef(jbarray);
    g_vm->DetachCurrentThread();
    return true;
}

bool deskTopDraw::sendOnImgNum(uint32_t num) {
    static int p_i = 0;
    p_i++;
    if (p_i != 2)
    {
        return true;
    }
    p_i = 0;
    char p_temp[128] = {};
    sprintf(p_temp,  "fun=onImgCallBack&id=%u", num);
    m_udphelp.sendData(p_temp, strlen(p_temp));
    return true;
}
bool deskTopDraw::sizeChange(float w, float h) {
    m_oldw=w;
    m_oldh=h;
    reSetShowSize(w,h);
    m_yuv.init(w,h);
    //m_yuv.setScaleDraw(m_scalePoint,2);
   return true;
}
std::string deskTopDraw::getBaseInfo() {
    char p_str[128]={};
    sprintf(p_str,"func=setGameScreenSize&x=%d&y=%d&w=%d&h=%d&s=%f",(int)m_showRC.x,(int)m_showRC.y,(int)m_showRC.w,(int)m_showRC.h,m_showScale);
    return p_str;
}

bool deskTopDraw::setShowType(bool software) {

}
bool deskTopDraw::scale(int point) {
    if(point==-1)
    {
        m_scalePoint=-1;
        m_yuv.setUnScale();
        return true;
    }
    m_scalePoint=point;
    m_yuv.setScaleDraw(m_scalePoint,2);

}

bool deskTopDraw::getMousePoint(float &x, float &y) {
    if (x < m_showRC.x || x > m_showRC.midX) {
        return false;
    }
    if (y < m_showRC.y || y > m_showRC.midY) {
        return false;
    }
    x = x - m_showRC.x;
    x /= m_showScale;
    y = y - m_showRC.y;
    y /= m_showScale;
    return true;
}

bool deskTopDraw::OnImg(strPtr str,uint32_t retid){
    if (retid != 0) {
        sendRecvImg(retid);
    }
    int p_len = webbase::post_i(*str, "imgL");
    int p_w = webbase::post_i(*str, "imgNW");
    int p_h = webbase::post_i(*str, "imgH");
    int p_end=webbase::post_i(*str,"end");
    int c = str->find("data=");
    if (c == -1)
    {
        return false;
    }
    c += 5;
    if (p_len + c > str->length())
    {
        return true;
    }
    if(p_end==0){
        if(m_imgBuf.size()>1024*900)
        {
            m_imgBuf.resize(0);
        }
        m_imgBuf.append(&(*str)[c], p_len);
        return false;
    }
    sendOnImgNum(static_cast<uint32_t>(webbase::post_i((*str), "imgNum")));
    char* p_imgData= nullptr;

    if(m_imgBuf.size()>0)
    {
        m_imgBuf.append(&(*str)[c], p_len);
        p_imgData=&m_imgBuf[0];
        p_len=m_imgBuf.size();
    } else{
        p_imgData=&(*str)[c];
    }
    char**	p_retData = NULL;
    int		p_retLength = 0;
    if(!m_softwareShow){
        mediaShow(p_imgData,p_len);
        m_imgBuf.resize(0);
        return true;
    }

    struct timeval tpstart, tpend;
    float timeuse;
    gettimeofday(&tpstart, NULL);
    if (!m_h264Decode.deCode(p_w, p_h, p_w, p_h, p_imgData, p_len, &p_retData, p_retLength)) {
        m_imgBuf.resize(0);
        requestIDR();
        return  false;
    }
    m_imgBuf.resize(0);

    if(p_w!=m_oldw || p_h != m_oldh){
        sizeChange(p_w,p_h);
    }

    gettimeofday(&tpend, NULL);
    timeuse = 1000000 * (tpend.tv_sec - tpstart.tv_sec) + tpend.tv_usec - tpstart.tv_usec;
    timeuse /= 1000;
    Elog("decodeTime:[%f][%d]",timeuse,p_len);

    gettimeofday(&tpstart, NULL);
    m_yuv.setData((const char**)p_retData);

    mRenderer.nativeDraw();
    gettimeofday(&tpend, NULL);
    timeuse = 1000000 * (tpend.tv_sec - tpstart.tv_sec) + tpend.tv_usec - tpstart.tv_usec;
    timeuse /= 1000;
    Elog("showTime:[%f]",timeuse);
    return true;
}

bool deskTopDraw::reSetShowSize(float w, float h) {
    float p_wScale = g_screenWidth / w;
    float p_hScale = g_screenHeight / h;
    m_showScale = p_wScale < p_hScale ? p_wScale : p_hScale;
    m_showRC.w = w * m_showScale;
    m_showRC.h = h * m_showScale;
    m_showRC.x = (g_screenWidth - m_showRC.w) / 2;
    m_showRC.y = (g_screenHeight - m_showRC.h) / 2;
    m_showRC.midX = m_showRC.w + m_showRC.x;
    m_showRC.midY = m_showRC.h + m_showRC.y;
    ntl::bridgeToWeb(getBaseInfo().c_str());
    return true;
}

deskTopDraw g_deskTopDraw;