package dttv.app;



import android.annotation.SuppressLint;
import android.app.ActionBar;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.preference.CheckBoxPreference;
import android.preference.PreferenceManager;
import android.support.v4.app.FragmentActivity;
import android.support.v4.app.FragmentManager;
import android.text.TextUtils;
import android.util.Log;
import android.view.ContextMenu;
import android.view.KeyEvent;
import android.view.MenuItem;
import android.view.View;
import android.view.ContextMenu.ContextMenuInfo;
//import android.view.Window;
//import android.view.WindowManager;
import android.widget.Toast;



import dttv.app.impl.I_KeyIntercept;
import dttv.app.utils.Constant;
import dttv.app.widget.SlideTabsFragment;
import dttv.app.widget.SlideTabsFragment.ChangeActionModeListener;

@SuppressLint("NewApi")
public class MainActivity extends FragmentActivity implements ChangeActionModeListener,I_KeyIntercept{
	
	final String TAG = "ActionBarViewpager";
	private int CURRENTACTION = Constant.LOCAL_FILE;
	SlideTabsFragment slideFragment;
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		super.onCreate(savedInstanceState);	
		/*getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, WindowManager.LayoutParams.FLAG_FULLSCREEN);
		requestWindowFeature(Window.FEATURE_NO_TITLE);*/
		setContentView(R.layout.activity_main);
		fillFragment();
	}
	
	private void fillFragment(){
		slideFragment = new SlideTabsFragment(this, this);
		FragmentManager fragmentManager = getSupportFragmentManager();
		fragmentManager.beginTransaction().replace(R.id.dt_main_content_frame, slideFragment).commit();
	}

	protected void createActionMode(int mode, ContextMenu menu){
		menu.clear();
		switch(mode){
		case Constant.LOCAL_VIDEO:
			/*SearchView searchView = new SearchView(getSupportActionBar().getThemedContext());
			searchView.setMaxWidth(600);
			searchView.setQueryHint("input words");
			searchView.setOnQueryTextListener(this);
			searchView.setOnSuggestionListener(this);*/
			
			menu.add("Search").setIcon(R.drawable.dt_action_search_icon)
			//.setActionView(searchView)
			.setShowAsAction(MenuItem.SHOW_AS_ACTION_IF_ROOM);
			menu.add("Refresh").setIcon(R.drawable.dt_action_refresh_icon)
			.setShowAsAction(MenuItem.SHOW_AS_ACTION_IF_ROOM );
			menu.add("Setting").setShowAsAction(MenuItem.SHOW_AS_ACTION_WITH_TEXT);
			break;
		case Constant.LOCAL_AUDIO:
			menu.add("Plus").setIcon(R.drawable.dt_action_plus_icon)
			.setShowAsAction(MenuItem.SHOW_AS_ACTION_IF_ROOM);
			menu.add("Setting").setShowAsAction(MenuItem.SHOW_AS_ACTION_WITH_TEXT);
			break;
		case Constant.LOCAL_FILE:
			menu.add("Refresh").setIcon(R.drawable.dt_action_refresh_icon)
			.setShowAsAction(MenuItem.SHOW_AS_ACTION_IF_ROOM);
			menu.add("Setting").setShowAsAction(MenuItem.SHOW_AS_ACTION_WITH_TEXT);
			break;
		}
	}
	
	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event) {
		// TODO Auto-generated method stub
		/*if(isNeedInterceptKey){
			return true;
		}*/
		if(keyCode==KeyEvent.KEYCODE_BACK && isNeedInterceptKey){
			slideFragment.myOnKeyDown(keyCode);
			return true;
		}
		return super.onKeyDown(keyCode, event);
	}
	
	/*@Override
	public void onBackPressed() {
		// TODO Auto-generated method stub
		if(isNeedInterceptKey){
		}else{
			super.onBackPressed();
		}
		isNeedInterceptKey = false;
	}*/
	
	@Override
	public void onCreateContextMenu(ContextMenu menu, View v,
			ContextMenuInfo menuInfo) {
		// TODO Auto-generated method stub
		createActionMode(CURRENTACTION,menu);
		super.onCreateContextMenu(menu, v, menuInfo);
	}
	
	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		// TODO Auto-generated method stub
		String res = item.getTitleCondensed().toString();
		//Toast.makeText(this, res, 1).show();
		if(!TextUtils.isEmpty(res) && res.equals("Setting")){
			openSettingUI();
		}
		return super.onOptionsItemSelected(item);
	}
	
	public void openSettingUI(){
		startActivity(new Intent(this,SettingActivity.class));
	}

	/*@Override
	public boolean onSuggestionSelect(int position) {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public boolean onSuggestionClick(int position) {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public boolean onQueryTextSubmit(String query) {
		Toast.makeText(this, "You searched for: " + query, Toast.LENGTH_LONG).show();
		return true;
	}*/

	@Override
	protected void onResume() {
		// TODO Auto-generated method stub
		super.onResume();
		/*if(slideFragment.getView() != null)
			slideFragment.getView().requestFocus();*/
	}
	
	/*@Override
	public boolean onQueryTextChange(String newText) {
		// TODO Auto-generated method stub
		return false;
	}*/

	@Override
	public void changeActionMode(int mode) {
		// TODO Auto-generated method stub
		CURRENTACTION = mode;
		if(CURRENTACTION == Constant.LOCAL_FILE){
			isNeedInterceptKey = true;
		}else{
			isNeedInterceptKey = false;
		}
		invalidateOptionsMenu();
	}
	private boolean isNeedInterceptKey;
	@Override
	public boolean isNeedIntercept(boolean isNeed) {
		// TODO Auto-generated method stub
		Toast.makeText(this, "enter isNeedIntercept", 1).show();
		isNeedInterceptKey = isNeed;
		return isNeed;
	}
	
  
}