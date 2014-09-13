package dttv.app.widget;

import dttv.app.R;
import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;


public class GlVideoView extends GLSurfaceView {

	public GlVideoView(Context context, AttributeSet attrs) {
		super(context, attrs);
		setEGLContextClientVersion(2);
		// TODO Auto-generated constructor stub
	}
	
	@Override
	protected void onDraw(Canvas canvas) {
		// TODO Auto-generated method stub
		super.onDraw(canvas);
		/*Paint paint = new Paint();
		canvas.drawText("画圆=========：", 10, 20, paint);// 画文本
		canvas.restore();
		canvas.drawCircle(100, 150, 50, paint);
		canvas.restore();
		Drawable drawable = getResources().getDrawable(R.drawable.ic_launcher);
		BitmapDrawable  bitmapDrawable = (BitmapDrawable)drawable;
		Bitmap bitmap = bitmapDrawable.getBitmap();
		canvas.drawBitmap(bitmap, 100, 300, paint);*/
	}
}
