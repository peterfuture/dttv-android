package dttv.app.utils;

import dttv.app.R;

import android.app.Activity;
import android.content.Context;
import android.content.SharedPreferences;
import android.preference.PreferenceManager;

/**
 * 获取设置中的信息
 *
 * @author shihx1
 */
public class SettingUtil {

    private Activity mActivity;
    private SharedPreferences prefs;

    public SettingUtil(Activity activity) {
        this.mActivity = activity;
        prefs = PreferenceManager.getDefaultSharedPreferences(mActivity);
    }

    public boolean isFilterAudio() {
        boolean result = prefs.getBoolean(Constant.KEY_SETTING_FILTER_AUDIO_FILTER, false);
        return result;
    }

    public boolean isFilterVideo() {
        boolean result = prefs.getBoolean(Constant.KEY_SETTING_FILTER_VIDEO_FILTER, false);
        return result;
    }

    public String getVideoPlayerDecodeType() {
        String result = prefs.getString(Constant.KEY_SETTING_DECODER_TYPE, "1");
        return result;
    }

    public int isHWCodecEnable() {
        String result = prefs.getString(Constant.KEY_SETTING_DECODER_TYPE, "1");
        return Integer.parseInt(result);
    }

    public int getVideoPlayerDisplayMode() {
        String result = prefs.getString(Constant.KEY_SETTING_DISPLAY_MODE, "1");
        return Integer.parseInt(result);
    }


    public boolean isVideoPlayerDisplayFullScreen() {
        boolean result = prefs.getBoolean(Constant.KEY_SETTING_DISPLAY_MODE, true);
        return result;
    }

    public int getBrowserMode() {
        String result = prefs.getString(Constant.KEY_SETTING_BROWSER_MODE, "0");
        return Integer.parseInt(result);
    }
}
