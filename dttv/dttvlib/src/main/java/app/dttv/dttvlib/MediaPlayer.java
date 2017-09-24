package app.dttv.dttvlib;

import android.os.Parcel;
import android.os.PowerManager;

import java.io.File;
import java.io.FileDescriptor;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.UnsupportedEncodingException;
import java.lang.ref.WeakReference;
import java.nio.charset.Charset;
import java.util.HashMap;
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
import android.text.TextUtils;
import android.util.SparseArray;
import android.view.Surface;
import android.view.SurfaceHolder;

import app.dttv.dttvlib.utils.Log;
import app.dttv.dttvlib.utils.FileUtils;

public class MediaPlayer {

    private final static String TAG = "MediaPlayer";

    private static final int MEDIA_NOP = 0;
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

    private static final String MEDIA_CACHING_SEGMENTS = "caching_segment";
    private static final String MEDIA_CACHING_TYPE = "caching_type";
    private static final String MEDIA_CACHING_INFO = "caching_info";
    private static final String MEDIA_SUBTITLE_STRING = "sub_string";
    private static final String MEDIA_SUBTITLE_BYTES = "sub_bytes";
    private static final String MEDIA_SUBTITLE_TYPE = "sub_type";
    private static final int SUBTITLE_TEXT = 0;
    private static final int SUBTITLE_BITMAP = 1;

    // set parameter cmd. match c definitions
    public static final int KEY_PARAMETER_USEHWCODEC = 0x0;
    public static final int KEY_PARAMETER_SET_GLFILTER = 0x1;
    public static final int KEY_PARAMETER_GLRENDER_GET_PIXFMT = 0x100;
    public static final int KEY_PARAMETER_GLRENDER_SET_FILTER_PARAMETER = 0x101;

    // GL Filter Parameter
    public static final int GL_FILTER_YUV=0;
    public static final int GL_FILTER_RGB=1;
    public static final int GL_FILTER_SATURATION=2;

    private Context mContext;
    private long mNativeContext; // accessed by native methods
    private Surface mSurface;
    private SurfaceHolder mSurfaceHolder;
    private static EventHandler mEventHandler;
    private PowerManager.WakeLock mWakeLock = null;
    private boolean mScreenOnWhilePlaying;
    private boolean mStayAwake;
    private Metadata mMeta;
    private TrackInfo[] mInbandTracks;
    private TrackInfo mOutOfBandTracks;
    private AssetFileDescriptor mFD = null;

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


    static {
        System.loadLibrary("dttv_jni");
        native_init();
    }

    /**
     * Default constructor. The same as Android's MediaPlayer().
     * <p>
     * When done with the MediaPlayer, you should call {@link #release()}, to free
     * the resources. If not released, too many MediaPlayer instances may result
     * in an exception.
     * </p>
     */
    public MediaPlayer(Context ctx) {
        this(ctx, true);
    }

    public MediaPlayer(Context ctx, boolean preferHWDecoder) {
        mContext = ctx;

        Looper looper;
        if ((looper = Looper.myLooper()) != null)
            mEventHandler = new EventHandler(this, looper);
        else if ((looper = Looper.getMainLooper()) != null)
            mEventHandler = new EventHandler(this, looper);
        else
            mEventHandler = null;

        int ret = native_setup(new WeakReference<MediaPlayer>(this));
        Log.d(TAG, "Native Setup.ret:" + ret + " use hw:" + (preferHWDecoder ? 1 : 0));
        if (ret >= 0) {
            native_set_parameter(KEY_PARAMETER_USEHWCODEC, preferHWDecoder ? 1 : 0, 0);
        }
    }

    private static void postEventFromNative(Object mediaplayer_ref,
                                            int what, int arg1, int arg2, Object obj) {
        MediaPlayer mp = (MediaPlayer) ((WeakReference) mediaplayer_ref).get();
        if (mp == null)
            return;

        try {
            //synchronized (mp.mEventHandler) {
            if (mp.mEventHandler != null) {
                Message m = mp.mEventHandler.obtainMessage(what, arg1, arg2, obj);
                mp.mEventHandler.sendMessage(m);
            }
        } catch (Exception e) {
            Log.e(TAG, "exception: " + e.toString());
        }

    }

