package dttv.app.utils;

import dttv.app.VideoPlayerActivity;
import android.app.Activity;
import android.content.Intent;

public class PlayerUtil {
	private static PlayerUtil instance;
	private PlayerUtil(){}
	
	public static PlayerUtil getInstance(){
		if(instance==null)
			instance = new PlayerUtil();
		return instance;
	}
	
	
	public void beginToPlayer(Activity cr,String uri,String name){
		Intent intent = new Intent();
		intent.putExtra(Constant.FILE_MSG, uri);
		intent.putExtra(Constant.MEIDA_NAME_STR, name);
		intent.setClass(cr, VideoPlayerActivity.class);
		cr.startActivity(intent);
	}
}
