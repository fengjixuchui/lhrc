//
// Created by weiersyuan on 2016/11/24.
//

#include "GLbase.h"
#include <string>
#include <dlfcn.h>
#include <publicbase/baseFunc.h>
#include <EGL/egl.h>
#include <GLES3/gl3ext.h>
#include <android/hardware_buffer.h>

char vRGBShaderStr[] =
        "#version 300 es                            \n"
        "layout(location = 0) in vec4 a_position;   \n"
        "layout(location = 1) in vec2 a_texCoord;   \n"
        "out vec2 v_texCoord;                       \n"
        "void main()                                \n"
        "{                                          \n"
        "   gl_Position = a_position;               \n"
        "   v_texCoord = a_texCoord;                \n"
        "}                                          \n";


char fRGBShaderStr[] =
        "#version 300 es                                     \n"
        "precision mediump float;                            \n"
        "in vec2 v_texCoord;                                 \n"
        "layout(location = 0) out vec4 outColor;             \n"
        "uniform sampler2D s_TextureMap;                     \n"
        "void main()                                         \n"
        "{                                                   \n"
        "  outColor = texture(s_TextureMap, v_texCoord);     \n"
        "}                                                   \n";


GLchar vYUVShaderStr[] =
        "#version 300 es                                         \n"
        "layout (location = 0) in lowp vec3 position;           \n"
        "layout (location = 1) in lowp vec2 inTexcoord;         \n"
        "out lowp vec2 texcoord;                                \n"
        "void main()                                             \n"
        "{                                                       \n"
        "gl_Position = vec4(position, 1.0);                      \n"
        "texcoord = inTexcoord;                                  \n"
        "}                                                       \n";

const GLchar *fYUVShaderStr =
        "#version 300 es                                         \n"
        "precision lowp  float;                                  \n"
        "in vec2 texcoord;                                       \n"
        "out vec4 FragColor;                                     \n"
        "uniform lowp sampler2D yPlaneTex;                       \n"
        "uniform lowp sampler2D uPlaneTex;                       \n"
        "uniform lowp sampler2D vPlaneTex;                       \n"
        "void main()                                             \n"
        "{                                                       \n"
        "   // (1) y - 16 (2) rgb * 1.164                        \n"
        "    vec3 yuv;                                           \n"
        "    yuv.x = texture(yPlaneTex, texcoord).r;             \n"
        "    yuv.y = texture(uPlaneTex, texcoord).r - 0.5f;      \n"
        "    yuv.z = texture(vPlaneTex, texcoord).r - 0.5f;      \n"
        "    mat3 trans = mat3(1.0, 1.0 ,1.0,                    \n"
        "                     0.0, -0.34414, 1.772,              \n"
        "                     1.402, -0.71414, 0.0               \n"
        "                     );                                 \n"
        "    FragColor = vec4(trans*yuv, 1.0);                   \n"
        "}                                                       \n";

const GLchar *fYUVShaderStr2 =
        "#version 300 es                                         \n"
        "precision lowp  float;                                  \n"
        "in vec2 texcoord;                                       \n"
        "out vec4 FragColor;                                     \n"
        "uniform lowp sampler2D yPlaneTex;                       \n"
        "uniform lowp sampler2D uPlaneTex;                       \n"
        "uniform lowp sampler2D vPlaneTex;                       \n"
        "void main()                                             \n"
        "{                                                       \n"
        "   // (1) y - 16 (2) rgb * 1.164                        \n"
        "    float r, g, b, y, u, v;                             \n"
        "    y = texture(yPlaneTex, texcoord).r;                 \n"
        "    u = texture(uPlaneTex, texcoord).r - 0.5;           \n"
        "    v = texture(vPlaneTex, texcoord).r - 0.5;           \n"
        "    r = y + 1.13983*v;                                  \n"
        "    g = y - 0.39465*u - 0.58060*v;                      \n"
        "    b = y + 2.03211*u;                                  \n"
        "    FragColor = vec4(r,g,b, 1.0);                       \n"
        "}                                                       \n";


GLbase::GLbase(){
    m_rgbProgram = 0;
    m_rgbProgramMap = 0;

    m_yuvProgram = 0;

    m_yuvProgramMap[0] = 0;
    m_yuvProgramMap[1] = 0;
    m_yuvProgramMap[2] = 0;


}
GLbase::~GLbase()
{
    uninit();
}



GLuint GLbase::buildShader(const char* source, GLenum shaderType)
{
    GLuint shaderHandle = glCreateShader(shaderType);

    if (shaderHandle)
    {
        glShaderSource(shaderHandle, 1, &source, 0);
        glCompileShader(shaderHandle);

        GLint compiled = 0;
        glGetShaderiv(shaderHandle, GL_COMPILE_STATUS, &compiled);
        if (!compiled)
        {
            GLint infoLen = 0;
            glGetShaderiv(shaderHandle, GL_INFO_LOG_LENGTH, &infoLen);
            if (infoLen)
            {
                char* buf = (char*) malloc(infoLen);
                if (buf)
                {
                    glGetShaderInfoLog(shaderHandle, infoLen, NULL, buf);
                    free(buf);
                }
                glDeleteShader(shaderHandle);
                shaderHandle = 0;
            }
        }
    }
    return shaderHandle;
}

 int GLbase::get2n(int i) {
    int p_mod = i % 32;
    if (p_mod == 0) {
        return i;
    }
    return i + 32 - p_mod;
}
bool GLbase::init(){
    m_rgbProgram =  createProgram(vRGBShaderStr, fRGBShaderStr);
    m_rgbProgramMap = glGetUniformLocation(m_rgbProgram, "s_TextureMap");

    m_yuvProgram =  createProgram(vYUVShaderStr, fYUVShaderStr);

    m_yuvProgramMap[0] = glGetUniformLocation(m_yuvProgram, "yPlaneTex");
    m_yuvProgramMap[1] = glGetUniformLocation(m_yuvProgram, "uPlaneTex");
    m_yuvProgramMap[2] = glGetUniformLocation(m_yuvProgram, "vPlaneTex");
    return true;
}
bool GLbase::uninit(){
    if (m_rgbProgram) {
        glDeleteProgram(m_rgbProgram);
        m_rgbProgram = 0;
    }
    if (m_yuvProgram) {
        glDeleteProgram(m_yuvProgram);
        m_yuvProgram = 0;
    }
    return  true;
}

GLuint GLbase::createProgram(const char *vertexShaderCode, const char *fragmentShaderCode) {


    GLuint programHandle = glCreateProgram();
    if (programHandle)
    {
        GLuint vertexShader = buildShader(vertexShaderCode, GL_VERTEX_SHADER);;
        GLuint fragmentShader = buildShader(fragmentShaderCode, GL_FRAGMENT_SHADER);

        glAttachShader(programHandle, vertexShader);
        glAttachShader(programHandle, fragmentShader);

        glLinkProgram(programHandle);

        GLint linkStatus = GL_FALSE;
        glGetProgramiv(programHandle, GL_LINK_STATUS, &linkStatus);
        if (linkStatus != GL_TRUE) {
            GLint bufLength = 0;
            glGetProgramiv(programHandle, GL_INFO_LOG_LENGTH, &bufLength);
            if (bufLength) {
                char* buf = (char*) malloc(bufLength);
                if (buf) {
                    glGetProgramInfoLog(programHandle, bufLength, NULL, buf);
                    free(buf);
                }
            }
            glDeleteProgram(programHandle);
            programHandle = 0;
        }

    }
    return programHandle;
}


GLbase g_GLbse;