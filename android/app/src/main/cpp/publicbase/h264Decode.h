//
// Created by Administrator on 2019/12/31.
//

#ifndef ROCKER_MASTER_H264DECODE_H
#define ROCKER_MASTER_H264DECODE_H

#include <string>
extern "C" {
#include "ffmpeg/libavutil/avutil.h"
#include "ffmpeg/libavutil/imgutils.h"
#include "ffmpeg/libavcodec/avcodec.h"
#include "ffmpeg/libavformat/avformat.h"
#include "ffmpeg/libswscale/swscale.h"
#include "ffmpeg/libavcodec/jni.h"
}


class x264Decode {
public:
    x264Decode();
    ~x264Decode();
    bool deCode(int w, int h, int nw, int nh, const char * data, int dataLength, char *** retData, int& retLength);
    AVFrame*		m_pFrame;
private:

    bool init(const char* extradata,int extraSize);
    bool unInit();
    bool initSws(int w, int h, int nw, int nh);



    AVPacket*		m_packet;
    AVCodecContext* m_pCodecCtx;

    AVFrame*		m_pFrameOut;
    uint8_t*		m_out_buffer;
    SwsContext*		m_img_convert_ctx;
    char*           m_yuvBuff;
    int m_oldW;
    int m_oldH;
    int m_showW;
    int m_showH;
};


#endif //ROCKER_MASTER_H264DECODE_H
