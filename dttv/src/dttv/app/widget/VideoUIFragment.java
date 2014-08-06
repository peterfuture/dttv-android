package dttv.app.widget;


import dttv.app.R;
import android.annotation.SuppressLint;
import android.content.AsyncQueryHandler;
import android.content.ContentResolver;
import android.database.Cursor;
import android.os.Bundle;
import android.provider.MediaStore;
import android.support.v4.app.Fragment;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ListView;
import android.widget.SimpleCursorAdapter;


@SuppressLint("NewApi")
public class VideoUIFragment extends Fragment {
	final static String TAG = "VideoUIFragment";
	View rootView;
	ListView video_listview;
	//private AsyncQueryHandler mQueryHandler;
	@Override
	public View onCreateView(LayoutInflater inflater, ViewGroup container,
			Bundle savedInstanceState) {
		
		rootView = inflater.inflate(R.layout.dt_video_ui_fragment, container, false);
		
		/*TextView tv_tabName = (TextView) rootView.findViewById(R.id.tv_tabName);
		Bundle bundle = getArguments();
		tv_tabName.setText(bundle.getString(Constant.ARGUMENTS_NAME, ""));*/
		
		return rootView;
	}
	
	
	@Override
	public void onActivityCreated(Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		super.onActivityCreated(savedInstanceState);
		initViews();
	}
	
	private void initViews(){
		video_listview = (ListView)rootView.findViewById(R.id.video_listview);
		/*mQueryHandler = new QueryHandler(getActivity().getContentResolver());
		getQueryCursor(mQueryHandler, null);*/
		MakeCursor();
		fillDataToListView();
	}
	
	class QueryHandler extends AsyncQueryHandler {
        QueryHandler(ContentResolver res) {
            super(res);
        }
        
        @Override
        protected void onQueryComplete(int token, Object cookie, Cursor cursor) {
            //mActivity.init(cursor);
        	//Log.i(TAG, "cursor.getCount() is:"+cursor.getCount());
        	//fillDataToListView(cursor);
        }
    }
	
	private void fillDataToListView(){
		String[] fromColumns = new String[] {MediaStore.Video.Media.TITLE};
		int[] toLayoutIDs = new int[]{R.id.media_row_name};
		//Cursor cursor = readDataFromSD(getActivity());
		SimpleCursorAdapter adapter = new SimpleCursorAdapter(getActivity(), R.layout.dt_media_item, mCursor, fromColumns, toLayoutIDs, 0);
		
		video_listview.setAdapter(adapter);
	}
	
	
	private void MakeCursor() {
        String[] cols = new String[] {
                MediaStore.Video.Media._ID,
                MediaStore.Video.Media.TITLE,
                MediaStore.Video.Media.DATA,
                MediaStore.Video.Media.MIME_TYPE,
                MediaStore.Video.Media.ARTIST
        };
        ContentResolver resolver = getActivity().getContentResolver();
        if (resolver == null) {
            System.out.println("resolver = null");
        } else {
            mSortOrder = MediaStore.Video.Media.TITLE + " COLLATE UNICODE";
            mWhereClause = MediaStore.Video.Media.TITLE + " != ''";
            mCursor = resolver.query(MediaStore.Video.Media.EXTERNAL_CONTENT_URI,
                cols, mWhereClause , null, mSortOrder);
        }
    }
	
	@Override
	public void onDestroyView() {
		// TODO Auto-generated method stub
		super.onDestroyView();
	}
	
	@Override
	public void onDestroy() {
		// TODO Auto-generated method stub
		if(mCursor!=null)
			mCursor.close();
		super.onDestroy();
	}
	
	private Cursor mCursor;
	private String mWhereClause;
    private String mSortOrder;
}
