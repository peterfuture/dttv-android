package dttv.app.utils;

import android.app.Activity;
import android.content.ContentResolver;
import android.content.Context;
import android.net.Uri;
import android.provider.Settings;
import android.provider.Settings.SettingNotFoundException;
import android.provider.Settings.System;
import android.view.WindowManager;

public class ControlLightness {
	
	private static ControlLightness controlLightness;
	
	private ControlLightness() {
		// TODO Auto-generated constructor stub
	}
	
	public static ControlLightness getInstance(){
		if(controlLightness==null)
			controlLightness = new ControlLightness();
		return controlLightness;
	}
	
	/**
	 * judge if auto open brightness
	 * @param context
	 * @return
	 */
	public boolean isAutoBrightness(Context context){
		boolean automicBrightness = false;
		ContentResolver ctr = context.getContentResolver();
		try {
			automicBrightness = Settings.System.getInt(ctr, 
			Settings.System.SCREEN_BRIGHTNESS_MODE) == Settings.System.SCREEN_BRIGHTNESS_MODE_AUTOMATIC;
		} catch (SettingNotFoundException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		return automicBrightness;
	}
	
	/**
	 * 
	 * @param context
	 * @param value
	 */
	public void setLightness(Activity act,int value){
		System.putInt(act.getContentResolver(), System.SCREEN_BRIGHTNESS, value);
		try {
			WindowManager.LayoutParams lp = act.getWindow().getAttributes();
			lp.screenBrightness = (value < 0 ? 1 : value)/255f;
			act.getWindow().setAttributes(lp);
			
			//saveBrightness(act,value);
		} catch (Exception e) {
			// TODO: handle exception
			e.printStackTrace();
		}
	}
	
	/**
	 * get lightness
	 * @param cr
	 * @return
	 */
	public int getLightness(Activity cr){
		return Settings.System.getInt(cr.getContentResolver(), Settings.System.SCREEN_BRIGHTNESS,255);
	}
	
	/**
	 * get current brightness
	 * @param cr
	 * @return
	 */
	public int getCurrentBrightness(Activity cr){
		int currentNess = Settings.System.getInt(cr.getContentResolver(),
				Settings.System.SCREEN_BRIGHTNESS, 255);
		return currentNess >20 ? currentNess : 20;
	}
	
	/**
	 * stop auto change
	 * brightness
	 * @param cr
	 */
	public void stopAutoBrightness(Activity cr) {
		Settings.System.putInt(cr.getContentResolver(),
				System.SCREEN_BRIGHTNESS, System.SCREEN_BRIGHTNESS_MODE_MANUAL);
	}
	
	/**
	 * start auto change
	 * brightness
	 * @param activity
	 */
	public void startAutoBrightness(Activity activity) {
		Settings.System.putInt(activity.getContentResolver(),
				Settings.System.SCREEN_BRIGHTNESS_MODE,
				Settings.System.SCREEN_BRIGHTNESS_MODE_AUTOMATIC);
	}
	
	/**
	 * save brightness value for entire
	 * system
	 * @param activity
	 * @param value
	 */
	public void saveBrightness(Activity activity,int value){
		Uri uri = Settings.System.getUriFor(Settings.System.SCREEN_BRIGHTNESS);
		Settings.System.putInt(activity.getContentResolver(), Settings.System.SCREEN_BRIGHTNESS, value);
		activity.getContentResolver().notifyChange(uri, null);
	}
	
	/**
	 * just this method
	 * @param cr
	 * @param value
	 */
	public void setBrightness(Activity cr,int value){
		if (0 <= value && value <= 255) {
			Settings.System.putInt(cr.getContentResolver(),
					Settings.System.SCREEN_BRIGHTNESS,	value); // 0-255
			value = Settings.System.getInt(cr.getContentResolver(),
					Settings.System.SCREEN_BRIGHTNESS, -1);
			// Cupcake way..... sucks
			WindowManager.LayoutParams lp = cr.getWindow().getAttributes();
			lp.screenBrightness = value;
			cr.getWindow().setAttributes(lp);
		}
	}
}
