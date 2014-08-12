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
import android.support.v4.app.Fragment;
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
import dttv.app.utils.TimesUtil;



@SuppressLint("NewApi")
public class AudioUIFragment extends Fragment implements I_OnMyKey,OnClickListener{
	static final String TAG = "AudioUIFragment";
	View rootView;
	ListView audio_listview;
	private RelativeLayout dt_play_bar_lay;
	private TextView currentTimeTxt,totalTimeTxt;
	private ImageButton preBtn,nextBtn,pauseBtn;
	private SeekBar playerProgressBar;
	private DtPlayer dtPlayer;
	private Cursor mCursor;
	//private SimpleCursorAdapter adapter;*/
	private int currentPosition;
	private List<String> playList;
	
	@Override
	public View onCreateView(LayoutInflater inflater, ViewGroup container,
			Bundle savedInstanceState) {
		
		rootView = inflater.inflate(R.layout.dt_audio_ui_fragment, container, false);
		
		return rootView;
	}
	@Override
	public void onActivityCreated(Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		super.onActivityCreated(savedInstanceState);
		dtPlayer = new DtPlayer(getActivity());
		initViews();
		initListener();
		initFillData();
	}
	
	private void initViews(){
		audio_listview = (ListView)rootView.findViewById(R.id.audio_listview);
		dt_play_bar_lay = (RelativeLayout)rootView.findViewById(R.id.dt_play_bar_lay);
		currentTimeTxt = (TextView)rootView.findViewById(R.id.dt_play_current_time);
		totalTimeTxt = (TextView)rootView.findViewById(R.id.dt_play_total_time);
		preBtn = (ImageButton)rootView.findViewById(R.id.dt_play_prev_btn);
		pauseBtn = (ImageButton)rootView.findViewById(R.id.dt_play_pause_btn);
		nextBtn = (ImageButton)rootView.findViewById(R.id.dt_play_next_btn);
		playerProgressBar = (SeekBar)rootView.findViewById(R.id.dt_play_progress_seekbar);
	}
	
	private void initListener(){
		audio_listview.setOnItemClickListener(new ListItemClickListener());
		dtPlayer.setOnPreparedListener(new PrePareListener());
		dtPlayer.setOnCompletionListener(new OnCompleteListener());
		playerProgressBar.setOnSeekBarChangeListener(new OnSeekChangeListener());
		preBtn.setOnClickListener(this);
		nextBtn.setOnClickListener(this);
		pauseBtn.setOnClickListener(this);
	}
	
	class PrePareListener implements OnPreparedListener{
		@Override
		public void onPrepared(DtPlayer mp) {
			// TODO Auto-generated method stub
			Log.i(Constant.LOGTAG, "enter onPrepared");
			//Toast.makeText(getActivity(), "enter onPrepared", 1).show();
			dtPlayer.start();
			int duration = mp.getDuration();
            if(duration>0){
            	totalTimeTxt.setText(TimesUtil.getTime(duration));
            	playerProgressBar.setMax(duration);
            }
            startTimerTask();
		}
	}
	
	class OnCompleteListener implements OnCompletionListener{
		@Override
		public void onCompletion(DtPlayer mp) {
			// TODO Auto-generated method stub
			playNextSong();
		}
	}
	
	class OnSeekChangeListener implements OnSeekBarChangeListener{

		@Override
		public void onProgressChanged(SeekBar seekBar, int progress,
				boolean fromUser) {
			//int currentTime = seekBar.getProgress();
			// TODO Auto-generated method stub
			//Log.d(Constant.LOGTAG, "----1---SeekTo:"+currentTime);
		}

		@Override
		public void onStartTrackingTouch(SeekBar seekBar) {
			// TODO Auto-generated method stub
			
		}

		@Override
		public void onStopTrackingTouch(SeekBar seekBar) {
			// TODO Auto-generated method stub
			int currentTime = seekBar.getProgress();
			dtPlayer.seekTo(currentTime);
			//dtPlayer.start();
			Log.d(Constant.LOGTAG, "----2---SeekTo:"+currentTime);
		}
		
	}
	
	private class ListItemClickListener implements OnItemClickListener{
		@Override
		public void onItemClick(AdapterView<?> ad, View v, int position,
				long arg3) {
			// TODO Auto-generated method stub
			dt_play_bar_lay.setVisibility(View.VISIBLE);
			//String uri = ad.getChildAt(position);
			currentPosition = position;
			playSong(currentPosition);
			/*String uri = ((TextView)v.findViewById(R.id.media_row_uri)).getText().toString();
			Toast.makeText(getActivity(), uri, 1).show();*/
		}
	}
	
	Handler doActionHandler = new Handler(Looper.getMainLooper()){
		public void handleMessage(android.os.Message msg) {
			int msgId = msg.what;
			switch(msgId){
			case Constant.REFRESH_TIME_MSG:
				int currentTime = dtPlayer.getCurrentPosition();
				currentTimeTxt.setText(TimesUtil.getTime(currentTime));
				playerProgressBar.setProgress(currentTime);
				break;
			case Constant.BEGIN_MEDIA_MSG:
				
	            //startTimerTask();
				break;
			case Constant.HIDE_OPREATE_BAR_MSG:
				dt_play_bar_lay.setVisibility(View.GONE);
				break;
			}
		};
	};
	
	private Timer mTimer;
	private void startTimerTask(){
		mTimer = new Timer();
		mTimer.schedule(new TimerTask() {
			
			@Override
			public void run() {
				// TODO Auto-generated method stub
				doActionHandler.sendEmptyMessage(Constant.REFRESH_TIME_MSG);
			}
		}, Constant.REFRESH_TIME, Constant.REFRESH_TIME);
	}
	
	private void releaseTimerAndHandler(){
		//isEnableTime = false;
		if(mTimer!=null)
			mTimer.cancel();
		doActionHandler.removeCallbacksAndMessages(null);
	}

    @Override
    public void onPause()
    {
    	releaseTimerAndHandler();
    	dtPlayer.pause();
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
		Cursor c = MusicUtils.query(context, MediaStore.Audio.Media.EXTERNAL_CONTENT_URI,
				str, MediaStore.Audio.Media.IS_MUSIC + "=1",
                null, MediaStore.Audio.Media.TITLE_KEY);
		return c;
	}
	
	private void initFillData(){
		playList = new ArrayList<String>();
		String[] fromColumns = new String[] {MediaStore.Audio.Media.TITLE, 
				MediaStore.Audio.Media.ARTIST,MediaStore.Audio.Media.DATA};
		int[] toLayoutIDs = new int[]{R.id.media_row_name,R.id.media_row_artist,R.id.media_row_uri};
		mCursor = readDataFromSD(getActivity());
		SimpleCursorAdapter adapter = new SimpleCursorAdapter(getActivity(), R.layout.dt_media_item, mCursor, fromColumns, toLayoutIDs, 0);
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
		dtPlayer.stop();
		dtPlayer.release();
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
		switch(v.getId()){
		case R.id.dt_play_next_btn:
			dtPlayer.stop();
			playNextSong();
			break;
		case R.id.dt_play_prev_btn:
			dtPlayer.stop();
			playPrevSong();
			break;
		case R.id.dt_play_pause_btn:
			handlePausePlay();
			break;
		}
	}
	
	private void playNextSong(){
		currentPosition = currentPosition + 1;
		if(currentPosition < playList.size()){
			playSong(currentPosition);
		}
	}
	
	private void playSong(int index){
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
			/*dtPlayer.reset();*/			
			dtPlayer.setDataSource(uri);
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
	}
	
	
	
	private void handlePausePlay(){
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
	}
}
