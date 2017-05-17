package dttv.app;

import android.app.Activity;
import android.content.Intent;
import android.os.Environment;
import android.os.Bundle;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

import java.io.IOException;

import app.dttv.dttvlib.MediaPlayer;
import dttv.app.utils.Constant;

public class PipTestActivity extends Activity {

    private String TAG = "PipTestActivity";
    private  String SAMPLE;

    private SurfaceView mSurfaceView;
    private MediaPlayer mMediaPlayer = null;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_pip_test);

        Intent intent = getIntent();
        SAMPLE = intent.getStringExtra(Constant.FILE_MSG);

        mSurfaceView = (SurfaceView) findViewById(R.id.surfaceView);
        mSurfaceView.getHolder().addCallback(callback);

        mMediaPlayer = new MediaPlayer(this, true);
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
                mMediaPlayer.start();
            }catch(IOException ex) {}
        }

        @Override
        public void surfaceDestroyed(SurfaceHolder holder) {
            Log.i(TAG, "SurfaceHolder destroy");
            mMediaPlayer.stop();
        }

    };


}
