package dttv.app.multithread;

import dttv.app.impl.I_Async;
import android.os.AsyncTask;

/**
 * multi-thread for lot of data
 * @author shihx1
 *
 */
public class DataAsyncTask extends AsyncTask<String, Integer, Integer> {
	
	private I_Async mAsync;
	
	public DataAsyncTask(I_Async async) {
		// TODO Auto-generated constructor stub
		this.mAsync = async;
	}
	
	@Override
	protected void onPreExecute() {
		// TODO Auto-generated method stub
		super.onPreExecute();
		mAsync.onPreHandleData();
	}
	
	@Override
	protected Integer doInBackground(String... arg0) {
		// TODO Auto-generated method stub
		mAsync.onHandleData();
		return null;
	}
	
	@Override
	protected void onCancelled() {
		// TODO Auto-generated method stub
		super.onCancelled();
	}
	
	@Override
	protected void onPostExecute(Integer result) {
		// TODO Auto-generated method stub
		super.onPostExecute(result);
		mAsync.onPostHandleData();
	}
}
