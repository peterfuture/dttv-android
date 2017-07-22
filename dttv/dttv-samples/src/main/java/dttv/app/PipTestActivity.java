package dttv.app;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.provider.MediaStore;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.Display;
import android.view.MotionEvent;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageButton;
import android.widget.LinearLayout;
import android.widget.RelativeLayout;
import android.widget.SeekBar;
import android.widget.TextView;

import java.io.IOException;
import java.util.Timer;
import java.util.TimerTask;

import app.dttv.dttvlib.MediaPlayer;
import app.dttv.dttvlib.Metadata;
import dttv.app.utils.Constant;
import dttv.app.utils.SettingUtil;
import dttv.app.utils.TimesUtil;

public class PipTestActivity extends Activity implements View.OnClickListener, View.OnTouchListener {

    private String TAG = "PipTestActivity";
    private String SAMPLE;

    private SurfaceView mSurfaceView;
    private MediaPlayer mMediaPlayer = null;

    private Timer mTimer = null;
    private TimerTask mTimerTask = null;
    // message definitions
    private static final int UPDATE_TEXTVIEW = 0;

    private LinearLayout mCtlPanel;
    private RelativeLayout mCtlBar;
    private ImageButton mPauseButton;
    private ImageButton mNextButton;
    private ImageButton mInfoButton;
    private ImageButton mRatioButton;
    private SeekBar mSeekBarProgress;
    private TextView mTextViewCurTime;
    private TextView mTextViewDuration;
    private TextView mTextViewInfo;

    private SettingUtil mSettingUtil;
    private int mHWCodecEnable = 1;

    // contrl
    private int mPaused = 0;
    private int mStopped = 0;

    private int mSeeking = 0;
    private int mSeekPosition = -1;
    private int mSeekCurPosition = -1;
    private int mRatio = 0; // 0 full 1 normal
    private int mResumePosition = -1;

