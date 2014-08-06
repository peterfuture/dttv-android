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
import android.widget.Toast;



import dttv.app.utils.Constant;
import dttv.app.widget.ViewPagerFragment;
import dttv.app.widget.ViewPagerFragment.ChangeActionModeListener;

@SuppressLint("NewApi")
public class MainActivity extends SherlockFragmentActivity implements SearchView.OnQueryTextListener,SearchView.OnSuggestionListener,ChangeActionModeListener{
	
	final String TAG = "ActionBarViewpager";
	private int CURRENTACTION = Constant.LOCAL_VIDEO;
	ActionMode currentMode;
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);
		//currentMode = startActionMode(new LocalVideoActionBar());
		fillFragment();
	}
	
	private void fillFragment(){
		FragmentManager fragmentManager = getSupportFragmentManager();
		fragmentManager.beginTransaction().replace(R.id.dt_main_content_frame, new ViewPagerFragment(this)).commit();
	}
	
	/*public void showVideoAction(View v){
		CURRENTACTION = LOCAL_VIDEO;
		//int i = CURRENTACTION == LOCAL_VIDEO ? 1 : 0;
		invalidateOptionsMenu();
	}
	
	public void showAudioAction(View v){
		CURRENTACTION = LOCAL_AUDIO;
		invalidateOptionsMenu();
	}
		
	
	public void showFileAction(View v){
		CURRENTACTION = LOCAL_FILE;
		invalidateOptionsMenu();
	}*/

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
		invalidateOptionsMenu();
	}
	
  
}