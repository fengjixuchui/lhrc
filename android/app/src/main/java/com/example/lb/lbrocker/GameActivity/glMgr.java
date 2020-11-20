package com.example.lb.lbrocker.GameActivity;

import android.content.Context;
import android.content.res.AssetManager;
import android.util.AttributeSet;
import android.view.MotionEvent;
import android.view.ScaleGestureDetector;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;


public class glMgr extends SurfaceView implements SurfaceHolder.Callback{

    private ScaleGestureDetector mScaleGestureDetector;
    private float mScaleFactor = 1.0f;
    private GameActivity m_wnd=null;


    public glMgr(final Context context) {
        super(context);
        getHolder().addCallback(this);
    }

    public glMgr(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
        getHolder().addCallback(this);
    }

    public glMgr(Context context, AttributeSet attrs) {
        super(context, attrs);
        getHolder().addCallback(this);
    }



    public void setWnd(GameActivity wnd){
        m_wnd=wnd;
    }
public void init(String user,String psw){
    nativeInit(user,psw);
}




    @Override
    public void surfaceCreated(SurfaceHolder surfaceHolder) {
        nativeStartRender(getHolder().getSurface(),getContext().getAssets(),this);
    }

    @Override
    public void surfaceChanged(SurfaceHolder surfaceHolder, int i, int w, int h) {
        if(m_wnd!=null){
            m_wnd.hideBottomUIMenu();
        }
        nativeSurfaceChanged(surfaceHolder.getSurface(),w,h);
    }
    @Override
    public void surfaceDestroyed(SurfaceHolder surfaceHolder) {
        nativeSurfaceDestroyed();
    }




    private  native void nativeInit(String user,String psw);


    private static native void nativeScaleEvent(int point);
    private static native void nativeSurfaceChanged(Surface surface,int w,int h);
    private static native void nativeSurfaceDestroyed();
    private static native void nativeStartRender(Surface surface, AssetManager am, glMgr this_);

}
