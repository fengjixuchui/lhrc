//
// Created by Administrator on 2020/1/7.
//

#include "TextureRGB.h"
#include "GLbase.h"
#include <thread>
extern int             g_screenWidth;
extern int             g_screenHeight;

extern double          g_screenWscale;
extern double          g_screenHscale;

TextureRGB::TextureRGB() {
    m_w = 0;
    m_h = 0;
    m_showW = 0;
    m_showH = 0;
    m_textureId=0;
}

TextureRGB::~TextureRGB() {
    clean();
}

bool TextureRGB::clean() {
    if (m_textureId) {
        glDeleteTextures(1, &m_textureId);
        m_textureId = 0;
    }
    return true;
}

bool TextureRGB::init(int w, int h,GLuint type) {
    clean();
    m_type=type;
    m_SamplerLoc=g_GLbse.m_rgbProgramMap;
    m_program=g_GLbse.m_rgbProgram;
    m_w=GLbase::get2n(w);
    m_h=GLbase::get2n(h);
    m_showW=w;
    m_showH=h;
    glUseProgram (m_program);
    glGenTextures(1, &m_textureId);
    glBindTexture(GL_TEXTURE_2D, m_textureId);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D,
                 0,           //细节基本 0默认
                 type,//gpu内部格式 亮度，灰度图
                 m_w,m_h, //拉升到全屏
                 0,             //边框
                 type,//数据的像素格式 亮度，灰度图 要与上面一致
                 GL_UNSIGNED_BYTE, //像素的数据类型
                 NULL                    //纹理的数据
    );
    glUniform1i(m_SamplerLoc, 0);
    glBindTexture(GL_TEXTURE_2D, GL_NONE);
    return true;
}

bool TextureRGB::setData(const char *data) {

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_textureId);

    glTexSubImage2D(GL_TEXTURE_2D,0,0,0,m_showW,m_showH,m_type,GL_UNSIGNED_BYTE,data);
    glBindTexture(GL_TEXTURE_2D, 0);
    return true;
}
int TextureRGB::draw(float x,float y,float w,float h) {
    if(!m_textureId){
        return 0;
    }
    GLfloat p_drawPoint[12] = {};
    static GLfloat p_coordVertices[] = {
            0.0f, 0.0f,
            0.0f, 1.0f,
            1.0f, 1.0f,
            1.0f, 0.0f,
    };
    static  GLushort indices[] = { 0, 1, 2, 0, 2, 3 };
    if (w == 0) {
        w = m_showW;
    }
    if (h == 0) {
        h = m_showH;
    }
    w += x;
    h += y;

    p_drawPoint[0] = static_cast<GLfloat>(x * g_screenWscale - 1);
    p_drawPoint[1] = static_cast<GLfloat>(0 - (y * g_screenHscale - 1));

    p_drawPoint[3] = p_drawPoint[0];
    p_drawPoint[4] = static_cast<GLfloat>(0 - (h * g_screenHscale - 1));

    p_drawPoint[6] =  static_cast<GLfloat>(w * g_screenWscale - 1);
    p_drawPoint[7] = p_drawPoint[4];

    p_drawPoint[9] =   p_drawPoint[6];
    p_drawPoint[10] = p_drawPoint[1];
    glUseProgram (m_program);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_textureId);
    glUniform1i(m_SamplerLoc, 0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), p_drawPoint);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), p_coordVertices);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);
    glBindTexture(GL_TEXTURE_2D, 0);
return 1;
}

float TextureRGB::getWidth() {
    return m_w;
}
float TextureRGB::getHeight() {
    return m_h;
}