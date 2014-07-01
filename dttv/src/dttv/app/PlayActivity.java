package dttv.app;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import android.app.Activity;
import android.content.Context;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.util.AttributeSet;
import android.view.Menu;

public class PlayActivity extends Activity {

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_play);
		
		//start playing video
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.play, menu);
		return true;
	}

}

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
		public void onSurfaceCreated(GL10 gl, EGLConfig c) { /* do nothing */ }

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

//	static {
//		System.loadLibrary("dtp-jni");
//	}
	
}

