package dttv.app.widget;

import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.Timer;
import java.util.TimerTask;

import android.annotation.SuppressLint;
import android.content.Context;
import android.database.Cursor;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.provider.MediaStore;
import android.provider.MediaStore.Audio.Media;
import android.support.v4.app.Fragment;
import android.text.TextUtils;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ImageButton;
import android.widget.ListView;
import android.widget.RelativeLayout;
import android.widget.SeekBar;
import android.widget.SeekBar.OnSeekBarChangeListener;
import android.widget.SimpleCursorAdapter;
import android.widget.TextView;
import android.widget.Toast;
import dttv.app.DtPlayer;
import dttv.app.DtPlayer.OnCompletionListener;
import dttv.app.DtPlayer.OnPreparedListener;
import dttv.app.R;
import dttv.app.impl.I_OnMyKey;
import dttv.app.utils.Constant;
import dttv.app.utils.MusicUtils;
import dttv.app.utils.PlayerUtil;
import dttv.app.utils.SettingUtil;
import dttv.app.utils.TimesUtil;



@SuppressLint("NewApi")
public class AudioUIFragment extends Fragment implements I_OnMyKey,OnClickListener{
	static final String TAG = "AudioUIFragment";
	View rootView;
	ListView audio_listview;
	private Cursor mCursor;
	//private SimpleCursorAdapter adapter;*/
	private int currentPosition;
	private List<String> playList;
	private int trick_seek = 0;
	SettingUtil settingUtil;
	
	public AudioUIFragment() {
		// TODO Auto-generated constructor stub
	}
	
	@Override
	public View onCreateView(LayoutInflater inflater, ViewGroup container,
			Bundle savedInstanceState) {
		
		rootView = inflater.inflate(R.layout.dt_audio_ui_fragment, container, false);
		settingUtil = new SettingUtil(getActivity());
		return rootView;
	}
	@Override
	public void onActivityCreated(Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		super.onActivityCreated(savedInstanceState);
		initViews();
		initListener();
		initFillData();
	}
	
	private void initViews(){
		audio_listview = (ListView)rootView.findViewById(R.id.audio_listview);
	}
	
	private void initListener(){
		audio_listview.setOnItemClickListener(new ListItemClickListener());
	}
	
	
	@Override
	public void onResume() {
		// TODO Auto-generated method stub
		/*if(dtPlayer!=null)
			dtPlayer.start();*/
		super.onResume();
	}
	
	
	private class ListItemClickListener implements OnItemClickListener{
		@Override
		public void onItemClick(AdapterView<?> ad, View v, int position,
				long arg3) {
			// TODO Auto-generated method stub
			/*dt_play_bar_lay.setVisibility(View.VISIBLE);
			//String uri = ad.getChildAt(position);
			currentPosition = position;
			playSong(currentPosition);*/
			String uri = playList.get(position);
			String name = ((TextView)v.findViewById(R.id.media_row_name)).getText().toString();
			Toast.makeText(getActivity(), name, 1).show();
			PlayerUtil.getInstance().beginToPlayer(getActivity(), uri, name,Constant.LOCAL_AUDIO);
		}
	}
	
    @Override
    public void onPause()
    {
    	//releaseTimerAndHandler();
        super.onPause();
        Log.d(TAG,"--PAUSE--");    
    }
	
