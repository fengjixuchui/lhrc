package com.example.lb.lbrocker.GameActivity;

import android.Manifest;
import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.graphics.drawable.Drawable;
import android.os.Handler;
import android.os.Message;
import android.support.annotation.NonNull;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;
import android.util.AttributeSet;
import android.view.View;
import android.webkit.JavascriptInterface;
import android.webkit.ValueCallback;
import android.webkit.WebResourceError;
import android.webkit.WebResourceRequest;
import android.webkit.WebSettings;
import android.webkit.WebView;
import android.webkit.WebViewClient;
import android.widget.Toast;

import com.example.lb.lbrocker.webbase;
import com.example.lb.lbrocker.zbar.CaptureActivity;

import static android.app.Activity.RESULT_OK;
import static android.content.Context.MODE_PRIVATE;

public class uiMgr extends WebView {
    public eventInterface m_eventInterface=null;
    private Context m_context;
    private static final int REQUEST_CODE_SCAN = 0x0000;// 扫描二维码
    private SharedPreferences m_cfg;
    Handler m_hmessage;




    public uiMgr(Context context) {
        super(context);
    }

    public  uiMgr(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);

    }

    public  uiMgr(Context context, AttributeSet attrs) {
        super(context, attrs);

    }


    public void uiInit(Context context){
        init(context);
        m_cfg = context.getSharedPreferences("UserInfo", MODE_PRIVATE);
       m_hmessage= new Handler()
        {
            @Override
            public void handleMessage(Message msg) {
                // TODO Auto-generated method stub
                if(msg.arg1==1)
                {
                    String p_str= (String) msg.obj;
                    String p_func= webbase.post(p_str,"func");
                    if(p_func.equals("loginEnd")){
                        m_eventInterface.loginEnd();;
                    }

                }
                super.handleMessage(msg);
            }
        };
    }

    @JavascriptInterface
    public int getPing(){
        return nativeGetPing();
    }
    @JavascriptInterface
    public String getBaseInfo(){
        return nativeGetBaseInfo();
    }
    @JavascriptInterface
    public void  controlEvent(String msg){
        nativecontrolEvent(msg);
    }
    @JavascriptInterface
    public void qrCodeScan(){

                //动态权限申请
                if (ContextCompat.checkSelfPermission(m_context, Manifest.permission.CAMERA) != PackageManager.PERMISSION_GRANTED) {
                    ActivityCompat.requestPermissions((Activity) m_context, new String[]{Manifest.permission.CAMERA}, 1);
                } else {
                    goScan();
                }



    }

@JavascriptInterface
public void login(String user, String psw){
        m_eventInterface.login(user,psw);
}

    private void init(Context context){
        this.m_context=context;
        setLayerType(View.LAYER_TYPE_HARDWARE,null);
        WebSettings webSettings = getSettings();
        webSettings.setJavaScriptEnabled(true);
        addJavascriptInterface(this, "ld");

        //webSettings.setUseWideViewPort(true); //将图片调整到适合webview的大小
        //webSettings.setLoadWithOverviewMode(true); // 缩放至屏幕的大小
        webSettings.setSupportZoom(false); //禁用缩放，默认为true。
        webSettings.setCacheMode(WebSettings.LOAD_NO_CACHE); //关闭webview中缓存
        webSettings.setAllowFileAccess(true); //设置可以访问文件
        webSettings.setJavaScriptCanOpenWindowsAutomatically(false); //不支持通过JS打开新窗口
        webSettings.setLoadsImagesAutomatically(true); //支持自动加载图片
        webSettings.setDefaultTextEncodingName("utf-8");//设置编码格式
        setWebContentsDebuggingEnabled(false);//禁用调试
        setBackgroundColor(0);
        Drawable bg= getBackground();
        if(bg!=null)
        bg.setAlpha(0);
        nativeUIInit();
        setWebViewClient(new WebViewClient() {
            @Override
            public boolean shouldOverrideUrlLoading(WebView view, String url) {
                view.loadUrl(url);
                return true;
            }
            @Override
            public void onPageFinished(WebView view, String url) {
                super.onPageFinished(view, url);
                // 加载完成
            }

            @Override
            public void onReceivedError(WebView view, WebResourceRequest request, WebResourceError error) {
                super.onReceivedError(view, request, error);
                if (request.isForMainFrame()) { // 或者： if(request.getUrl().toString() .equals(getUrl()))
                    // 在这里显示自定义错误页
                }
            }
        });

        SharedPreferences p_cfg = m_context.getSharedPreferences("UserInfo", MODE_PRIVATE);
        String p_user= p_cfg.getString("user","");
        String p_psw=p_cfg.getString("psw","");
        loadUrl("http://129.204.163.30/login/index.html?&u="+p_user+"&p="+p_psw);
    }

    public void goUrl(String url){
        loadUrl(url);
    }

    /**
     * 跳转到扫码界面扫码
     */
    public void goScan() {
        Intent intent = new Intent(m_context, CaptureActivity.class);
        ((Activity)m_context).startActivityForResult(intent, REQUEST_CODE_SCAN);
    }

    private void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        switch (requestCode) {
            case 1:
                if (grantResults.length > 0 && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                    goScan();
                } else {
                    Toast.makeText(m_context, "你拒绝了权限申请，可能无法打开相机扫码哟！", Toast.LENGTH_SHORT).show();
                }
                break;
            default:
        }
    }

    public void bridgeToWeb(String str) {
        final String js = "javascript:bridgeToWeb('" + str + "');";
        this.post(new Runnable() {
            public void run() {
                evaluateJavascript(js, new ValueCallback<String>() {
                    public void onReceiveValue(String s) {
                    }
                });
            }
        });
        return;
    }

    public void bridgeToJava(String str) {
        Message msg =new Message();
        msg.arg1=1;
        msg.obj=str;
        m_hmessage.sendMessage(msg);
        return;
    }

    private native void nativeUIInit();
    private static native int nativeGetPing();
    private static native String nativeGetBaseInfo();
    private static native void nativecontrolEvent(String msg);
}
