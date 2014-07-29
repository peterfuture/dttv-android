package dttv.app;

import java.io.IOException;

import dttv.app.utils.Constant;
import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.widget.Toast;
/**
 * just test for
 * audio
 * @author shihx1
 *	
 */
public class NewPlayerActivity extends Activity {
	
	private DtPlayer dtPlayer;
	private String mPath;
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		super.onCreate(savedInstanceState);
		setContentView(R.layout.audio_play);
		dtPlayer = new DtPlayer(this);
		initExtraData();
		initView();
		initListener();
	}
	
	private void initExtraData(){
		Intent intent = getIntent();
		mPath = intent.getStringExtra(Constant.FILE_MSG);
		Toast.makeText(this, "mPath is:"+mPath, 1).show();
		try {
			dtPlayer.setDataSource(mPath);
			dtPlayer.prepare();
			dtPlayer.start();
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
	}
	
	private void initView(){
		
	}
	
	private void initListener(){
		
	}
	
	@Override
	protected void onStop() {
		// TODO Auto-generated method stub
		super.onStop();
	}
	
	@Override
	protected void onDestroy() {
		// TODO Auto-generated method stub
		super.onDestroy();
	}
}
