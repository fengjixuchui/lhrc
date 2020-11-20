//
// Created by Administrator on 2020/1/9.
//

#include "TextureYUV.h"
#include "GLbase.h"
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <publicbase/baseFunc.h>
extern double          g_screenWscale;
extern double          g_screenHscale;

TextureYUV::TextureYUV()
        : m_drawPoint{} {
    m_w = 0;
    m_h = 0;
    m_showW = 0;
    m_showH = 0;
    m_textureId[0] = 0;
    m_dbuff[0] = 0;
    m_isScale = false;
}
TextureYUV::~TextureYUV(){
    clean();
}

bool TextureYUV::init(int w,int h){

    clean();
    //m_rect.init(256,256,GL_RGBA);
    m_isScale=false;

    //m_rect.setData((char*)ca_bmp);

    m_SamplerLoc[0]=g_GLbse.m_yuvProgramMap[0];
    m_SamplerLoc[1]=g_GLbse.m_yuvProgramMap[1];
    m_SamplerLoc[2]=g_GLbse.m_yuvProgramMap[2];
    m_program=g_GLbse.m_yuvProgram;
    m_w=GLbase::get2n(w);
    //m_h=GLbase::get2n(h);
    m_h=h;
    m_showW=w;
    m_showH=h;

    glGenTextures(3, m_textureId);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_textureId[0]);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, m_w, m_h, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, NULL);

    /*
    m_dbuff[0] = new GraphicBuffer(m_w,
                                   m_h,
                                   HAL_PIXEL_FORMAT_YV12,
                                   GraphicBuffer::USAGE_SW_WRITE_OFTEN |
                                   GraphicBuffer::USAGE_HW_TEXTURE);
     */
    glBindTexture(GL_TEXTURE_2D, GL_NONE);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_textureId[1]);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, m_w/2, m_h/2, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, NULL);
    glBindTexture(GL_TEXTURE_2D, GL_NONE);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, m_textureId[2]);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, m_w/2, m_h/2, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, NULL);
    glBindTexture(GL_TEXTURE_2D, GL_NONE);

    m_drawPoint[0] = static_cast<GLfloat>(m_showRc->x * g_screenWscale - 1);
    m_drawPoint[1] = static_cast<GLfloat>(0 - (m_showRc->y * g_screenHscale - 1));

    m_drawPoint[3] = m_drawPoint[0];
    m_drawPoint[4] = static_cast<GLfloat>(0 - (m_showRc->midY * g_screenHscale - 1));

    m_drawPoint[6] = static_cast<GLfloat>(m_showRc->midX * g_screenWscale - 1);
    m_drawPoint[7] = m_drawPoint[4];

    m_drawPoint[9] = m_drawPoint[6];
    m_drawPoint[10] = m_drawPoint[1];



    return true;
}
bool TextureYUV::setData(const char*data[3]){

    glBindTexture(GL_TEXTURE_2D, m_textureId[0]);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_showW, m_showH, GL_LUMINANCE, GL_UNSIGNED_BYTE, data[0]);
    //glBindTexture(GL_TEXTURE_2D, GL_NONE);

    glBindTexture(GL_TEXTURE_2D, m_textureId[1]);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_showW / 2, m_showH / 2, GL_LUMINANCE, GL_UNSIGNED_BYTE, data[1]);
    //glBindTexture(GL_TEXTURE_2D, GL_NONE);

    glBindTexture(GL_TEXTURE_2D, m_textureId[2]);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_showW / 2, m_showH / 2, GL_LUMINANCE, GL_UNSIGNED_BYTE, data[2]);
    //glBindTexture(GL_TEXTURE_2D, GL_NONE);
    return true;
}

bool TextureYUV::drawScale() {

    if(!m_isScale){
        return false;
    }

    static  GLushort indices[] = { 0, 1, 2, 0, 2, 3 };
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_textureId[1]);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_textureId[0]);
    glUniform1i(m_SamplerLoc[0], 0);

    glUniform1i(m_SamplerLoc[1], 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, m_textureId[2]);
    glUniform1i(m_SamplerLoc[2], 2);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), m_drawPoint);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), m_scalePoint);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);
    glBindTexture(GL_TEXTURE_2D, 0);

    //m_rect.draw(m_scaleRc.x,m_scaleRc.y);
   // m_rect.draw(m_scaleRc.x,m_scaleRc.y,m_scaleRc.w,m_scaleRc.h);
    return true;

}

int TextureYUV::setScaleDraw(int des, float Scale) { //纹理坐標
    float x=0;
    float y=0;
    float w=m_showW/2;
    float h=m_showH/2;
    switch (des) {
        case 1:
            x = w;
            break;
        case 2:
            y = h;
            break;
        case 3:
            x = w;
            y = h;
            break;
    }

    y-=h;//yuv是倒的
    if(y<0){
        y=0-y;
    }
    double p_xScale = 1.0f / m_showW;
    double p_yScale = 1.0f / m_showH;
    //y=m_showH-y;//yuv是反的


    m_scalePoint[0] = x * p_xScale;
    m_scalePoint[1] =1.0f- (y+h)*p_yScale;

    m_scalePoint[2] = m_scalePoint[0];
    m_scalePoint[3] =1.0f-y*p_yScale;

    m_scalePoint[4] =(x+w)*p_xScale;
    m_scalePoint[5] = m_scalePoint[3];

    m_scalePoint[6] = m_scalePoint[4];
    m_scalePoint[7] = m_scalePoint[1];

    m_isScale=true;
    return 1;
}
int TextureYUV::setUnScale() {
    m_isScale=false;
}
int  TextureYUV::draw(){
    if(!m_textureId[0]){
        return 0;
    }

    static GLfloat p_coordVertices[] = {
            0.0f, 0.0f,
            0.0f, 1.0f,
            1.0f, 1.0f,
            1.0f, 0.0f,
    };
    static  GLushort indices[] = { 0, 1, 2, 0, 2, 3 };
    glUseProgram (m_program);
    if (drawScale()) {
        return true;
    }
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_textureId[0]);
    glUniform1i(m_SamplerLoc[0], 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_textureId[1]);
    glUniform1i(m_SamplerLoc[1], 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, m_textureId[2]);
    glUniform1i(m_SamplerLoc[2], 2);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), m_drawPoint);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), p_coordVertices);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);
    glBindTexture(GL_TEXTURE_2D, 0);
    return 1;
}
bool TextureYUV::clean(){
    if(m_textureId[0]==0)
    {
        return true;
    }
    glDeleteTextures(3,m_textureId);
    m_textureId[0]=0;
    return true;
}
bool TextureYUV::setRc(const frc &rc) {
    m_showRc=&rc;
    return true;
}