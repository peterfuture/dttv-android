package dttv.app.frament;


import android.content.Context;
import android.database.Cursor;
import android.provider.MediaStore;
import android.support.v4.app.Fragment;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.BaseAdapter;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;

import butterknife.BindView;
import dttv.app.R;
import dttv.app.base.SimpleFragment;

/**
 * A simple {@link Fragment} subclass.
 */
public class LocalVideoFragment extends SimpleFragment {

    @BindView(R.id.local_video_list)
    ListView localVideoListView;

    private Cursor videocursor;
    private int video_column_index;
    int count;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_local_video;
    }

    @Override
    protected void initEventAndData() {
        init_phone_video_grid();
    }

    public LocalVideoFragment() {
        // Required empty public constructor
    }

    private void init_phone_video_grid() {
        //System.gc();
        String[] proj = { MediaStore.Video.Media._ID,
                MediaStore.Video.Media.DATA,
                MediaStore.Video.Media.DISPLAY_NAME,
                MediaStore.Video.Media.SIZE, MediaStore.Video.Media.DEFAULT_SORT_ORDER };
        videocursor = getActivity().managedQuery(MediaStore.Video.Media.EXTERNAL_CONTENT_URI,
                proj, null, null, null);
        count = videocursor.getCount();
        localVideoListView.setAdapter(new VideoAdapter(getActivity().getApplicationContext()));
        localVideoListView.setOnItemClickListener(videogridlistener);
    }

    public class VideoAdapter extends BaseAdapter {
        private Context vContext;

        public VideoAdapter(Context c) {
            vContext = c;
        }

        public int getCount() {
            return count;
        }

        public Object getItem(int position) {
            return position;
        }

        public long getItemId(int position) {
            return position;
        }

        public View getView(int position, View convertView, ViewGroup parent) {
            System.gc();
            TextView tv = new TextView(vContext.getApplicationContext());
            String id = null;
            if (convertView == null) {
                video_column_index = videocursor
                        .getColumnIndexOrThrow(MediaStore.Video.Media.DISPLAY_NAME);
                videocursor.moveToPosition(position);
                id = videocursor.getString(video_column_index);
                video_column_index = videocursor
                        .getColumnIndexOrThrow(MediaStore.Video.Media.SIZE);
                videocursor.moveToPosition(position);
                id += " Size(KB):" + videocursor.getString(video_column_index);
                tv.setText(id);
            } else
                tv = (TextView) convertView;
            return tv;
        }
    }

    private AdapterView.OnItemClickListener videogridlistener = new AdapterView.OnItemClickListener() {
        public void onItemClick(AdapterView parent, View v, int position,
                                long id) {
            System.gc();
            video_column_index = videocursor
                    .getColumnIndexOrThrow(MediaStore.Video.Media.DATA);
            videocursor.moveToPosition(position);
            String filename = videocursor.getString(video_column_index);
            /*   Intent intent = new Intent(MainActivity.this, ViewVideo.class);
                  intent.putExtra("videofilename", filename);
                  startActivity(intent);*/
            Toast.makeText(getActivity().getApplicationContext(), filename, Toast.LENGTH_SHORT).show();
        }
    };

}
