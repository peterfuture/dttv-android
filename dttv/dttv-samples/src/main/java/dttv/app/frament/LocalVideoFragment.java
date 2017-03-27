package dttv.app.frament;


import android.content.Context;
import android.database.Cursor;
import android.provider.MediaStore;
import android.support.v4.app.Fragment;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;

import butterknife.BindView;
import dttv.app.R;
import dttv.app.base.SimpleFragment;
import dttv.app.utils.DateUtils;
import dttv.app.utils.FileNewUtils;
import dttv.app.utils.Log;

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
        initVideos();
    }

    public LocalVideoFragment() {
        // Required empty public constructor
    }

    private void initVideos() {
        //System.gc();
        String[] proj = { MediaStore.Video.Media._ID,
                MediaStore.Video.Media.DATA,
                MediaStore.Video.Media.DISPLAY_NAME,
                MediaStore.Video.Media.DATE_MODIFIED,
                MediaStore.Video.Media.SIZE,
                MediaStore.Video.Media.DEFAULT_SORT_ORDER
        };
        videocursor = getActivity().getContentResolver().query(MediaStore.Video.Media.EXTERNAL_CONTENT_URI,
                proj, null, null, null);
        count = videocursor.getCount();
        Log.i(TAG,"count is:"+count);
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

            ViewHolder viewHolder = null;

            if (convertView == null) {
                viewHolder = new ViewHolder();
                convertView = LayoutInflater.from(vContext).inflate(R.layout.dt_video_item,null);
                viewHolder.nameTxt = (TextView) convertView.findViewById(R.id.media_row_name);
                viewHolder.dateTxt = (TextView) convertView.findViewById(R.id.media_row_date);
                viewHolder.imageView = (ImageView)convertView.findViewById(R.id.media_row_icon);
                convertView.setTag(viewHolder);
            } else {
                viewHolder = (ViewHolder) convertView.getTag();
            }

            video_column_index = videocursor
                    .getColumnIndexOrThrow(MediaStore.Video.Media.DISPLAY_NAME);
            videocursor.moveToPosition(position);
            String name = videocursor.getString(video_column_index);
            viewHolder.nameTxt.setText(name);

            video_column_index = videocursor.getColumnIndexOrThrow(MediaStore.Video.Media.DATE_MODIFIED);
            String date = videocursor.getString(video_column_index);
            video_column_index = videocursor.getColumnIndexOrThrow(MediaStore.Video.Media.SIZE);
            String size = videocursor.getString(video_column_index);
            try {
                viewHolder.dateTxt.setText(DateUtils.getInstanse().getmstodate(Long.parseLong(date+"000"), DateUtils.YYYYMMDDHHMMSS
                ) + "  " + FileNewUtils.getInstance().FormetFileSize(Long.parseLong(size)));
                videocursor.moveToPosition(position);
            }catch (Exception e) {
                e.printStackTrace();
            }

            return convertView;
        }

        class ViewHolder{
            ImageView imageView;
            TextView nameTxt;
            TextView dateTxt;
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

    @Override
    public void onDestroy() {
        super.onDestroy();
        videocursor.close();
    }
}
