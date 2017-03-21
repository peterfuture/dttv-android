package dttv.app;

import android.app.Activity;
import android.app.Application;
import android.content.Context;
import android.os.Environment;
import android.support.multidex.MultiDex;
import android.support.v7.app.AppCompatDelegate;
import android.util.DisplayMetrics;
import android.view.Display;
import android.view.WindowManager;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.HashSet;
import java.util.Set;

import dttv.app.exception.CrashHandler;

/**
 * Created by shihuaxian on 2017/2/3.
 */

public class AppApplication extends Application {

    private static Context mContext;
    public static String filePath = android.os.Environment.getExternalStorageDirectory()+"/dttv";

    private static AppApplication instance;
    private Set<Activity> allActivities;

    public static int SCREEN_WIDTH = -1;
    public static int SCREEN_HEIGHT = -1;
    public static float DIMEN_RATE = -1.0F;
    public static int DIMEN_DPI = -1;

    public static synchronized AppApplication getInstance() {
        return instance;
    }

    static {
        AppCompatDelegate.setDefaultNightMode(
                AppCompatDelegate.MODE_NIGHT_NO);
    }

    @Override
    public void onCreate() {
        super.onCreate();
        this.mContext = this;
        instance = this;

        //初始化屏幕宽高
        getScreenSize();

        //CrashHandler.getInstance().init(this);//初始化全局异常管理
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

    protected void attachBaseContext(Context base) {
        super.attachBaseContext(base);
        MultiDex.install(this);
    }

    public void addActivity(Activity act) {
        if (allActivities == null) {
            allActivities = new HashSet<>();
        }
        allActivities.add(act);
    }

    public void removeActivity(Activity act) {
        if (allActivities != null) {
            allActivities.remove(act);
        }
    }

    public void exitApp() {
        if (allActivities != null) {
            synchronized (allActivities) {
                for (Activity act : allActivities) {
                    act.finish();
                }
            }
        }
        android.os.Process.killProcess(android.os.Process.myPid());
        System.exit(0);
    }

    public void getScreenSize() {
        WindowManager windowManager = (WindowManager)this.getSystemService(Context.WINDOW_SERVICE);
        DisplayMetrics dm = new DisplayMetrics();
        Display display = windowManager.getDefaultDisplay();
        display.getMetrics(dm);
        DIMEN_RATE = dm.density / 1.0F;
        DIMEN_DPI = dm.densityDpi;
        SCREEN_WIDTH = dm.widthPixels;
        SCREEN_HEIGHT = dm.heightPixels;
        if(SCREEN_WIDTH > SCREEN_HEIGHT) {
            int t = SCREEN_HEIGHT;
            SCREEN_HEIGHT = SCREEN_WIDTH;
            SCREEN_WIDTH = t;
        }
    }
}
