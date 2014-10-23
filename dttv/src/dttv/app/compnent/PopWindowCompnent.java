package dttv.app.compnent;

import android.app.Activity;
import android.content.Context;
import android.graphics.drawable.ColorDrawable;
import android.util.Log;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.View;
import android.view.WindowManager.LayoutParams;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import android.widget.PopupWindow;
import dttv.app.R;
import dttv.app.impl.ICallBack;
import dttv.app.impl.I_PopWindow;
import dttv.app.utils.Constant;

public class PopWindowCompnent extends Compnent implements I_PopWindow {
	String TAG = "PopWindowCompnent";
	private PopupWindow effectWindow;
	private ListView mListView;
	private Activity mActivity;
	
	public PopWindowCompnent(Activity activity,Context context) {
		super(activity);
		mActivity = activity;
		// TODO Auto-generated constructor stub
		Log.i(TAG, "enter PopWindowCompnent");
		initialize();
	}
	
	@Override
	public void initialize() {
		// TODO Auto-generated method stub
		super.initialize();
		View view = LayoutInflater.from(mActivity).inflate(R.layout.effect_popwindow, null);
		mListView = (ListView)view.findViewById(R.id.pop_listview);
		fillData();
		effectWindow = new PopupWindow(view,LayoutParams.WRAP_CONTENT,LayoutParams.WRAP_CONTENT);
		effectWindow.setAnimationStyle(R.style.pop_win_style);
		ColorDrawable dw = new ColorDrawable(0xb0ffffff);
		effectWindow.setBackgroundDrawable(dw);
		mListView.setOnItemClickListener(new ItemClickListener());
	}
	
	private class  ItemClickListener implements OnItemClickListener{
		@Override
		public void onItemClick(AdapterView<?> parent, View view, int position,
				long id) {
			// TODO Auto-generated method stub
			callback.doItemClickListener(parent, view, position, id);
			effectWindow.dismiss();
		}
	}
	 
	private void fillData(){
		ArrayAdapter<String> adapter = new ArrayAdapter<String>(mActivity, 
				android.R.layout.simple_list_item_1, Constant.gEqulizerPresets);
		mListView.setAdapter(adapter);
	}

	@Override
	public void onItemClick(View v) {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void onProgressChanged(int widgetId) {
		// TODO Auto-generated method stub
		
	}
	
	public void show(View v,boolean isShowing){
		if(isShowing){
			int location[] = new int[2];
			v.getLocationOnScreen(location);
			int _x = location[0];
			int _y = location[1] - effectWindow.getHeight();
			effectWindow.showAtLocation(v, Gravity.NO_GRAVITY, _x, _y);
		}
		else
			effectWindow.dismiss();
	}
	
	@Override
	protected void setContentView(View contentView) {
		// TODO Auto-generated method stub
		super.setContentView(contentView);
	}
	
	private ICallBack callback;
	public void setCallback(ICallBack callback){
		this.callback = callback;
	}
}
