package dttv.app;

import java.io.File;
import java.io.FilenameFilter;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import android.view.View;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.Button;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;

import dttv.app.adapter.FileAdapter;
import dttv.app.impl.I_Async;
import dttv.app.model.Item;
import dttv.app.multithread.DataAsyncTask;
import dttv.app.utils.Constant;

public class FileShowActivity extends Activity implements I_Async {

    private final String TAG = "FileShowActivity";
    private TextView indexTxt;
    private ListView mListView;
    private ArrayList<String> pathDirsList;
    private FileAdapter mFileAdapter;
    private File path;
    private String chosenFile;
    private DataAsyncTask mAsyncTask;
    private List<Item> fileList;

    private static int currentAction = -1;
    private static final int SELECT_DIRECTORY = 1;
    private static final int SELECT_FILE = 2;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        // TODO Auto-generated method stub
        super.onCreate(savedInstanceState);
        setContentView(R.layout.file_browser);
        initViews();
        initData();
        parseDirectoryPath();
        startDataTask("begin");
        initListener();
    }

    private void initData() {
        pathDirsList = new ArrayList<String>();
        mAsyncTask = new DataAsyncTask(this);
        fileList = new ArrayList<Item>();
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
        //mListView = (ListView) findViewById(R.id.dt_file_listview);
        //indexTxt = (TextView) findViewById(R.id.dt_file_index_txt);
    }

    private void showToast(String message) {
        Toast.makeText(this, message, Toast.LENGTH_LONG).show();
    }

    private void initListener() {
        mListView.setOnItemClickListener(new OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> adapter, View v, int position,
                                    long arg3) {
                // TODO Auto-generated method stub
                chosenFile = fileList.get(position).file;
                File sel = new File(path + "/" + chosenFile);
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

    @Override
    public void onBackPressed() {
        // TODO Auto-generated method stub
        if (indexTxt.getText().toString().equals("/")) {
            super.onBackPressed();
        } else {
            loadDirectoryUp();
        }
    }

    private void startAudioPlayer(String uri) {
        Intent retIntent = new Intent();
        retIntent.setClass(this, VideoPlayerActivity.class);
        retIntent.putExtra(Constant.FILE_MSG, uri);
        startActivity(retIntent);
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
            ((Button) this.findViewById(R.id.upDirectoryButton))
                    .setEnabled(false);
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
            for (int i = 0; i < fList.length; i++) {
                Item item = new Item(path, fList[i]);
                if (item.isDirectory()) {
                    item.setIcon(R.drawable.dt_browser_folder);
                } else {
                    item.setIcon(R.drawable.dt_browser_file);
                }
                fileList.add(item);
            }
            if (fileList.size() == 0) {
            } else {
                Collections.sort(fileList, new ItemFileNameComparator());
            }
        }
    }

    public class ItemFileNameComparator implements Comparator<Item> {
        public int compare(Item lhs, Item rhs) {
            return lhs.file.toLowerCase().compareTo(rhs.file.toLowerCase());
        }
    }

    @Override
    protected void onResume() {
        // TODO Auto-generated method stub
        super.onResume();
    }


    @Override
    protected void onStop() {
        // TODO Auto-generated method stub
        super.onStop();
    }

    @Override
    protected void onDestroy() {
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
            mFileAdapter = new FileAdapter(this, fileList);
            mListView.setAdapter(mFileAdapter);
        } else {
            mFileAdapter.freshData(fileList);
            mFileAdapter.notifyDataSetChanged();
        }
        updateCurrentDirTextView();
    }
}
