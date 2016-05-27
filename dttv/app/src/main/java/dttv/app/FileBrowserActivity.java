package dttv.app;

import android.content.ActivityNotFoundException;
import android.content.IntentFilter;
import android.os.storage.*;

import java.io.BufferedReader;
import java.io.File;
import java.io.FilenameFilter;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.Date;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import android.os.Handler;

import dttv.app.model.Item;
import dttv.app.utils.Constant;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.res.Configuration;
import android.graphics.Color;
import android.net.Uri;
import android.os.Bundle;
import android.os.Environment;
import android.os.Message;
import android.os.PowerManager;
import android.os.StatFs;
import android.os.storage.StorageManager;
import android.util.Log;
import android.view.KeyEvent;
import android.view.View;
import android.view.View.OnClickListener;

import android.view.ViewGroup;
import android.view.ViewGroup.LayoutParams;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.ListAdapter;
import android.widget.ListView;
import android.widget.ProgressBar;
import android.widget.SimpleAdapter;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ToggleButton;
//Android imports 
//Import of resources file for file browser
import dttv.app.FileBrowserDatabase.ThumbnailCursor;
import dttv.app.FileOp.FileOpReturn;
import dttv.app.FileOp.FileOpTodo;
import dttv.app.utils.FileUtils;

public class FileBrowserActivity extends Activity {

    public static final String TAG = "FileBrowser";

    private static final String ROOT = "/storage";
    private static final String SHEILD_EXT_STOR = Environment.getExternalStorageDirectory().getPath();
    private static final String NAND_PATH = Environment.getExternalStorageDirectory().getPath();
    /*
    private static final String SD_PATH = Environment.getExternalStorageDirectory().getPath();
    private static String USB_PATH = Environment.getExternalStorageDirectory().getPath();
    private static final String SATA_PATH = Environment.getExternalStorageDirectory().getPath();
    private static final String NFS_PATH = "/mnt/nfs";*/

    private static final String SD_PATH = "/storage/external_storage/sdcard1";
    private static final String USB_PATH = "/storage/external_storage";
    private static final String SATA_PATH = "/storage/external_storage/sata";
    private static final String NFS_PATH = "/mnt/nfs";

    StorageManager mStorageManager = null;
    public static FileBrowserDatabase mDataBase;
    public static FileBrowserDatabase.FileMarkCursor mCursor;
    private ListView mListView;
    private int mItemSelected, mItemFirst, mItemLast;
    private int mItemTop;
    public static Handler mProgressHandler;
    private List<Map<String, Object>> mList;
    private boolean mListLoaded = false;
    private boolean mLoadCancel = false;
    private boolean mIsSorted = false;

    private static final int FILEBROWSER_SORT_BY_NONE = 0;
    private static final int FILEBROWSER_SORT_BY_DATE = 1;
    private static final int FILEBROWSER_SORT_BY_SIZE = 2;
    private static final int FILEBROWSER_SORT_BY_NAME = 3;
    private int mSortType = FILEBROWSER_SORT_BY_NONE;
    private List<String> mDevList = new ArrayList<String>();


    private static final String FILEINFO_KEY_FULL_NAMNE = "fileinfo_item_full_name";
    private static final String FILEINFO_KEY_NAMNE = "fileinfo_item_name";
    private static final String FILEINFO_KEY_PATH = "fileinfo_item_path";
    private static final String FILEINFO_KEY_SELECTED = "fileinfo_item_selected";
    private static final String FILEINFO_KEY_TYPE = "fileinfo_item_type";
    private static final String FILEINFO_KEY_SUFFIX = "fileinfo_item_suffix";
    private static final String FILEINFO_KEY_ACCESS = "fileinfo_item_access";
    private static final String FILEINFO_KEY_DATE_DISPLAY = "fileinfo_item_date_display";
    private static final String FILEINFO_KEY_DATE_SORT = "fileinfo_item_date_sort";
    private static final String FILEINFO_KEY_SIZE_DISPLAY = "fileinfo_item_size_display";
    private static final String FILEINFO_KEY_SIZE_SORT = "fileinfo_item_size_sort";

