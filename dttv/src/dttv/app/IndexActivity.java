package dttv.app;

import dttv.app.compnent.PopWindowCompnent;
import dttv.app.impl.ICallBack;
import dttv.app.utils.Constant;
import android.annotation.SuppressLint;
import android.app.Activity;
import android.content.Intent;
import android.graphics.drawable.ColorDrawable;
import android.os.Bundle;
import android.util.Log;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.View;
import android.view.WindowManager.LayoutParams;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import android.widget.PopupWindow;
import android.widget.TextView;
import android.widget.Toast;

/**
 * index for media file list
 * @author shihx1
 *
 */
@SuppressLint("NewApi")
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
		/*PopWindowCompnent compnent = new PopWindowCompnent(this,this);
		compnent.show(v, true);
		compnent.setCallback(new ICallBack() {
			@Override
			public void doItemClickListener(AdapterView<?> parent, View v,
					int position, long id) {
				// TODO Auto-generated method stub
				super.doItemClickListener(parent, v, position, id);
				TextView effectTxt = (TextView)v.findViewById(android.R.id.text1);
				String effectStr = effectTxt.getText().toString();
				String effectStr2 = Constant.gEqulizerPresets[position];
				Toast.makeText(IndexActivity.this, "effectStr is:"+effectStr+"-0-0 effectStr2 is:"+effectStr2, Toast.LENGTH_LONG).show();
			}
		});*/
	}
	
	public void open_pager(View v){
		/*Intent intent = new Intent();
		intent.setClass(this, MainActivity.class);
		startActivity(intent);*/
		/*PopWindowCompnent compnent = new PopWindowCompnent(this,this);
		compnent.show(v, true);*/
		View view = LayoutInflater.from(this).inflate(R.layout.effect_popwindow, null);
		ListView listView = (ListView)view.findViewById(R.id.pop_listview);
		TextView textView = (TextView)view.findViewById(R.id.pop_window_txt);
		float textSize = textView.getTextSize();
		Log.i("textSize", "----textSize is:"+textSize);
		textView.setTextSize(textSize<18 ? 18 : 21);
		PopupWindow popupWindow = new PopupWindow(view, LayoutParams.WRAP_CONTENT, LayoutParams.WRAP_CONTENT);
		//popupWindow.setBackgroundDrawable(R.drawable.)
		ArrayAdapter<String> adapter = new ArrayAdapter<String>(this, 
				android.R.layout.simple_list_item_1, Constant.gEqulizerPresets);
		popupWindow.setAnimationStyle(R.style.pop_win_style);
		ColorDrawable dw = new ColorDrawable(0xb0000000);
		popupWindow.setBackgroundDrawable(dw);
		listView.setAdapter(adapter);
		//popupWindow.showAsDropDown(v);
		int location[] = new int[2];
		v.getLocationOnScreen(location);
		int _x = location[0];
		int _y = location[1] - popupWindow.getHeight();
		popupWindow.showAtLocation(v, Gravity.NO_GRAVITY, _x, _y);
		
		//popupWindow.showAsDropDown(v, _x, _y);
	}
}