	private Cursor readDataFromSD(Context context) {
		Log.d(TAG, "scanFile");
		String[] str = new String[]{MediaStore.Audio.Media._ID,
				MediaStore.Audio.Media.DISPLAY_NAME,
				MediaStore.Audio.Media.TITLE,
				MediaStore.Audio.Media.DURATION,
				MediaStore.Audio.Media.ARTIST,
				MediaStore.Audio.Media.ALBUM,
				MediaStore.Audio.Media.YEAR,
				MediaStore.Audio.Media.MIME_TYPE,
				MediaStore.Audio.Media.SIZE,
				MediaStore.Audio.Media.DATA};
		StringBuffer select = new StringBuffer("");
		// 查询语句：检索出.mp3为后缀名，时长大于1分钟，文件大小大于1MB的媒体文件
		/*if(sp.getFilterSize()) {
			select.append(" and " + Media.SIZE + " > " + FILTER_SIZE);
		}*/
		if(settingUtil.isFilterAudio()) {
			select.append(" and " + Media.DURATION + " > " + Constant.FILTER_DURATION);
		}

		/*if (!TextUtils.isEmpty(selections)) {
			select.append(selections);
		}*/
		Cursor c = MusicUtils.query(context, MediaStore.Audio.Media.EXTERNAL_CONTENT_URI,
				str, MediaStore.Audio.Media.IS_MUSIC + "=1"+select,
                null, MediaStore.Audio.Media.TITLE_KEY);
		return c;
	}
	
	private void initFillData(){
		playList = new ArrayList<String>();
		String[] fromColumns = new String[] {MediaStore.Audio.Media.TITLE, 
				MediaStore.Audio.Media.ARTIST,MediaStore.Audio.Media.DATA};
		int[] toLayoutIDs = new int[]{ R.id.media_row_name,R.id.media_row_artist,R.id.media_row_uri};
		mCursor = readDataFromSD(getActivity());
		SimpleCursorAdapter adapter = new SimpleCursorAdapter(getActivity(), R.layout.dt_audio_item, mCursor, fromColumns, toLayoutIDs, 0);
		audio_listview.setAdapter(adapter);
		while(mCursor.moveToNext()){
			playList.add(mCursor.getString(mCursor.getColumnIndex(MediaStore.Audio.Media.DATA)));
		}
		//mCursor.close();
	}
	
	@Override
	public void onDestroyView() {
		// TODO Auto-generated method stub
		super.onDestroyView();
	}
	
	@Override
	public void onDestroy() {
		// TODO Auto-generated method stub
		mCursor.close();
		audio_listview = null;
		super.onDestroy();
	}
	@Override
	public void myOnKeyDown(int keyCode) {
		// TODO Auto-generated method stub
		
	}
	@Override
	public void onClick(View v) {
		// TODO Auto-generated method stub
		/*switch(v.getId()){
		case R.id.dt_play_next_btn:
			Log.i(TAG, "click next song btn");
			playNextSong();
			break;
		case R.id.dt_play_prev_btn:
			playPrevSong();
			break;
		case R.id.dt_play_pause_btn:
			handlePausePlay();
			break;
		}*/
	}
	
	/*private void playNextSong(){
		Log.i(TAG, "enter play nextSong function");
		currentPosition = currentPosition + 1;
		if(currentPosition < playList.size()){
			playSong(currentPosition);
		}
	}*/
	
	/*private void playSong(int index){
		dtPlayer.stop();
		String uri = playList.get(index);
		Log.i(TAG, "setDataSource uri is:"+uri);
		if(uri==null){
			Toast.makeText(getActivity(), "uri is null", 1).show();
			return;
		}else{
			//Toast.makeText(getActivity(), uri, 1).show();
		}
		try {
			//dtPlayer.release();
			dtPlayer.reset();		
			if(dtPlayer.setDataSource(uri) == -1)
				return;
			dtPlayer.prepare();
		} catch (IllegalArgumentException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (SecurityException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (IllegalStateException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}
	
	private void playPrevSong(){
		currentPosition = currentPosition - 1;
		if(currentPosition>0){
			playSong(currentPosition);
		}
	}*/
	
	
	
	/*private void handlePausePlay(){
		try {
			if(dtPlayer.isPlaying()){
				dtPlayer.pause();
				pauseBtn.setBackgroundResource(R.drawable.btn_mu_pause);
			}else{
				dtPlayer.pause();
				pauseBtn.setBackgroundResource(R.drawable.btn_mu_play);
			}
		} catch (IllegalStateException e) {
			// TODO: handle exception
		} catch (Exception e) {
			// TODO: handle exception
		}
	}*/
}