    public static String mCurrentPath = ROOT;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.file_browser);
        mStorageManager = (StorageManager) getSystemService(Context.STORAGE_SERVICE);

        /* setup database */
        mDataBase = new FileBrowserDatabase(this);

        /* setup file list */
        mListView = (ListView) findViewById(R.id.filebrowser_listview);
        mList = new ArrayList<Map<String, Object>>();

        if (mCurrentPath != null) {
            File file = new File(mCurrentPath);
            if (!file.exists())
                mCurrentPath = ROOT;
        } else {
            mCurrentPath = ROOT;
        }
        mList = new ArrayList<Map<String, Object>>();

        /* ListView OnItemClickListener */
        mListView.setOnItemClickListener(new OnItemClickListener() {
            public void onItemClick(AdapterView<?> parent, View view, int pos, long id) {
                Map<String, Object> item = (Map<String, Object>) parent.getItemAtPosition(pos);
                String mPath = (String) item.get(FILEINFO_KEY_FULL_NAMNE);
                if (mPath.equals(NFS_PATH)) {
                    return;
                }

                File mFile = new File(mPath);
                if (!mFile.exists()) {
                    return;
                }
                if (Intent.ACTION_GET_CONTENT.equalsIgnoreCase(FileBrowserActivity.this.getIntent().getAction())) {
                    if (mFile.isDirectory()) {
                        mCurrentPath = mPath;
                        mListView.setAdapter(getFileListAdapterSorted(mCurrentPath, mSortType));
                    } else {
                        FileBrowserActivity.this.setResult(Activity.RESULT_OK, new Intent(null, Uri.fromFile(mFile)));
                        FileBrowserActivity.this.finish();
                    }
                } else {
                    if (mFile.isDirectory()) {
                        mCurrentPath = mPath;
                        mListView.setAdapter(getFileListAdapterSorted(mCurrentPath, mSortType));
                    } else {
                        openFile(mFile);
                        //showDialog(CLICK_DIALOG_ID);
                    }

                    mItemSelected = mListView.getSelectedItemPosition();
                    mItemFirst = mListView.getFirstVisiblePosition();
                    mItemLast = mListView.getLastVisiblePosition();
                    View mView = mListView.getChildAt(mItemSelected - mItemFirst);
                    if (mView != null) {
                        mItemTop = mView.getTop();
                    }
                }
            }
        });

        Button btn_home = (Button) findViewById(R.id.btn_home);
        btn_home.setOnClickListener(new OnClickListener() {
            public void onClick(View v) {
                if (mCurrentPath.equals(ROOT))
                    return;
                mCurrentPath = ROOT;
                DeviceScan();
            }
        });

        Button btn_back = (Button) findViewById(R.id.btn_back);
        btn_back.setOnClickListener(new OnClickListener() {
            public void onClick(View v) {
                if (!mCurrentPath.equals(ROOT)) {
                    File file = new File(mCurrentPath);
                    String parent_path = file.getParent();
                    if(mCurrentPath.equals(NAND_PATH) || mCurrentPath.equals(SD_PATH)
                            || mCurrentPath.equals(NFS_PATH) ||parent_path.equals(USB_PATH)) {
                        mCurrentPath = ROOT;
                        DeviceScan();
                    } else {
                        mCurrentPath = parent_path;
                        mListView.setAdapter(getFileListAdapterSorted(parent_path, mSortType));
                    }
                }
            }
        });

        Button btn_style = (Button) findViewById(R.id.btn_style);
        btn_style.setOnClickListener(new OnClickListener() {
            public void onClick(View v) {
                Log.i(TAG, "Switch to Thumb Mode");
            }
        });

        Button btn_search = (Button) findViewById(R.id.btn_search);
        boolean isEnableSearch = false;
        if(!isEnableSearch)
            btn_search.setVisibility(View.GONE);
        btn_search.setOnClickListener(new OnClickListener() {
            public void onClick(View v) {
                Log.i(TAG, "Search Mode");
            }
        });

        Button btn_about = (Button) findViewById(R.id.btn_about);
        btn_about.setOnClickListener(new OnClickListener() {
            public void onClick(View v) {
                Log.i(TAG, "About");
            }
        });

    }

    /**
     * Called when the activity is first created or resumed.
     */
    @Override
    public void onResume() {
        super.onResume();

        mProgressHandler = new Handler() {
            @Override
            public void handleMessage(Message msg) {
                super.handleMessage(msg);

                switch (msg.what) {
                    case 9:        //file copy cancel
                        if ((FileOp.copying_file != null) && (FileOp.copying_file.exists())) {
                            try {
                                if (FileOp.copying_file.isDirectory())
                                    FileUtils.deleteDirectory(FileOp.copying_file);
                                else
                                    FileOp.copying_file.delete();
                            } catch (Exception e) {
                                Log.e("Exception when delete", e.toString());
                            }
                        }

                        FileOp.copy_cancel = false;
                        FileOp.copying_file = null;
                        mDataBase.deleteAllFileMark();
                        mListView.setAdapter(getFileListAdapterSorted(mCurrentPath, mSortType));
                        FileOp.file_op_todo = FileOpTodo.TODO_NOTHING;
                        break;
                    case 10:    //update list
                        //((BaseAdapter) lv.getAdapter()).notifyDataSetChanged();
                        if (mListLoaded == false) {
                            break;
                        }
                        mListView.setAdapter(getFileListAdapterSorted(mCurrentPath, mSortType));
                        mListLoaded = false;
                        break;
                    default:
                        break;

                }
            }
        };

        if (mListLoaded == true) {
            mListLoaded = false;
        }

        if (mCurrentPath.equals(ROOT)) {
            DeviceScan();
        } else {
            mListView.setAdapter(getFileListAdapterSorted(mCurrentPath, mSortType));
        }
        mListView.setSelectionFromTop(mItemSelected, mItemTop);
    }

    @Override
    public void onPause() {
        super.onPause();

        mLoadCancel = true;
        //update sharedPref
        SharedPreferences settings = getSharedPreferences("settings", Activity.MODE_PRIVATE);
        SharedPreferences.Editor editor = settings.edit();
        editor.putString("cur_path", mCurrentPath);
        editor.commit();

        FileOp.copy_cancel = true;
        if (mListLoaded)
            mListLoaded = false;
        mDataBase.deleteAllFileMark();
        mDataBase.close();
    }

    /**
     * onDestory()
     */
    @Override
    public void onDestroy() {
        super.onDestroy();
        mDataBase.deleteAllFileMark();
        mDataBase.close();
    }

    protected void openFile(File f) {
        Intent intent = new Intent();
        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        intent.setAction(android.content.Intent.ACTION_VIEW);
        String type = "*/*";
        type = FileOp.CheckMediaType(f);
        intent.setDataAndType(Uri.fromFile(f), type);
        try {
            startActivity(intent);
        } catch (ActivityNotFoundException e) {
        }
    }

    private void DeviceScan() {
        mDevList.clear();
        String internal = getString(R.string.memory_device_str);
        String sdcard = getString(R.string.sdcard_device_str);
        String usb = getString(R.string.usb_device_str);
        String cdrom = getString(R.string.cdrom_device_str);
        String sdcardExt = getString(R.string.ext_sdcard_device_str);
        String nfs = getString(R.string.nfs_device_str);
        String DeviceArray[] = {internal, sdcard, usb, cdrom, sdcardExt, nfs};

        int length = 0;
        length = DeviceArray.length;

        for (int i = 0; i < length; i++) {
            if (FileOp.deviceExist(DeviceArray[i])) {
                mDevList.add(DeviceArray[i]);
            }
        }
        mListView.setAdapter(getDeviceListAdapter());
    }

    private ListAdapter getDeviceListAdapter() {
        return new SimpleAdapter(this, getDeviceListData(), R.layout.filebrowser_device_item,
                new String[]{
                        FILEINFO_KEY_TYPE,
                        FILEINFO_KEY_NAMNE,
                        FILEINFO_KEY_ACCESS,
                        FILEINFO_KEY_SIZE_DISPLAY
                },
                new int[]{
                        R.id.filebrowser_device_type,
                        R.id.filebrowser_device_name,
                        R.id.filebrowser_device_access,
                        R.id.filebrowser_device_size
                });
    }

    private List<Map<String, Object>> getDeviceListData() {
        List<Map<String, Object>> list = new ArrayList<Map<String, Object>>();
        Map<String, Object> map;
        File dir = new File(NAND_PATH);
        if (dir.exists() && dir.isDirectory()) {
            map = new HashMap<String, Object>();
            map.put(FILEINFO_KEY_NAMNE, getText(R.string.sdcard_device_str));
            map.put(FILEINFO_KEY_FULL_NAMNE, NAND_PATH);
            map.put(FILEINFO_KEY_TYPE, R.drawable.filebrowser_icon_sdcard);
            map.put(FILEINFO_KEY_DATE_DISPLAY, 0);
            map.put(FILEINFO_KEY_SIZE_SORT, 1);    //for sort
            map.put(FILEINFO_KEY_SIZE_DISPLAY, null);
            map.put(FILEINFO_KEY_ACCESS, null);
            map.put(FILEINFO_KEY_SUFFIX, null);
            list.add(map);
        }

        dir = new File(SD_PATH);
        if (dir.exists() && dir.isDirectory()) {
            map = new HashMap<String, Object>();
            String label = null;
            map.put(FILEINFO_KEY_NAMNE, (label == null) ? getText(R.string.ext_sdcard_device_str) : label);
            map.put(FILEINFO_KEY_FULL_NAMNE, SD_PATH);
            map.put(FILEINFO_KEY_TYPE, R.drawable.filebrowser_icon_sdcard);
            map.put(FILEINFO_KEY_DATE_DISPLAY, 0);
            map.put(FILEINFO_KEY_SIZE_SORT, 1);    //for sort
            map.put(FILEINFO_KEY_SIZE_DISPLAY, null);
            map.put(FILEINFO_KEY_ACCESS, null);
            map.put(FILEINFO_KEY_SUFFIX, null);
            list.add(map);
        }

        dir = new File(USB_PATH);
        if (dir.exists() && dir.isDirectory()) {
            if (dir.listFiles() != null) {
                int dev_count = 0;
                for (File file : dir.listFiles()) {
                    if (file.isDirectory()) {
                        String devname = null;
                        String path = file.getAbsolutePath();
                        if (path.startsWith(USB_PATH + "/sd") && !path.equals(SD_PATH)) {
                            map = new HashMap<String, Object>();
                            dev_count++;
                            char data = (char) ('A' + dev_count - 1);
                            String label = null;

                            devname = getText(R.string.usb_device_str) + "(" + data + ":)";
                            map.put(FILEINFO_KEY_NAMNE, (label == null) ? devname : label);
                            map.put(FILEINFO_KEY_FULL_NAMNE, path);
                            map.put(FILEINFO_KEY_TYPE, R.drawable.filebrowser_icon_usb);
                            map.put(FILEINFO_KEY_DATE_DISPLAY, 0);
                            map.put(FILEINFO_KEY_SIZE_SORT, 3);    //for sort
                            map.put(FILEINFO_KEY_SIZE_DISPLAY, null);
                            map.put(FILEINFO_KEY_ACCESS, null);
                            map.put(FILEINFO_KEY_SUFFIX, null);
                            list.add(map);
                        }
                    }
                }
            }
        }

        dir = new File(USB_PATH);
        if (dir.exists() && dir.isDirectory()) {
            if (dir.listFiles() != null) {
                int dev_count = 0;
                for (File file : dir.listFiles()) {
                    if (file.isDirectory()) {
                        String devname = null;
                        String path = file.getAbsolutePath();
                        if (path.startsWith(USB_PATH + "/sr") && !path.equals(SD_PATH)) {
                            map = new HashMap<String, Object>();
                            dev_count++;
                            char data = (char) ('A' + dev_count - 1);
                            devname = getText(R.string.cdrom_device_str) + "(" + data + ":)";
                            map.put(FILEINFO_KEY_NAMNE, devname);
                            map.put(FILEINFO_KEY_FULL_NAMNE, path);
                            map.put(FILEINFO_KEY_TYPE, R.drawable.filebrowser_icon_cdrom);
                            map.put(FILEINFO_KEY_DATE_DISPLAY, 0);
                            map.put(FILEINFO_KEY_SIZE_SORT, 3);    //for sort
                            map.put(FILEINFO_KEY_SIZE_DISPLAY, null);
                            map.put(FILEINFO_KEY_ACCESS, null);
                            map.put(FILEINFO_KEY_SUFFIX, null);
                            list.add(map);
                        }
                    }
                }
            }
        }

        dir = new File(SATA_PATH);
        if (dir.exists() && dir.isDirectory()) {
            map = new HashMap<String, Object>();
            map.put(FILEINFO_KEY_NAMNE, getText(R.string.sata_device_str));
            map.put(FILEINFO_KEY_FULL_NAMNE, SATA_PATH);
            map.put(FILEINFO_KEY_TYPE, R.drawable.filebrowser_icon_sata);
            map.put(FILEINFO_KEY_DATE_DISPLAY, 0);
            map.put(FILEINFO_KEY_SIZE_SORT, 1);    //for sort
            map.put(FILEINFO_KEY_SIZE_DISPLAY, null);
            map.put(FILEINFO_KEY_ACCESS, null);
            map.put(FILEINFO_KEY_SUFFIX, null);
            list.add(map);
        }

        dir = new File(NFS_PATH);
        if (dir.exists() && dir.isDirectory()) {
            map = new HashMap<String, Object>();
            map.put(FILEINFO_KEY_NAMNE, getText(R.string.nfs_device_str));
            map.put(FILEINFO_KEY_FULL_NAMNE, NFS_PATH);
            map.put(FILEINFO_KEY_TYPE, R.drawable.filebrowser_icon_nfs);
            map.put(FILEINFO_KEY_DATE_DISPLAY, 0);
            map.put(FILEINFO_KEY_SIZE_SORT, 1);
            map.put(FILEINFO_KEY_SIZE_DISPLAY, null);
            map.put(FILEINFO_KEY_ACCESS, null);
            map.put(FILEINFO_KEY_SUFFIX, null);
            list.add(map);
        }

        updatePathShow(ROOT);
        if (!list.isEmpty()) {
            Collections.sort(list, new Comparator<Map<String, Object>>() {
                public int compare(Map<String, Object> object1, Map<String, Object> object2) {
                    return ((Integer) object1.get(FILEINFO_KEY_SIZE_SORT)).compareTo(
                            (Integer) object2.get(FILEINFO_KEY_SIZE_SORT));
                }
            });
        }
        return list;
    }

    /**
     * updatePathShow
     */
    private void updatePathShow(String path) {
        TextView tv = (TextView) findViewById(R.id.filebrowser_current_path);
        if (path.equals(ROOT))
            tv.setText(getText(R.string.device_list));
        else
            tv.setText(path);
    }

    /**
     * getFileListAdapterSorted
     */
    private SimpleAdapter getFileListAdapterSorted(String path, int sort_type) {

        if (path.equals(ROOT)) {
            return new SimpleAdapter(FileBrowserActivity.this, getDeviceListData(),
                    R.layout.filebrowser_device_item,
                    new String[]{
                            FILEINFO_KEY_TYPE,
                            FILEINFO_KEY_NAMNE,
                            FILEINFO_KEY_ACCESS,
                            FILEINFO_KEY_SIZE_DISPLAY
                    },
                    new int[]{
                            R.id.filebrowser_device_type,
                            R.id.filebrowser_device_name,
                            R.id.filebrowser_device_access,
                            R.id.filebrowser_device_size
                    });
        } else {
            return new SimpleAdapter(FileBrowserActivity.this, getFileListDataSorted(path, sort_type),
                    R.layout.filebrowser_filelist_item,
                    new String[]{
                            FILEINFO_KEY_TYPE,
                            FILEINFO_KEY_NAMNE,
                            FILEINFO_KEY_SELECTED,
                            FILEINFO_KEY_SIZE_DISPLAY,
                            FILEINFO_KEY_DATE_DISPLAY,
                            FILEINFO_KEY_ACCESS,
                            FILEINFO_KEY_SUFFIX,
                            FILEINFO_KEY_FULL_NAMNE
                    },
                    new int[]{
                            R.id.filebrowser_file_type,
                            R.id.filebrowser_file_name,
                            R.id.filebrowser_file_selected,
                            R.id.filebrowser_file_size,
                            R.id.filebrowser_file_date,
                            R.id.filebrowser_file_access,
                            R.id.filebrowser_file_suffix,
                            R.id.filebrowser_file_fullname
                    });
        }
    }

    private List<Map<String, Object>> getFileListDataSorted(String path, int sort_type) {
        updatePathShow(path);
        if (!mListLoaded) {
            mListLoaded = true;
            //showDialog(LOAD_DIALOG_ID);

            final String ppath = path;
            final int ssort_type = sort_type;
            new Thread("getFileListDataSortedAsync") {
                @Override
                public void run() {
                    mList = getFileListDataSortedAsync(ppath, ssort_type);
                    if (null != mProgressHandler) {
                        mProgressHandler.sendMessage(Message.obtain(mProgressHandler, 10));
                    }
                }
            }.start();
            return new ArrayList<Map<String, Object>>();
        } else {
            return mList;
        }
    }

    private List<Map<String, Object>> getFileListDataSortedAsync(String path, int sort_type) {
        List<Map<String, Object>> list = new ArrayList<Map<String, Object>>();
        try {
            File file_path = new File(path);
            if (file_path != null && file_path.exists()) {
                if (file_path.listFiles() != null) {
                    if (file_path.listFiles().length > 0) {
                        for (File file : file_path.listFiles()) {
                            if (mLoadCancel) return list;
                            Map<String, Object> map = new HashMap<String, Object>();
                            String file_abs_path = file.getAbsolutePath();

                            //shield external_sdcard and usbdrive under /storage/sdcard0/
                            if ((file_abs_path.equals(SD_PATH)) || (file_abs_path.equals(USB_PATH)) || (file_abs_path.equals(SHEILD_EXT_STOR)))
                                continue;

                            map.put(FILEINFO_KEY_NAMNE, file.getName());
                            map.put(FILEINFO_KEY_FULL_NAMNE, file_abs_path);
                            map.put(FILEINFO_KEY_PATH, file_abs_path);

                            if (file.isDirectory()) {
                                if (FileOp.isFileSelected(file_abs_path, "list"))
                                    map.put(FILEINFO_KEY_SELECTED, R.drawable.filebrowser_file_selected);
                                else
                                    map.put(FILEINFO_KEY_SELECTED, R.drawable.filebrowser_file_unselected);
                                map.put(FILEINFO_KEY_TYPE, R.drawable.filebrowser_file_type_dir);
                                String rw = "d";
                                if (file.canRead())
                                    rw += "r";
                                else
                                    rw += "-";
                                if (file.canWrite())
                                    rw += "w";
                                else
                                    rw += "-";
                                map.put(FILEINFO_KEY_ACCESS, rw);

                                long file_date = file.lastModified();
                                String date = new SimpleDateFormat("yyyy/MM/dd HH:mm")
                                        .format(new Date(file_date));
                                map.put(FILEINFO_KEY_DATE_DISPLAY, " | " + date + " | ");
                                map.put(FILEINFO_KEY_DATE_SORT, file_date);    //use for sorting

                                long file_size = file.length();
                                map.put(FILEINFO_KEY_SIZE_SORT, file_size);    //use for sorting
                                map.put(FILEINFO_KEY_SIZE_DISPLAY, file_size);
                            } else {
                                if (FileOp.isFileSelected(file_abs_path, "list"))
                                    map.put(FILEINFO_KEY_SELECTED, R.drawable.filebrowser_file_selected);
                                else
                                    map.put(FILEINFO_KEY_SELECTED, R.drawable.filebrowser_file_unselected);

                                map.put(FILEINFO_KEY_TYPE, FileOp.getFileTypeImg(file.getName()));
                                map.put(FILEINFO_KEY_NAMNE, getFileDescripe(file));
                                map.put(FILEINFO_KEY_SUFFIX, getFileType(file) + " | ");

                                String rw = "-";
                                if (file.canRead())
                                    rw += "r";
                                else
                                    rw += "-";
                                if (file.canWrite())
                                    rw += "w";
                                else
                                    rw += "-";
                                map.put(FILEINFO_KEY_ACCESS, rw + " | ");

                                long file_date = file.lastModified();
                                String date = new SimpleDateFormat("yyyy/MM/dd HH:mm")
                                        .format(new Date(file_date));
                                map.put(FILEINFO_KEY_DATE_DISPLAY, " | " + date + " | ");
                                map.put(FILEINFO_KEY_DATE_SORT, file_date);    //use for sorting

                                long file_size = file.length();
                                map.put(FILEINFO_KEY_SIZE_SORT, file_size);    //use for sorting
                                map.put(FILEINFO_KEY_SIZE_DISPLAY, FileOp.getFileSizeStr(file_size));
                            }
                            if (!file.isHidden()) {
                                list.add(map);
                            }

                        }
                    }
                }
            }
        } catch (Exception e) {
            Log.e(TAG, "Exception when getFileListData(): ", e);
            return list;
        }

        /* sorting */
        if (!list.isEmpty()) {
            if (sort_type == FILEBROWSER_SORT_BY_NAME) {
                if (mIsSorted == false) {
                    mIsSorted = true;
                    Collections.sort(list, new Comparator<Map<String, Object>>() {
                        public int compare(Map<String, Object> object1, Map<String, Object> object2) {
                            File file1 = new File((String) object1.get(FILEINFO_KEY_PATH));
                            File file2 = new File((String) object2.get(FILEINFO_KEY_PATH));
                            if (file1.isFile() && file2.isFile() || file1.isDirectory() && file2.isDirectory()) {
                                return ((String) object1.get(FILEINFO_KEY_NAMNE)).toLowerCase()
                                        .compareTo(((String) object2.get(FILEINFO_KEY_NAMNE)).toLowerCase());
                            } else {
                                return file1.isFile() ? 1 : -1;
                            }
                        }

                    });
                } else {
                    mIsSorted = false;
                    Collections.sort(list, new Comparator<Map<String, Object>>() {
                        public int compare(Map<String, Object> object1, Map<String, Object> object2) {
                            File file1 = new File((String) object1.get(FILEINFO_KEY_PATH));
                            File file2 = new File((String) object2.get(FILEINFO_KEY_PATH));
                            if (file1.isFile() && file2.isFile() || file1.isDirectory() && file2.isDirectory()) {
                                return ((String) object2.get(FILEINFO_KEY_NAMNE)).toLowerCase()
                                        .compareTo(((String) object1.get(FILEINFO_KEY_NAMNE)).toLowerCase());
                            } else {
                                return file1.isFile() ? -1 : 1;
                            }
                        }

                    });
                }

            } else if (sort_type == FILEBROWSER_SORT_BY_DATE) {
                if (mIsSorted == false) {
                    mIsSorted = true;
                    Collections.sort(list, new Comparator<Map<String, Object>>() {
                        public int compare(Map<String, Object> object1, Map<String, Object> object2) {
                            int compareResult;
                            compareResult = ((Long) object1.get(FILEINFO_KEY_DATE_SORT)).compareTo(
                                    (Long) object2.get(FILEINFO_KEY_DATE_SORT));
                            compareResult = (~compareResult) + 1;
                            return (compareResult);
                        }
                    });
                } else {
                    mIsSorted = false;
                    Collections.sort(list, new Comparator<Map<String, Object>>() {
                        public int compare(Map<String, Object> object1, Map<String, Object> object2) {
                            return ((Long) object1.get(FILEINFO_KEY_DATE_SORT)).compareTo((Long) object2.get(FILEINFO_KEY_DATE_SORT));
                        }
                    });
                }
            } else if (sort_type == FILEBROWSER_SORT_BY_SIZE) {
                if (mIsSorted == false) {
                    mIsSorted = true;
                    Collections.sort(list, new Comparator<Map<String, Object>>() {
                        public int compare(Map<String, Object> object1, Map<String, Object> object2) {
                            return ((Long) object1.get(FILEINFO_KEY_SIZE_SORT)).compareTo((Long) object2.get(FILEINFO_KEY_SIZE_SORT));
                        }
                    });
                } else {
                    mIsSorted = false;
                    Collections.sort(list, new Comparator<Map<String, Object>>() {
                        public int compare(Map<String, Object> object1, Map<String, Object> object2) {
                            return ((Long) object2.get(FILEINFO_KEY_SIZE_SORT)).compareTo((Long) object1.get(FILEINFO_KEY_SIZE_SORT));
                        }
                    });
                }
            }
        }
        return list;
    }

    private String getFileType(File file) {
        String fileName = file.getName();
        String fileType = "unknown";
        if (fileName.contains(".")) {
            fileType = fileName.substring(fileName.lastIndexOf(".") + 1, fileName.length());
        } else
            fileType = "unknown";
        return fileType;
    }

    private String getFileDescripe(File file) {
        String fileDescripe = file.getName();
        if (fileDescripe.contains(".")) {
            fileDescripe = fileDescripe.substring(0, fileDescripe.lastIndexOf("."));
        }
        return fileDescripe;
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        if (keyCode == KeyEvent.KEYCODE_BACK) {
            if (!mCurrentPath.equals(ROOT)) {
                File file = new File(mCurrentPath);
                String parent_path = file.getParent();
                if (mCurrentPath.equals(NAND_PATH) || mCurrentPath.equals(SD_PATH)
                        || mCurrentPath.equals(NFS_PATH) || parent_path.equals(USB_PATH)) {
                    mCurrentPath = ROOT;
                    DeviceScan();
                    Log.d(TAG, "onKeyDown(),keyCode : " + keyCode);
                    return false;
                } else {
                    if(mCurrentPath.equals(parent_path) == false) {
                        mCurrentPath = parent_path;
                        mListView.setAdapter(getFileListAdapterSorted(mCurrentPath, mSortType));
                    }
                    return false;
                }
            }
        }

        return super.onKeyDown(keyCode, event);
    }

}// END public class FileBrowserActivity extends Activity {
