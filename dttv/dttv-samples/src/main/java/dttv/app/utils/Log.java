package dttv.app.utils;

/**
 * @author shihx1
 *         just simple log for debug
 */
public class Log {
    private static boolean isDebug = true;
    private static final String T_TAG = "[Dttv-Player]";

    public static void i(String TAG, String msg) {
        TAG = (TAG == null ? T_TAG : TAG);
        if (isDebug)
            android.util.Log.i(TAG, msg);
    }

    public static void d(String TAG, String msg) {
        TAG = (TAG == null ? T_TAG : TAG);
        if (isDebug)
            android.util.Log.d(TAG, msg);
    }

    public static void e(String TAG, String msg) {
        TAG = (TAG == null ? T_TAG : TAG);
        if (isDebug)
            android.util.Log.e(TAG, msg);
    }

    public static void w(String TAG, String msg) {
        TAG = (TAG == null ? T_TAG : TAG);
        if (isDebug)
            android.util.Log.w(TAG, msg);
    }

    public static void v(String TAG, String msg) {
        TAG = (TAG == null ? T_TAG : TAG);
        if (isDebug)
            android.util.Log.v(TAG, msg);
    }
}
