package dttv.app.widget;


import java.io.File;
import java.io.FilenameFilter;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;

import dttv.app.VideoPlayerActivity;
import dttv.app.R;
import dttv.app.adapter.FileAdapter;
import dttv.app.impl.I_Async;
import dttv.app.impl.I_KeyIntercept;
import dttv.app.impl.I_OnMyKey;
import dttv.app.model.Item;
import dttv.app.multithread.DataAsyncTask;
import dttv.app.utils.Constant;
import dttv.app.utils.MultiMediaTypeUtil;
import dttv.app.utils.PlayerUtil;

import android.annotation.SuppressLint;
import android.content.Intent;
import android.os.Bundle;
import android.os.Environment;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentManager.OnBackStackChangedListener;
import android.util.Log;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnKeyListener;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.AdapterView.OnItemClickListener;


@SuppressLint("ValidFragment")
public class FilesUIFragment extends Fragment implements I_Async, OnBackStackChangedListener, OnKeyListener, I_OnMyKey {

    private final String TAG = "FilesUIFragment";

    private View rootView;
    private TextView indexTxt;
    private ListView mListView;
    private ArrayList<String> pathDirsList;
    private FileAdapter mFileAdapter;
    private File path;
    private String chosenFile;
    private DataAsyncTask mAsyncTask;
    private List<Item> fileList;
    MultiMediaTypeUtil mediaTypeUtil;

    private static int currentAction = -1;
    private static final int SELECT_DIRECTORY = 1;
    private static final int SELECT_FILE = 2;

    private I_KeyIntercept mKeyIntercept;

    public FilesUIFragment() {
        // TODO Auto-generated constructor stub
    }

    public FilesUIFragment(I_KeyIntercept intercept) {
        // TODO Auto-generated constructor stub
        mKeyIntercept = intercept;
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        Log.i(TAG, "enter onCreateView this.getId() is:" + this.getId());
        rootView = inflater.inflate(R.layout.file_browser, container, false);
        return rootView;
    }

    @Override
    public void onActivityCreated(Bundle savedInstanceState) {
        // TODO Auto-generated method stub
        super.onActivityCreated(savedInstanceState);
        Log.i(TAG, "enter onActivityCreated");
        initViews();
        initData();
        parseDirectoryPath();
        startDataTask("begin");
        initListener();
    }

    @Override
    public void onResume() {
        // TODO Auto-generated method stub
        super.onResume();
        Log.i(TAG, "enter onResume");
    }

    private void initData() {
        pathDirsList = new ArrayList<String>();
        mAsyncTask = new DataAsyncTask(this);
        fileList = new ArrayList<Item>();
        mediaTypeUtil = new MultiMediaTypeUtil();
        mediaTypeUtil.initReflect();
        if (Environment.getExternalStorageDirectory().isDirectory()
                && Environment.getExternalStorageDirectory().canRead())
            path = Environment.getExternalStorageDirectory();
        else
            path = new File("/");
    }

    private void startDataTask(String index) {
        stopDataTask();
        if (mAsyncTask == null) {
            mAsyncTask = new DataAsyncTask(this);
        }
        mAsyncTask.execute(index);
    }

    private void stopDataTask() {
        if (mAsyncTask != null) {
            mAsyncTask.cancel(true);
            mAsyncTask = null;
        }
    }

    private void initViews() {
        mListView = (ListView) rootView.findViewById(R.id.dt_file_listview);
        indexTxt = (TextView) rootView.findViewById(R.id.dt_file_index_txt);
    }

    private void showToast(String message) {
        Toast.makeText(getActivity(), message, Toast.LENGTH_LONG).show();
    }

