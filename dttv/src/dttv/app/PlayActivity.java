package dttv.app;

import java.util.Timer;
import java.util.TimerTask;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import dttv.app.utils.Constant;
import dttv.app.utils.TimesUtil;
import dttv.app.widget.GlVideoView;
import android.app.Activity;
import android.content.Intent;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;
import android.view.Menu;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.View.OnTouchListener;
import android.widget.ImageButton;
import android.widget.RelativeLayout;
import android.widget.SeekBar;
import android.widget.SeekBar.OnSeekBarChangeListener;
import android.widget.TextView;


public class PlayActivity extends Activity implements OnClickListener, OnTouchListener{
	
	private final String TAG = "DT-PLAYING";
    private int surface_width = 320;
    private int surface_height = 240;
    
    private final int PLAYER_STATUS_IDLE=0;
    private final int PLAYER_STATUS_RUNNING=1;
    private final int PLAYER_STATUS_PAUSED=2;
    private final int PLAYER_STATUS_QUIT=3;
    
    private boolean isEnableTime = false;
    
    private String strFileName;
    private int playerStatus=PLAYER_STATUS_IDLE;
        
	//Native API declare
    private static native void native_gl_resize(int w, int h);
	private native int native_ui_init(int w, int h);
	private native int native_disp_frame();
	private native int native_ui_stop();
	
	private native int native_playerStart(String url);
	private native int native_playerPause();
	private native int native_playerResume();
	private native int native_playerStop();
	private native int native_playerSeekTo(int pos);
	
	private native int native_getCurrentPostion();
	private native int native_getDuration();
	private native int native_getPlayerStatus();
	
