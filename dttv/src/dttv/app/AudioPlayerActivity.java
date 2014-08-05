package dttv.app;

import java.io.IOException;
import java.util.Timer;
import java.util.TimerTask;

import dttv.app.DtPlayer.OnCompletionListener;
import dttv.app.DtPlayer.OnPreparedListener;
import dttv.app.utils.Constant;
import dttv.app.utils.Log;
import dttv.app.utils.TimesUtil;
import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.ImageButton;
import android.widget.RelativeLayout;
import android.widget.SeekBar;
import android.widget.SeekBar.OnSeekBarChangeListener;
import android.widget.TextView;
import android.widget.Toast;
/**
 * just test for
 * audio
 * @author shihx1
 *	
 */
public class AudioPlayerActivity extends Activity implements OnClickListener{
	private String TAG = "AudioPlayerActivity";
	private DtPlayer dtPlayer;
	private String mPath;
	
	private View mBarView;
	private RelativeLayout playerBarLay;
	private TextView currentTimeTxt,totalTimeTxt;
	private ImageButton preBtn,nextBtn,pauseBtn;
	private SeekBar playerProgressBar;
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		super.onCreate(savedInstanceState);
		setContentView(R.layout.audio_play);
		dtPlayer = new DtPlayer(this);
		initExtraData();
		initView();
		initListener();
	}
	
	private void initExtraData(){
		Intent intent = getIntent();
		mPath = intent.getStringExtra(Constant.FILE_MSG);
		Toast.makeText(this, "mPath is:"+mPath, 1).show();
		try {
			dtPlayer.setDataSource(mPath);
			dtPlayer.prepare();
			//dtPlayer.start();
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
	
	private void initView(){
		mBarView = (View)findViewById(R.id.audio_player_bar_lay);
		playerBarLay = (RelativeLayout)mBarView.findViewById(R.id.dt_play_bar_lay);
		currentTimeTxt = (TextView)mBarView.findViewById(R.id.dt_play_current_time);
		totalTimeTxt = (TextView)mBarView.findViewById(R.id.dt_play_total_time);
		preBtn = (ImageButton)mBarView.findViewById(R.id.dt_play_prev_btn);
		pauseBtn = (ImageButton)mBarView.findViewById(R.id.dt_play_pause_btn);
		nextBtn = (ImageButton)mBarView.findViewById(R.id.dt_play_next_btn);
		playerProgressBar = (SeekBar)mBarView.findViewById(R.id.dt_play_progress_seekbar);
	}
	
	private void initListener(){
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
			
		}
	}
	
	class OnSeekChangeListener implements OnSeekBarChangeListener{

		@Override
		public void onProgressChanged(SeekBar seekBar, int progress,
				boolean fromUser) {
			// TODO Auto-generated method stub
			
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
			dtPlayer.start();
		}
		
	}
	
	@Override
	protected void onResume() {
		// TODO Auto-generated method stub
		super.onResume();
		if(dtPlayer!=null){
			dtPlayer.start();
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
				playerBarLay.setVisibility(View.GONE);
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
	
	
	@Override
	protected void onStop() {
		// TODO Auto-generated method stub
		dtPlayer.stop();
		dtPlayer.release();
		super.onStop();
	}
	
	

	@Override
	public void onClick(View v) {
		// TODO Auto-generated method stub
		switch(v.getId()){
		case R.id.dt_play_next_btn:
			break;
		case R.id.dt_play_prev_btn:
			break;
		case R.id.dt_play_pause_btn:
			handlePausePlay();
			break;
		}
	}
	
	private void handlePausePlay(){
		try {
			if(dtPlayer.isPlaying()){
				dtPlayer.pause();
				pauseBtn.setBackgroundResource(R.drawable.btn_mu_pause);
			}else{
				dtPlayer.start();
				pauseBtn.setBackgroundResource(R.drawable.btn_mu_play);
			}
		} catch (IllegalStateException e) {
			// TODO: handle exception
		} catch (Exception e) {
			// TODO: handle exception
		}
	}
	
	
	@Override
	protected void onDestroy() {
		// TODO Auto-generated method stub
		super.onDestroy();
	}
}
