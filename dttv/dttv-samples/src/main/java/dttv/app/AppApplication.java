package dttv.app;

import android.app.Application;
import android.content.Context;
import android.os.Environment;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;

import dttv.app.exception.CrashHandler;

/**
 * Created by shihuaxian on 2017/2/3.
 */

public class AppApplication extends Application {

    private static Context mContext;
    public static String filePath = android.os.Environment.getExternalStorageDirectory()+"/dttv";

    @Override
    public void onCreate() {
        super.onCreate();
        this.mContext = this;
        CrashHandler.getInstance().init(this);//初始化全局异常管理
        new Thread(new Runnable() {
            @Override
            public void run() {
                writeToSdCard();
            }
        }).start();
    }



    private void writeToSdCard(){
        if (isExist()){
            return;
        }
        boolean sdCardExist = Environment.getExternalStorageState().equals(android.os.Environment.MEDIA_MOUNTED);
        if (sdCardExist){
            try {
                InputStream sdpInStream = getClass().getResourceAsStream("/assets/tower.sdp");
                if (sdpInStream == null){
                    throw new IOException("未加载sdp文件");
                }
                File file = new File(filePath);
                if (!file.exists()){
                    file.mkdirs();
                }
                FileOutputStream outputStream = new FileOutputStream(filePath + "/tower.sdp");
                byte[] buffer = new byte[512];
                int count = 0;
                while ((count = sdpInStream.read(buffer)) > 0){
                    outputStream.write(buffer,0,count);
                }
                outputStream.flush();
                outputStream.close();
                sdpInStream.close();
            }catch (IOException e){
                e.printStackTrace();
            }
        }
    }

    private boolean isExist() {
        File file = new File(filePath + "/tower.sdp");
        if (file.exists()) {
            return true;
        } else {
            return false;
        }
    }

    public static Context getContext(){
        return mContext;
    }
}
