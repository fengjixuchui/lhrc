//
// Created by weiersyuan on 2016/11/24.
//

#ifndef NATIVEGLESVIEW_GLUTIL_H
#define NATIVEGLESVIEW_GLUTIL_H

#include <GLES3/gl3.h>
#include <android/log.h>
#define LOGI(level, ...) __android_log_print(ANDROID_LOG_INFO, "NATIVE_LOG", __VA_ARGS__)

class GLbase {

public:
    GLbase();
    ~GLbase();
     bool init();
     bool uninit();
     GLuint createProgram(const char * vertexShaderCode, const char * fragmentShaderCode);
     static int get2n(int i);


    GLuint m_rgbProgram;
    GLuint m_rgbProgramMap;

    GLuint m_yuvProgram;
    GLuint m_yuvProgramMap[3];

private:
    GLuint buildShader(const char* source, GLenum shaderType);
};

extern  GLbase g_GLbse;
#endif //NATIVEGLESVIEW_GLUTIL_H
