package dttv.app;




import java.io.File;
import java.io.FileDescriptor;
import java.io.FileInputStream;
import java.io.IOException;
import java.util.Map;

import android.annotation.SuppressLint;
import android.content.ContentResolver;
import android.content.Context;
import android.content.res.AssetFileDescriptor;
import android.graphics.Bitmap;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.PowerManager;
import android.view.Surface;
import android.view.SurfaceHolder;
import dttv.app.utils.Constant;
import dttv.app.utils.FileUtil;
import dttv.app.utils.Log;

/**
 * 
 * @author shihx1
 * @email 
 * @date 2014-7-17
 * @purpose making up a standard class likes android
 */
public class DtPlayer {
	
	private Context mContext;
	private SurfaceHolder mSurfaceHolder;
	private Surface mSurface;
	
	private PowerManager.WakeLock mWakeLock = null;
	private boolean mScreenOnWhilePlaying;
	private boolean mStayAwake;
	
	private AssetFileDescriptor mFD;
	
	private OnHWRenderFailedListener mOnHWRenderFailedListener;
	private OnPreparedListener mOnPreparedListener;
	private OnFreshVideo mOnFreshVideo;
	private OnCompletionListener mOnCompletionListener;
	private OnBufferingUpdateListener mOnBufferingUpdateListener;
	private OnCachingUpdateListener mOnCachingUpdateListener;
	private OnSeekCompleteListener mOnSeekCompleteListener;
	private OnVideoSizeChangedListener mOnVideoSizeChangedListener;
	private OnErrorListener mOnErrorListener;
	
	private OnInfoListener mOnInfoListener;
	private OnTimedTextListener mOnTimedTextListener;
	
	private static EventHandler mEventHandler;
	
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
	
	public DtPlayer(Context ctx) {
		// TODO Auto-generated constructor stub
		this(ctx,false);
		native_setup();
	}
	
	public DtPlayer(Context ctx,boolean isHardWare) {
		// TODO Auto-generated constructor stub
		mContext = ctx;
		if(isHardWare){
			//loadHardWare so
		}else{
			//soft parse so
		}
		Looper looper;
		if((looper = Looper.myLooper())!=null)
			mEventHandler = new EventHandler(this,looper);
		else if((looper = Looper.getMainLooper()) !=null)
			mEventHandler = new EventHandler(this,looper);
		else
			mEventHandler = null;
		//native_init();
	}
	
	static{
		System.loadLibrary("gnustl_shared");
		System.loadLibrary("dtp");
		System.loadLibrary("dtap");
		System.loadLibrary("dtp_jni");		
	}
	
	public void setDisplay(SurfaceHolder surfaceHolder){
		if(surfaceHolder==null){
			releaseDisplay();
		}else{
			mSurfaceHolder = surfaceHolder;
			mSurface = surfaceHolder.getSurface();
			native_set_video_surface(mSurface);
			updateSurfaceScreenOn();
		}
	}
	
	public void setSurface(Surface surface) {
		if (surface == null) {
			releaseDisplay();
		} else {
			mSurfaceHolder = null;
			mSurface = surface;
			native_set_video_surface(mSurface);
			updateSurfaceScreenOn();
		}
	}
	
