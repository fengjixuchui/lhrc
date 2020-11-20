//
// Created by weiersyuan on 2016/11/30.
//

#pragma once

#include <pthread.h>
#include <android/native_window.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include "publicbase/threadPool.h"
#include <functional>
#include <map>
#include <memory>

class Renderer {
public:

    typedef  std::function<int()> drawFunc;
    Renderer();
    virtual ~Renderer();

    void start(ANativeWindow * pWindow);
    void requestSurfaceChanged(EGLint width, EGLint height);
    void requestRenderFrame();
    void requestDestroy();
    std::future<int>  requestFunc(std::function<int()> func);
    int  requestRegDrawFunc(drawFunc func);

    void nativeDraw();

private:

    int  regDrawFunc(drawFunc func);
    void initEGL();
    void terminateDisplay();
    void nativeSurfaceCreated();
    void nativeSurfaceChanged(EGLint width, EGLint height);



    ANativeWindow *mWindow;
    EGLDisplay mDisplay;
    EGLSurface mSurface;
    EGLContext mContext;
    EGLint mWidth;
    EGLint mHeight;
    std::vector<drawFunc> m_drawFuncs;
    threadpool m_threadPool;
};

extern Renderer mRenderer;
