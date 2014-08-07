package dttv.app.widget;

import dttv.app.R;
import dttv.app.impl.I_OnMyKey;
import dttv.app.utils.MusicUtils;
import android.annotation.SuppressLint;
import android.content.Context;
import android.database.Cursor;
import android.os.Bundle;
import android.provider.MediaStore;
import android.support.v4.app.Fragment;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ListView;
import android.widget.SimpleCursorAdapter;



@SuppressLint("NewApi")
public class AudioUIFragment extends Fragment implements I_OnMyKey{
	static final String TAG = "AudioUIFragment";
	View rootView;
	ListView audio_listview;
	@Override
	public View onCreateView(LayoutInflater inflater, ViewGroup container,
			Bundle savedInstanceState) {
		
		rootView = inflater.inflate(R.layout.dt_audio_ui_fragment, container, false);
		
		return rootView;
	}
	@Override
	public void onActivityCreated(Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		super.onActivityCreated(savedInstanceState);
		initViews();
		initListener();
		initFillData();
	}
	
	private void initViews(){
		audio_listview = (ListView)rootView.findViewById(R.id.audio_listview);
	}
	
	private void initListener(){
		audio_listview.setOnItemClickListener(new ListItemClickListener());
	}
	
	private class ListItemClickListener implements OnItemClickListener{
		@Override
		public void onItemClick(AdapterView<?> arg0, View v, int position,
				long arg3) {
			// TODO Auto-generated method stub
			
		}
	}
	
	private Cursor readDataFromSD(Context context) {
		Log.d(TAG, "scanFile");
		String[] str = new String[]{MediaStore.Audio.Media._ID,
				MediaStore.Audio.Media.DISPLAY_NAME,
				MediaStore.Audio.Media.TITLE,
				MediaStore.Audio.Media.DURATION,
				MediaStore.Audio.Media.ARTIST,
				MediaStore.Audio.Media.ALBUM,
				MediaStore.Audio.Media.YEAR,
				MediaStore.Audio.Media.MIME_TYPE,
				MediaStore.Audio.Media.SIZE,
				MediaStore.Audio.Media.DATA};
		Cursor c = MusicUtils.query(context, MediaStore.Audio.Media.EXTERNAL_CONTENT_URI,
				str, MediaStore.Audio.Media.IS_MUSIC + "=1",
                null, MediaStore.Audio.Media.TITLE_KEY);
		return c;
	}
	
	private void initFillData(){
		String[] fromColumns = new String[] {MediaStore.Audio.Media.TITLE, 
				MediaStore.Audio.Media.ARTIST};
		int[] toLayoutIDs = new int[]{R.id.media_row_name,R.id.media_row_artist};
		Cursor cursor = readDataFromSD(getActivity());
		SimpleCursorAdapter adapter = new SimpleCursorAdapter(getActivity(), R.layout.dt_media_item, cursor, fromColumns, toLayoutIDs, 0);
		
		audio_listview.setAdapter(adapter);
	}
	
	@Override
	public void onDestroyView() {
		// TODO Auto-generated method stub
		super.onDestroyView();
	}
	
	@Override
	public void onDestroy() {
		// TODO Auto-generated method stub
		super.onDestroy();
	}
	@Override
	public void myOnKeyDown(int keyCode) {
		// TODO Auto-generated method stub
		
	}
}
