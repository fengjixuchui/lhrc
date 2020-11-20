//
// Created by Administrator on 2020/1/10.
//

#pragma once

#include <gl/TextureRGB.h>
#include "publicbase/def.h"

extern int g_screenWidth;
extern int g_screenHeight;

class rockerMgr {

    enum MOVE_TYPE{
        M_GO=0,
        M_RUN=1
    };
public:
    int init(udpSendData func);
    int draw();
    int inputEvent(float x, float y ,int type);
 private:
    bool reSetBtn();
    bool reSetBg();
    int onDown(float x, float y);
    int onMove(float x, float y);
    int onUp(float x, float y);
    int sendMouseEvent(const char* func,int x,int y);
    int moveBg(float x, float y);
    double getRad(float px1, float py1, float px2, float py2);

    bool m_isdown;
    float m_defaultX;
    float m_defaultY;
    float m_bg_R;
    frc m_btnRC;
    frc m_bgRC;
    unsigned  int m_downTime;
    bool m_ismove;
    MOVE_TYPE m_moveType;
    udpSendData m_send;
    TextureRGB m_bg;
    TextureRGB m_btn;

};
