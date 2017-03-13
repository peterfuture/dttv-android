package dttv.app;

import java.io.IOException;
import java.util.Timer;
import java.util.TimerTask;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

import javax.microedition.khronos.opengles.GL10;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.app.ActivityManager;
import android.content.Context;
import android.content.Intent;
import android.content.pm.ActivityInfo;
import android.content.pm.ConfigurationInfo;
import android.content.res.Configuration;
import android.opengl.GLSurfaceView;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.util.DisplayMetrics;
import android.view.Display;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.View.OnTouchListener;
import android.view.ViewGroup;
import android.view.Window;
import android.widget.AdapterView;
import android.widget.Button;
import android.widget.ImageButton;
import android.widget.LinearLayout;
import android.widget.LinearLayout.LayoutParams;
import android.widget.ProgressBar;
import android.widget.RelativeLayout;
import android.widget.SeekBar;
import android.widget.SeekBar.OnSeekBarChangeListener;
import android.widget.TextView;
import android.widget.Toast;


import app.dttv.dttvlib.MediaPlayer;
import app.dttv.dttvlib.MediaPlayer.OnCompletionListener;
import app.dttv.dttvlib.MediaPlayer.OnPreparedListener;
import app.dttv.dttvlib.MediaPlayer.OnFreshVideo;

import dttv.app.compnent.PopWindowCompnent;
import dttv.app.impl.ICallBack;
import dttv.app.utils.Constant;
import dttv.app.utils.ControlLightness;
import dttv.app.utils.Log;
import dttv.app.utils.SettingUtil;
import dttv.app.utils.TimesUtil;
import dttv.app.utils.VolumeUtil;
import dttv.app.widget.GlVideoView;
import dttv.app.widget.OnTouchMoveListener;

/**
 * VideoPlayer Activity
 *
 * @author shihx1
 */

public class VideoPlayerActivity extends Activity implements OnClickListener, OnTouchListener {

    private String TAG = "VideoPlayerActivity";

    /*MACRO*/
    private static final int PLAYER_IDLE = 0x0;
    private static final int PLAYER_INIT_START = 0x1;
    private static final int PLAYER_INITED = 0x2;
    private static final int PLAYER_PREPAR = 0x3;
    private static final int PLAYER_PREPARED = 0x4;
    private static final int PLAYER_RUNNING = 0x5;
    private static final int PLAYER_PAUSED = 0x6;
    private static final int PLAYER_BUFFERING_START = 0x7;
    private static final int PLAYER_BUFFERING_END = 0x8;
    private static final int PLAYER_SEEKING = 0x8;
    private static final int PLAYER_STOP = 0x100;
    private static final int PLAYER_EXIT = 0x101;

    private final int SCREEN_ORIGINAL = 0;
    private final int SCREEN_169value = 1;
    private final int SCREEN_FULLSCREEN = 2;
    private final int SCREEN_43value = 3;
    private final int SCREEN_NORMALSCALE = 4;

    /*view*/

    private RelativeLayout mRelativeLayoutRootView;

    private RelativeLayout mRelativeLayoutSurface;

    private LinearLayout mLinearLayoutControlPanel;
    private LinearLayout mLinearLayoutTopBar;

    private ImageButton mButtonPause;
    private ImageButton mButtonBack;
    private ImageButton mButtonSetting;
    private ImageButton mButtonRotate;
    private ImageButton mButtonPre;
    private ImageButton mButtonNext;
    private ImageButton mButtonRatio;

    private Button mButtonAudioEffect;

    private TextView mTextViewCurrentTime;
    private TextView mTextViewDuration;
    private TextView mTextViewUrl;
    private TextView mTextViewDecoderType;

    private SeekBar mSeekBarProgress;
    private ProgressBar mProgressBarBright, mProgressBarVolume;
    private GlVideoView mGLSurfaceView;

    /*varibles*/

    private MediaPlayer mMediaPlayer;
    private String mPath;
    private int mState = PLAYER_IDLE;
    private int mSeekFlag = 0;
    private boolean mDisableScale = false;

    private int mSurfaceWidth = 320;
    private int mSurfaceHeight = 240;
    private int mCurrentPosition = -1;

    private int mScreenWidth;
    private int mScreenHeight;
    private int mCurrentLightness;

