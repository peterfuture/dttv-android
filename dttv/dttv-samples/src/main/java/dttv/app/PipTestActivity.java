package dttv.app;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.MotionEvent;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.widget.ImageButton;
import android.widget.LinearLayout;
import android.widget.RelativeLayout;
import android.widget.SeekBar;
import android.widget.TextView;

import java.io.IOException;
import java.util.Timer;
import java.util.TimerTask;

import app.dttv.dttvlib.MediaPlayer;
import dttv.app.utils.Constant;
import dttv.app.utils.SettingUtil;
import dttv.app.utils.TimesUtil;

public class PipTestActivity extends Activity implements View.OnClickListener, View.OnTouchListener {

    private String TAG = "PipTestActivity";
    private  String SAMPLE;

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
    private ImageButton mSettingButton;
    private ImageButton mRatioButton;
    private SeekBar mSeekBarProgress;
    private TextView mTextViewCurTime;
    private TextView mTextViewDuration;

    private SettingUtil mSettingUtil;
    private int mHWCodecEnable = 1;

    private int mPaused = 0;

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

    private void setupView() {
        mCtlPanel = (LinearLayout) findViewById(R.id.control_panel);
        mCtlPanel.setVisibility(View.VISIBLE);

        mCtlBar = (RelativeLayout) findViewById(R.id.control_bar);
        mCtlBar.setVisibility(View.VISIBLE);

        mPauseButton = (ImageButton) findViewById(R.id.btn_pause);
        mNextButton = (ImageButton) findViewById(R.id.btn_next);
        mSettingButton = (ImageButton) findViewById(R.id.btn_setting);
        mRatioButton = (ImageButton) findViewById(R.id.btn_ratio);

        mSeekBarProgress = (SeekBar) findViewById(R.id.seekbar_time);
        mTextViewCurTime = (TextView) findViewById(R.id.txt_cur);
        mTextViewDuration = (TextView) findViewById(R.id.txt_dur);
    }

    private void setupLisenner() {
        mPauseButton.setOnClickListener(this);
        mNextButton.setOnClickListener(this);
        mSettingButton.setOnClickListener(this);
        mRatioButton.setOnClickListener(this);

        mSeekBarProgress.setOnSeekBarChangeListener(seekLisenner);
    }

    // Click Touch SeekBar Handle
    public void onClick(View v) {
        int id = v.getId();
        switch (id) {
            case R.id.btn_pause:
                mMediaPlayer.pause();
                if(mPaused == 0) {
                    mPauseButton.setBackgroundResource(R.drawable.play);
                    mPaused = 1;
                } else {
                    mPauseButton.setBackgroundResource(R.drawable.pause);
                    mPaused = 0;
                }
                break;
            case R.id.btn_next:
                break;
            case R.id.btn_setting:
                break;
            case R.id.btn_ratio:
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
            mMediaPlayer.seekTo(seekBar.getProgress());
        }
    };

    // Timer Handle
    private void startTimer() {
        if(mTimer != null || mTimerTask != null) {
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
        if(mTimer != null || mTimerTask != null) {
            return;
        }

        mTimer.cancel();
        mTimerTask.cancel();
    }

    // Handle Message
    Handler mHandle = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case UPDATE_TEXTVIEW:
                    mTextViewCurTime.setText(TimesUtil.getTime(mMediaPlayer.getCurrentPosition()));
                    mSeekBarProgress.setProgress(mMediaPlayer.getCurrentPosition());
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

    // SurfaceView callback
    private SurfaceHolder.Callback callback = new SurfaceHolder.Callback() {

        @Override
        public void surfaceCreated(SurfaceHolder holder) {
            Log.i(TAG, "SurfaceHolder create");
        }

        @Override
        public void surfaceChanged(SurfaceHolder holder, int format, int width,
                                   int height) {

            try {
                Log.i(TAG, "SurfaceHolder changed. w" + width + " h:" + height);
                mMediaPlayer.setDataSource(SAMPLE);
                mMediaPlayer.setDisplay(holder);
                mMediaPlayer.prepare();

                // init view
                startTimer();
                mTextViewDuration.setText(TimesUtil.getTime(mMediaPlayer.getDuration()));
                mSeekBarProgress.setMax(mMediaPlayer.getDuration());
            }catch(IOException ex) {}
        }

        @Override
        public void surfaceDestroyed(SurfaceHolder holder) {
            Log.i(TAG, "SurfaceHolder destroy");
            stopTimer();
            mMediaPlayer.stop();
        }

    };


}
