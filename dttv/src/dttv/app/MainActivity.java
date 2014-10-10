package dttv.app;

import com.actionbarsherlock.app.SherlockFragmentActivity;
import com.actionbarsherlock.view.ActionMode;
import com.actionbarsherlock.view.MenuItem;
import com.actionbarsherlock.widget.SearchView;
import com.actionbarsherlock.view.Menu;





import android.annotation.SuppressLint;
import android.os.Bundle;
import android.support.v4.app.FragmentManager;
import android.util.Log;
import android.view.KeyEvent;
import android.widget.Toast;



import dttv.app.impl.I_KeyIntercept;
import dttv.app.utils.Constant;
import dttv.app.widget.SlideTabsFragment;
import dttv.app.widget.SlideTabsFragment.ChangeActionModeListener;

@SuppressLint("NewApi")
public class MainActivity extends SherlockFragmentActivity implements SearchView.OnQueryTextListener,SearchView.OnSuggestionListener,ChangeActionModeListener,I_KeyIntercept{
	
	final String TAG = "ActionBarViewpager";
	private int CURRENTACTION = Constant.LOCAL_FILE;
	ActionMode currentMode;
	SlideTabsFragment slideFragment;
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		super.onCreate(savedInstanceState);	
		setContentView(R.layout.activity_main);
		//currentMode = startActionMode(new LocalVideoActionBar());
		fillFragment();
	}
	
	private void fillFragment(){
		slideFragment = new SlideTabsFragment(this, this);
		FragmentManager fragmentManager = getSupportFragmentManager();
		fragmentManager.beginTransaction().replace(R.id.dt_main_content_frame, slideFragment).commit();
	}

	protected void createActionMode(int mode, Menu menu){
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
	public boolean onCreateOptionsMenu(Menu menu) {
		// TODO Auto-generated method stub
		Log.i(TAG, "enter onCreateOptionsMenu");
		createActionMode(CURRENTACTION,menu);
		return true;
	}
	
	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		// TODO Auto-generated method stub
		return super.onOptionsItemSelected(item);
	}

	@Override
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
	}

	@Override
	public boolean onQueryTextChange(String newText) {
		// TODO Auto-generated method stub
		return false;
	}

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