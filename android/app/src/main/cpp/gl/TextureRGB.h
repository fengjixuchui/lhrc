//
// Created by Administrator on 2020/1/7.
//

#ifndef ROCKER_MASTER_TEXTURERGB_H
#define ROCKER_MASTER_TEXTURERGB_H
#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>

class TextureRGB {
public:
    TextureRGB();
    virtual ~TextureRGB();
    bool init(int w,int h,GLuint type);
    bool setData(const char*data);
    int  draw(float x,float y,float w=0,float h=0);
    bool clean();
    float getWidth();
    float getHeight();
private:
    float       m_showW;
    float       m_showH;
    float       m_w;
    float       m_h;
    GLuint      m_type;
    GLuint      m_program;
    GLuint      m_SamplerLoc;
    GLuint      m_textureId;
};


#endif //ROCKER_MASTER_TEXTURERGB_H
