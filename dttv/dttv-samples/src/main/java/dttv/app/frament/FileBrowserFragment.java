package dttv.app.frament;


import android.support.v4.app.Fragment;
import android.widget.ListView;

import java.io.File;
import java.util.List;

import dttv.app.R;
import dttv.app.base.SimpleFragment;
import dttv.app.utils.SettingUtil;
import dttv.app.utils.StorageUtils;

/**
 * A simple {@link Fragment} subclass.
 * 本地文件管理器
 */
public class FileBrowserFragment extends SimpleFragment {

    private static final String ROOT = "/mnt";
    public static String mCurrentPath = ROOT;

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

    SettingUtil mSettingUtil;
    private int mBrowserMode = 0; // 0 normal  1 audio only 2 video only

    public FileBrowserFragment() {
        // Required empty public constructor
    }

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_file_browser;
    }

    @Override
    protected void initEventAndData() {
        if (mCurrentPath != null) {
            File file = new File(mCurrentPath);
            if (!file.exists())
                mCurrentPath = ROOT;
        } else {
            mCurrentPath = ROOT;
        }
    }
}
