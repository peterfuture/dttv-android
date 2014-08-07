package dttv.app;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;

/**
 * index for media file list
 * @author shihx1
 *
 */
public class IndexActivity extends Activity {
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		super.onCreate(savedInstanceState);
		setContentView(R.layout.index);
	}
	
	public void open_folder(View v){
		Intent intent = new Intent();
		intent.setClass(this, FileBrowserActivity.class);
		startActivity(intent);
	}
	
	public void open_pager(View v){
		Intent intent = new Intent();
		intent.setClass(this, MainActivity.class);
		startActivity(intent);
	}
}