    public native void native_set_video_surface(Surface surface);

    /**
     * Sets the SurfaceHolder to use for displaying the video portion of the
     * media. This call is optional. Not calling it when playing back a video will
     * result in only the audio track being played.
     *
     * @param sh the SurfaceHolder to use for video display
     */
    public void setDisplay(SurfaceHolder sh) {
        if (sh == null) {
            releaseDisplay();
            return;
        }
        mSurfaceHolder = sh;
        mSurface = sh.getSurface();
        native_set_video_surface(mSurface);
        updateSurfaceScreenOn();
    }

    /**
     * Sets the Surface to use for displaying the video portion of the media. This
     * is similar to {@link #setDisplay(SurfaceHolder)}.
     *
     * @param surface the Surface to use for video display
     */
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

    /**
     * Sets the data source (file-path or http/rtsp URL) to use.
     *
     * @param path the path of the file, or the http/rtsp URL of the stream you want
     *             to play
     * @throws IllegalStateException if it is called in an invalid state
     *                               When <code>path</code> refers to a local file, the file may
     *                               actually be opened by a process other than the calling
     *                               application. This implies that the pathname should be an absolute
     *                               path (as any other process runs with unspecified current working
     *                               directory), and that the pathname should reference a
     *                               world-readable file. As an alternative, the application could
     *                               first open the file for reading, and then use the file descriptor
     *                               form {@link #setDataSource(FileDescriptor)}.
     */
    public void setDataSource(String path) throws IOException, IllegalArgumentException, SecurityException, IllegalStateException {
        native_set_datasource(path, null, null);
    }

