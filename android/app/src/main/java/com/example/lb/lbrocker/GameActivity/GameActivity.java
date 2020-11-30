package com.example.lb.lbrocker.GameActivity;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.app.ActivityManager;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.ActivityInfo;
import android.content.res.Resources;
import android.hardware.SensorManager;
import android.os.Build;
import android.os.Bundle;
import android.view.KeyEvent;
import android.view.OrientationEventListener;
import android.view.View;
import android.view.WindowManager;
import com.example.lb.lbrocker.R;
import com.example.lb.lbrocker.webbase;
import com.example.lb.lbrocker.zbar.CaptureActivity;

import java.lang.reflect.Method;


public class GameActivity extends Activity implements eventInterface {
    static {
        System.loadLibrary("ffmpeg-lib");
        System.loadLibrary("udphelp-lib");
    }

    private OrientationEventListener mOrientationListener;
    private int m_oldAtion=270;
    AvcPlayer player;
    uiMgr m_ui;
    glMgr m_gl;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        if((getIntent().getFlags() & Intent.FLAG_ACTIVITY_BROUGHT_TO_FRONT) != 0){
            finish();
            return;
        }
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON,
        WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        super.onCreate(savedInstanceState);
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_UNSPECIFIED);
        setContentView(R.layout.activity_game_land);

        m_ui= (uiMgr)findViewById(R.id.web);
        m_gl=(glMgr)findViewById(R.id.GL);
        m_gl.setWnd(this);
        m_ui.m_eventInterface=this;
        m_ui.uiInit(this);

        //player=(AvcPlayer)findViewById(R.id.avcPlayer);
        //player.init(1920, 1080);
        //nativeRegOnimg();
    }
    private  void  initStart(String str){
        String user= webbase.post(str,"u");
        String psw= webbase.post(str,"p");
        if(user.isEmpty() || psw.isEmpty()){
            showScanMsg();
            return;
        }
        login(user,psw);
    }

    void showScanMsg(){
        new AlertDialog.Builder(this)
                .setTitle("提示")
                .setMessage("扫码失败!")
                .setPositiveButton("重试", new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int whichButton) {
                        m_ui.goScan();
                    }
                }).setNegativeButton("取消", new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int whichButton) {

            }
        }).show();
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        switch (requestCode) {
            case 0:// 二维码
                // 扫描二维码回传
                if (resultCode == RESULT_OK) {
                    if (data != null) {
                        //获取扫描结果
                        Bundle bundle = data.getExtras();
                        String result = bundle.getString(CaptureActivity.EXTRA_STRING);
                        initStart(result);
                        return ;
                    }
                }
                break;
            default:
                break;
        }

    }


    /**
     * 判断是否存在虚拟按键
     * @return
     */
    private boolean checkDeviceHasNavigationBar() {
        boolean hasNavigationBar = false;
        Resources rs = getResources();
        int id = rs.getIdentifier("config_showNavigationBar", "bool", "android");
        if (id > 0) {
            hasNavigationBar = rs.getBoolean(id);
        }
        try {
            Class<?> systemPropertiesClass = Class.forName("android.os.SystemProperties");
            Method m = systemPropertiesClass.getMethod("get", String.class);
            String navBarOverride = (String) m.invoke(systemPropertiesClass, "qemu.hw.mainkeys");
            if ("1".equals(navBarOverride)) {
                hasNavigationBar = false;
            } else if ("0".equals(navBarOverride)) {
                hasNavigationBar = true;
            }
        } catch (Exception e) {

        }
        return hasNavigationBar;
    }

    public void hideBottomUIMenu() {
        //隐藏虚拟按键，并且全屏
        if (Build.VERSION.SDK_INT > 11 && Build.VERSION.SDK_INT < 19) { // lower api
            View v = this.getWindow().getDecorView();
            v.setSystemUiVisibility(View.GONE);
        } else if (checkDeviceHasNavigationBar()) {
            //for new api versions.
            View decorView = getWindow().getDecorView();
            int flag = View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                    | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                    | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                    | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION // hide
                    | View.SYSTEM_UI_FLAG_FULLSCREEN // hide status bar
                    | View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY;
            decorView.setSystemUiVisibility(flag);
        }
    }
    @Override
    public boolean dispatchKeyEvent(KeyEvent event) {
        if (event.getKeyCode() != KeyEvent.KEYCODE_BACK ) {
            return super.dispatchKeyEvent(event);
        }
        if (event.getAction() != KeyEvent.ACTION_UP) {
            return true;
        }
        new AlertDialog.Builder(this)
                .setTitle("提示")
                .setMessage("确定退出?")
                .setPositiveButton("确定", new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int whichButton) {
                        ActivityManager am = (ActivityManager)getSystemService (Context.ACTIVITY_SERVICE);
                        assert am != null;
                       finish();
                       System.exit(0);
                    }
                })
                .setNegativeButton("取消", new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int whichButton) {
                    }
                }).show();
        return false;
    }

    private  void regDirectionEvent(){
        mOrientationListener = new OrientationEventListener(this,
                SensorManager.SENSOR_DELAY_NORMAL) {

            @SuppressLint("SourceLockedOrientationActivity")
            @Override
            public void onOrientationChanged(int orientation) {
                if (orientation == OrientationEventListener.ORIENTATION_UNKNOWN) {
                    return;  //手机平放时，检测不到有效的角度
                }
                if( orientation > 350 || orientation< 10 ) { //0度
                   return;
                }
                else if( orientation > 80 &&orientation < 100 ) { //90度
                    orientation= 90;
                }
                else if( orientation > 170 &&orientation < 190 ) { //180度
                    return;
                }
                else if( orientation > 260 &&orientation < 280 ) { //270度
                    orientation= 270;
                }
                else {
                    return;
                }
                if (m_oldAtion == orientation) {
                    return;
                }
                m_oldAtion = orientation;
                if(m_oldAtion ==270){
                    setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
                }else {
                    setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_REVERSE_LANDSCAPE);
                }
                return;
            }
        };

        if (mOrientationListener.canDetectOrientation()) {
            mOrientationListener.enable();
        } else {
            mOrientationListener.disable();
        }

    };
    @Override
    protected void onDestroy() {
        super.onDestroy();
        if(mOrientationListener!=null)
        mOrientationListener.disable();
    }

    public void  login(String user, String psw) {

        m_ui.bridgeToWeb("func=setLoginStatus&s=1");
        SharedPreferences p_cfg = getSharedPreferences("UserInfo", MODE_PRIVATE);
        SharedPreferences.Editor p_editor = p_cfg.edit();
        p_editor.putString("user",user);
        p_editor.putString("psw",psw);
        p_editor.apply();
        m_gl.init(user,psw);
    }

    @SuppressLint("SourceLockedOrientationActivity")
    @Override
    public void loginEnd() {
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
        hideBottomUIMenu();
        regDirectionEvent();
    }


    public void onimg(byte[] data,int len){
        player.startPreview();
        player.decodeNalu(data);
    }

    private native void nativeRegOnimg();

}
