package dttv.app.exception;

import android.content.Context;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Environment;
import android.os.Looper;
import android.util.Log;
import android.widget.Toast;

import java.io.File;
import java.io.FileOutputStream;
import java.io.PrintWriter;
import java.io.StringWriter;
import java.io.Writer;
import java.lang.reflect.Field;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.HashMap;
import java.util.Map;

import dttv.app.AppManager;

/**
 * Created by shihuaxian on 2017/2/3.
 */

public class CrashHandler implements Thread.UncaughtExceptionHandler {

    private Thread.UncaughtExceptionHandler mDefaultHandler;

    private Context mContext;

    private Map<String,String> paramsMap = new HashMap<String, String>();

    /**
     * 格式化时间
     */
    private SimpleDateFormat format = new SimpleDateFormat("yyyy-MM-dd-HH-mm-ss");
    private String TAG = this.getClass().getSimpleName();

    private static CrashHandler mInstance;

    private CrashHandler(){}

    public static synchronized CrashHandler getInstance(){
        if(mInstance == null){
            mInstance = new CrashHandler();
        }
        return mInstance;
    }

    public void init(Context context){
        mContext = context;
        mDefaultHandler = Thread.getDefaultUncaughtExceptionHandler();
        //设置该CrashHandler为系统默认的
        Thread.setDefaultUncaughtExceptionHandler(this);
    }


    @Override
    public void uncaughtException(Thread thread, Throwable throwable) {
        if (!handleException(throwable) && mDefaultHandler != null){//如果自己没有处理则交给系统处理
            mDefaultHandler.uncaughtException(thread,throwable);
        }else {//自己处理
            try {
                Thread.sleep(3000);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
            //退出程序
            AppManager.getInstance().AppExit(mContext);
        }
    }

    private boolean handleException(Throwable ex){
        if (ex == null){
            return false;
        }
        //收集设备参数信息
        collectDeviceInfo(mContext);
        //添加自定义信息
        addCustomInfo();
        //使用Toast来显示异常信息
        new Thread(new Runnable() {
            @Override
            public void run() {
                Looper.prepare();
                Toast.makeText(mContext,"程序报错...",Toast.LENGTH_LONG).show();
                Looper.loop();
            }
        }).start();
        //保存日志
        saveCrashInfo2File(ex);
        return true;
    }

    public void collectDeviceInfo(Context context){
        try {
            PackageManager pm = context.getPackageManager();
            PackageInfo pi = pm.getPackageInfo(context.getPackageName(),PackageManager.GET_ACTIVITIES);
            if (pi != null){
                String versionName = pi.versionName == null ? "null" : pi.versionName;
                String versionCode = pi.versionCode + "";
                paramsMap.put("versionName",versionName);
                paramsMap.put("versionCdde",versionCode);
            }
        }catch (PackageManager.NameNotFoundException e){
            Log.e(TAG, "an error occured when collect package info", e);
        }
        //获取所有系统信息
        Field[] fields = Build.class.getFields();
        for (Field field : fields){
            field.setAccessible(true);
            try {
                paramsMap.put(field.getName(),field.get(null).toString());
            }catch (Exception e){
                Log.e(TAG, "an error occured when collect crash info", e);
            }
        }
    }

    /**
     * 添加自定义参数
     */
    private void addCustomInfo() {

    }

    /**
     * 保存错误信息到文件中
     *
     * @param ex
     * @return  返回文件名称,便于将文件传送到服务器
     */
    private String saveCrashInfo2File(Throwable ex){
        StringBuffer sb = new StringBuffer();
        for (Map.Entry<String,String> entry:paramsMap.entrySet()){
            String key = entry.getKey();
            String value = entry.getValue();
            sb.append(key + "=" + value + "\n");
        }

        Writer writer = new StringWriter();
        PrintWriter printWriter = new PrintWriter(writer);
        ex.printStackTrace(printWriter);
        Throwable cause = ex.getCause();
        while (cause != null){
            cause.printStackTrace(printWriter);
            cause = cause.getCause();
        }
        printWriter.close();
        String result = writer.toString();
        sb.append(result);
        try{
            long timestamp = System.currentTimeMillis();
            String time = format.format(new Date());
            String fileName = "crash-" +time +"-" +  timestamp + ".log";
            if (Environment.getExternalStorageState().equals(Environment.MEDIA_MOUNTED)){
                String path = Environment.getExternalStorageDirectory().getAbsolutePath() + "/crash/";
                File dir = new File(path);
                if (! dir.exists()){
                    dir.mkdirs();
                }
                FileOutputStream fos = new FileOutputStream(path + fileName);
                fos.write(sb.toString().getBytes());
                fos.close();
            }
            return fileName;
        }catch (Exception e){
            Log.e(TAG, "an error occured while writing file...", e);
        }
        return null;
    }


}