    // Info
    private int mScreenWidth = 0;
    private int mScreenHeight = 0;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_pip_test);

        Intent intent = getIntent();
        SAMPLE = intent.getStringExtra(Constant.FILE_MSG);

        mSurfaceView = (SurfaceView) findViewById(R.id.surfaceView);
        mSurfaceView.getHolder().addCallback(callback);

        mSettingUtil = new SettingUtil(this);
        mHWCodecEnable = mSettingUtil.isHWCodecEnable();

        mMediaPlayer = new MediaPlayer(this, mHWCodecEnable == 1);

        setupView();
        setupLisenner();
    }

    @Override
    public void onPause() {
        mMediaPlayer.pause();
        super.onPause();
    }

    @Override
    protected void onResume() {
        if (mStopped == 0) {
            mMediaPlayer.start();
        }
        // else do nothing - resume play in surface create
        super.onResume();
    }

    @Override
    protected void onStop() {
        stopMediaPlayer();
        super.onStop();
    }

    @Override
    protected void onDestroy() {
        mMediaPlayer = null;
        super.onDestroy();
    }

    private void setupView() {
        mCtlPanel = (LinearLayout) findViewById(R.id.control_panel);
        mCtlPanel.setVisibility(View.VISIBLE);

        mCtlBar = (RelativeLayout) findViewById(R.id.control_bar);
        mCtlBar.setVisibility(View.VISIBLE);

        mPauseButton = (ImageButton) findViewById(R.id.btn_pause);
        mNextButton = (ImageButton) findViewById(R.id.btn_next);
        mInfoButton = (ImageButton) findViewById(R.id.btn_info);
        mRatioButton = (ImageButton) findViewById(R.id.btn_ratio);

        mSeekBarProgress = (SeekBar) findViewById(R.id.seekbar_time);
        mTextViewCurTime = (TextView) findViewById(R.id.txt_cur);
        mTextViewDuration = (TextView) findViewById(R.id.txt_dur);
        mTextViewInfo = (TextView) findViewById(R.id.txt_info);

        DisplayMetrics displayMetrics = new DisplayMetrics();
        getWindowManager().getDefaultDisplay().getMetrics(displayMetrics);
        mScreenWidth = displayMetrics.widthPixels;
        mScreenHeight = displayMetrics.heightPixels;
    }

    private void setupLisenner() {
        mMediaPlayer.setOnPreparedListener(new OnPrepareListener());
        mMediaPlayer.setOnSeekCompleteListener(new OnSeekCompleteListner());
        mMediaPlayer.setOnCompletionListener(new OnCompleteListener());

        mPauseButton.setOnClickListener(this);
        mNextButton.setOnClickListener(this);
        mInfoButton.setOnClickListener(this);
        mRatioButton.setOnClickListener(this);

        mSeekBarProgress.setOnSeekBarChangeListener(seekLisenner);
    }

    // Click Touch SeekBar Handle
    public void onClick(View v) {
        int id = v.getId();
        switch (id) {
            case R.id.btn_pause:
                mMediaPlayer.pause();
                if (mPaused == 0) {
                    mPauseButton.setBackgroundResource(R.drawable.play);
                    mPaused = 1;
                } else {
                    mPauseButton.setBackgroundResource(R.drawable.pause);
                    mPaused = 0;
                }
                break;
            case R.id.btn_next:
                break;
            case R.id.btn_info:
                // display video info dialog
                String details = "uri:" + SAMPLE + "\n";
                details += "video:\n";
                details += "resolusion: " + mMediaPlayer.getVideoWidth() + "*" + mMediaPlayer.getVideoHeight() + "\n";
                mTextViewInfo.setText(details);
                mTextViewInfo.setVisibility(View.VISIBLE);
                break;
            case R.id.btn_ratio:

                if (mRatio == 0) {
                    // switch to normal
                    mRatio = 1;
                    ViewGroup.LayoutParams lp = mSurfaceView.getLayoutParams();
                    lp.width = mMediaPlayer.getVideoWidth();
                    lp.height = mMediaPlayer.getVideoHeight();
                    mSurfaceView.setLayoutParams(lp);
                } else {
                    // switch to fullscreen
                    mRatio = 0;
                    ViewGroup.LayoutParams lp = mSurfaceView.getLayoutParams();
                    lp.width = mScreenWidth;
                    lp.height = mScreenHeight;
                    mSurfaceView.setLayoutParams(lp);
                }
                break;
            default:
                break;
        }
    }

    public boolean onTouch(View v, MotionEvent event) {
        return false;
    }

    private SeekBar.OnSeekBarChangeListener seekLisenner = new SeekBar.OnSeekBarChangeListener() {

        public void onProgressChanged(SeekBar seekBar, int progress,
                                      boolean fromUser) {
        }

        public void onStartTrackingTouch(SeekBar seekBar) {
        }

        public void onStopTrackingTouch(SeekBar seekBar) {
            int position = seekBar.getProgress();
            if (mSeeking == 1) {
                mSeekCurPosition = position;
                return;
            } else {
                mSeeking = 1;
                mSeekCurPosition = mSeekPosition = position;
            }
            mMediaPlayer.seekTo(mSeekPosition);
            Log.i(TAG, "Seekto " + mSeekPosition);
        }
    };

    // Timer Handle
    private void startTimer() {
        if (mTimer != null || mTimerTask != null) {
            return;
        }

        mTimer = new Timer();
        mTimerTask = new TimerTask() {
            @Override
            public void run() {
                // update current time
                sendMessage(UPDATE_TEXTVIEW);
            }
        };

        mTimer.schedule(mTimerTask, 300, 300); // 3 times on second
    }

    private void stopTimer() {
        if (mTimer == null && mTimerTask == null) {
            return;
        }

        mTimer.cancel();
        mTimerTask.cancel();
        mHandle.removeCallbacksAndMessages(null);
        mTimer = null;
        mTimerTask = null;
    }

    // Handle Message
    Handler mHandle = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case UPDATE_TEXTVIEW:
                    if (mSeeking == 0) {
                        mTextViewCurTime.setText(TimesUtil.getTime(mMediaPlayer.getCurrentPosition()));
                        mSeekBarProgress.setProgress(mMediaPlayer.getCurrentPosition());
                    }
                    break;
                default:
                    break;
            }
        }
    };

    private void sendMessage(int id) {
        Message message = Message.obtain(mHandle, id);
        mHandle.sendMessage(message);
    }

    // VideoPlayer Lisenter
    class OnPrepareListener implements MediaPlayer.OnPreparedListener {
        @Override
        public void onPrepared(MediaPlayer mp) {
            Log.i(TAG, "prepared");
        }
    }

    class OnSeekCompleteListner implements MediaPlayer.OnSeekCompleteListener {
        @Override
        public void onSeekComplete(MediaPlayer mp) {
            // check no seek left
            if (mSeekCurPosition != mSeekPosition) {
                mSeekPosition = mSeekCurPosition;
                mMediaPlayer.seekTo(mSeekPosition);
                Log.i(TAG, "cache seekto " + mSeekPosition);
                return;
            }

            mSeeking = 0;
            mSeekCurPosition = mSeekPosition = -1;
            startTimer();
            Log.i(TAG, "seek complete.");
        }
    }

    ;

    class OnCompleteListener implements MediaPlayer.OnCompletionListener {
        @Override
        public void onCompletion(MediaPlayer mp) {
            stopMediaPlayer();
        }
    }

    private void startMediaPlayer() {
        try {
            if (mMediaPlayer == null)
                return;
            if (mMediaPlayer.isPlaying())
                return;
            mMediaPlayer.setDataSource(SAMPLE);
            mMediaPlayer.setDisplay(mSurfaceView.getHolder());
            mMediaPlayer.prepare();
            mMediaPlayer.start();
            if (mResumePosition > 0) {
                mMediaPlayer.seekTo(mResumePosition);
                mResumePosition = -1;
            }

            //Metadata meta =  mMediaPlayer.getMetadata();
            //MediaPlayer.TrackInfo[] track_info =  mMediaPlayer.getTrackInfo();

            startTimer();
            mTextViewDuration.setText(TimesUtil.getTime(mMediaPlayer.getDuration()));
            mSeekBarProgress.setMax(mMediaPlayer.getDuration());
            mStopped = 0;
        } catch (IOException ex) {
        }
    }

    private void stopMediaPlayer() {
        if (mStopped == 1)
            return;

        mResumePosition = mMediaPlayer.getCurrentPosition();
        stopTimer();
        mMediaPlayer.stop();
        mStopped = 1;
    }

    // SurfaceView callback
    private SurfaceHolder.Callback callback = new SurfaceHolder.Callback() {

        @Override
        public void surfaceCreated(SurfaceHolder holder) {
            Log.i(TAG, "SurfaceHolder create");
        }

        @Override
        public void surfaceChanged(SurfaceHolder holder, int format, int width,
                                   int height) {
            Log.i(TAG, "SurfaceHolder changed. w" + width + " h:" + height);
            startMediaPlayer();
        }

        @Override
        public void surfaceDestroyed(SurfaceHolder holder) {
            Log.i(TAG, "SurfaceHolder destroy");
            stopMediaPlayer();
        }

    };

}