	private GlVideoView glSurfaceView;
	private SeekBar playerBar;
	private TextView currentTimeTxt,totalTimeTxt;
	private ImageButton pauseBtn,nextBtn,preBtn;
	private RelativeLayout opreateLay;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_play);
		
		//start playing video
		Intent intent = getIntent();
		strFileName = intent.getStringExtra(Constant.FILE_MSG);
		Log.d(TAG, "Start playing "+strFileName);
		
		initWidget();
		
		//glSurfaceView = new GLSurfaceView(this);
        //glSurfaceView.setRenderer(new GLSurfaceViewRender());  
        //glSurfaceView.setRenderMode(GLSurfaceView.RENDERMODE_CONTINUOUSLY);
        //this.setContentView(glSurfaceView);
        initListener();
	}
	
	private void initWidget(){
		playerBar = (SeekBar)findViewById(R.id.dt_play_progress_seekbar);
		glSurfaceView = (GlVideoView)findViewById(R.id.glvideo_view);
		glSurfaceView.setRenderer(new GLSurfaceViewRender());  
        glSurfaceView.setRenderMode(GLSurfaceView.RENDERMODE_CONTINUOUSLY);
        glSurfaceView.setOnTouchListener(this);
		currentTimeTxt = (TextView)findViewById(R.id.dt_play_current_time);
		totalTimeTxt = (TextView)findViewById(R.id.dt_play_total_time);
		pauseBtn = (ImageButton)findViewById(R.id.dt_play_pause_btn);
		nextBtn = (ImageButton)findViewById(R.id.dt_play_next_btn);
		preBtn = (ImageButton)findViewById(R.id.dt_play_prev_btn);
		opreateLay = (RelativeLayout)findViewById(R.id.dt_play_bar_lay);
	}
	
	private void initListener(){
		pauseBtn.setOnClickListener(this);
		nextBtn.setOnClickListener(this);
		preBtn.setOnClickListener(this);
		playerBar.setOnSeekBarChangeListener(new SeekChangeListener());
	}
	
	
	private class SeekChangeListener implements OnSeekBarChangeListener{

		@Override
		public void onProgressChanged(SeekBar bar, int arg1, boolean arg2) {
			// TODO Auto-generated method stub
			
		}

		@Override
		public void onStartTrackingTouch(SeekBar bar) {
			// TODO Auto-generated method stub
			
		}

		@Override
		public void onStopTrackingTouch(SeekBar bar) {
			// TODO Auto-generated method stub
			int currentPosition = bar.getProgress();
			Log.i(TAG, "----currentPosition is:"+currentPosition);
			native_playerSeekTo(currentPosition);
		}
	}
	
	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.play, menu);
		return true;
	}
	
	Handler doActionHandler = new Handler(Looper.getMainLooper()){
		public void handleMessage(android.os.Message msg) {
			int msgId = msg.what;
			switch(msgId){
			case Constant.REFRESH_TIME_MSG:
				int duration = native_getDuration();
	            if(duration>0){
	            	totalTimeTxt.setText(TimesUtil.getTime(duration));
	            	playerBar.setMax(duration);
	            }
				int currentTime = native_getCurrentPostion();
				Log.i(TAG, "currentTime is:"+currentTime+" duration is:"+duration);
				currentTimeTxt.setText(TimesUtil.getTime(currentTime));
				playerBar.setProgress(currentTime);
				break;
			case Constant.BEGIN_MEDIA_MSG:
				
	            //startTimerTask();
				break;
			case Constant.HIDE_OPREATE_BAR_MSG:
				opreateLay.setVisibility(View.GONE);
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
		isEnableTime = false;
		if(mTimer!=null)
			mTimer.cancel();
		doActionHandler.removeCallbacksAndMessages(null);
	}

    @Override
    public void onPause()
    {
    	releaseTimerAndHandler();
        super.onPause();
        Log.d(TAG,"--PAUSE--");    
    }
    
    @Override
    public void onDestroy()
    {
        super.onDestroy();
        Log.d(TAG,"--DESTROY--");

        if(playerStatus != PLAYER_STATUS_IDLE)
        {
            native_playerStop();
            playerStatus = PLAYER_STATUS_IDLE;
        }
        releaseTimerAndHandler();
    }


	class GLSurfaceViewRender implements GLSurfaceView.Renderer {  
		  
        @Override  
        public void onSurfaceCreated(GL10 gl, EGLConfig config) {  
            Log.i(TAG, "onSurfaceCreated");  
             
            // 设置背景颜色 - load backgroud picture 
            //gl.glClearColor(0.0f, 0f, 1f, 0.5f);
        }  
  
        @Override  
        public void onSurfaceChanged(GL10 gl, int width, int height) {  
            // 设置输出屏幕大小 
        	native_gl_resize(width, height);
        	surface_width = width;
        	surface_height = height;

            if(playerStatus == PLAYER_STATUS_IDLE)
            {
                Log.i(TAG, "surface changed, start play:"+strFileName);  
                native_ui_init(surface_width,surface_height); 
                native_playerStart(strFileName);
                playerStatus = PLAYER_STATUS_RUNNING;
            }
            startTimerTask();
            Log.i(TAG, "enter onSurfaceChanged");
            Log.i(TAG, "cur time:"+native_getCurrentPostion() +"full_time:"+native_getDuration());
            //other case
        }
  
        @Override  
        public void onDrawFrame(GL10 gl) {  
            //Log.i(TAG, "onDrawFrame");  
            // 清除屏幕和深度缓存(如果不调用该代码, 将不显示glClearColor设置的颜色)  
            // 同样如果将该代码放到 onSurfaceCreated 中屏幕会一直闪动  
        	isEnableTime = true;
            //gl.glClear(GL10.GL_COLOR_BUFFER_BIT);
            native_disp_frame();
            //Log.i(TAG, "cur time:"+native_getCurrentPostion() +"full_time:"+native_getDuration());  
        }  
  
    }  
	
	static {
		System.loadLibrary("dtp_jni");
	}

	@Override
	public void onClick(View v) {
		// TODO Auto-generated method stub
		switch(v.getId()){
		case R.id.dt_play_pause_btn:
			handlePausePlay();
			break;
		case R.id.dt_play_prev_btn:
			break;
		case R.id.dt_play_next_btn:
			break;
		}
	}
	
	private void handlePausePlay(){
		switch(playerStatus){
		case PLAYER_STATUS_RUNNING:
			native_playerPause();
			pauseBtn.setBackgroundResource(R.drawable.btn_mu_play);
			playerStatus = PLAYER_STATUS_PAUSED;
			break;
		case PLAYER_STATUS_PAUSED:
			native_playerResume();
			pauseBtn.setBackgroundResource(R.drawable.btn_mu_pause);
			playerStatus = PLAYER_STATUS_RUNNING;
			break;
		}
	}
	@Override
	public boolean onTouch(View v, MotionEvent motionEvent) {
		// TODO Auto-generated method stub
		Log.i(TAG, "enter onTouch");
		opreateLay.setVisibility(View.VISIBLE);
		doActionHandler.removeMessages(Constant.HIDE_OPREATE_BAR_MSG);
		doActionHandler.sendEmptyMessageDelayed(Constant.HIDE_OPREATE_BAR_MSG, 5*Constant.REFRESH_TIME);
		return false;
	}

}
