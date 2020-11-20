//
// Created by Administrator on 2019/12/31.
//

#pragma once

#include <string>
#include <publicbase/def.h>
#include "publicbase/h264Decode.h"
#include "gl/Renderer.h"
#include "gl/TextureRGB.h"
#include "gl/TextureYUV.h"


class deskTopDraw {
public:
    deskTopDraw();
    bool init();
    bool OnImg(strPtr str,uint32_t retid);
    bool reSetImg();
    bool getMousePoint(float& x, float& y);
    bool scale(int point);
    bool setShowType(bool software);
    std::string getBaseInfo();
protected:
    bool sendRecvImg(uint32_t retid);
private:
    bool mediaShow(const char* data,int len);
    bool sendOnImgNum(uint32_t num);
    bool sizeChange(float w , float h);
    bool requestIDR();
    int  draw();
    bool reSetShowSize(float w, float h);
    int         m_oldw;
    int         m_oldh;
    int         m_scalePoint;
    frc         m_showRC;
    float       m_showScale;
    std::string m_imgBuf;
    TextureYUV  m_yuv;
    x264Decode  m_h264Decode;
    bool        m_softwareShow;
};
extern  deskTopDraw g_deskTopDraw;
