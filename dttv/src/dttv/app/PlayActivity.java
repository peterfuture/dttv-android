package dttv.app;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import android.app.Activity;
import android.content.Intent;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.util.Log;
import android.view.Menu;


public class PlayActivity extends Activity {
	
	private final String TAG = "DT-PLAYING";
	
	//Native API declare
	private native int native_ui_init(int w, int h);
	private native int native_dispframe();
	private native int native_ui_stop();
	
	private native int native_playerStart(String url);
	private native int native_playerPause();
	private native int native_playerResume();
	private native int native_playerStop();
	private native int native_playerSeekTo(int pos);
	

	
	private GLSurfaceView glSurefaceView;  

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_play);
		
		//start playing video
		Intent intent = getIntent();
		String file_name = intent.getStringExtra(MainActivity.FILE_MSG);
		Log.d(TAG, "Start playing "+file_name);
		
		glSurefaceView = new GLSurfaceView(this);		
        glSurefaceView.setRenderer(new GLSurfaceViewRender());  
        glSurefaceView.setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);
        this.setContentView(glSurefaceView); 
        
        native_ui_init(720,480);
        native_playerStart(file_name);
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.play, menu);
		return true;
	}
	
	class GLSurfaceViewRender implements GLSurfaceView.Renderer {  
		  
        @Override  
        public void onSurfaceCreated(GL10 gl, EGLConfig config) {  
            Log.i(TAG, "onSurfaceCreated");  
  
            // 设置背景颜色  
            gl.glClearColor(0.0f, 0f, 1f, 0.5f);
        }  
  
        @Override  
        public void onSurfaceChanged(GL10 gl, int width, int height) {  
            // 设置输出屏幕大小  
            gl.glViewport(0, 0, width, height);  
            Log.i(TAG, "onSurfaceChanged");  
        }  
  
  
        @Override  
        public void onDrawFrame(GL10 gl) {  
            Log.i(TAG, "onDrawFrame");  
            // 清除屏幕和深度缓存(如果不调用该代码, 将不显示glClearColor设置的颜色)  
            // 同样如果将该代码放到 onSurfaceCreated 中屏幕会一直闪动  
            
            //gl.glClear(GL10.GL_COLOR_BUFFER_BIT);
            
  
        }  
  
    }  
	
	static {
		System.loadLibrary("dtp_jni");
	}

}

/*
class GlBufferView extends GLSurfaceView {

	public GlBufferView(Context context, AttributeSet attrs) {
		super(context, attrs);

		setRenderer(new MyRenderer());
		requestFocus();
		setFocusableInTouchMode(true);
		setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);   // render on demand
		//setRenderMode(GLSurfaceView.RENDERMODE_CONTINUOUSLY);//continue render  
	}

	class MyRenderer implements GLSurfaceView.Renderer {
		@Override
		public void onSurfaceCreated(GL10 gl, EGLConfig c) { 
		// do nothing 
		}

		@Override
		public void onSurfaceChanged(GL10 gl, int w, int h) {
			//native_gl_resize(w, h);
			//do nothing too
		}

		@Override
		public void onDrawFrame(GL10 gl) {
			//native_gl_render();
			//draw frame
		}
	}


	
}
*/
