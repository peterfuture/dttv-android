package app.dttv.dttvlib;

import android.os.PowerManager;
import java.io.File;
import java.io.FileDescriptor;
import java.io.FileInputStream;
import java.io.IOException;
import java.lang.ref.WeakReference;
import java.util.Map;

import android.annotation.SuppressLint;
import android.content.ContentResolver;
import android.content.Context;
import android.content.res.AssetFileDescriptor;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.view.Surface;
import android.view.SurfaceHolder;

import app.dttv.dttvlib.utils.Log;
import app.dttv.dttvlib.utils.FileUtils;

public class MediaPlayer {

    private final static String TAG = "MediaPlayer";


    private PowerManager.WakeLock mWakeLock = null;
    private boolean mScreenOnWhilePlaying;
    private boolean mStayAwake;

    private AssetFileDescriptor mFD;

    private OnHWRenderFailedListener mOnHWRenderFailedListener;
    private OnPreparedListener mOnPreparedListener;
    private OnFreshVideo mOnFreshVideo;
    private OnCompletionListener mOnCompletionListener;
    private OnBufferingUpdateListener mOnBufferingUpdateListener;
    private OnSeekCompleteListener mOnSeekCompleteListener;
    private OnVideoSizeChangedListener mOnVideoSizeChangedListener;
    private OnErrorListener mOnErrorListener;
    private OnInfoListener mOnInfoListener;
    private OnTimedTextListener mOnTimedTextListener;

    private static final int MEDIA_PREPARED = 1;
    private static final int MEDIA_PLAYBACK_COMPLETE = 2;
    private static final int MEDIA_BUFFERING_UPDATE = 3;
    private static final int MEDIA_SEEK_COMPLETE = 4;
    private static final int MEDIA_SET_VIDEO_SIZE = 5;
    private static final int MEDIA_FRESH_VIDEO = 99; // ugly code

    private static final int MEDIA_ERROR = 100;
    private static final int MEDIA_INFO = 200;
    private static final int MEDIA_CACHE = 300;
    private static final int MEDIA_HW_ERROR = 400;
    private static final int MEDIA_TIMED_TEXT = 1000;
    private static final int MEDIA_CACHING_UPDATE = 2000;

    /*
    * KEY DEFINITIONS - SET_PARAMETER & GET_PARAMETER
    * */
    public static final int KEY_PARAMETER_USEHWCODEC = 0x0;

    private Context mContext;
    private long mNativeContext; // accessed by native methods
    private boolean mUseHwCodec = true;
    private static EventHandler mEventHandler;

    private SurfaceHolder mSurfaceHolder;
    private Surface mSurface;

    static {
        System.loadLibrary("dttv_jni");
        native_init();
    }

    public MediaPlayer(Context ctx) {
        this(ctx, false);
    }

    public MediaPlayer(Context ctx, boolean hw) {

        mContext = ctx;
        mUseHwCodec = hw;

        Looper looper;
        if ((looper = Looper.myLooper()) != null)
            mEventHandler = new EventHandler(this, looper);
        else if ((looper = Looper.getMainLooper()) != null)
            mEventHandler = new EventHandler(this, looper);
        else
            mEventHandler = null;

        int ret = native_setup(new WeakReference<MediaPlayer>(this));
        Log.d(TAG, "Native Setup.ret:" + ret);
        if (ret >= 0) {
            native_set_parameter(KEY_PARAMETER_USEHWCODEC, mUseHwCodec ? 1 : 0, 0);
        }
    }

    public void setDisplay(SurfaceHolder surfaceHolder) {
        if (surfaceHolder == null) {
            releaseDisplay();
            return;
        }
        mSurfaceHolder = surfaceHolder;
        mSurface = surfaceHolder.getSurface();
        native_set_video_surface(mSurface);
        updateSurfaceScreenOn();

    }

    public void setSurface(Surface surface) {
        if (surface == null) {
            releaseDisplay();
            return;
        }
        mSurfaceHolder = null;
        mSurface = surface;
        native_set_video_surface(mSurface);
        updateSurfaceScreenOn();
    }

    public void releaseDisplay() {
        native_release_surface();
        mSurfaceHolder = null;
        mSurface = null;
    }

    public void setScreenOnWhilePlaying(boolean screenOn) {
        if (mScreenOnWhilePlaying != screenOn) {
            mScreenOnWhilePlaying = screenOn;
            updateSurfaceScreenOn();
        }
    }