    private final static int VIDEOPLAYER_DISPLAY_ORIGINAL = 0;
    private final static int VIDEOPLAYER_DISPLAY_FULLSCREEN = 1;
    private int mDisplayMode = VIDEOPLAYER_DISPLAY_ORIGINAL;

    private int mHWCodecEnable = 1;

    /*utils*/
    private VolumeUtil mVolumeUtil;
    private SettingUtil mSettingUtil;


    private final int HANDLE_UP = 0x0110;
    private final int HANDLE_DOWN = HANDLE_UP + 1;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        setContentView(R.layout.videoplayer_layout);

        if (GLES2Support() == 0) {
            return;
        }

        /*mMediaPlayer need to init prior to mGLSurfaceView*/
        mState = PLAYER_IDLE;

        mSettingUtil = new SettingUtil(this);
        mDisplayMode = mSettingUtil.getVideoPlayerDisplayMode();
        mHWCodecEnable = mSettingUtil.isHWCodecEnable();
        Log.i(TAG, "getDisplaymode: " + mDisplayMode + " HWCodec Enable:" + mHWCodecEnable);

        mMediaPlayer = new MediaPlayer(this, mHWCodecEnable!=0);

        getWindow().setBackgroundDrawableResource(R.color.videoplayer_background);
        initView();
        initDisplay();
        /*new Thread(new Runnable() {
            @Override
            public void run() {

            }
        }).start();*/
        setDataSource();

