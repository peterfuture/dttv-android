package dttv.app.adapter;

import java.sql.Date;
import java.text.SimpleDateFormat;
import java.util.List;

import dttv.app.R;
import dttv.app.model.Item;
import android.annotation.SuppressLint;
import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.TextView;

@SuppressLint("SimpleDateFormat")
public class FileAdapter extends BaseAdapter {
	
	private LayoutInflater inflater;
	private List<Item> mList;
	private Context mContext;
	public FileAdapter(Context context,List<Item> list) {
		// TODO Auto-generated constructor stub
		this.mList = list;
		this.mContext = context;
		inflater = LayoutInflater.from(context);
	}
	
	public void freshData(List<Item> list){
		this.mList = list;
	}

	@Override
	public int getCount() {
		// TODO Auto-generated method stub
		return mList.size();
	}

	@Override
	public Object getItem(int arg0) {
		// TODO Auto-generated method stub
		return mList.get(arg0);
	}

	@Override
	public long getItemId(int arg0) {
		// TODO Auto-generated method stub
		return arg0;
	}
	ViewHolder viewHolder;
	@Override
	public View getView(int position, View contentView, ViewGroup parent) {
		// TODO Auto-generated method stub
		Item file = mList.get(position);
		if(contentView==null){
			viewHolder = new ViewHolder();
			contentView = inflater.inflate(R.layout.file_browser_item, null);
			viewHolder.iconImg = (ImageView)contentView.findViewById(R.id.dt_file_item_icon);
			viewHolder.nameTxt = (TextView)contentView.findViewById(R.id.dt_file_item_name);
			viewHolder.timeTxt = (TextView)contentView.findViewById(R.id.dt_file_item_time);
			contentView.setTag(viewHolder);
		}else{
			viewHolder = (ViewHolder)contentView.getTag();
		}
		/*if(file.isDirectory()){
			viewHolder.iconImg.setBackgroundResource(R.drawable.dt_browser_folder);
		}else{
			viewHolder.iconImg.setBackgroundResource(R.drawable.dt_browser_file);
		}*/
		viewHolder.iconImg.setBackgroundResource(file.getIcon());
		viewHolder.nameTxt.setText(file.getName());
		viewHolder.timeTxt.setText(longToDate(file.lastModified()));
		return contentView;
	}
	
	private String longToDate(long data){
		Date date = new Date(data);
		SimpleDateFormat dateFormat = new SimpleDateFormat("yyyy-MM-dd HH:MM");
		return dateFormat.format(date);
	}
	
	class ViewHolder{
		ImageView iconImg;
		TextView nameTxt;
		TextView timeTxt;
	}
}
