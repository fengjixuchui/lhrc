//
// Created by Administrator on 2019/12/30.
//

#include <iconv.h>
#include <locale.h>
#include "baseFunc.h"
#include <string.h>
#include <stdlib.h>
#include <android/asset_manager_jni.h>
#include <android/asset_manager.h>

#include "cryptopp/rsa.h"
#include "cryptopp/randpool.h"
#include "cryptopp/rc4.h"

extern  AAssetManager* aAssetManager;

long webbase::post_i(const std::string& buff, const char* var)
{

    std::string p = var;
    p = p + "=";
    int i = buff.find(p);
    if (i == -1)
    {
        return 0;
    }
    i += p.length();
    long buffLen = strlen(buff.c_str());
    int i2 = buff.find("&", i);
    if (i2 == -1)
    {
        i2 = buffLen;
    }
    return	atol(buff.substr(i, i2 - i).c_str());
}

webbase::uint webbase::post_ui(const std::string& buff, const char* var)
{

    std::string p = var;
    p = p + "=";
    int i = buff.find(p);
    if (i == -1)
    {
        return 0;
    }
    i += p.length();
    long buffLen = strlen(buff.c_str());
    int i2 = buff.find("&", i);
    if (i2 == -1)
    {
        i2 = buffLen;
    }
    return	(uint)atoll(buff.substr(i, i2 - i).c_str());
}

std::string webbase::post(const std::string& buff, const char* var, bool utf82GBK)
{
    std::string p = var;
    p += "=";

    int i = buff.find(p);
    if (i == -1)
    {
        return "";
    }
    i += p.length();
    long buffLen = strlen(buff.c_str());
    int i2 = buff.find("&", i);
    if (i2 == -1)
    {
        i2 = buffLen;
    }
    return URLDecode(buff.substr(i, i2 - i), utf82GBK);


}
std::string webbase::URLEncode(const std::string& URL, bool UTF_8)
{
    std::string result = "", url2;
    const char * t;
    int size = 0;
    if (UTF_8)
    {
        url2 = ntl::AToUTF8(URL.c_str());
        t = url2.c_str();
        size = url2.size();
    }
    else
    {
        size = URL.size();
        t = URL.c_str();
    }

    for (int i = 0; i < size; i++) {
        char c = t[i];
        if (
                ('0' <= c && c <= '9') ||
                ('a' <= c && c <= 'z') ||
                ('A' <= c && c <= 'Z') ||
                c == '/' || c == '.'
                ) {
            result += c;
        }
        else {
            int j = (short int)c;
            if (j < 0) {
                j += 256;
            }
            int i1, i0;
            i1 = j / 16;
            i0 = j - i1 * 16;
            result += '%';
            result += dec2hexChar(i1);
            result += dec2hexChar(i0);
        }
    }
    return result;
}
std::string webbase::URLDecode(const std::string &URL, bool UTF_8) {
    std::string result;
    unsigned int p = URL.size() - 1;
    for (unsigned int i = 0; i < URL.size(); i++) {
        char c = URL[i];
        if (c != '%') {
            result += c;
        }
        else  if (i < p) {
            char c1 = URL[++i];
            char c0 = URL[++i];
            int num = 0;
            num += hexChar2dec(c1) * 16 + hexChar2dec(c0);
            result += char(num);
        }
    }
    if (UTF_8)
        return ntl::UTF8ToA(result.c_str());
    return result;

}

