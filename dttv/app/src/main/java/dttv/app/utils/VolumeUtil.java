package dttv.app.utils;

import android.app.Activity;
import android.app.Service;
import android.media.AudioManager;

public class VolumeUtil {
	private Activity cr;
	private AudioManager audioManager;
	public VolumeUtil(Activity activity) {
		// TODO Auto-generated constructor stub
		this.cr = activity;
		audioManager = (AudioManager)cr.getSystemService(Service.AUDIO_SERVICE);
	}
	
	
	public void upVolume(int value){
		audioManager.adjustStreamVolume(AudioManager.STREAM_MUSIC, AudioManager.ADJUST_RAISE, 
				AudioManager.FLAG_PLAY_SOUND);
	}
	
	public void downVolume(int value){
		audioManager.adjustStreamVolume(AudioManager.STREAM_MUSIC, AudioManager.ADJUST_LOWER,
				AudioManager.FLAG_PLAY_SOUND);
				//AudioManager.FLAG_PLAY_SOUND | AudioManager.FLAG_SHOW_UI);
	}
	
	public int getCurrentVolume(){
		int current = audioManager.getStreamVolume(AudioManager.STREAM_MUSIC);
		return current;
	}
	
	public int getMaxVolume(){
		int max = audioManager.getStreamMaxVolume(AudioManager.STREAM_MUSIC);
		return max;
	}
}
