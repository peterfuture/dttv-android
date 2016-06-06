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
    SharedPreferences prefs;

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
        String type = prefs.getString(Constant.KEY_SETTING_DECODER_TYPE,
                mActivity.getString(R.string.setting_preference_video_decoder_soft));
        return type;
    }

    public boolean isVideoPlayerDisplayFullScreen() {
        boolean result = prefs.getBoolean(Constant.KEY_SETTING_DISPLAY_MODE, true);
        return result;
    }

    public int getBrowserMode() {
        String result = prefs.getString(Constant.KEY_SETTING_BROWSER_MODE, "normal");
        if(result.contains("normal"))
            return 0;
        if(result.contains("audio_only"))
            return 1;
        if(result.contains("video_only"))
            return 2;

        return 0;
    }
}