std::string webbase::BaseEncode(unsigned char* Data, int DataByte)
{
    //编码表
    const char EncodeTable[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    //返回值
    std::string strEncode;
    unsigned char Tmp[4] = { 0 };
    int LineLength = DataByte;
    int Mod = DataByte % 3;
    char * re;
    unsigned char * bin = Data;
    if (Mod > 0)
    {
        LineLength = LineLength + 3 - Mod;
        bin = new unsigned char[LineLength];
        memcpy(bin, Data, DataByte);
        *(bin + DataByte) = 0;
        if (Mod == 1)
            *(bin + DataByte + 1) = 0;
    }
    re = new char[LineLength * 4 / 3 + 1];
    *(re + LineLength * 4 / 3) = 0;
    char * re2 = re;
    unsigned char * bin2 = bin;


    for (int i = 0; i < LineLength / 3; i++)
    {
        Tmp[1] = *bin++;
        Tmp[2] = *bin++;
        Tmp[3] = *bin++;

        *re++ = EncodeTable[Tmp[1] >> 2];
        *re++ = EncodeTable[((Tmp[1] << 4) | (Tmp[2] >> 4)) & 0x3F];
        *re++ = EncodeTable[((Tmp[2] << 2) | (Tmp[3] >> 6)) & 0x3F];
        *re++ = EncodeTable[Tmp[3] & 0x3F];
    }

    if (Mod > 0)
    {
        LineLength = LineLength * 4 / 3;
        if (Mod == 1)
            *(re2 + LineLength - 2) = 0;
        *(re2 + LineLength - 1) = 0;
        delete bin2;
    }

    strEncode = std::string(re2);
    delete re2;
    return strEncode;
}
std::string webbase::BaseDecode(const char* Data, int DataByte, int& OutByte)
{
    //解码表
    const char DecodeTable[] =
            {
                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                    62, // '+'
                    0, 0, 0,
                    63, // '/'
                    52, 53, 54, 55, 56, 57, 58, 59, 60, 61, // '0'-'9'
                    0, 0, 0, 0, 0, 0, 0,
                    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
                    13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, // 'A'-'Z'
                    0, 0, 0, 0, 0, 0,
                    26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38,
                    39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, // 'a'-'z'
            };
    //返回值

    std::string strDecode;
    //CString re;
    int nValue;
    int i = 0;
    while (i < DataByte)
    {
        if (*Data != '\r' && *Data != '\n')
        {
            nValue = DecodeTable[*Data++] << 18;
            nValue += DecodeTable[*Data++] << 12;
            strDecode += (nValue & 0x00FF0000) >> 16;
            OutByte++;
            if (i + 2 == DataByte)
                break;
            if (*Data != '=')
            {
                nValue += DecodeTable[*Data++] << 6;
                strDecode += (nValue & 0x0000FF00) >> 8;
                OutByte++;
                if (i + 3 == DataByte)
                    break;
                if (*Data != '=')
                {
                    nValue += DecodeTable[*Data++];
                    strDecode += nValue & 0x000000FF;
                    OutByte++;
                    if (i + 4 == DataByte)
                        break;
                }
            }
            i += 4;
        }
        else// 回车换行,跳过
        {
            Data++;
            i++;
        }
    }
    return strDecode;
}
char		webbase::dec2hexChar(short int n) {
    if (0 <= n && n <= 9) {
        return char(short('0') + n);
    }
    else if (10 <= n && n <= 15) {
        return char(short('A') + n - 10);
    }
    else {
        return char(0);
    }
}
short int	webbase::hexChar2dec(char c) {
    if ('0' <= c && c <= '9') {
        return short(c - '0');
    }
    else if ('a' <= c && c <= 'f') {
        return (short(c - 'a') + 10);
    }
    else if ('A' <= c && c <= 'F') {
        return (short(c - 'A') + 10);
    }
    else {
        return -1;
    }
}

namespace ntl{


    std::string WToA(const wchar_t *str) {
        int p_len = wcslen(str);
        std::string p_ret;
        setlocale(LC_ALL,"zh_CN.GB18030");
        p_ret.resize(p_len);
        p_len = wcstombs(&p_ret[0], str, p_ret.length());
        if (p_len == -1) {
        return "";
        }
        p_ret.resize(p_len);
        return p_ret;
    }

    std::wstring AToW(const char * str)
    {
        int p_len = strlen(str);
        std::wstring p_ret;
        setlocale(LC_ALL,"zh_CN.GB18030");
        p_ret.resize(p_len * 3);
        p_len = mbtowc(&p_ret[0], str, p_ret.length());
        if (p_len == -1) {
            return L"";
        }
        p_ret.resize(p_len);
        return p_ret;
    }
    std::string AToUTF8(const char * str)
    {
        return WToUTF8(AToW(str).c_str());
    }

    std::string WToUTF8(const wchar_t * str)
    {
        return "";
    }

    std::wstring UTF8ToW(const char * str)
    {

        return L"";
    }

    std::string UTF8ToA(const char * str)
    {
        return WToA(UTF8ToW(str).c_str());
    }

    bool LoadFileContent(const char *path, std::string &data) {
        long filesSize = 0;
        AAsset *asset = AAssetManager_open(aAssetManager, path, AASSET_MODE_UNKNOWN);
        if (asset == nullptr) {
            return false;
        }
        filesSize = AAsset_getLength(asset);
        data.resize(filesSize);
        AAsset_read(asset, &data[0], filesSize);
        AAsset_close(asset);
        return true;
    }

    std::string   jstringToChar(JNIEnv *env, jstring jstr)
    {
        std::string p_ret;
        jclass clsstring = env->FindClass("java/lang/String");
        jstring strencode = env->NewStringUTF("utf-8");
        jmethodID mid = env->GetMethodID(clsstring, "getBytes", "(Ljava/lang/String;)[B");
        jbyteArray barr = (jbyteArray) env->CallObjectMethod(jstr, mid, strencode);
        jsize alen = env->GetArrayLength(barr);
        jbyte *ba = env->GetByteArrayElements(barr, JNI_FALSE);
        if (alen > 0) {
            p_ret.resize(alen);
            memcpy(&p_ret[0], ba, alen);
        }
        env->ReleaseByteArrayElements(barr, ba, 0);
        return p_ret;
    }
    jstring CStr2Jstring( JNIEnv* env, const char* pat )
    {
        // 定义java String类 strClass
        jclass strClass = (env)->FindClass("Ljava/lang/String;");
        // 获取java String类方法String(byte[],String)的构造器,用于将本地byte[]数组转换为一个新String
        jmethodID ctorID = (env)->GetMethodID(strClass, "<init>", "([BLjava/lang/String;)V");
        // 建立byte数组
        jbyteArray bytes = (env)->NewByteArray((jsize)strlen(pat));
        // 将char* 转换为byte数组
        (env)->SetByteArrayRegion(bytes, 0, (jsize)strlen(pat), (jbyte*)pat);
        //设置String, 保存语言类型,用于byte数组转换至String时的参数
        jstring encoding = (env)->NewStringUTF("GB2312");
        //将byte数组转换为java String,并输出
        return (jstring)(env)->NewObject(strClass, ctorID, bytes, encoding);

    }


    unsigned int timeGetTime() {
        struct timeval t;
        gettimeofday(&t, NULL);
        return static_cast<unsigned int>(1000 * t.tv_sec + t.tv_usec / 1000);
    }

    bool isInside(const frc &rc, float x, float y) {

        if (x < rc.x || x > rc.x + rc.w) {
            return false;
        }
        if (y < rc.y || y > rc.y + rc.h) {
            return false;
        }
        return true;
    }
/*
    std::string rsaEncode(const char* data,int datalen , const std::string& publicKey)
    {
        std::string p_ret;
        if (datalen < rsakeylen)
        {
            p_ret.resize(rsakeylen);
        }
        else
        {
            p_ret.resize(datalen * 2);
        }

        uint64_t putLen = 0;
        try
        {
            CryptoPP::RSAES_OAEP_SHA_Encryptor enc;
            CryptoPP::RandomPool randomPool;
            CryptoPP::ArraySource keyArr((byte*)&publicKey[0], publicKey.length(), true);
            enc.AccessKey().Load(keyArr);
           // randomPool.IncorporateEntropy();
            uint64_t fixedLen = enc.FixedMaxPlaintextLength();
            size_t p_datalen = datalen;
            size_t p_retLen = p_ret.length();
            byte* p_retAddr = (byte*)&p_ret[0];
            byte* p_dataAddr = (byte*)data;
            uint64_t p_len = 0;
            for (uint64_t i = 0; i < p_datalen; i += fixedLen)
            {
                p_len = fixedLen < (p_datalen - i) ? fixedLen : (p_datalen - i);
                CryptoPP::ArraySink* dstArr = new CryptoPP::ArraySink(p_retAddr + putLen, p_retLen - putLen);
                CryptoPP::ArraySource source(p_dataAddr + i, p_len, true, new CryptoPP::PK_EncryptorFilter(randomPool, enc, dstArr));
                putLen += dstArr->TotalPutLength();
            }
        }
        catch (const std::exception& e)
        {
            return "";
        }
        p_ret.resize(putLen);
        return p_ret;
    }
*/
    std::string     getRandStr(int minSize,int maxSize)
    {
        std::string p_ret;
        int p_retLen=minSize+ rand()% (maxSize-minSize);
        p_ret.resize((p_retLen));
        for (int i = 0; i < p_retLen; ++i) {
            p_ret[i]= 97 + rand() % 26;
        }
    return p_ret;
    }

    std::string getlastValue(const std::string& data, const char* key)
    {
        size_t p_index = data.find(key);
        if (p_index == std::string::npos)
        {
            return "";
        }
        return data.substr(p_index + strlen(key));
    }

    int32_t reverseWord(int32_t word) {
        unsigned char *byte, temp;
        byte = (unsigned char *) &word;
        temp = byte[0];
        byte[0] = byte[3];
        byte[3] = temp;

        temp = byte[1];
        byte[1] = byte[2];
        byte[2] = temp;
        return word;
    }

    stringPtr makeStringPtr(int len) {
        return stringPtr(new std::string(len, '\0'));
    }
}