    /**
     * Sets the data source as a content Uri.
     *
     * @param context the Context to use when resolving the Uri
     * @param uri     the Content URI of the data you want to play
     * @throws IllegalStateException if it is called in an invalid state
     */
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
            setDataSource(mFD.getParcelFileDescriptor().getFileDescriptor());
            return;
        } catch (Exception e) {
            closeFD();
        }
        setDataSource(uri.toString(), headers);
    }

    /**
     * Sets the data source (file-path or http/rtsp URL) to use.
     *
     * @param path    the path of the file, or the http/rtsp URL of the stream you want to play
     * @param headers the headers associated with the http request for the stream you want to play
     * @throws IllegalStateException if it is called in an invalid state
     */
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

    /**
     * Sets the data source (file-path or http/rtsp URL) to use.
     *
     * @param path   the path of the file, or the http/rtsp URL of the stream you want to play
     * @param keys   AVOption key
     * @param values AVOption value
     * @throws IllegalStateException if it is called in an invalid state
     */
    public void setDataSource(String path, String[] keys, String[] values) throws IOException, IllegalArgumentException, SecurityException, IllegalStateException {
        final Uri uri = Uri.parse(path);
        if ("file".equals(uri.getScheme())) {
            path = uri.getPath();
        }

        final File file = new File(path);
        if (file.exists()) {
            FileInputStream is = new FileInputStream(file);
            FileDescriptor fd = is.getFD();
            setDataSource(fd);
            is.close();
        } else {
            native_set_datasource(path, keys, values);
        }
    }

    /**
     * Sets the data source (file-path or http/rtsp/mms URL) to use.
     *
     * @param path   the path of the file, or the http/rtsp/mms URL of the stream you
     *               want to play
     * @param keys   AVOption key
     * @param values AVOption value
     * @throws IllegalStateException if it is called in an invalid state
     */
    public native void native_set_datasource(String path, String[] keys, String[] values)
            throws IOException, IllegalArgumentException, IllegalStateException;

    /**
     * Sets the data source (FileDescriptor) to use. It is the caller's responsibility
     * to close the file descriptor. It is safe to do so as soon as this call returns.
     *
     * @param fd the FileDescriptor for the file you want to play
     * @throws IllegalStateException if it is called in an invalid state
     */
    public void setDataSource(FileDescriptor fd)
            throws IOException, IllegalArgumentException, IllegalStateException {
        // intentionally less than LONG_MAX
        setDataSource(fd, 0, 0x7ffffffffffffffL);
    }

    /**
     * Sets the data source (FileDescriptor) to use.  The FileDescriptor must be
     * seekable (N.B. a LocalSocket is not seekable). It is the caller's responsibility
     * to close the file descriptor. It is safe to do so as soon as this call returns.
     *
     * @param fd     the FileDescriptor for the file you want to play
     * @param offset the offset into the file where the data to be played starts, in bytes
     * @param length the length in bytes of the data to be played
     * @throws IllegalStateException if it is called in an invalid state
     */
    public void setDataSource(FileDescriptor fd, long offset, long length)
            throws IOException, IllegalArgumentException, IllegalStateException {
        native_set_datasource(fd, offset, length);
    }

    /**
     * Sets the data source (FileDescriptor) to use. It is the caller's
     * responsibility to close the file descriptor. It is safe to do so as soon as
     * this call returns.
     *
     * @param fileDescriptor the FileDescriptor for the file you want to play
     * @throws IllegalStateException if it is called in an invalid state
     */
    public native void native_set_datasource(FileDescriptor fileDescriptor, long offset, long length)
            throws IOException, IllegalArgumentException, IllegalStateException;

    /**
     * Prepares the player for playback, synchronously.
     *
     * After setting the datasource and the display surface, you need to either
     * call prepare() or prepareAsync(). For files, it is OK to call prepare(),
     * which blocks until MediaPlayer is ready for playback.
     *
     * @throws IllegalStateException if it is called in an invalid state
     */
    public void prepare() throws IllegalStateException, IOException {
        native_prepare();
    }

    public native int native_prepare() throws IllegalStateException;

    /**
     * Prepares the player for playback, asynchronously.
     *
     * After setting the datasource and the display surface, you need to either
     * call prepare() or prepareAsync(). For streams, you should call
     * prepareAsync(), which returns immediately, rather than blocking until
     * enough data has been buffered.
     *
     * @throws IllegalStateException if it is called in an invalid state
     */
    public void prepareAsync() throws IllegalStateException, IOException {
        native_prepare_async();
    }

    public native int native_prepare_async() throws IllegalStateException;

    /**
     * Starts or resumes playback. If playback had previously been paused,
     * playback will continue from where it was paused. If playback had been
     * stopped, or never started before, playback will start at the beginning.
     *
     * @throws IllegalStateException if it is called in an invalid state
     */
    public void start() throws IllegalStateException {
        stayAwake(true);
        native_start();
    }

    public native int native_start() throws IllegalStateException;

    /**
     * The same as {@link #stop()}
     *
     * @throws IllegalStateException if the internal player engine has not been initialized.
     */
    public void stop() throws IllegalStateException {
        stayAwake(false);
        native_stop();
    }

    public native int native_stop() throws IllegalStateException;

    /**
     * The same as {@link #pause()}
     *
     * @throws IllegalStateException if the internal player engine has not been initialized.
     */
    public void pause() throws IllegalStateException {
        stayAwake(false);
        native_pause();
    }

    public native int native_pause() throws IllegalStateException;

    /**
     * audio effect API
     *
     * @param type
     * @return
     */
    public int setAuxEffectSendLevel(int type) {
        Log.d(TAG, "setaudio effect:" + type);
        return native_set_audio_effect(type);
    }

    /**
     * Set the low-level power management behavior for this MediaPlayer. This can
     * be used when the MediaPlayer is not playing through a SurfaceHolder set
     * with {@link #setDisplay(SurfaceHolder)} and thus can use the high-level
     * {@link #setScreenOnWhilePlaying(boolean)} feature.
     *
     * This function has the MediaPlayer access the low-level power manager
     * service to control the device's power usage while playing is occurring. The
     * parameter is a combination of {@link android.os.PowerManager} wake flags.
     * Use of this method requires {@link android.Manifest.permission#WAKE_LOCK}
     * permission. By default, no attempt is made to keep the device awake during
     * playback.
     *
     * @param context the Context to use
     * @param mode    the power/wake mode to set
     * @see android.os.PowerManager
     */
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

    /**
     * Control whether we should use the attached SurfaceHolder to keep the screen
     * on while video playback is occurring. This is the preferred method over
     * {@link #setWakeMode} where possible, since it doesn't require that the
     * application have permission for low-level wake lock access.
     *
     * @param screenOn Supply true to keep the screen on, false to allow it to turn off.
     */
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

    /**
     * Returns the width of the video.
     *
     * @return the width of the video, or 0 if there is no video, or the width has
     * not been determined yet. The OnVideoSizeChangedListener can be
     * registered via
     * {@link #setOnVideoSizeChangedListener(OnVideoSizeChangedListener)}
     * to provide a notification when the width is available.
     */
    public int getVideoWidth() {

        return native_get_video_width();
    }

    public native int native_get_video_width();

    /**
     * Returns the height of the video.
     *
     * @return the height of the video, or 0 if there is no video, or the height
     * has not been determined yet. The OnVideoSizeChangedListener can be
     * registered via
     * {@link #setOnVideoSizeChangedListener(OnVideoSizeChangedListener)}
     * to provide a notification when the height is available.
     */
    public int getVideoHeight() {

        return native_get_video_height();
    }

    public native int native_get_video_height();

    /**
     * Checks whether the MediaPlayer is playing.
     *
     * @return true if currently playing, false otherwise
     */
    public boolean isPlaying() {
        return (native_is_playing() == 1) ? true : false;
    }

    /**
     * Seeks to specified time position.
     *
     * @param msec the offset in milliseconds from the start to seek to
     * @throws IllegalStateException if the internal player engine has not been initialized
     */
    public void seekTo(int msec) {
        native_seekTo(msec);
    }

    public native int native_seekTo(int msec) throws IllegalStateException;

    /**
     * Gets the current playback position.
     *
     * @return the current position in milliseconds
     */
    public int getCurrentPosition() {
        return native_get_current_position();
    }

    public native int native_get_current_position();

    /**
     * Gets the duration of the file.
     *
     * @return the duration in milliseconds
     */
    public int getDuration() {
        return native_get_duration();
    }

    public native int native_get_duration();

    /**
     * Gets the media metadata.
     *
     * @return The metadata, possibly empty. null if an error occurred.
     */
    public Metadata getMetadata() {
        if (mMeta == null) {
            mMeta = new Metadata();
            Map<byte[], byte[]> meta = new HashMap<byte[], byte[]>();

            if (!native_getMetadata(meta)) {
                return null;
            }

            if (!mMeta.parse(meta, getMetaEncoding())) {
                return null;
            }
        }
        return mMeta;
    }

    /**
     * Releases resources associated with this MediaPlayer object. It is
     * considered good practice to call this method when you're done using the
     * MediaPlayer.
     */
    public void release() {
        stayAwake(false);
        updateSurfaceScreenOn();
        native_stop();
        native_release();
        closeFD();
        removeListenners();
        mEventHandler = null;
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

    public native int native_release();

    /**
     * Resets the MediaPlayer to its uninitialized state. After calling this
     * method, you will have to initialize it again by setting the data source and
     * calling prepare().
     */
    public void reset() {
        stayAwake(false);
        native_reset();
        mEventHandler.removeCallbacksAndMessages(null);
        closeFD();
    }

    public native int native_reset();

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

    /**
     * Sets the player to be looping or non-looping.
     *
     * @param looping whether to loop or not
     */
    public native void setLooping(boolean looping);

    /**
     * Checks whether the MediaPlayer is looping or non-looping.
     *
     * @return true if the MediaPlayer is currently looping, false otherwise
     */
    public native boolean isLooping();

    public native void setVolume(float leftVolume, float rightVolume);

    private native final boolean native_getTrackInfo(SparseArray<byte[]> trackSparse);

    private native final boolean native_getMetadata(Map<byte[], byte[]> meta);

    private static native final void native_init();

    private native final void native_finalize();


    /**
     * Returns an array of track information.
     *
     * @return Array of track info. The total number of tracks is the array
     * length. Must be called again if an external timed text source has
     * been added after any of the addTimedTextSource methods are called.
     */
    public TrackInfo[] getTrackInfo(String encoding) {
        TrackInfo[] trackInfo = getInbandTrackInfo(encoding);
        // add out-of-band tracks
        String timedTextPath = getTimedTextPath();
        if (TextUtils.isEmpty(timedTextPath)) {
            return trackInfo;
        }
        TrackInfo[] allTrackInfo = new TrackInfo[trackInfo.length + 1];
        System.arraycopy(trackInfo, 0, allTrackInfo, 0, trackInfo.length);
        int i = trackInfo.length;
        SparseArray<app.dttv.dttvlib.MediaFormat> trackInfoArray = new SparseArray<app.dttv.dttvlib.MediaFormat>();
        app.dttv.dttvlib.MediaFormat mediaFormat = new app.dttv.dttvlib.MediaFormat();
        mediaFormat.setString(app.dttv.dttvlib.MediaFormat.KEY_TITLE, timedTextPath.substring(timedTextPath.lastIndexOf("/")));
        mediaFormat.setString(app.dttv.dttvlib.MediaFormat.KEY_PATH, timedTextPath);
        SparseArray<app.dttv.dttvlib.MediaFormat> timedTextSparse = findTrackFromTrackInfo(TrackInfo.MEDIA_TRACK_TYPE_TIMEDTEXT, trackInfo);
        if (timedTextSparse == null || timedTextSparse.size() == 0)
            trackInfoArray.put(timedTextSparse.keyAt(0), mediaFormat);
        else
            trackInfoArray.put(timedTextSparse.keyAt(timedTextSparse.size() - 1), mediaFormat);
        mOutOfBandTracks = new TrackInfo(TrackInfo.MEDIA_TRACK_TYPE_SUBTITLE, trackInfoArray);
        allTrackInfo[i] = mOutOfBandTracks;
        return allTrackInfo;
    }

    private TrackInfo[] getInbandTrackInfo(String encoding) {
        if (mInbandTracks == null) {
            SparseArray<byte[]> trackSparse = new SparseArray<byte[]>();
            if (!native_getTrackInfo(trackSparse)) {
                return null;
            }

            int size = trackSparse.size();
            mInbandTracks = new TrackInfo[size];
            for (int i = 0; i < size; i++) {
                SparseArray<app.dttv.dttvlib.MediaFormat> sparseArray = parseTrackInfo(trackSparse.valueAt(i), encoding);
                TrackInfo trackInfo = new TrackInfo(trackSparse.keyAt(i), sparseArray);
                mInbandTracks[i] = trackInfo;
            }
        }
        return mInbandTracks;
    }

    /**
     * Use default chartset {@link #getTrackInfo()} method.
     *
     * @return array of {@link TrackInfo}
     */
    public TrackInfo[] getTrackInfo() {
        return getTrackInfo(Charset.defaultCharset().name());
    }

    private SparseArray<app.dttv.dttvlib.MediaFormat> parseTrackInfo(byte[] tracks, String encoding) {
        SparseArray<app.dttv.dttvlib.MediaFormat> trackSparse = new SparseArray<app.dttv.dttvlib.MediaFormat>();
        String trackString;
        int trackNum;
        try {
            trackString = new String(tracks, encoding);
        } catch (Exception e) {
            Log.e(TAG, "getTrackMap exception");
            trackString = new String(tracks);
        }
        for (String s : trackString.split("!#!")) {
            try {
                app.dttv.dttvlib.MediaFormat mediaFormat = null;
                String[] formats = s.split("\\.");
                if (formats == null)
                    continue;
                trackNum = Integer.parseInt(formats[0]);
                if (formats.length == 3) {
                    mediaFormat = app.dttv.dttvlib.MediaFormat.createSubtitleFormat(formats[2], formats[1]);
                } else if (formats.length == 2) {
                    mediaFormat = app.dttv.dttvlib.MediaFormat.createSubtitleFormat("", formats[1]);
                }
                trackSparse.put(trackNum, mediaFormat);
                Log.e(TAG, "parseTrackInfo:" + trackNum + mediaFormat.toString());
            } catch (NumberFormatException e) {
            }
        }

        return trackSparse;
    }

    /**
     * @param mediaTrackType
     * @param trackInfo
     * @return {@link TrackInfo#getTrackInfoArray()}
     */
    public SparseArray<app.dttv.dttvlib.MediaFormat> findTrackFromTrackInfo(int mediaTrackType, TrackInfo[] trackInfo) {
        for (int i = 0; i < trackInfo.length; i++) {
            if (trackInfo[i].getTrackType() == mediaTrackType) {
                return trackInfo[i].getTrackInfoArray();
            }
        }
        return null;
    }

    /**
     * Set the file-path of an external timed text.
     *
     * @param path must be a local file
     */
    public native void addTimedTextSource(String path);

    /**
     * Selects a track.
     * <p>
     * In any valid state, if it is called multiple times on the same type of
     * track (ie. Video, Audio, Timed Text), the most recent one will be chosen.
     * </p>
     * <p>
     * The first audio and video tracks are selected by default if available, even
     * though this method is not called. However, no timed text track will be
     * selected until this function is called.
     * </p>
     *
     * @param index the index of the track to be selected. The valid range of the
     *              index is 0..total number of track - 1. The total number of tracks
     *              as well as the type of each individual track can be found by
     *              calling {@link #getTrackInfo()} method.
     * @see app.dttv.dttvlib.MediaPlayer#getTrackInfo
     */
    public void selectTrack(int index) {
        selectOrDeselectBandTrack(index, true /* select */);
    }

    /**
     * Deselect a track.
     * <p>
     * Currently, the track must be a timed text track and no audio or video
     * tracks can be deselected.
     * </p>
     *
     * @param index the index of the track to be deselected. The valid range of the
     *              index is 0..total number of tracks - 1. The total number of tracks
     *              as well as the type of each individual track can be found by
     *              calling {@link #getTrackInfo()} method.
     * @see app.dttv.dttvlib.MediaPlayer#getTrackInfo
     */
    public void deselectTrack(int index) {
        selectOrDeselectBandTrack(index, false /* select */);
    }

    private void selectOrDeselectBandTrack(int index, boolean select) {
        if (mOutOfBandTracks != null) {
            SparseArray<app.dttv.dttvlib.MediaFormat> mediaSparse = mOutOfBandTracks.getTrackInfoArray();
            int trackIndex = mediaSparse.keyAt(0);
            app.dttv.dttvlib.MediaFormat mediaFormat = mediaSparse.valueAt(0);
            if (index == trackIndex && select) {
                addTimedTextSource(mediaFormat.getString(app.dttv.dttvlib.MediaFormat.KEY_PATH));
                return;
            }
        }
        selectOrDeselectTrack(index, select);
    }

    private native void selectOrDeselectTrack(int index, boolean select);

    @Override
    protected void finalize() {
        native_finalize();
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


    private void updateSub(int subType, byte[] bytes, String encoding, int width, int height) {
        if (mEventHandler != null) {
            Message m = mEventHandler.obtainMessage(MEDIA_TIMED_TEXT, width, height);
            Bundle b = m.getData();
            if (subType == SUBTITLE_TEXT) {
                b.putInt(MEDIA_SUBTITLE_TYPE, SUBTITLE_TEXT);
                if (encoding == null) {
                    b.putString(MEDIA_SUBTITLE_STRING, new String(bytes));
                } else {
                    try {
                        b.putString(MEDIA_SUBTITLE_STRING, new String(bytes, encoding.trim()));
                    } catch (UnsupportedEncodingException e) {
                        Log.e("updateSub", e.toString());
                        b.putString(MEDIA_SUBTITLE_STRING, new String(bytes));
                    }
                }
            } else if (subType == SUBTITLE_BITMAP) {
                b.putInt(MEDIA_SUBTITLE_TYPE, SUBTITLE_BITMAP);
                b.putByteArray(MEDIA_SUBTITLE_BYTES, bytes);
            }
            mEventHandler.sendMessage(m);
        }
    }

    /**
     * Calling this result in only the audio track being played.
     */
    public void releaseDisplay() {
        native_release_surface();
        mSurfaceHolder = null;
        mSurface = null;
    }

    public native void native_release_surface();

    /**
     * Get the encoding if haven't set with {@link #setMetaEncoding(String)}
     *
     * @return the encoding
     */
    public native String getMetaEncoding();

    /**
     * Set the encoding MediaPlayer will use to determine the metadata
     *
     * @param encoding e.g. "UTF-8"
     */
    public native void setMetaEncoding(String encoding);

    /**
     * Get the audio track number in playback
     *
     * @return track number
     */
    public native int getAudioTrack();

    /**
     * Get the video track number in playback
     *
     * @return track number
     */
    public native int getVideoTrack();

    /**
     * Tell the MediaPlayer whether to show timed text
     *
     * @param shown true if wanna show
     */
    public native void setTimedTextShown(boolean shown);

    /**
     * Set the encoding to display timed text.
     *
     * @param encoding MediaPlayer will detet it if null
     */
    public native void setTimedTextEncoding(String encoding);

    public native int getTimedTextLocation();

    /**
     * You can get the file-path of the external subtitle in use.
     *
     * @return null if no external subtitle
     */
    public native String getTimedTextPath();

    /**
     * Get the subtitle track number in playback
     *
     * @return track number
     */
    public native int getTimedTextTrack();


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
     * Class for MediaPlayer to return each audio/video/subtitle track's metadata.
     *
     * @see app.dttv.dttvlib.MediaPlayer#getTrackInfo
     */
    static public class TrackInfo {
        public static final int MEDIA_TRACK_TYPE_UNKNOWN = 0;
        public static final int MEDIA_TRACK_TYPE_VIDEO = 1;
        public static final int MEDIA_TRACK_TYPE_AUDIO = 2;
        public static final int MEDIA_TRACK_TYPE_TIMEDTEXT = 3;
        public static final int MEDIA_TRACK_TYPE_SUBTITLE = 4;
        final int mTrackType;
        final SparseArray<app.dttv.dttvlib.MediaFormat> mTrackInfoArray;

        TrackInfo(int trackType, SparseArray<app.dttv.dttvlib.MediaFormat> trackInfoArray) {
            mTrackType = trackType;
            mTrackInfoArray = trackInfoArray;
        }

        /**
         * Gets the track type.
         *
         * @return TrackType which indicates if the track is video, audio, timed
         * text.
         */
        public int getTrackType() {
            return mTrackType;
        }

        /**
         * Gets the track info
         *
         * @return map trackIndex to MediaFormat
         */
        public SparseArray<app.dttv.dttvlib.MediaFormat> getTrackInfoArray() {
            return mTrackInfoArray;
        }
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

    // private api = set glfilter
    public int setGlFilter(long arg1, long arg2) {
        native_set_parameter(KEY_PARAMETER_SET_GLFILTER, arg1, arg2);
        return 0;
    }

    public int setGlFilterParameter(long arg1, long arg2) {
        native_set_parameter(KEY_PARAMETER_GLRENDER_SET_FILTER_PARAMETER, arg1, arg2);
        return 0;
    }

    public int setGlFilterParameter(int[] arg) {
        native_set_gl_parameter(KEY_PARAMETER_GLRENDER_SET_FILTER_PARAMETER, arg);
        return 0;
    }

    public native int native_setup(Object thiz);

    public native int native_is_playing();

    public native int native_get_parameter(int cmd, long arg1, long arg2);

    public native int native_set_parameter(int cmd, long arg1, long arg2);

    public native int native_set_gl_parameter(int cmd, int[] arg);

    //opengl esv2
    public native int native_surface_create();

    public native int native_surface_change(int w, int h);

    public native int native_draw_frame();

    public native int native_set_audio_effect(int t);

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
                    if (mOnSeekCompleteListener != null) {
                        mOnSeekCompleteListener.onSeekComplete(mMediaPlayer);
                        Log.i(TAG, "seek complete");
                    }
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
                    Log.e(TAG, "Error occured. Error ID = " + msg.what);
                    if(mOnErrorListener != null) {
                        mOnErrorListener.onError(mMediaPlayer, MEDIA_ERROR, 0);
                    }
                    release();
                    break;
                case MEDIA_HW_ERROR:
                    if (mOnHWRenderFailedListener != null)
                        mOnHWRenderFailedListener.onFailed();
                    return;
                default:
                    Log.e(TAG, "Unknown message type " + msg.what);
                    return;
            }
        }
    }

}
