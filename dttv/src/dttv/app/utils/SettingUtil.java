package dttv.app.utils;

import dttv.app.R;
import android.app.Activity;
import android.content.Context;
import android.content.SharedPreferences;
import android.preference.PreferenceManager;

/**
 * 获取设置中的信息
 * @author shihx1
 *
 */
public class SettingUtil {
	
	private  Activity mActivity;
	SharedPreferences prefs;
	
	public SettingUtil(Activity activity) {
		// TODO Auto-generated constructor stub
		this.mActivity = activity;
		prefs = PreferenceManager.getDefaultSharedPreferences(mActivity);
	}
	
	public  boolean isFilterAudio(){
		boolean result = prefs.getBoolean(Constant.FILTER_AUDIO_KEY, false);
		return result;
	}
	
	public  boolean isFilterVideo(){
		boolean result = prefs.getBoolean(Constant.FILTER_VIDEO_KEY, false);
		return result;
	}
	
	public  String getDecodeType(){
		String type = prefs.getString(Constant.DECODE_STYLE_KEY, 
				mActivity.getString(R.string.dt_decode_type_soft));
		return type;
	}
}
