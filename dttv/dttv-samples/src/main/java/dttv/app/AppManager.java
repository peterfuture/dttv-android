package dttv.app;

import android.app.Activity;
import android.content.Context;
import android.os.Process;

import java.util.Stack;

/**
 * Created by shihuaxian on 2017/2/3.
 */

public class AppManager {

    //Activity 栈
    private static Stack<Activity> activityStack;
    //单例模式
    private static AppManager instance;

    private AppManager(){}

    public static AppManager getInstance(){
        synchronized (instance){
            if (instance == null){
                instance = new AppManager();
            }
        }
        return instance;
    }

    /**
     * 添加Activity到堆栈
     * @param activity
     */
    public void addActivity(Activity activity){
        if (activityStack == null){
            activityStack = new Stack<Activity>();
        }
        activityStack.add(activity);
    }

    /**
     * 获取当前Activity
     * @return
     */
    public Activity currentActivity(){
        Activity activity = null;
        if (activityStack != null){
            activity = activityStack.lastElement();
        }
        return activity;
    }

    /**
     * 结束当前Activity（堆栈中最后一个压入的）
     */
    public void finishActivity(){
        Activity activity = activityStack.lastElement();
        finishCurrentActivity(activity);
    }

    public void finishCurrentActivity(Activity activity){
        if (activity != null){
            activityStack.remove(activity);
            activity.finish();
            activity = null;
        }
    }

    public void finishAllActivity(){
        for (int i = 0; i < activityStack.size(); i++){
            if (null != activityStack.get(i)){
                activityStack.get(i).finish();
            }
        }
        activityStack.clear();
    }

    public void AppExit(Context context){
        try {
            finishAllActivity();
            Process.killProcess(Process.myPid());
            System.exit(0);
        }catch (Exception e){}
    }
}
