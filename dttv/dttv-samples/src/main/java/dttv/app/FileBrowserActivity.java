package dttv.app;

import java.io.File;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.Date;
import java.util.HashMap;
import java.util.List;
import java.util.Map;


import android.app.Activity;
import android.content.Intent;
import android.content.SharedPreferences;
import android.net.Uri;
import android.os.Bundle;


import android.support.v7.app.AppCompatActivity;
import android.text.TextUtils;
import android.util.Log;
import android.view.ContextMenu;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;

import android.widget.AdapterView;
import android.widget.Button;
import android.widget.ListAdapter;
import android.widget.ListView;
import android.widget.SimpleAdapter;
import android.widget.TextView;
import android.widget.AdapterView.OnItemClickListener;

import dttv.app.utils.Constant;
import dttv.app.utils.PlayerUtil;
import dttv.app.utils.SettingUtil;
import dttv.app.utils.StorageUtils;
import dttv.app.widget.AddUrlDialog;

import android.content.ActivityNotFoundException;

public class FileBrowserActivity extends AppCompatActivity {

    public static final String TAG = "FileBrowser";

    private static final String ROOT = "/mnt";

    List<StorageUtils.StorageInfo> mRootDevices = null;
    private ListView mListView;
    private int mItemSelected, mItemFirst, mItemLast;
    private int mItemTop;

    private boolean mListLoaded = false;
    private boolean mLoadCancel = false;
    private boolean mIsSorted = false;

    private static final int FILEBROWSER_SORT_BY_NONE = 0;
    private static final int FILEBROWSER_SORT_BY_DATE = 1;
    private static final int FILEBROWSER_SORT_BY_SIZE = 2;
    private static final int FILEBROWSER_SORT_BY_NAME = 3;
    private static int mSortType = FILEBROWSER_SORT_BY_NONE;

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

    private static final String CON_UDP = "UDP";

    public static String mCurrentPath = ROOT;

