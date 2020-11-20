//
// Created by weiersyuan on 2016/11/30.
//

#include <unistd.h>
#include <deskTopDraw.h>
#include "publicbase/baseFunc.h"
#include "GLbase.h"
#include "Renderer.h"

extern int             g_screenWidth;
extern int             g_screenHeight;

extern double          g_screenWscale;
extern double          g_screenHscale;

Renderer::Renderer() {
    mDisplay = EGL_NO_DISPLAY;
    mSurface = EGL_NO_SURFACE;
    mContext = EGL_NO_CONTEXT;
}

Renderer::~Renderer() {
    terminateDisplay();
}

int  Renderer::regDrawFunc(drawFunc func){
    m_drawFuncs.push_back(func);
    Elog("drawFuncNum[%d]",m_drawFuncs.size());
    return 1;
}

int  Renderer::requestRegDrawFunc(drawFunc func){
    m_threadPool.commit(std::bind(&Renderer::regDrawFunc,this,func));
    return true;
}

void Renderer::start(ANativeWindow * pWindow) {
    mWindow = pWindow;
    m_threadPool.commit(std::bind(&Renderer::initEGL,this)).get();
}
void Renderer::requestSurfaceChanged(EGLint width, EGLint height)
{
    m_threadPool.commit(std::bind(&Renderer::nativeSurfaceChanged,this,width,height));
}


void Renderer::requestRenderFrame() {
    static auto p_func= std::bind(&Renderer::nativeDraw,this);
    m_threadPool.commit(p_func);
    return;

}

void Renderer::requestDestroy() {
    m_threadPool.commit(std::bind(&Renderer::terminateDisplay,this));
}
std::future<int> Renderer::requestFunc(std::function<int()> func){
    return m_threadPool.commit(func);
}

void Renderer::initEGL() {
    terminateDisplay();
    
    const EGLint attribs[] = {
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_BLUE_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_RED_SIZE, 8,
            EGL_NONE
    };
    EGLint width, height, format;
    EGLint numConfigs;
    EGLConfig config;
    EGLSurface surface;
    EGLContext context;

    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

    eglInitialize(display, 0, 0);

    eglChooseConfig(display, attribs, &config, 1, &numConfigs);

    surface = eglCreateWindowSurface(display, config, mWindow, NULL);
    EGLint attrs[]= {EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE};
    context = eglCreateContext(display, config, NULL, attrs);

    if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE) {
        LOGI(1, "------EGL-FALSE");
        return ;
    }

    eglQuerySurface(display, surface, EGL_WIDTH, &width);
    eglQuerySurface(display, surface, EGL_HEIGHT, &height);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    mDisplay = display;
    mSurface = surface;
    mContext = context;
    mWidth = width;
    mHeight = height;
    Elog("init:[%d][%d]",width,height);
    g_GLbse.init();
    nativeSurfaceCreated();
    nativeSurfaceChanged( width,height);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void Renderer::terminateDisplay() {
    if (mDisplay != EGL_NO_DISPLAY) {
        eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (mContext != EGL_NO_CONTEXT) {
            eglDestroyContext(mDisplay, mContext);
        }
        if (mSurface != EGL_NO_SURFACE) {
            eglDestroySurface(mDisplay, mSurface);
        }
        eglTerminate(mDisplay);
    }

    mDisplay = EGL_NO_DISPLAY;
    mSurface = EGL_NO_SURFACE;
    mContext = EGL_NO_CONTEXT;
    m_drawFuncs.clear();
}


void Renderer::nativeSurfaceCreated() {
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glDisable(GL_DEPTH_TEST);
}

void Renderer::nativeSurfaceChanged(EGLint width, EGLint height) {
    Elog("initchange:[%d][%d]",width,height);
    mWidth=width;
    mHeight= height;
    g_screenWidth=mWidth;
    g_screenHeight=mHeight;
    g_screenWscale=2.0f/mWidth;
    g_screenHscale=2.0f/mHeight;
    glViewport(0, 0, mWidth, mHeight);
    g_deskTopDraw.reSetImg();
}

void Renderer::nativeDraw() {

    struct timeval tpstart, tpend;
    float timeuse;
    gettimeofday(&tpstart, NULL);
    glClear(GL_COLOR_BUFFER_BIT );

    for (int i = 0; i < m_drawFuncs.size(); ++i) {
        m_drawFuncs[i]();
    }
    eglSwapBuffers(mDisplay, mSurface);
    gettimeofday(&tpend,NULL);
    timeuse = 1000000 * (tpend.tv_sec - tpstart.tv_sec) + tpend.tv_usec - tpstart.tv_usec;
    timeuse /= 1000;
    Elog("showTime2:[%f]",timeuse);
}
Renderer  mRenderer;