    @SuppressLint("Wakelock")
    private void stayAwake(boolean awake) {
        if (mWakeLock != null) {
            if (awake && !mWakeLock.isHeld()) {
                mWakeLock.acquire();
            } else if (!awake && mWakeLock.isHeld()) {
                mWakeLock.release();
            }
        }
        mStayAwake = awake;
        updateSurfaceScreenOn();
    }

    private void updateSurfaceScreenOn() {
        if (mSurfaceHolder != null)
            mSurfaceHolder.setKeepScreenOn(mScreenOnWhilePlaying && mStayAwake);
    }

    public void setDataSource(String path) throws IOException, IllegalArgumentException, SecurityException, IllegalStateException {
        Log.d(TAG, "SetDataSource");
        native_set_datasource(path);
    }

    public void setDataSource(Context context, Uri uri) throws IOException, IllegalArgumentException, SecurityException, IllegalStateException {
        setDataSource(context, uri, null);
    }

    public void setDataSource(Context context, Uri uri, Map<String, String> headers) throws IOException, IllegalArgumentException, SecurityException, IllegalStateException {
        if (context == null || uri == null)
            throw new IllegalArgumentException();
        String scheme = uri.getScheme();
        if (scheme == null || scheme.equals("file")) {
            setDataSource(FileUtils.getPath(uri.toString()));
            return;
        }

        ContentResolver resolver = context.getContentResolver();
        mFD = resolver.openAssetFileDescriptor(uri, "r");
        if (mFD == null)
            return;
        try {
            native_set_datasource(mFD.getParcelFileDescriptor().getFileDescriptor());
            return;
        } catch (Exception e) {
            closeFD();
        }
        setDataSource(uri.toString(), headers);
    }

    public void setDataSource(String path, Map<String, String> headers) throws IOException, IllegalArgumentException, SecurityException, IllegalStateException {
        String values[] = null;
        String keys[] = null;
        if (headers != null) {
            keys = new String[headers.size()];
            values = new String[headers.size()];
            int i = 0;
            for (Map.Entry<String, String> entry : headers.entrySet()) {
                keys[i] = entry.getKey();
                values[i] = entry.getValue();
                i++;
            }
        }
        setDataSource(path, keys, values);
    }

    public void setDataSource(String path, String[] keys, String[] values) throws IOException, IllegalArgumentException, SecurityException, IllegalStateException {
        final Uri uri = Uri.parse(path);
        if ("file".equals(uri.getScheme())) {
            path = uri.getPath();
        }

        final File file = new File(path);
        if (file.exists()) {
            FileInputStream is = new FileInputStream(file);
            FileDescriptor fd = is.getFD();
            native_set_datasource(fd);
            is.close();
        } else {
            native_set_datasource(path, keys, values);
        }
    }

    public void prepare() throws IllegalStateException, IOException {
        native_prepare();
    }

    public void prepareAsync() throws IllegalStateException, IOException {
        native_prepare_async();
    }

    public void start() throws IllegalStateException {
        stayAwake(true);
        native_start();
    }

    public void pause() throws IllegalStateException {
        stayAwake(false);
        native_pause();
    }

    public void stop() throws IllegalStateException {
        stayAwake(false);
        native_stop();
    }

    @SuppressLint("Wakelock")
    public void setWakeMode(Context context, int mode) {
        boolean washeld = false;
        if (mWakeLock != null) {
            if (mWakeLock.isHeld()) {
                washeld = true;
                mWakeLock.release();
            }
            mWakeLock = null;
        }

        PowerManager pm = (PowerManager) context.getSystemService(Context.POWER_SERVICE);
        mWakeLock = pm.newWakeLock(mode | PowerManager.ON_AFTER_RELEASE, MediaPlayer.class.getName());
        mWakeLock.setReferenceCounted(false);
        if (washeld) {
            mWakeLock.acquire();
        }
    }

