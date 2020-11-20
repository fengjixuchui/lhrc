#include <jni.h>
#include "gl/Renderer.h"
#include <android/native_window_jni.h>
#include <android/asset_manager_jni.h>
#include <android/asset_manager.h>
#include <endian.h>
#include <arpa/inet.h>
#include "network/p2p_udp.h"
#include "network/udphelplib.h"
#include "rocker/rockerMgr.h"
#include "audioPlayer.h"
#include "serverAgent/proxyClient.h"


ANativeWindow *mWindow;
AAssetManager *aAssetManager = nullptr;
p2p_udp m_udphelp;
audioPlayer m_audioPlayer;


int g_screenWidth = 0;
int g_screenHeight = 0;
double g_screenWscale = 0;
double g_screenHscale = 0;

JavaVM* g_vm;
jobject g_this;
jmethodID g_webMid;
jmethodID g_javaMid;

extern jobject g_imgThis;
extern jmethodID g_imgMid;






jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
    g_vm=vm;
    //av_jni_set_java_vm(vm, nullptr);
    return JNI_VERSION_1_6;
}
namespace ntl{ ;

void bridgeToWeb(const char *str) {

    JNIEnv *env;
    g_vm->AttachCurrentThread(&env, NULL);
    jstring p_str = env->NewStringUTF(str);
    env->CallVoidMethod(g_this, g_webMid, p_str);
    env->DeleteLocalRef(p_str);
    g_vm->DetachCurrentThread();
}

    void bridgeToJava(const char *str) {

        JNIEnv *env;
        g_vm->AttachCurrentThread(&env, NULL);
        jstring p_str = env->NewStringUTF(str);
        env->CallVoidMethod(g_this, g_javaMid, p_str);
        env->DeleteLocalRef(p_str);
        g_vm->DetachCurrentThread();
    }
}

extern "C" {

JNIEXPORT void JNICALL
Java_com_example_lb_lbrocker_GameActivity_glMgr_nativeInit(JNIEnv *env,
                                                           jobject obj,jstring user,jstring psw) {

    //std::thread(test).detach();

    std::string p_user=ntl::jstringToChar(env, user);
    std::string p_psw=ntl::jstringToChar(env, psw);
    Elog("glMgr_nativeInit[%s][%s]",p_user.c_str(),p_psw.c_str());
    m_audioPlayer.init();
    m_udphelp.init("129.204.163.30", 5821,p_user.c_str(),p_psw.c_str());

}


JNIEXPORT void JNICALL
Java_com_example_lb_lbrocker_GameActivity_glMgr_nativeStartRender(JNIEnv *env,
                                                                          jclass type,
                                                                          jobject surface,
                                                                          jobject am,
                                                                          jobject this_) {
    mWindow = ANativeWindow_fromSurface(env, surface);
    aAssetManager = AAssetManager_fromJava(env, am);
    mRenderer.start(mWindow);
    g_deskTopDraw.init();
    m_udphelp.sendStateSignal(p2p_udp::SIGNAL_STATUS::START);
}


JNIEXPORT void JNICALL
Java_com_example_lb_lbrocker_GameActivity_glMgr_nativeSurfaceChanged(JNIEnv *env,
                                                                             jclass type,
                                                                             jobject surface, int w,
                                                                             int h) {
    mWindow = ANativeWindow_fromSurface(env, surface);
    mRenderer.requestSurfaceChanged(w, h);

}

JNIEXPORT void JNICALL
Java_com_example_lb_lbrocker_GameActivity_glMgr_nativeSurfaceDestroyed(JNIEnv *env,

                                                                               jclass type) {
    m_udphelp.sendStateSignal(p2p_udp::SIGNAL_STATUS::STOP);
    mRenderer.requestDestroy();
    ANativeWindow_release(mWindow);

}

JNIEXPORT void JNICALL
Java_com_example_lb_lbrocker_GameActivity_glMgr_nativeRequestRender(JNIEnv *env,
                                                                            jclass type) {
    mRenderer.requestRenderFrame();

}

JNIEXPORT void JNICALL
Java_com_example_lb_lbrocker_GameActivity_glMgr_nativeMotionEvent(JNIEnv *env, jclass type,
                                                                          float x, float y,
                                                                          int etype) {
    /*
    if (m_rockerMgr.inputEvent(x, y, etype)) {
        return;
    }
    if (m_mouseMgr.inputEvent(x, y, etype)) {
        return;
    }
     */
    return;
}

JNIEXPORT void JNICALL
Java_com_example_lb_lbrocker_GameActivity_glMgr_nativeScaleEvent(JNIEnv *env, jclass type,
                                                                         int point) {
    g_deskTopDraw.scale(point);
}

JNIEXPORT jint JNICALL
Java_com_example_lb_lbrocker_GameActivity_uiMgr_nativeGetPing(JNIEnv *env, jclass type
                                                                 ) {
    return m_udphelp.getPing();
}
JNIEXPORT jstring JNICALL
Java_com_example_lb_lbrocker_GameActivity_uiMgr_nativeGetBaseInfo(JNIEnv *env, jclass type
) {
    std::string p_str= g_deskTopDraw.getBaseInfo();
    return env->NewStringUTF(p_str.c_str());
}

JNIEXPORT void JNICALL
Java_com_example_lb_lbrocker_GameActivity_uiMgr_nativeUIInit(JNIEnv *env, jobject  jobj
) {
    jclass jclaz = env->GetObjectClass(jobj);
    g_this = env->NewGlobalRef(jobj);//创建对象的本地变量
    g_webMid =(*env).GetMethodID(jclaz, "bridgeToWeb", "(Ljava/lang/String;)V");//获取JAVA方法的ID
    g_javaMid =(*env).GetMethodID(jclaz, "bridgeToJava", "(Ljava/lang/String;)V");//获取JAVA方法的ID
}

JNIEXPORT void JNICALL
Java_com_example_lb_lbrocker_GameActivity_uiMgr_nativecontrolEvent(JNIEnv *env, jclass type,
                                                                   jstring msg
) {
    std::string p_msg = ntl::jstringToChar(env, msg);
    std::string p_data="to=session&fun=event&"+p_msg;
    m_udphelp.sendData(&p_data[0], p_data.length());
    return;
}

}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_lb_lbrocker_GameActivity_GameActivity_nativeRegOnimg(JNIEnv *env, jobject thiz) {

    jclass jclaz = env->GetObjectClass(thiz);
    g_imgThis = env->NewGlobalRef(thiz);//创建对象的本地变量
    g_imgMid =(*env).GetMethodID(jclaz, "onimg", "([BI)V");//获取JAVA方法的ID
}