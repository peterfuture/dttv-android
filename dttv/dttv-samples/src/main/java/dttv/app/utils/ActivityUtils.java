package dttv.app.utils;

import android.app.Activity;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;

/**
 * Created by shx on 2017/4/5.
 */

public class ActivityUtils {
    private ActivityUtils() {
        throw new UnsupportedOperationException("u can't instantiate me...");
    }

    /**
     * 判断是否存在Activity
     *
     * @param context     上下文
     * @param packageName 包名
     * @param className   activity全路径类名
     * @return {@code true}: 是<br>{@code false}: 否
     */
    public static boolean isActivityExists(Context context, String packageName, String className) {
        Intent intent = new Intent();
        intent.setClassName(packageName, className);
        return !(context.getPackageManager().resolveActivity(intent, 0) == null ||
                intent.resolveActivity(context.getPackageManager()) == null ||
                context.getPackageManager().queryIntentActivities(intent, 0).size() == 0);
    }

    /**
     * 打开Activity
     *
     * @param context     上下文
     * @param packageName 包名
     * @param className   全类名
     */
    public static void launchActivity(Context context, String packageName, String className) {
        launchActivity(context, packageName, className, null);
    }

    /**
     *
     * @param context
     * @param className
     */
    public static void launchActivity(Context context, String className) {
        launchActivity(context, context.getPackageName(), className, null);
    }

    public static void startIntent(Context context, Class class1) {
        startIntent(context, class1, false);
    }

    @SuppressWarnings("rawtypes")
    public static void startIntent(Context context, Class class1, boolean flag) {
        startIntent(context, null, class1, flag);
    }

    public static void startIntent(Context context, Bundle bundle, @SuppressWarnings("rawtypes") Class class1, boolean
            isFinish) {
        try {
            Intent intent = new Intent(context, class1);
            if (null != bundle) {
                intent.putExtras(bundle);
            }
            context.startActivity(intent);
            if (isFinish) {
                ((Activity) context).finish();
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    /**
     * 打开Activity
     *
     * @param context     上下文
     * @param packageName 包名
     * @param className   全类名
     * @param bundle      bundle
     */
    public static void launchActivity(Context context, String packageName, String className, Bundle bundle) {
        context.startActivity(getComponentIntent(packageName, className, bundle));
    }

    public static Intent getComponentIntent(String packageName, String className, Bundle bundle) {
        Intent intent = new Intent(Intent.ACTION_VIEW);
        if (bundle != null) intent.putExtras(bundle);
        ComponentName cn = new ComponentName(packageName, className);
        intent.setComponent(cn);
        return intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
    }
}