    private void closeFD() {
        if (mFD != null) {
            try {
                mFD.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
            mFD = null;
        }
    }

    public void reset() {
        stayAwake(false);
        native_reset();
        mEventHandler.removeCallbacksAndMessages(null);
        closeFD();
    }

    private void removeListenners() {
        mOnPreparedListener = null;
        mOnFreshVideo = null;
        mOnBufferingUpdateListener = null;
        mOnCompletionListener = null;
        mOnSeekCompleteListener = null;
        mOnErrorListener = null;
        mOnInfoListener = null;
        mOnVideoSizeChangedListener = null;
        mOnHWRenderFailedListener = null;
    }

    public void release() {
        stayAwake(false);
        updateSurfaceScreenOn();
        native_stop();
        native_release();
        closeFD();
        removeListenners();
        mEventHandler = null;
    }


    public int getDuration() {
        return native_get_duration();
    }

    public int getCurrentPosition() {
        return native_get_current_position();
    }

    public int getVideoWidth() {

        return native_get_video_width();
    }

    public int getVideoHeight() {

        return native_get_video_height();
    }

    public void seekTo(int msec) {
        native_seekTo(msec);
    }

    public boolean isPlaying() {
        return (native_is_playing() == 1) ? true : false;
    }


    private class EventHandler extends Handler {
        private MediaPlayer mMediaPlayer;
        private Bundle mData;


        public EventHandler(MediaPlayer mp, Looper looper) {
            super(looper);
            mMediaPlayer = mp;
        }

        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case MEDIA_PREPARED:
                    if (mOnPreparedListener != null)
                        mOnPreparedListener.onPrepared(mMediaPlayer);
                    break;
                case MEDIA_PLAYBACK_COMPLETE:
                    if (mOnCompletionListener != null)
                        mOnCompletionListener.onCompletion(mMediaPlayer);
                    break;
                case MEDIA_BUFFERING_UPDATE:
                    if (mOnBufferingUpdateListener != null)
                        mOnBufferingUpdateListener.onBufferingUpdate(mMediaPlayer, msg.arg1);
                    break;
                case MEDIA_SEEK_COMPLETE:
                    if (native_is_playing() == 1 ? true : false)
                        stayAwake(true);
                    if (mOnSeekCompleteListener != null)
                        mOnSeekCompleteListener.onSeekComplete(mMediaPlayer);
                    break;
                case MEDIA_SET_VIDEO_SIZE:
                    if (mOnVideoSizeChangedListener != null)
                        mOnVideoSizeChangedListener.onVideoSizeChanged(
                                mMediaPlayer, msg.arg1, msg.arg2);
                    break;
                case MEDIA_FRESH_VIDEO:
                    if (mOnFreshVideo != null)
                        mOnFreshVideo.onFresh(mMediaPlayer);
                    break;
                case MEDIA_ERROR:
                    release();
                    break;
                case MEDIA_HW_ERROR:
                    if (mOnHWRenderFailedListener != null)
                        mOnHWRenderFailedListener.onFailed();
                    return;
                default:
                    Log.e("", "Unknown message type " + msg.what);
                    return;
            }
        }
    }

    /**
     * called from jni code
     */
    private static void postEventFromNative(Object dtp,
                                            int what, int arg1, int arg2, Object obj) {
        MediaPlayer mp = (MediaPlayer) ((WeakReference) dtp).get();
        if (mp == null) {
            return;
        }

        switch (what) {
            case MEDIA_PREPARED:
                mEventHandler.sendEmptyMessage(MEDIA_PREPARED);
                break;
            case MEDIA_PLAYBACK_COMPLETE:
                mEventHandler.sendEmptyMessage(MEDIA_PLAYBACK_COMPLETE);
                break;
            case MEDIA_ERROR:
                mEventHandler.sendEmptyMessage(MEDIA_ERROR);
                break;
            case MEDIA_FRESH_VIDEO:
                mEventHandler.sendEmptyMessage(MEDIA_FRESH_VIDEO);
                break;
        }

    }

    /*
    * Listenners Definitions
    * */

    public interface OnHWRenderFailedListener {
        public void onFailed();
    }

    public interface OnPreparedListener {
        void onPrepared(MediaPlayer mp);
    }

    public interface OnFreshVideo {
        void onFresh(MediaPlayer mp);
    }

    public interface OnCompletionListener {
        void onCompletion(MediaPlayer mp);
    }

    public interface OnSeekCompleteListener {
        void onSeekComplete(MediaPlayer mp);
    }

    public interface OnErrorListener {
        boolean onError(MediaPlayer mp, int what, int extra);
    }

    public interface OnVideoSizeChangedListener {
        public void onVideoSizeChanged(MediaPlayer mp, int width, int height);
    }

    public interface OnBufferingUpdateListener {
        void onBufferingUpdate(MediaPlayer mp, int percent);
    }

    public interface OnInfoListener {
        boolean onInfo(MediaPlayer mp, int what, int extra);
    }

    public interface OnTimedTextListener {
        /**
         * MediaPlayer.release();
         * MediaPlayer.reset();
         * Called to indicate that a text timed text need to display
         *
         * @param text the timedText to display
         */
        public void onTimedText(String text);

        /**
         * Called to indicate that an image timed text need to display
         *
         * @param pixels the pixels of the timed text image
         * @param width  the width of the timed text image
         * @param height the height of the timed text image
         */
        public void onTimedTextUpdate(byte[] pixels, int width, int height);
    }


    /**
     * Register a callback to be invoked when the media source is ready for
     * playback.
     *
     * @param listener the callback that will be run
     */
    public void setOnPreparedListener(OnPreparedListener listener) {
        mOnPreparedListener = listener;
    }

    public void setOnFreshVideo(OnFreshVideo listener) {
        mOnFreshVideo = listener;
    }

    /**
     * Register a callback to be invoked when the end of a media source has been
     * reached during playback.
     *
     * @param listener the callback that will be run
     */
    public void setOnCompletionListener(OnCompletionListener listener) {
        mOnCompletionListener = listener;
    }

    /**
     * Register a callback to be invoked when the status of a network stream's
     * buffer has changed.
     *
     * @param listener the callback that will be run.
     */
    public void setOnBufferingUpdateListener(OnBufferingUpdateListener listener) {
        mOnBufferingUpdateListener = listener;
    }

    /**
     * Register a callback to be invoked when a seek operation has been
     * completed.
     *
     * @param listener the callback that will be run
     */
    public void setOnSeekCompleteListener(OnSeekCompleteListener listener) {
        mOnSeekCompleteListener = listener;
    }

    /**
     * Register a callback to be invoked when the video size is known or
     * updated.
     *
     * @param listener the callback that will be run
     */
    public void setOnVideoSizeChangedListener(OnVideoSizeChangedListener listener) {
        mOnVideoSizeChangedListener = listener;
    }

    /**
     * Register a callback to be invoked when an error has happened during an
     * asynchronous operation.
     *
     * @param listener the callback that will be run
     */
    public void setOnErrorListener(OnErrorListener listener) {
        mOnErrorListener = listener;
    }

    public void setOnInfoListener(OnInfoListener listener) {
        mOnInfoListener = listener;
    }

    /**
     * Register a callback to be invoked when a timed text need to display.
     *
     * @param listener the callback that will be run
     */
    public void setOnTimedTextListener(OnTimedTextListener listener) {
        mOnTimedTextListener = listener;
    }

    //----------------------------------
    //OPENGL-ESV2
    public int onSurfaceCreated() {
        native_surface_create();
        return 0;
    }

    public int onSurfaceChanged(int w, int h) {
        native_surface_change(w, h);
        return 0;
    }

    public int onDrawFrame() {
        native_draw_frame();
        return 0;
    }

    /**
     * @param type
     * @return
     */
    public int setAuxEffectSendLevel(int type) {
        Log.d(TAG, "setaudio effect:" + type);
        return native_set_audio_effect(type);
    }

    /* Native API*/

    public static native void native_init();

    public native int native_setup(Object thiz);

    public native void native_set_datasource(String path);

    public native void native_set_datasource(String path, String[] keys, String[] values)
            throws IOException, IllegalArgumentException, IllegalStateException; // Not Support

    public native void native_set_datasource(FileDescriptor fileDescriptor)
            throws IOException, IllegalArgumentException, IllegalStateException; // Not Support

    public native int native_prepare() throws IllegalStateException;

    public native int native_prepare_async() throws IllegalStateException;

    public native int native_start() throws IllegalStateException;

    public native int native_stop() throws IllegalStateException;

    public native int native_pause() throws IllegalStateException;

    public native int native_seekTo(int msec) throws IllegalStateException;

    public native int native_reset();

    public native int native_release();

    public native int native_get_duration();

    public native int native_get_current_position();

    public native int native_get_video_width();

    public native int native_get_video_height();

    public native int native_is_playing();

    public native void native_set_video_surface(Surface surface);

    public native void native_release_surface();

    public native int native_set_parameter(int cmd, long arg1, long arg2);

    //opengl esv2
    public native int native_surface_create();

    public native int native_surface_change(int w, int h);

    public native int native_draw_frame();

    public native int native_set_audio_effect(int t);

}