	public void releaseDisplay(){
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
	private void stayAwake(boolean awake){
		if(mWakeLock != null){
			if(awake && !mWakeLock.isHeld()){
				mWakeLock.acquire();
			}else if(!awake && mWakeLock.isHeld()){
				mWakeLock.release();
			}
		}
		mStayAwake = awake;
		updateSurfaceScreenOn();
	}
	
	private void updateSurfaceScreenOn(){
		if(mSurfaceHolder!=null)
			mSurfaceHolder.setKeepScreenOn(mScreenOnWhilePlaying && mStayAwake);
	}
	
	public int setDataSource(String path) throws IOException, IllegalArgumentException,SecurityException,IllegalStateException{
		//_setDataSource(path, null, null);
		Log.d(Constant.LOGTAG, "DTPLAYER-JAVA:setDataSource");
		return native_setDataSource(path);
	}
	
	public void setDataSource(Context context,Uri uri) throws IOException, IllegalArgumentException, SecurityException, IllegalStateException{
		setDataSource(context,uri,null);
	}
	
	public void setDataSource(Context context,Uri uri, Map<String, String> headers) throws IOException,IllegalArgumentException,SecurityException,IllegalStateException{
		if(context==null || uri ==null)
			throw new IllegalArgumentException();
		String scheme = uri.getScheme();
		if(scheme == null || scheme.equals("file")){
			setDataSource(FileUtil.getPath(uri.toString()));
			return;
		}
			
		ContentResolver resolver = context.getContentResolver();
		mFD = resolver.openAssetFileDescriptor(uri, "r");
		if(mFD==null)
			return;
		try {
			setDataSource(mFD.getParcelFileDescriptor().getFileDescriptor());
			return;
		} catch (Exception e) {
			// TODO: handle exception
			closeFD();
		}
		setDataSource(uri.toString(), headers);
	}
	
	public void setDataSource(String path, Map<String,String> headers) throws IOException,IllegalArgumentException,SecurityException,IllegalStateException{
		String values[] = null;
		String keys[] = null;
		if(headers!=null){
			keys = new String[headers.size()];
			values = new String[headers.size()];
			int i = 0;
			for(Map.Entry<String, String> entry:headers.entrySet()){
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
			setDataSource(fd);
			is.close();
		} else {
			_setDataSource(path, keys, values);
		}
	}
	
	/**
	 * jni callback status for player
	 * @param status
	 * @throws IllegalStateException
	 */
	public static void updateState(int status) throws IllegalStateException{
		Log.d(Constant.LOGTAG, "Notify called, status_code:"+status);
		switch(status){
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
	
	public void start() throws IllegalStateException{
		stayAwake(true);
		native_start();
	}
	
	public void prepare() throws IllegalStateException, IOException{
		native_prePare();
	}
	
	public void stop() throws IllegalStateException{
		stayAwake(false);
		native_stop();
	}
	
	public void pause() throws IllegalStateException {
		stayAwake(false);
		native_pause();
	}
	
	@SuppressLint("Wakelock")
	public void setWakeMode(Context context, int mode){
		boolean washeld = false;
		if(mWakeLock !=null){
			if(mWakeLock.isHeld()){
				washeld = true;
				mWakeLock.release();
			}
			mWakeLock = null;
		}
		
		PowerManager pm = (PowerManager) context.getSystemService(Context.POWER_SERVICE);
		mWakeLock = pm.newWakeLock(mode | PowerManager.ON_AFTER_RELEASE, DtPlayer.class.getName());
		mWakeLock.setReferenceCounted(false);
		if(washeld){
			mWakeLock.acquire();
		}
	}
	
	
	private void closeFD(){
		if(mFD !=null){
			try {
				mFD.close();
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
			mFD = null;
		}
	}
	
	/**
	 * Resets the MediaPlayer to its uninitialized state. After calling this
	 * method, you will have to initialize it again by setting the data source
	 * and calling prepare().
	 */
	public void reset() {
		stayAwake(false);
		native_reset();
		mEventHandler.removeCallbacksAndMessages(null);
		closeFD();
	}
	
	private class EventHandler extends Handler{
		private DtPlayer mMediaPlayer;
		private Bundle mData;

		
		public EventHandler(DtPlayer mp,Looper looper) {
			// TODO Auto-generated constructor stub
			super(looper);
			mMediaPlayer = mp;
		}
		
		@Override
		public void handleMessage(Message msg) {
			// TODO Auto-generated method stub
			switch(msg.what){
			case MEDIA_PREPARED:
				if(mOnPreparedListener != null)
					mOnPreparedListener.onPrepared(mMediaPlayer);
				break;
			case MEDIA_PLAYBACK_COMPLETE:
				if(mOnCompletionListener != null)
					mOnCompletionListener.onCompletion(mMediaPlayer);
				break;
			case MEDIA_BUFFERING_UPDATE:
				if(mOnBufferingUpdateListener != null)
					mOnBufferingUpdateListener.onBufferingUpdate(mMediaPlayer, msg.arg1);
				break;
			case MEDIA_SEEK_COMPLETE:
				if (native_isPlaying()==1 ? true:false)
					stayAwake(true);
				if(mOnSeekCompleteListener != null)
					mOnSeekCompleteListener.onSeekComplete(mMediaPlayer);
				break;
			case MEDIA_SET_VIDEO_SIZE:
				if (mOnVideoSizeChangedListener != null)
					mOnVideoSizeChangedListener.onVideoSizeChanged(
							mMediaPlayer, msg.arg1, msg.arg2);
				break;
			case MEDIA_FRESH_VIDEO:
				if(mOnFreshVideo != null)
					mOnFreshVideo.onFresh(mMediaPlayer);
				break;
			case MEDIA_ERROR:
				
				break;
			case MEDIA_HW_ERROR:
				if (mOnHWRenderFailedListener != null)
					mOnHWRenderFailedListener.onFailed();
				return;
			default:
				Log.e("","Unknown message type " + msg.what);
				return;
			}
		}
	}

	/*public Metadata getMetadata() {
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
	}*/
	public void release(){
		stayAwake(false);
		updateSurfaceScreenOn();
		mOnPreparedListener = null;
		mOnFreshVideo = null;
		mOnBufferingUpdateListener = null;
		mOnCompletionListener = null;
		mOnSeekCompleteListener = null;
		mOnErrorListener = null;
		mOnInfoListener = null;
		mOnVideoSizeChangedListener = null;
		mOnCachingUpdateListener = null;
		mOnHWRenderFailedListener = null;
		native_stop();
		native_release();
		closeFD();
		mEventHandler  = null;
	}
	
	public interface OnHWRenderFailedListener{
		public void onFailed();
	}
	
	public interface OnPreparedListener{
		void onPrepared(DtPlayer mp);
	}
	
	public interface OnFreshVideo{
		void onFresh(DtPlayer mp);
	}
	
	public interface OnCompletionListener{
		void onCompletion(DtPlayer mp);
	}
	
	public interface OnSeekCompleteListener{
		void onSeekComplete(DtPlayer mp);
	}
	
	public interface OnErrorListener{
		boolean onError(DtPlayer mp,int what,int extra);
	}
	
	public interface OnVideoSizeChangedListener{
		public void onVideoSizeChanged(DtPlayer mp,int width,int height);
	}
	
	
	
	
	/**
	 * the under interface will be 
	 * accessed in the future
	 */
	
	public interface OnBufferingUpdateListener{
		/**
		 * Called to update status in buffering a media stream. Buffering is
		 * storing data in memory while caching on external storage.
		 * 
		 * @param mp
		 *            the MediaPlayer thenative_isPlaying update pertains to
		 * @param percent
		 *            the percentage (0-100) of the buffer that has been filled
		 *            thus far
		 */
		void onBufferingUpdate(DtPlayer mp,int percent);
	}
	
	public interface OnCachingUpdateListener {
		/**
		 * Called to update status in caching a media stream. Caching is storing
		 * data on external storage while buffering in memory.
		 * 
		 * @param mp
		 *            the MediaPlayer the update pertains to
		 * @param segments
		 *            the cached segments in bytes, in format [s1begin, s1end,
		 *            s2begin, s2end], s1begin < s1end < s2begin < s2end. e.g.
		 *            [124, 100423, 4321412, 214323433]
		 */
		void onCachingUpdate(DtPlayer mp, long[] segments);

		/**
		 * Cache speed
		 * 
		 * @param mp
		 *            the MediaPlayer the update pertains to
		 * @param speed
		 *            the cached speed size kb/s
		 */
		void onCachingSpeed(DtPlayer mp, int speed);

		/**
		 * Cache start
		 * 
		 * @param mp
		 */
		void onCachingStart(DtPlayer mp);

		/**
		 * Cache compelete
		 */
		void onCachingComplete(DtPlayer mp);

		/**
		 * Cache not available
		 * 
		 * @param mp
		 *            the MediaPlayer the update pertains to
		 * @param info
		 *            the not available info
		 *            <ul>
		 *            <li>{@link #CACHE_INFO_NO_SPACE}
		 *            <li>{@link #CACHE_INFO_STREAM_NOT_SUPPORT}
		 *            </ul>
		 */
		void onCachingNotAvailable(DtPlayer mp, int info);
	}
	
	public interface OnInfoListener {
		boolean onInfo(DtPlayer mp, int what, int extra);
	}
	
	public interface OnTimedTextListener {
		/**dtPlayer.release();
			dtPlayer.reset();
		 * Called to indicate that a text timed text need to display
		 * 
		 * @param text
		 *            the timedText to display
		 */
		public void onTimedText(String text);

		/**
		 * Called to indicate that an image timed text need to display
		 * 
		 * @param pixels
		 *            the pixels of the timed text image
		 * @param width
		 *            the width of the timed text image
		 * @param height
		 *            the height of the timed text image
		 */
		public void onTimedTextUpdate(byte[] pixels, int width, int height);
	}

	
	/**
	 * Register a callback to be invoked when the media source is ready for
	 * playback.
	 * 
	 * @param listener
	 *            the callback that will be run
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
	 * @param listener
	 *            the callback that will be run
	 */
	public void setOnCompletionListener(OnCompletionListener listener) {
		mOnCompletionListener = listener;
	}

	/**
	 * Register a callback to be invoked when the status of a network stream's
	 * buffer has changed.
	 * 
	 * @param listener
	 *            the callback that will be run.
	 */
	public void setOnBufferingUpdateListener(OnBufferingUpdateListener listener) {
		mOnBufferingUpdateListener = listener;
	}

	/**
	 * Register a callback to be invoked when the segments cached on storage has
	 * changed.
	 * 
	 * @param listener
	 *            the callback that will be run.
	 */
	public void setOnCachingUpdateListener(OnCachingUpdateListener listener) {
		mOnCachingUpdateListener = listener;
	}
	
	/**
	 * Register a callback to be invoked when a seek operation has been
	 * completed.
	 * 
	 * @param listener
	 *            the callback that will be run
	 */
	public void setOnSeekCompleteListener(OnSeekCompleteListener listener) {
		mOnSeekCompleteListener = listener;
	}

	/**
	 * Register a callback to be invoked when the video size is known or
	 * updated.
	 * 
	 * @param listener
	 *            the callback that will be run
	 */
	public void setOnVideoSizeChangedListener(
			OnVideoSizeChangedListener listener) {
		mOnVideoSizeChangedListener = listener;
	}

	/**
	 * Register a callback to be invoked when an error has happened during an
	 * asynchronous operation.
	 * 
	 * @param listener
	 *            the callback that will be run
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
	 * @param listener
	 *            the callback that will be run
	 */
	public void setOnTimedTextListener(OnTimedTextListener listener) {
		mOnTimedTextListener = listener;
	}
	
	public int getCurrentPosition(){
		return native_getCurrentPosition();
	}
	
	public int getDuration(){
		return native_getDuration();
	}
	
	public void seekTo(int msec){
		native_seekTo(msec);
	}
	
	public boolean isPlaying(){
		return native_isPlaying() ==1 ? true :false;
	}
	
	public int setVideoSize(int w, int h){
		return native_setVideoSize(w,h);
	}
	
	public int getVideoWidth(){
		return native_getVideoWidth();
	}
	
	public int getVideoHeight(){
		return native_getVideoHeight();
	}
	
	//----------------------------------
	//OPENGL-ESV2
	public int onSurfaceCreated()
	{
		native_onSurfaceCreated();
		return 0;
	}
	
	public int onSurfaceChanged(int w, int h)
	{
		native_onSurfaceChanged(w,h);
		return 0;
	}
	
	public int onDrawFrame()
	{
		native_onDrawFrame();
		return 0;
	}
	//----------------------------------
	
	public native int native_setup();
	public native int native_release();
	public native void native_release_surface();
	public native void native_set_video_surface(Surface surface);
	public native int native_setDataSource(String path);
	public native void _setDataSource(String path,String[] keys, String[] values) throws IOException, IllegalArgumentException, IllegalStateException;
	public native void setDataSource(FileDescriptor fileDescriptor) throws IOException,IllegalArgumentException,IllegalStateException;
	public native  int native_prePare() throws IOException,IllegalStateException;
	public native  int native_prePareAsync() throws IllegalStateException;
	public native int native_start() throws IllegalStateException;
	public native int native_stop() throws IllegalStateException;
	public native int native_pause() throws IllegalStateException;
	public native int native_reset();
	public native int native_setVideoMode(int mode); 
	public native int native_setVideoSize(int w, int h); 
	public native int native_getVideoWidth();
	public native int native_getVideoHeight(); 
	public native int native_isPlaying();
	
	public native void setAdaptiveStream(boolean adaptive);
	public native int native_seekTo(int msec) throws IllegalStateException;
	public native int native_getCurrentPosition();
	public native Bitmap getCurrentFrame();
	public native int native_getDuration();
	
	//opengl esv2
	public native int native_onSurfaceCreated();
	public native int native_onSurfaceChanged(int w, int h);
	public native int native_onDrawFrame();
	
	public native int native_setAudioEffect(int t);
	/**
	 * Set whether cache the online playback file
	 * @param cache
	 * about future
	 */
	public native void setUseCache(boolean cache);
	public native void setCacheDirectory(String directory);
}