    SettingUtil mSettingUtil;
    private int mBrowserMode = 0; // 0 normal  1 audio only 2 video only

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.file_browser);
        Log.i(TAG, "Enter onCreate");

        /* setup file list */
        mListView = (ListView) findViewById(R.id.filebrowser_listview);
        mSettingUtil = new SettingUtil(this);

        if (mCurrentPath != null) {
            File file = new File(mCurrentPath);
            if (!file.exists())
                mCurrentPath = ROOT;
        } else {
            mCurrentPath = ROOT;
        }

        /* ListView OnItemClickListener */
        mListView.setOnItemClickListener(new OnItemClickListener() {
            public void onItemClick(AdapterView<?> parent, View view, int pos, long id) {
                Map<String, Object> item = (Map<String, Object>) parent.getItemAtPosition(pos);
                String mPath = (String) item.get(FILEINFO_KEY_PATH);

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
                    if (isDeviceFile(mCurrentPath)) {
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
        if (!isEnableSearch)
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

    protected void updateMenu(Menu menu) {
        menu.clear();
        menu.add("Setting").setShowAsAction(MenuItem.SHOW_AS_ACTION_WITH_TEXT);
        menu.add(CON_UDP).setShowAsAction(MenuItem.SHOW_AS_ACTION_WITH_TEXT);
    }

    @Override
    public void onCreateContextMenu(ContextMenu menu, View v,
                                    ContextMenu.ContextMenuInfo menuInfo) {
        super.onCreateContextMenu(menu, v, menuInfo);
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        updateMenu(menu);
        return super.onCreateOptionsMenu(menu);
    }

    public void openSettingUI() {
        startActivity(new Intent(this, SettingActivity.class));
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        String res = item.getTitleCondensed().toString();
        if (!TextUtils.isEmpty(res) && res.equals("Setting")) {
            openSettingUI();
        }

        if (!TextUtils.isEmpty(res) && res.equals(CON_UDP)) {
            AddUrlDialog dialog = new AddUrlDialog(this);
            dialog.show();
        }
        return super.onOptionsItemSelected(item);
    }



    @Override
    public void onPause() {
        super.onPause();
        Log.i(TAG, "Enter onPause");
        mLoadCancel = true;
        //update sharedPref
        SharedPreferences settings = getSharedPreferences("settings", Activity.MODE_PRIVATE);
        SharedPreferences.Editor editor = settings.edit();
        editor.putString("cur_path", mCurrentPath);
        editor.commit();

        if (mListLoaded)
            mListLoaded = false;
    }

    @Override
    public void onResume() {
        super.onResume();
        mLoadCancel = false;
        mBrowserMode = mSettingUtil.getBrowserMode();

        Log.i(TAG, "Enter onResume");
        if (mCurrentPath.equals(ROOT)) {
            mListView.setAdapter(getDeviceListAdapter());
        } else {
            mListView.setAdapter(getFileListAdapterSorted(mCurrentPath, mSortType));
        }
        mListView.setSelectionFromTop(mItemSelected, mItemTop);
    }

    /**
     * onDestory()
     */
    @Override
    public void onDestroy() {
        super.onDestroy();
    }

    protected void openFile(File f) {
        String type = "*/*";
        String uri = f.getPath();
        type = FileInfo.getFileType(f);
        String name = f.getPath();
        //if (type.contains("video") || type.contains("audio")) {
            PlayerUtil.getInstance().beginToPlayer(this, uri, name, Constant.LOCAL_VIDEO);
            return;
        //}

        // Use system app
        /*Intent intent = new Intent();
        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        intent.setAction(android.content.Intent.ACTION_VIEW);
        intent.setDataAndType(Uri.fromFile(f), type);
        try {
            startActivity(intent);
        } catch (ActivityNotFoundException e) {
        }*/
    }

    private boolean isDeviceFile(String path) {
        if (mRootDevices.size() <= 0)
            mRootDevices = StorageUtils.getStorageList();
        Boolean isDevice = false;
        for (int i = 0; i < mRootDevices.size(); i++) {
            StorageUtils.StorageInfo device = mRootDevices.get(i);
            if (path.equals(device.path)) {
                isDevice = true;
                break;
            }
        }
        return isDevice;
    }

    private void DeviceScan() {
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
        File dir;

        mRootDevices = StorageUtils.getStorageList();
        for (int i = 0; i < mRootDevices.size(); i++) {
            StorageUtils.StorageInfo device = mRootDevices.get(i);
            map = new HashMap<String, Object>();
            map.put(FILEINFO_KEY_NAMNE, device.getDisplayName());
            map.put(FILEINFO_KEY_PATH, device.path);
            if (device.path.contains("sda")) {
                map.put(FILEINFO_KEY_TYPE, R.drawable.filebrowser_icon_usb);
                map.put(FILEINFO_KEY_SIZE_SORT, 1);
            } else {
                map.put(FILEINFO_KEY_TYPE, R.drawable.filebrowser_icon_sdcard);
                map.put(FILEINFO_KEY_SIZE_SORT, 0);
            }
            map.put(FILEINFO_KEY_DATE_DISPLAY, 0);
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

    private void updatePathShow(String path) {
        TextView tv = (TextView) findViewById(R.id.filebrowser_current_path);
        if (path.equals(ROOT))
            tv.setText(getText(R.string.device_list));
        else
            tv.setText(path);
    }

    private SimpleAdapter getFileListAdapterSorted(String path, int sort_type) {

        if (path.equals(ROOT)) {
            /*return new SimpleAdapter(FileBrowserActivity.this, getDeviceListData(),
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
                    });*/
            return new SimpleAdapter(FileBrowserActivity.this, getDeviceListData(),
                R.layout.filebrowser_simple_device_item,
                new String[]{
                    FILEINFO_KEY_TYPE,
                    FILEINFO_KEY_NAMNE,
                },
                new int[]{
                    R.id.filebrowser_simple_device_type,
                    R.id.filebrowser_simple_device_name
                });
        } else {
            /*return new SimpleAdapter(FileBrowserActivity.this, getFileListDataSorted(path, sort_type),
                    R.layout.filebrowser_filelist_item,
                    new String[]{
                            FILEINFO_KEY_TYPE,
                            FILEINFO_KEY_NAMNE,
                            FILEINFO_KEY_SELECTED,
                            FILEINFO_KEY_SIZE_DISPLAY,
                            FILEINFO_KEY_DATE_DISPLAY,
                            FILEINFO_KEY_ACCESS,
                            FILEINFO_KEY_SUFFIX,
                            FILEINFO_KEY_PATH
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
                    });*/
            return new SimpleAdapter(FileBrowserActivity.this, getFileListDataSorted(path, sort_type),
                R.layout.filebrowser_simple_filelist_item,
                new String[]{
                    FILEINFO_KEY_TYPE,
                    FILEINFO_KEY_NAMNE
                },
                new int[]{
                    R.id.filebrowser_simple_file_type,
                    R.id.filebrowser_simple_file_name
                });
        }
    }

    private List<Map<String, Object>> getFileListDataSorted(String path, int sort_type) {
        updatePathShow(path);
        return getFileListDataSortedAsync(path, sort_type);
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

                            if (isDeviceFile(file_abs_path))
                                continue;

                            map.put(FILEINFO_KEY_NAMNE, file.getName());
                            map.put(FILEINFO_KEY_PATH, file_abs_path);

                            if (file.isDirectory()) {
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

                                String strFileTypetype = FileInfo.getFileType(file);
                                if(mBrowserMode == 1) { // audio only
                                    if(strFileTypetype.contains("audio") == false)
                                        continue;
                                }
                                if(mBrowserMode == 2) { // video only
                                    if(strFileTypetype.contains("video") == false)
                                        continue;
                                }
                                map.put(FILEINFO_KEY_SELECTED, R.drawable.filebrowser_file_unselected);
                                map.put(FILEINFO_KEY_TYPE, FileInfo.getFileTypeIcon(file.getName()));
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
                                map.put(FILEINFO_KEY_SIZE_DISPLAY, FileInfo.getFileSizeStr(file_size));
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
                if (isDeviceFile(mCurrentPath)) {
                    mCurrentPath = ROOT;
                    DeviceScan();
                    Log.d(TAG, "onKeyDown(),keyCode : " + keyCode);
                    return false;
                } else {
                    if (mCurrentPath.equals(parent_path) == false) {
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
