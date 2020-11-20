//
// Created by Administrator on 2019/12/31.
//


#include "h264Decode.h"
#include "publicbase/baseFunc.h"
#define ENCODETYPE_FFMPEG AV_PIX_FMT_YUV420P




x264Decode::x264Decode()
{
    m_pCodecCtx = NULL;
    m_img_convert_ctx = NULL;
    m_showW = 0;
    m_oldW = 0;
    m_out_buffer = NULL;
    m_pFrame = NULL;
    m_pFrameOut = NULL;
    m_packet = NULL;
    return;

}

x264Decode::~x264Decode()
{
    unInit();
    if (m_out_buffer)
    {
        av_free(m_out_buffer);
        m_out_buffer = NULL;
    }
    if (m_pFrameOut)
    {
        av_frame_free(&m_pFrameOut);
        m_pFrameOut = NULL;
    }
    if (m_img_convert_ctx)
    {
        sws_freeContext(m_img_convert_ctx);
        m_img_convert_ctx = NULL;
    }
}

bool x264Decode::deCode(int w, int h, int nw, int nh, const char * data, int dataLength, char *** retData, int& retLength)
{
    int ret = 0;
    if (data == NULL || dataLength == 0) {
        Elog("de4");
        return false;
    }
    initSws(w, h, nw, nh);
    if ((nw != m_oldW) || (nh != m_oldH)) {
        unInit();
        m_oldW = nw;
        m_oldH = nh;
        if (!init(data,36)) {
            Elog("de3");
            return false;
        }
    }
    m_packet->data = (unsigned char *) data;
    m_packet->size = dataLength;
    m_packet->pts++;
    ret = avcodec_send_packet(m_pCodecCtx, m_packet);
    if (ret < 0) {

        Elog("de2[%d]",ret);
        return false;
    }
    ret = avcodec_receive_frame(m_pCodecCtx, m_pFrame);
    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF || ret < 0) {
        Elog("de1[%d]",ret);
        return false;
    }

     if (m_showW == 0) {
        return false;
    }
/*
   int len = sws_scale(m_img_convert_ctx, (const uint8_t *const *) m_pFrame->data, m_pFrame->linesize,
                    0, nh, m_pFrameOut->data, m_pFrameOut->linesize);
    if (len <= 0) {
        return false;
    }

    *retData = (char *) m_pFrameOut->data[0];
    retLength = m_pFrameOut->linesize[0] * m_showH;
    return retLength > 0;

*/
   *retData=(char**)m_pFrame->data;
   retLength=m_pFrame->linesize[0] * h;
    return retLength > 0;
}


bool x264Decode::init(const char* extradata,int extraSize)
{
    AVCodec *pCodec = nullptr;
    //pCodec = avcodec_find_decoder_by_name("vp8_mediacodec");
    pCodec = avcodec_find_decoder(AV_CODEC_ID_H264);
   //pCodec = avcodec_find_decoder(AV_CODEC_ID_MPEG4);

    if (pCodec == nullptr) {
        Elog("Codec not found.(没有找到解码器)");
        return false;
    }
    m_pCodecCtx = avcodec_alloc_context3(pCodec);
    m_pCodecCtx->thread_count = 6;
   m_pCodecCtx->thread_type=FF_THREAD_SLICE;
   //m_pCodecCtx->thread_type=FF_THREAD_FRAME;
    m_pCodecCtx->extradata = (uint8_t *) extradata;
    m_pCodecCtx->extradata_size = extraSize;
    m_pCodecCtx->time_base.den = 20;
    m_pCodecCtx->time_base.num = 1;
    m_pCodecCtx->width = m_oldW;
    m_pCodecCtx->height = m_oldH;
    m_pCodecCtx->flags|= AV_CODEC_FLAG_LOW_DELAY;;
    int  p_i=avcodec_open2(m_pCodecCtx, pCodec, 0);
    if (p_i < 0) {

        Elog("Could not open codec.(无法打开解码器)[%d]",p_i);
        return false;
    }

    m_pFrame = av_frame_alloc();
    m_packet = av_packet_alloc();
    m_packet->pts=0;
    return true;
}

bool x264Decode::unInit()
{
    if (m_pFrame)
    {
        av_frame_free(&m_pFrame);
        m_pFrame = NULL;
    }
    if (m_packet)
    {
        m_packet->data = NULL;
        m_packet->size = 0;
        av_packet_free(&m_packet);
        m_packet = NULL;
    }
    if (m_pCodecCtx)
    {
        avcodec_close(m_pCodecCtx);
        m_pCodecCtx = NULL;
    }

    return true;
}

bool x264Decode::initSws(int w,int h,int nw,int nh)
{
    if (w == m_showW && h == m_showH && nw == m_oldW && nh == m_oldH)
    {
        return true;
    }
    if (m_img_convert_ctx)
    {
        sws_freeContext(m_img_convert_ctx);
        m_img_convert_ctx = NULL;
    }
    m_img_convert_ctx = sws_getContext(nw, nh, ENCODETYPE_FFMPEG, w, h, AV_PIX_FMT_RGB24, SWS_FAST_BILINEAR, NULL, NULL, NULL);

    if (w == m_showW && h == m_showH)
    {
        return true;
    }
    m_showW = w;
    m_showH = h;
    if (m_out_buffer)
    {
        av_free(m_out_buffer);
        m_out_buffer = NULL;
    }
    if (m_pFrameOut)
    {
        av_frame_free(&m_pFrameOut);
        m_pFrameOut = NULL;
    }
    m_pFrameOut = av_frame_alloc();
    m_out_buffer = (uint8_t *)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_RGB24, m_showW, m_showH, 4));
    av_image_fill_arrays(m_pFrameOut->data, m_pFrameOut->linesize, m_out_buffer, AV_PIX_FMT_RGB24, m_showW, m_showH, 4);
    return true;
}
