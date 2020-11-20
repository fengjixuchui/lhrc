//
// Created by Administrator on 2019/12/30.
//
#pragma once
#ifndef ROCKER_MASTER_BASEFUNC_H
#define ROCKER_MASTER_BASEFUNC_H

#include <string>
#include <jni.h>
#include <android/log.h>
#include <mutex>
#include "def.h"

#define Elog(...) __android_log_print(ANDROID_LOG_WARN,"ERROR1",__VA_ARGS__)
typedef unsigned char byte;
#define maxStrPoolLen 4096
typedef std::shared_ptr<std::string>	stringPtr;
extern  std::string  g_rsaPublicKey;
typedef struct _sendVar
{
    uint32_t							clientId;
    std::mutex                          eventLock;
    std::condition_variable				event;
    std::shared_ptr<std::string>		retBin;
    ~_sendVar(){
        event.notify_all();
    }
}sendVar;
typedef std::shared_ptr<sendVar>		sendVarPtr;


class webbase
{
public:

    typedef unsigned int uint;

    static long		   post_i(const std::string& buff, const char* var);
    static uint		   post_ui(const std::string& buff, const char* var);
    static std::string post(const std::string& buff, const char* var, bool utf82GBK = false);

    static std::string URLEncode(const std::string& URL, bool UTF_8);
    static std::string URLDecode(const std::string& URL, bool UTF_8);


    /*编码
    DataByte
    [in]输入的数据长度,以字节为单位
    */
    static std::string  BaseEncode(unsigned char* Data, int DataByte);
    /*解码
    DataByte
    [in]输入的数据长度,以字节为单位
    OutByte
    [out]输出的数据长度,以字节为单位,请不要通过返回值计算
    输出数据的长度
    */
    static std::string  BaseDecode(const char* Data, int DataByte, int& OutByte);

private:
    static char			dec2hexChar(short int n);
    static short int	hexChar2dec(char c);

};


namespace ntl{
    std::string		WToA(const wchar_t* str);
    std::wstring	AToW(const char* str);

    std::string		AToUTF8(const char* str);
    std::string		WToUTF8(const wchar_t* str);

    std::wstring	UTF8ToW(const char* str);
    std::string		UTF8ToA(const char* str);

    std::string     jstringToChar(JNIEnv *env, jstring jstr);

    bool            LoadFileContent(const char *path , std::string& data);

    unsigned int    timeGetTime();
    int32_t         reverseWord(int32_t word);

    bool            isInside(const frc& rc, float x , float y);

    void            bridgeToWeb(const char* str);
    void            bridgeToJava(const char* str);

   // std::string     rsaEncode(const char* data,int datalen, const std::string& publicKey =g_rsaPublicKey);
    std::string     getRandStr(int minSize=4,int maxSize=10);
    std::string     getlastValue(const std::string& data, const char* key);

    stringPtr		makeStringPtr(int len);

}

#endif //ROCKER_MASTER_BASEFUNC_H
