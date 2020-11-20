//
// Created by Administrator on 2020/1/2.
//

#pragma once

#include <functional>

 struct frc {
    float x;
    float y;
    float w;
    float h;
     float midX;
     float midY;
    frc() {
        x = 0;
        y = 0;
        w = 0;
        h = 0;
        midX=0;
        midY=0;
    };

   void setmid(){
       midX = x + w / 2;
       midY = y + h / 2;
    }

    frc(float x, float y, float w, float h) {
        this->x = x;
        this->y = y;
        this->w = w;
        this->h = h;
        midX=x/2;
        midY=y/2;
    }
};

enum EACTION {
    ACTION_DOWN = 0,
    ACTION_UP = 1,
    ACTION_MOVE = 2,
    ACTION_CANCEL = 3,
    ACTION_OUTSIDE = 4,
    ACTION_POINTER_DOWN = 5,
    ACTION_POINTER_UP = 6,
    ACTION_HOVER_MOVE = 7,
    ACTION_SCROLL = 8,
};


class udpCallback
{
public:
    virtual bool sendData(const char* data,int len)=0;

};

typedef  std::function<bool(const char* data,int len)> udpSendData;
typedef  std::shared_ptr<std::string> strPtr;
