
package com.example.lb.lbrocker.GameActivity;
import android.content.Context;
import android.media.MediaCodec;
import android.media.MediaCodecInfo;
import android.media.MediaFormat;
import android.os.Build;
import android.util.AttributeSet;
import android.view.Surface;
import android.view.SurfaceView;

import java.io.IOException;
import java.nio.ByteBuffer;

//集成了AvcDecoder解码功能的SurfaceView
//使用AvcDecoder，则需要手动绑定一个SurfaceView


public class AvcPlayer extends SurfaceView {

    public static final String FEATURE_LowLatency = "low-latency";
    public static final String KEY_LOW_LATENCY = "low-latency";

    MediaCodec mediaCodec;
    int width;
    int height;

    boolean playing = false;

    public AvcPlayer(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    public void init(int width, int height) {
        this.width = width;
        this.height = height;
        MediaCodecHelper.initialize(this.getContext(),"");

    }

    public static boolean decoderSupportsLowLatency(MediaCodecInfo decoderInfo, String mimeType) {
        // KitKat added CodecCapabilities.isFeatureSupported()
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.KITKAT) {
            try {
                if (decoderInfo.getCapabilitiesForType(mimeType).isFeatureSupported(FEATURE_LowLatency)) {

                    return true;
                }
            } catch (Exception e) {
                // Tolerate buggy codecs
                e.printStackTrace();
            }
        }

        return false;
    }
    private void configureMediaCodec() {
        String mimeType = "video/avc";
        try {
            mediaCodec = MediaCodec.createDecoderByType(mimeType);
        } catch (IOException e) {
            e.printStackTrace();
        }

        MediaFormat mediaFormat = MediaFormat.createVideoFormat(mimeType, width, height);
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            // We use prefs.fps instead of redrawRate here because the low latency hack in Game.java
            // may leave us with an odd redrawRate value like 59 or 49 which might cause the decoder
            // to puke. To be safe, we'll use the unmodified value.
            mediaFormat.setInteger(MediaFormat.KEY_FRAME_RATE, 24);
        }

        boolean lowLatency = decoderSupportsLowLatency(mediaCodec.getCodecInfo(), mimeType);
        if (lowLatency) {
            mediaFormat.setInteger(MediaCodecHelper.KEY_LOW_LATENCY, 1);
        } else if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            // Set the Qualcomm vendor low latency extension if the Android R option is unavailable
            String selectedDecoderName =mediaCodec.getCodecInfo().getName();
            if (MediaCodecHelper.decoderSupportsQcomVendorLowLatency(selectedDecoderName)) {
                // MediaCodec supports vendor-defined format keys using the "vendor.<extension name>.<parameter name>" syntax.
                // These allow access to functionality that is not exposed through documented MediaFormat.KEY_* values.
                // https://cs.android.com/android/platform/superproject/+/master:hardware/qcom/sdm845/media/mm-video-v4l2/vidc/common/inc/vidc_vendor_extensions.h;l=67
                //
                // Examples of Qualcomm's vendor extensions for Snapdragon 845:
                // https://cs.android.com/android/platform/superproject/+/master:hardware/qcom/sdm845/media/mm-video-v4l2/vidc/vdec/src/omx_vdec_extensions.hpp
                // https://cs.android.com/android/_/android/platform/hardware/qcom/sm8150/media/+/0621ceb1c1b19564999db8293574a0e12952ff6c
                mediaFormat.setInteger("vendor.qti-ext-dec-low-latency.enable", 1);
            }
            mediaFormat.setInteger(MediaFormat.KEY_OPERATING_RATE, Short.MAX_VALUE);
            mediaCodec.configure(mediaFormat, getHolder().getSurface(), null, 0);
            mediaCodec.start();
        }
    }
    public void decodeNalu(byte[] nalu) {
        if (!playing) return;

        ByteBuffer[] inputBuffers = mediaCodec.getInputBuffers();
        int inputBufferIndex = mediaCodec.dequeueInputBuffer(-1);
        if (inputBufferIndex < 0)
            return;

        ByteBuffer inputBuffer = inputBuffers[inputBufferIndex];
        inputBuffer.clear();
        inputBuffer.put(nalu, 0, nalu.length);
        mediaCodec.queueInputBuffer(inputBufferIndex, 0, nalu.length, 0, 0);

        MediaCodec.BufferInfo bufferInfo = new MediaCodec.BufferInfo();
        int outputBufferIndex = mediaCodec.dequeueOutputBuffer(bufferInfo, 100);
        while (outputBufferIndex >= 0) {

            mediaCodec.releaseOutputBuffer(outputBufferIndex, true);
            outputBufferIndex = mediaCodec.dequeueOutputBuffer(bufferInfo, 0);
        }


    }

    public void startPreview() {
        if(playing) {
            return;
        }
        configureMediaCodec();
        playing = true;
    }

    public void stopPreview() {
        playing = false;
        mediaCodec.stop();
        mediaCodec.release();
        mediaCodec = null;
    }

}