    private void initListener() {
        mListView.setOnItemClickListener(new OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> adapter, View v, int position,
                                    long arg3) {
                // TODO Auto-generated method stub
                chosenFile = fileList.get(position).file;
                File sel = new File(path + "/" + chosenFile);
                /*if(position==0){
					loadDirectoryUp();
					return;
				}*/
                if (sel.isDirectory()) {
                    if (sel.canRead()) {
                        // Adds chosen directory to list
                        pathDirsList.add(chosenFile);
                        path = new File(sel + "");
                        //loadFileList();
                        startDataTask("child");
                        Log.d(TAG, path.getAbsolutePath());
                    } else {// if(sel.canRead()) {
                        showToast("Path does not exist or cannot be read");
                    }
                } else {
                    startAudioPlayer(sel.getAbsolutePath());
                }
            }
        });
    }

    private void startAudioPlayer(String uri) {
		/*Intent retIntent = new Intent();
		retIntent.setClass(getActivity(), VideoPlayerActivity.class);
		retIntent.putExtra(Constant.FILE_MSG, uri);
		startActivity(retIntent);*/

        PlayerUtil.getInstance().beginToPlayer(getActivity(), uri, path.getName(), Constant.LOCAL_FILE);
    }

    private void parseDirectoryPath() {
        pathDirsList.clear();
        String pathString = path.getAbsolutePath();
        String[] parts = pathString.split("/");
        int i = 0;
        while (i < parts.length) {
            pathDirsList.add(parts[i]);
            i++;
        }
    }

    private void updateCurrentDirTextView() {
        int i = 0;
        String curDirString = "";
        while (i < pathDirsList.size()) {
            curDirString += pathDirsList.get(i) + "/";
            i++;
        }
        if (pathDirsList.size() == 0) {
			/*((Button) this.findViewById(R.id.upDirectoryButton))
					.setEnabled(false);*/
            curDirString = "/";
        }
        indexTxt.setText(curDirString);
    }

    /**
     * for key back
     */
    private void loadDirectoryUp() {
        // present directory removed from list
        String s = pathDirsList.remove(pathDirsList.size() - 1);
        // path modified to exclude present directory
        path = new File(path.toString().substring(0,
                path.toString().lastIndexOf(s)));
        fileList.clear();
        startDataTask("back");
    }

    private void loadFileList() {
        try {
            path.mkdirs();
        } catch (SecurityException e) {
            Log.e(TAG, "unable to write on the sd card ");
        }
        fileList.clear();
        if (path.exists() && path.canRead()) {
            FilenameFilter filter = new FilenameFilter() {
                @Override
                public boolean accept(File dir, String filename) {
                    // TODO Auto-generated method stub
                    File sel = new File(dir, filename);
                    boolean showReadableFile = sel.canRead();
                    if (currentAction == SELECT_DIRECTORY) {
                        return (sel.isDirectory() && showReadableFile);
                    }
                    return true;
                }
            };
            String[] fList = path.list(filter);
            //fileList.add(new Item(path, ".."));
            for (int i = 0; i < fList.length; i++) {
                Item item = new Item(path, fList[i]);
                if (item.isDirectory()) {
                    item.setIcon(R.drawable.dt_browser_folder);
                } else {
                    item.setIcon(getDrawableId(path + "/" + item.file));
                }
                fileList.add(item);
            }
            if (fileList.size() == 0) {
            } else {
                Collections.sort(fileList, new ItemFileNameComparator());
            }
        }
    }

    private int getDrawableId(String path) {
        Log.i(TAG, "path is:" + path);
        int res;
        //res = mediaTypeUtil.isAudioFile(mediaTypeUtil.getMediaFileType(path)) == true ? R.drawable.dt_player_audio_icon : -1;
        if (mediaTypeUtil.isAudioFile(mediaTypeUtil.getMediaFileType(path))) {
            res = R.drawable.dt_player_audio_icon;
        } else if (mediaTypeUtil.isVideoFile(mediaTypeUtil.getMediaFileType(path))) {
            res = R.drawable.dt_player_video_icon;
        } else if (mediaTypeUtil.isImageFile(mediaTypeUtil.getMediaFileType(path))) {
            res = R.drawable.dt_browser_file;
        } else {
            res = R.drawable.dt_browser_file;
        }
        return res;
    }

    public class ItemFileNameComparator implements Comparator<Item> {
        public int compare(Item lhs, Item rhs) {
            return lhs.file.toLowerCase().compareTo(rhs.file.toLowerCase());
        }
    }

    @Override
    public void onPause() {
        // TODO Auto-generated method stub
        super.onPause();
        Log.i(TAG, "enter onPause");
    }

    @Override
    public void onStop() {
        // TODO Auto-generated method stub
        super.onStop();
        Log.i(TAG, "enter onStop");
    }

    @Override
    public void onDestroyView() {
        // TODO Auto-generated method stub
        super.onDestroyView();
        Log.i(TAG, "enter onDestroyView");
    }

    @Override
    public void onDestroy() {
        // TODO Auto-generated method stub
        mFileAdapter = null;
        pathDirsList.clear();
        pathDirsList = null;
        fileList.clear();
        fileList = null;
        stopDataTask();
        super.onDestroy();
    }


    @Override
    public void onPreHandleData() {
        // TODO Auto-generated method stub

    }

    @Override
    public void onHandleData() {
        // TODO Auto-generated method stub
        loadFileList();
    }

    @Override
    public void onPostHandleData() {
        // TODO Auto-generated method stub
        if (mFileAdapter == null) {
            mFileAdapter = new FileAdapter(getActivity(), fileList);
            mListView.setAdapter(mFileAdapter);
        } else {
            mFileAdapter.freshData(fileList);
            mFileAdapter.notifyDataSetChanged();
        }
        updateCurrentDirTextView();
    }

    @Override
    public void onBackStackChanged() {
        // TODO Auto-generated method stub

    }

    @Override
    public boolean onKey(View v, int keyCode, KeyEvent event) {
        // TODO Auto-generated method stub
        if (event.getAction() == KeyEvent.ACTION_DOWN) {
            if (keyCode == KeyEvent.KEYCODE_BACK) {
                mKeyIntercept.isNeedIntercept(true);
                loadDirectoryUp();
                return true;
            }
        }
        return false;
    }

    @Override
    public void myOnKeyDown(int keyCode) {
        // TODO Auto-generated method stub
        if (keyCode == KeyEvent.KEYCODE_BACK) {
            if (path.getAbsolutePath().toString().equals("/")) {
                mKeyIntercept.isNeedIntercept(false);
            } else {
                loadDirectoryUp();
            }
        }
    }
}