        prepare();
        initListener();
    }

    private boolean isProbablyEmulator() {
        return Build.VERSION.SDK_INT >= Build.VERSION_CODES.ICE_CREAM_SANDWICH_MR1
                && (Build.FINGERPRINT.startsWith("generic")
                || Build.FINGERPRINT.startsWith("unknown")
                || Build.MODEL.contains("google_sdk")
                || Build.MODEL.contains("Emulator")
                || Build.MODEL.contains("Android SDK built for x86"));
    }

    private int GLES2Support() {
        ActivityManager activityManager = (ActivityManager) getSystemService(Context.ACTIVITY_SERVICE);
        ConfigurationInfo configurationInfo = activityManager.getDeviceConfigurationInfo();

        boolean supportsEs2 = configurationInfo.reqGlEsVersion >= 0x20000 || isProbablyEmulator();
        if (supportsEs2)
            return 1;
        else
            return 0;
    }

    private void initView() {

        mLinearLayoutTopBar = (LinearLayout) findViewById(R.id.videoplayer_top_bar);
        mLinearLayoutControlPanel = (LinearLayout) findViewById(R.id.videoplayer_control_panel);
        mRelativeLayoutRootView = (RelativeLayout) findViewById(R.id.videoplayer_layout_rootview);
        mRelativeLayoutSurface = (RelativeLayout) findViewById(R.id.videoplayer_relativeout_surface);

        // mGLSurfaceView
        mGLSurfaceView = (GlVideoView) mRelativeLayoutSurface.findViewById(R.id.videoplayer_glvideoview);
        mGLSurfaceView.setRenderer(new MyGLSurfaceViewRender());
        mGLSurfaceView.setTouchMoveListener(new GLMoveTouchListener());
        //mGLSurfaceView.setRenderMode(mGLSurfaceView.RENDERMODE_CONTINUOUSLY);
        mGLSurfaceView.setRenderMode(mGLSurfaceView.RENDERMODE_WHEN_DIRTY);
        //mGLSurfaceView.setOnTouchListener((OnTouchListener) this);

        mTextViewUrl = (TextView) mLinearLayoutTopBar.findViewById(R.id.videoplayer_url);
        mTextViewCurrentTime = (TextView) mLinearLayoutControlPanel.findViewById(R.id.videoplayer_textview_current_time);
        mTextViewDuration = (TextView) mLinearLayoutControlPanel.findViewById(R.id.videoplayer_textview_duration);

        mButtonBack = (ImageButton) mLinearLayoutTopBar.findViewById(R.id.videoplayer_button_back);
        mButtonSetting = (ImageButton) mLinearLayoutTopBar.findViewById(R.id.videoplayer_button_setting);
        mButtonPause = (ImageButton) mLinearLayoutControlPanel.findViewById(R.id.videoplayer_button_pause);
        mButtonRatio = (ImageButton) mLinearLayoutControlPanel.findViewById(R.id.videoplayer_button_ratio);
        if (mDisplayMode == VIDEOPLAYER_DISPLAY_ORIGINAL)
            mButtonRatio.setBackgroundResource(R.drawable.videoplayer_button_ratio_normal);
        else
            mButtonRatio.setBackgroundResource(R.drawable.videoplayer_button_ratio_fullscreen);

        mSeekBarProgress = (SeekBar) mLinearLayoutControlPanel.findViewById(R.id.videoplayer_seekbar_progress);

        mProgressBarBright = (ProgressBar) findViewById(R.id.videoplayer_bright_progressbar);
        mProgressBarVolume = (ProgressBar) findViewById(R.id.videoplayer_volume_progress);

        mVolumeUtil = new VolumeUtil(this);
        mProgressBarVolume.setMax(mVolumeUtil.getMaxVolume());
        mProgressBarVolume.setProgress(mVolumeUtil.getCurrentVolume());
    }

    private void initDisplay() {
        DisplayMetrics displayMetrics = new DisplayMetrics();
        getWindowManager().getDefaultDisplay().getMetrics(displayMetrics);
        mScreenWidth = displayMetrics.widthPixels;
        mScreenHeight = displayMetrics.heightPixels;
        if (mScreenWidth == 0) {
            Display display = getWindowManager().getDefaultDisplay();
            mScreenWidth = display.getWidth();
            mScreenHeight = display.getHeight();
        }
    }

    private void updateSettingPanel(int hasaudio, int hasvideo, int hassub) {
        return;
    }

    @SuppressLint("ShowToast")
    private void setDataSource() {
        Intent intent = getIntent();
        mPath = intent.getStringExtra(Constant.FILE_MSG);
        String mediaName = intent.getStringExtra(Constant.MEIDA_NAME_STR);
        mTextViewUrl.setText(mediaName);

        //----------------------add type--------------------------------------
        //Toast.makeText(this, "playing:" + mPath, 1).show();
        try {
            mState = PLAYER_INIT_START;
            mMediaPlayer.setDataSource(mPath);
            mState = PLAYER_INITED;

            //setup video size
            int width = mMediaPlayer.getVideoWidth();
            int height = mMediaPlayer.getVideoHeight();
            if (mDisplayMode == VIDEOPLAYER_DISPLAY_FULLSCREEN) {
                width = mScreenWidth;
                height = mScreenHeight;
            }

            Log.d(TAG, "--width:" + width + "  height:" + height);
            if (width > 0 && height > 0 && width <= 1920 && height <= 1088) {
                ViewGroup.LayoutParams layoutParams = mGLSurfaceView.getLayoutParams();
                layoutParams.width = width;
                layoutParams.height = height;
                mGLSurfaceView.setLayoutParams(layoutParams);
            }
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
        mCurrentLightness = ControlLightness.getInstance().getLightness(this);
        if (mCurrentLightness >= 255) {
            mCurrentLightness = 255;
        } else if (mCurrentLightness <= 0) {
            mCurrentLightness = 0;
        }
    }

    private void prepare() {
        try {
            mState = PLAYER_PREPAR;
            mMediaPlayer.prepareAsync();
        } catch (IOException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
    }

    private void initListener() {
        mMediaPlayer.setOnPreparedListener(new PrePareListener());
        mMediaPlayer.setOnFreshVideo(new FreshVideo());
        mMediaPlayer.setOnCompletionListener(new OnCompleteListener());

        mSeekBarProgress.setOnSeekBarChangeListener(new OnSeekChangeListener());
/*
        mButtonPre.setOnClickListener(this);
        mButtonNext.setOnClickListener(this);
        mButtonRatio.setOnClickListener(this);
        mButtonAudioEffect.setOnClickListener(this);
        mButtonRatio.setOnClickListener(this);
        mButtonRotate.setOnClickListener(this);
        mButtonRotate.setOnTouchListener(this);
*/
        mButtonBack.setOnClickListener(this);
        mButtonSetting.setOnClickListener(this);
        mButtonPause.setOnClickListener(this);
        mButtonRatio.setOnClickListener(this);

        mLinearLayoutControlPanel.setOnTouchListener(this);
        mLinearLayoutTopBar.setOnTouchListener(this);

        //mTextViewDecoderType.setOnClickListener(this);
        mRelativeLayoutRootView.setOnTouchListener(this);
    }

    class PrePareListener implements OnPreparedListener {
        @Override
        public void onPrepared(MediaPlayer mp) {
            // TODO Auto-generated method stub
            Log.i(TAG, "enter onPrepared");
            mState = PLAYER_PREPARED;
            mMediaPlayer.start();
            mState = PLAYER_RUNNING;
            int duration = mp.getDuration();
            if (duration > 0) {
                mTextViewDuration.setText(TimesUtil.getTime(duration));
                mSeekBarProgress.setMax(duration);
            }
            startTimerTask();
            //setVideoScale(1);
        }
    }

    class FreshVideo implements OnFreshVideo {
        @Override
        public void onFresh(MediaPlayer mp) {
            mGLSurfaceView.requestRender();
        }
    }

    class OnCompleteListener implements OnCompletionListener {
        @Override
        public void onCompletion(MediaPlayer mp) {
            // TODO Auto-generated method stub
            mState = PLAYER_EXIT;
            finish();
        }
    }

    class OnSeekChangeListener implements OnSeekBarChangeListener {

        @Override
        public void onProgressChanged(SeekBar seekBar, int progress,
                                      boolean fromUser) {
            // TODO Auto-generated method stub
            if (mSeekFlag == 1) {
                //int currentTime = seekBar.getProgress();
                //mMediaPlayer.seekTo(currentTime);
            }
        }

        @Override
        public void onStartTrackingTouch(SeekBar seekBar) {
            // TODO Auto-generated method stub
            mSeekFlag = 1;
        }

        @Override
        public void onStopTrackingTouch(SeekBar seekBar) {
            // TODO Auto-generated method stub
            mState = PLAYER_SEEKING;
            int currentTime = seekBar.getProgress();
            mMediaPlayer.seekTo(currentTime);
            mMediaPlayer.start();
            mSeekFlag = 0;
        }

    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        if (mMediaPlayer != null) {
            mMediaPlayer.seekTo(mCurrentPosition);
            mMediaPlayer.start();
        }
    }

    ;

    Handler doActionHandler = new Handler(Looper.getMainLooper()) {
        public void handleMessage(android.os.Message msg) {
            int msgId = msg.what;
            switch (msgId) {
                case Constant.REFRESH_TIME_MSG:
                    int currentTime = mMediaPlayer.getCurrentPosition();
                    mCurrentPosition = currentTime;
                    int duration = mMediaPlayer.getDuration();
                    if (currentTime < 0)
                        currentTime = 0;
                    if (currentTime > duration)
                        currentTime = duration;
                    mTextViewCurrentTime.setText(TimesUtil.getTime(currentTime));
                    mSeekBarProgress.setProgress(currentTime);
                    break;
                case Constant.BEGIN_MEDIA_MSG:
                    //startTimerTask();
                    break;
                case Constant.HIDE_OPREATE_BAR_MSG:
                    //mLinearLayoutControlPanel.setVisibility(View.GONE);
                    Log.i(TAG, "enter HIDE_OPREATE_BAR_MSG");
                    showToolsBar(false);
                    break;
                case Constant.HIDE_PROGRESS_BAR_MSG:
                    showProgressBar(false);
                    break;
            }
        }

        ;
    };

    private Timer mTimer;

    private void startTimerTask() {
        mTimer = new Timer();
        mTimer.schedule(new TimerTask() {

            @Override
            public void run() {
                // TODO Auto-generated method stub
                doActionHandler.sendEmptyMessage(Constant.REFRESH_TIME_MSG);
            }
        }, Constant.REFRESH_TIME, Constant.REFRESH_TIME);
    }

    private void releaseTimerAndHandler() {
        //isEnableTime = false;
        if (mTimer != null)
            mTimer.cancel();
        doActionHandler.removeCallbacksAndMessages(null);
    }

    @Override
    public void onPause() {
        releaseTimerAndHandler();
        mGLSurfaceView.onPause();
        mMediaPlayer.pause();
        super.onPause();
        if (mState == PLAYER_PAUSED)
            mState = PLAYER_RUNNING;
        if (mState == PLAYER_RUNNING)
            mState = PLAYER_PAUSED;
        Log.d(TAG, "--PAUSE--");
    }

    @Override
    protected void onResume() {
        super.onResume();
        //mGLSurfaceView.onResume();
        Log.i(TAG, "enter onResume mMediaPlayer is:" + mMediaPlayer);
        if (mMediaPlayer != null) {
            mMediaPlayer.seekTo(mCurrentPosition);
            mMediaPlayer.start();
        }
    }

    @Override
    protected void onStop() {
        Log.i(TAG, "enterStop");
        mState = PLAYER_STOP;
        //mMediaPlayer.release();
        mMediaPlayer.stop();
        super.onStop();
    }


    @Override
    public void onClick(View v) {
        //showAudioEffect(false);
        int i = v.getId();
        if (i == R.id.videoplayer_button_back) {
            handleBack();

        } else if (i == R.id.videoplayer_button_pause) {
            handlePausePlay();

        } else if (i == R.id.videoplayer_button_ratio) {//setVideoScale(temp_flag);
            handleRatioChange();

        }
    }

    private PopWindowCompnent audioCompnent;

    private void showAudioEffect(boolean isShowing) {

        if (audioCompnent == null)//
            audioCompnent = new PopWindowCompnent(this, this);
        audioCompnent.show(mButtonAudioEffect, isShowing);
        audioCompnent.setCallback(new ICallBack() {
            @Override
            public void doItemClickListener(AdapterView<?> parent, View v,
                                            int position, long id) {
                // TODO Auto-generated method stub
                super.doItemClickListener(parent, v, position, id);
                TextView effectTxt = (TextView) v.findViewById(android.R.id.text1);
                String effectStr = effectTxt.getText().toString();
                String effectStr2 = Constant.gEqulizerPresets[position];
                //Toast.makeText(mActivity, "effectStr is:"+effectStr+"-0-0 effectStr2 is:"+effectStr2, Toast.LENGTH_LONG).show();
                if (mMediaPlayer != null) {
                    mMediaPlayer.setAuxEffectSendLevel(position);
                    mButtonAudioEffect.setText(effectStr);
                }
            }
        });
    }

    /**
     * 横竖屏幕切换
     */
    private void changeConfigration() {
        if (getResources().getConfiguration().orientation == Configuration.ORIENTATION_LANDSCAPE) {
            setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_PORTRAIT);
        } else {
            setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
        }
    }

    private void showToolsBar(boolean isNeed) {
        mLinearLayoutControlPanel.setVisibility(isNeed == true ? View.VISIBLE : View.GONE);
        mLinearLayoutTopBar.setVisibility(isNeed == true ? View.VISIBLE : View.GONE);
        //mButtonRotate.setVisibility(isNeed == true ? View.VISIBLE : View.GONE);
    }

    private void showProgressBar(boolean isShow) {
        mProgressBarBright.setVisibility(isShow == true ? View.VISIBLE : View.GONE);
        mProgressBarVolume.setVisibility(isShow == true ? View.VISIBLE : View.GONE);
    }

    private void handleBack() {
        try {
            super.finish();
        } catch (IllegalStateException e) {
        } catch (Exception e) {
        }
    }

    private void handlePausePlay() {
        try {
            if (mMediaPlayer.isPlaying()) {
                mMediaPlayer.pause();
                mButtonPause.setBackgroundResource(R.drawable.videoplayer_button_pause);
            } else {
                mMediaPlayer.start();
                mButtonPause.setBackgroundResource(R.drawable.videoplayer_button_play);
            }
        } catch (IllegalStateException e) {
            // TODO: handle exception
        } catch (Exception e) {
            // TODO: handle exception
        }
    }

    private void handleRatioChange() {
        try {
            ViewGroup.LayoutParams layoutParams = mGLSurfaceView.getLayoutParams();
            if (mDisplayMode == VIDEOPLAYER_DISPLAY_ORIGINAL) {
                mDisplayMode = VIDEOPLAYER_DISPLAY_FULLSCREEN;
                mButtonRatio.setBackgroundResource(R.drawable.videoplayer_button_ratio_fullscreen);
                layoutParams.width = mScreenWidth;
                layoutParams.height = mSurfaceHeight;
            } else {
                mDisplayMode = VIDEOPLAYER_DISPLAY_ORIGINAL;
                mButtonRatio.setBackgroundResource(R.drawable.videoplayer_button_ratio_normal);
                layoutParams.width = mMediaPlayer.getVideoWidth();
                layoutParams.height = mMediaPlayer.getVideoHeight();
            }

            mGLSurfaceView.setLayoutParams(layoutParams);

        } catch (IllegalStateException e) {
        } catch (Exception e) {
        }
    }

    @Override
    protected void onDestroy() {
        Log.i(TAG, "enter onDestroy");
        mMediaPlayer.stop();
        mMediaPlayer.release();
        mMediaPlayer = null;
        super.onDestroy();
    }

    //--------------------------onMoveTouch-------------------------//

    class GLMoveTouchListener implements OnTouchMoveListener {

        @Override
        public void onTouchMoveUp(float posX) {
            // TODO Auto-generated method stub
            if (posX < (mScreenWidth / 2 - 10)) {//left up handle audiovolume
                //Log.i(TAG, "left up handle audiovolume");
                handleAudioVolume(HANDLE_UP);
            } else if (posX > (mScreenWidth / 2 + 10)) {//right up handle lightless
                //Log.i(TAG, "right up handle lightless");
                handleLightless(HANDLE_UP);
            }
        }

        @Override
        public void onTouchMoveDown(float posX) {
            // TODO Auto-generated method stub
            if (posX < (mScreenWidth / 2 - 10)) {//left down handle audiovolume
                Log.i(TAG, "left down handle audiovolume");
                handleAudioVolume(HANDLE_DOWN);
            } else if (posX > (mScreenWidth / 2 + 10)) {//right down handle lightless
                //Log.i(TAG, "right down handle lightless");
                handleLightless(HANDLE_DOWN);
            }
        }

        @Override
        public void onTouch(MotionEvent event) {
            // TODO Auto-generated method stub
            Log.i(TAG, "Touched, show control panel");
            showToolsBar(true);
            doActionHandler.removeMessages(Constant.HIDE_OPREATE_BAR_MSG);
            doActionHandler.sendEmptyMessageDelayed(Constant.HIDE_OPREATE_BAR_MSG, 5 * Constant.REFRESH_TIME);
        }
    }

    //---------------------------OPENGL------------------------------//

    class MyGLSurfaceViewRender implements GLSurfaceView.Renderer {

        private Lock lock = new ReentrantLock();

        @Override
        public void onSurfaceCreated(GL10 gl,
                                     javax.microedition.khronos.egl.EGLConfig config) {
            // TODO Auto-generated method stub
            Log.i(TAG, "gl create enter");
            //gl.glClearColor(0.0f, 0f, 1f, 0.5f); // display blue at first
            //gl.glClear(GL10.GL_COLOR_BUFFER_BIT);
            mMediaPlayer.onSurfaceCreated();

        }

        @Override
        public void onSurfaceChanged(GL10 gl, int width, int height) {
            //other case
            lock.lock();
            Log.i(TAG, "gl surface change enter, width:" + width + " height:" + height);
            mMediaPlayer.onSurfaceChanged(width, height);
            mSurfaceWidth = width;
            mSurfaceHeight = height;

            lock.unlock();
        }

        @Override
        public void onDrawFrame(GL10 gl) {
            //Log.i(TAG, "onDrawFrame");  
            // 清除屏幕和深度缓存(如果不调用该代码, 将不显示glClearColor设置的颜色)  
            // 同样如果将该代码放到 onSurfaceCreated 中屏幕会一直闪动  
            //gl.glClear(GL10.GL_COLOR_BUFFER_BIT);
            //Log.i(TAG, "draw enter");
            lock.lock();
            mMediaPlayer.onDrawFrame();
            lock.unlock();
        }
    }

    private int temp_flag = 0;

    private void setVideoScale(int flag) {
        temp_flag++;
        flag = temp_flag % 3;
        Log.i(TAG, "setVideoScale flag is:" + flag);
        //LayoutParams lp = (LayoutParams) mGLSurfaceView.getLayoutParams();
        LayoutParams lp = new LayoutParams(mSurfaceWidth, mSurfaceHeight);
        Log.i(TAG, "begin");
        switch (flag) {
            case SCREEN_169value:
                if (mScreenWidth * 9 > mScreenHeight * 16) {
                    lp.height = mScreenHeight;
                    lp.width = mScreenHeight * 16 / 9;
                } else {
                    lp.height = mScreenWidth * 9 / 16;
                    lp.width = mScreenWidth;
                }
                mButtonRatio.setBackgroundResource(R.drawable.dt_player_control_ratio_fullscreen);
                break;
            case SCREEN_43value:
                if (mScreenWidth * 3 > mScreenHeight * 4) {
                    lp.height = mScreenHeight;
                    lp.width = mScreenHeight * 4 / 3;
                } else {
                    lp.height = mScreenWidth * 3 / 4;
                    lp.width = mScreenWidth;
                }
                mButtonRatio.setBackgroundResource(R.drawable.dt_player_control_ratio_1_1);
                break;
            case SCREEN_ORIGINAL:
                lp.width = mMediaPlayer.getVideoWidth();
                lp.height = mMediaPlayer.getVideoHeight();
                mButtonRatio.setBackgroundResource(R.drawable.dt_player_control_ratio_16_9);
                break;
            case SCREEN_FULLSCREEN:
                lp.width = mScreenWidth;
                lp.height = mScreenHeight;
                Log.i(TAG, "SCREEN_FULLSCREEN lp.width is:" + lp.width + "----lp.height is:" + lp.height);
                mButtonRatio.setBackgroundResource(R.drawable.dt_player_control_ratio_normal);
                break;
            case SCREEN_NORMALSCALE:
                lp.width = mScreenWidth;
                lp.height = mScreenHeight;
                int temp_width = 0;
                int temp_height = 0;
                if (mSurfaceWidth > 0) {
                    if (lp.width / mSurfaceWidth > lp.height / mSurfaceHeight) {
                        temp_width = (int) mSurfaceWidth * lp.height / mSurfaceHeight;
                        temp_height = lp.height;
                    } else {
                        temp_width = lp.width;
                        temp_height = mSurfaceHeight * lp.width / mSurfaceWidth;
                    }
                } else {

                }
                lp.width = temp_width;
                lp.height = temp_height;
                mButtonRatio.setBackgroundResource(R.drawable.dt_player_control_ratio_16_9);
                break;
        }
        Log.i(TAG, "lp.width is:" + lp.width + "----lp.height is:" + lp.height);
        //mGLSurfaceView.setLayoutParams(lp);
        Log.i(TAG, "before setVideoSize");

        ViewGroup.LayoutParams layoutParams = mGLSurfaceView.getLayoutParams();
        layoutParams.width = lp.width;
        layoutParams.height = lp.height;
        mGLSurfaceView.setLayoutParams(layoutParams);
    }

    @Override
    public boolean onTouch(View v, MotionEvent event) {
        // TODO Auto-generated method stub
        Log.i(TAG, "enter onTouch");
        if (v != mButtonAudioEffect) {
            showAudioEffect(false);
        }
        showToolsBar(true);
        doActionHandler.removeMessages(Constant.HIDE_OPREATE_BAR_MSG);
        doActionHandler.sendEmptyMessageDelayed(Constant.HIDE_OPREATE_BAR_MSG, 5 * Constant.REFRESH_TIME);
        return false;
    }

    private void handleAudioVolume(int type) {
        switch (type) {
            case HANDLE_UP:
                mVolumeUtil.upVolume(0);
                break;
            case HANDLE_DOWN:
                mVolumeUtil.downVolume(0);
                break;
        }
        mProgressBarVolume.setVisibility(View.VISIBLE);
        mProgressBarVolume.setProgress(mVolumeUtil.getCurrentVolume());
        doActionHandler.removeMessages(Constant.HIDE_PROGRESS_BAR_MSG);
        doActionHandler.sendEmptyMessageDelayed(Constant.HIDE_PROGRESS_BAR_MSG, 5 * Constant.REFRESH_TIME);
    }

    private void handleLightless(int type) {
        mCurrentLightness = ControlLightness.getInstance().getLightness(this);
        Log.i(TAG, "mCurrentLightness is:" + mCurrentLightness);
        switch (type) {
            case HANDLE_UP:
                mCurrentLightness += 5;
                mCurrentLightness = mCurrentLightness >= 255 ? 255 : mCurrentLightness;
                break;
            case HANDLE_DOWN:
                mCurrentLightness -= 5;
                mCurrentLightness = mCurrentLightness <= 0 ? 0 : mCurrentLightness;
                break;
        }
        ControlLightness.getInstance().setBrightness(this, mCurrentLightness);
        mProgressBarBright.setVisibility(View.VISIBLE);
        mProgressBarBright.setProgress(mCurrentLightness);
        doActionHandler.removeMessages(Constant.HIDE_PROGRESS_BAR_MSG);
        doActionHandler.sendEmptyMessageDelayed(Constant.HIDE_PROGRESS_BAR_MSG, 5 * Constant.REFRESH_TIME);
    }

}
