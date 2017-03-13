package dttv.app;

import android.app.Application;
import android.content.Context;

import dttv.app.exception.CrashHandler;

/**
 * Created by shihuaxian on 2017/2/3.
 */

public class AppApplication extends Application {

    private static Context mContext;

    @Override
    public void onCreate() {
        super.onCreate();
        this.mContext = this;
        //CrashHandler.getInstance().init(this);//初始化全局异常管理
    }

    public static Context getContext(){
        return mContext;
    }
}
