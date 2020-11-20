//
// Created by Administrator on 2020/1/9.
//

#ifndef ROCKER_MASTER_TEXTUREYUV_H
#define ROCKER_MASTER_TEXTUREYUV_H
#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>
#include <publicbase/def.h>
#include "GraphicBuffer.h"
#include "TextureRGB.h"
class TextureYUV {

public:
    TextureYUV();
    virtual ~TextureYUV();
    bool init(int w,int h);
    bool setData(const char*data[3]);
    int  draw();
    int  setUnScale();
    int  setScaleDraw(int des,float Scale);
    bool clean();
    bool setRc(const frc& rc);

private:

    bool drawScale();

    float       m_showW;
    float       m_showH;
    float       m_w;
    float       m_h;
    GLfloat     m_scalePoint[8];
    GLfloat     m_drawPoint[12];

    bool        m_isScale;
    const frc*  m_showRc;
    frc         m_scaleRc;
    GLuint      m_program;
    GLuint      m_SamplerLoc[3];
    GLuint      m_textureId[3];
    TextureRGB  m_rect;
    GraphicBuffer*      m_dbuff[3];
};


#endif //ROCKER_MASTER_TEXTUREYUV_H
