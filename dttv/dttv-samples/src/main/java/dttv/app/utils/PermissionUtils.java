package dttv.app.utils;

import android.annotation.TargetApi;
import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.net.Uri;
import android.os.Build;
import android.provider.Settings;
import android.support.v4.content.ContextCompat;
import android.view.View;

import java.util.ArrayList;
import java.util.List;

public class PermissionUtils {
    private static int mRequestCode = -1;

    public static void requestPermissionsResult(Activity activity, int requestCode
            , String[] permission, OnPermissionListener callback){
        requestPermissions(activity, requestCode, permission, callback);
    }

    public static void requestPermissionsResult(android.app.Fragment fragment, int requestCode
            , String[] permission, OnPermissionListener callback){
        requestPermissions(fragment, requestCode, permission, callback);
    }

    public static void requestPermissionsResult(android.support.v4.app.Fragment fragment, int requestCode
            , String[] permission, OnPermissionListener callback){
        requestPermissions(fragment, requestCode, permission, callback);
    }

    /**
     * 请求权限处理
     * @param object        activity or fragment
     * @param requestCode   请求码
     * @param permissions   需要请求的权限
     * @param callback      结果回调
     */
    @TargetApi(Build.VERSION_CODES.M)
    private static void requestPermissions(Object object, int requestCode
            , String[] permissions, OnPermissionListener callback){

        checkCallingObjectSuitability(object);
        mOnPermissionListener = callback;

        List<String> deniedPermissions = getDeniedPermissions(getContext(object), permissions);
        if(deniedPermissions.size() > 0){
            mRequestCode = requestCode;

            if(object instanceof Activity){
                ((Activity) object).requestPermissions(deniedPermissions
                        .toArray(new String[deniedPermissions.size()]), requestCode);
            }else if(object instanceof android.app.Fragment){
                ((android.app.Fragment) object).requestPermissions(deniedPermissions
                        .toArray(new String[deniedPermissions.size()]), requestCode);
            }else if(object instanceof android.support.v4.app.Fragment){
                ((android.support.v4.app.Fragment) object).requestPermissions(deniedPermissions
                        .toArray(new String[deniedPermissions.size()]), requestCode);
            }
        }else
        if(checkPermissions(getContext(object), permissions)){
            if(mOnPermissionListener != null)
                mOnPermissionListener.onPermissionGranted();
        }else{

        }
        /*List<String> requestPermissionList = new ArrayList<>();
        //找出所有未授权的权限
        for (String permission : permissions) {
            if (ContextCompat.checkSelfPermission(getContext(object), permission) != PackageManager.PERMISSION_GRANTED) {
                requestPermissionList.add(permission);
            }
        }
        if (requestPermissionList.isEmpty()) {
            //已经全部授权
            if(mOnPermissionListener != null)
                mOnPermissionListener.onPermissionGranted();
        } else {
            //申请授权
            requestPermissions(requestPermissionList.toArray(object,requestCode,new String[requestPermissionList.size()]));
        }*/
    }

    /**
     * 获取上下文
     */
    private static Context getContext(Object object) {
        Context context;
        if(object instanceof android.app.Fragment){
            context = ((android.app.Fragment) object).getActivity();
        }else if(object instanceof android.support.v4.app.Fragment){
            context = ((android.support.v4.app.Fragment) object).getActivity();
        }else{
            context = (Activity) object;
        }
        return context;
    }

    /**
     * 请求权限结果，对应onRequestPermissionsResult()方法。
     */
    public static void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {
        if(requestCode == mRequestCode){
            if(verifyPermissions(grantResults)){
                if(mOnPermissionListener != null)
                    mOnPermissionListener.onPermissionGranted();
            }else{
                if(mOnPermissionListener != null){
                    mOnPermissionListener.onPermissionDenied();
                }
            }
        }
    }

    /**
     * 显示提示对话框
     */
    static AlertDialog mExitDialog;
    static  AlertWindowSettingPage alertWindowSettingPage;
    public static void showTipsDialog(final Activity context,final String content) {

        if (mExitDialog == null) {
            final AlertDialog.Builder builder = new AlertDialog.Builder(context);
            mExitDialog = builder
                    .setMessage(content).setTitle("温馨提示")
                    .setPositiveButton("确定",new DialogInterface.OnClickListener() {
                        @Override
                        public void onClick(DialogInterface dialog, int which) {
                            mExitDialog.dismiss();
                            //startAppSettings(context);
                            if (alertWindowSettingPage == null) {
                                alertWindowSettingPage = new AlertWindowSettingPage(context);
                            }
                            alertWindowSettingPage.start(mRequestCode);
                        }
                    }).setNegativeButton("取消",new DialogInterface.OnClickListener() {
                        @Override
                        public void onClick(DialogInterface dialog, int which) {
                            mExitDialog.dismiss();
                        }
                    }).show();
        } else {
            if (mExitDialog!= null && !mExitDialog.isShowing())
                mExitDialog.show();
        }
    }

    /**
     * 启动当前应用设置页面
     */
    private static void startAppSettings(Context context) {
        Intent intent = new Intent(Settings.ACTION_APPLICATION_DETAILS_SETTINGS);
        intent.setData(Uri.parse("package:" + context.getPackageName()));
        context.startActivity(intent);
    }

    /**
     * 验证权限是否都已经授权
     */
    private static boolean verifyPermissions(int[] grantResults) {
        // 如果请求被取消，则结果数组为空
        if(grantResults.length <= 0)
            return false;

        // 循环判断每个权限是否被拒绝
        for (int grantResult : grantResults) {
            if (grantResult != PackageManager.PERMISSION_GRANTED) {
                return false;
            }
        }
        return true;
    }

    /**
     * 获取权限列表中所有需要授权的权限
     * @param context       上下文
     * @param permissions   权限列表
     * @return
     */
    private static List<String> getDeniedPermissions(Context context, String... permissions){
        List<String> deniedPermissions = new ArrayList<>();
        for (String permission : permissions) {
            if(ContextCompat.checkSelfPermission(context, permission) == PackageManager.PERMISSION_DENIED){
                deniedPermissions.add(permission);
            }
        }
        return deniedPermissions;
    }

    /**
     * 检查所传递对象的正确性
     * @param object 必须为 activity or fragment
     */
    private static void checkCallingObjectSuitability(Object object) {
        if (object == null) {
            throw new NullPointerException("Activity or Fragment should not be null");
        }

        boolean isActivity = object instanceof android.app.Activity;
        boolean isSupportFragment = object instanceof android.support.v4.app.Fragment;
        boolean isAppFragment = object instanceof android.app.Fragment;

        if(!(isActivity || isSupportFragment || isAppFragment)){
            throw new IllegalArgumentException(
                    "Caller must be an Activity or a Fragment");
        }
    }

    /**
     * 检查所有的权限是否已经被授权
     * @param permissions 权限列表
     * @return
     */
    private static boolean checkPermissions(Context context, String... permissions){
        if(isOverMarshmallow()){
            for (String permission : permissions) {
                if(ContextCompat.checkSelfPermission(context, permission) == PackageManager.PERMISSION_DENIED){
                    return false;
                }
            }
        }
        return true;
    }

    /**
     * 判断当前手机API版本是否 >= 6.0
     */
    private static boolean isOverMarshmallow() {
        return Build.VERSION.SDK_INT >= Build.VERSION_CODES.M;
    }

    public interface OnPermissionListener{
        void onPermissionGranted();
        void onPermissionDenied();
        //void onPermissionRationale();
    }

    //permission.shouldShowRequestPermissionRationale

    private static OnPermissionListener mOnPermissionListener;
}
