//
// Created by Administrator on 2020/1/10.
//

#include "rockerMgr.h"
#include <string>
#include <publicbase/baseFunc.h>
#include "publicbase/def.h"
#include <math.h>
int rockerMgr::init(udpSendData func) {
    Elog("rockerMgr::init");
    m_downTime=0;
    m_send=func;
    m_moveType=M_GO;
    std::string p_data;
    m_bg.init(160, 160, GL_RGBA);
    ntl::LoadFileContent("rocker_bg.rgba", p_data);
    m_bg.setData(&p_data[0]);

    m_btn.init(64, 64, GL_RGBA);
    ntl::LoadFileContent("rocker_btn.rgba", p_data);
    m_btn.setData(&p_data[0]);

    m_isdown=false;
    m_defaultX=100;
    m_defaultY=700;

    m_bgRC.w=300;
    m_bgRC.h=300;

    m_btnRC.w=100;
    m_btnRC.h=100;
    reSetBg();
    reSetBtn();
    return 1;
}

int rockerMgr::inputEvent(float x, float y, int type) {

    switch (type) {
        case ACTION_DOWN:
            return onDown(x, y);
        case ACTION_UP:
            return onUp(x, y);
        case ACTION_MOVE:
            return onMove(x, y);
    }
    return 0;
}
int rockerMgr::onDown(float x, float y) {
    if(x > 450){
        return 0;
    }
    moveBg(x,y);
    m_isdown=true;
    m_downTime=ntl::timeGetTime();
    m_ismove=false;
    sendMouseEvent(m_moveType==M_GO?"mouseldown":"mouserdown",g_screenWidth/2,g_screenHeight/2);
    return 1;
}

int rockerMgr::onMove(float x, float y) {
    if (!m_isdown) {
        return 0;
    }
    double p_rad= getRad(m_bgRC.midX,m_bgRC.midY,x,y);
    Elog("rad[%f][%f][%f]x[%f]y[%f]x2[%f]y2[%f]",(float)p_rad,(float)cos(p_rad),(float)sin(p_rad),m_bgRC.midX,m_bgRC.midY,x,y);
    m_btnRC.x = (float) ((m_btnRC.w+50) * cos(p_rad)) + m_bgRC.midX-m_btnRC.w/2;
    m_btnRC.y = (float) ((m_btnRC.h+50) * sin(p_rad)) + m_bgRC.midY-m_btnRC.h/2;

    return 1;
}

int rockerMgr::onUp(float x, float y) {
    if (!m_isdown) {
        return false;
    }
    reSetBg();
    reSetBtn();
    m_isdown = false;
    sendMouseEvent(m_moveType==M_GO?"mouselup":"mouserup",g_screenWidth/2,g_screenHeight/2);
    if (!m_ismove && ntl::timeGetTime() - m_downTime < 5) {//走跑切换
        m_moveType = m_moveType == M_GO ? M_RUN : M_GO;
    }
    return 1;
}

int rockerMgr::moveBg(float x, float y) {
    m_defaultX=x-m_bgRC.w/2;
    m_defaultY=y-m_bgRC.h/2;
    reSetBg();
    reSetBtn();
    return 1;
}

bool rockerMgr::reSetBg() {
    m_bgRC.x=m_defaultX;
    m_bgRC.y=m_defaultY;
    m_bgRC.setmid();
    return true;
}

bool rockerMgr::reSetBtn(){
    m_btnRC.x=m_bgRC.x+(m_bgRC.w-m_btnRC.w)/2;
    m_btnRC.y=m_bgRC.y+(m_bgRC.h-m_btnRC.h)/2;
    m_btnRC.setmid();
    return true;
}
int rockerMgr::draw() {
    m_bg.draw(m_bgRC.x, m_bgRC.y,m_bgRC.w,m_bgRC.h);
    m_btn.draw(m_btnRC.x, m_btnRC.y,m_btnRC.w,m_btnRC.h);
    return 1;
}



double rockerMgr::getRad(float px1, float py1, float px2, float py2) {
    //得到两点X的距离
    float x = px2 - px1;
    //得到两点Y的距离
    float y = py1 - py2;
    //算出斜边长
    auto xie = static_cast<float>(sqrt(pow(x, 2) + pow(y, 2)));
    //得到这个角度的余弦值（通过三角函数中的定理 ：邻边/斜边=角度余弦值）
    auto cosAngle = x / xie;
    //通过反余弦定理获取到其角度的弧度
    double rad = acos(cosAngle);
    //注意：当触屏的位置Y坐标<摇杆的Y坐标我们要取反值-0~-180
    if (py2 < py1) {
        rad = -rad;
    }
    return rad;
}

int rockerMgr::sendMouseEvent(const char *func, int x, int y) {
    char p_str[256] = {};
    sprintf(p_str, "fun=web_mouse_func&type=%s&x=%d&y=%d", func, x, y);
    m_send(p_str, strlen(p_str));
    return 1;
}