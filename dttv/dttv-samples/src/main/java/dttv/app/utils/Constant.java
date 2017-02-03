package dttv.app.utils;

import dttv.app.R;

public class Constant {

    public final static String EXTRA_MESSAGE = "com.example.simpleplayer.MESSAGE";
    public final static String FILE_MSG = "dttp.app.media.URL";
    public final static String FILE_TYPE = "dttp.app.media.type";
    public static final String LOGTAG = "DTTV";


    public final static int REFRESH_TIME_MSG = 0x1000;
    public final static int BEGIN_MEDIA_MSG = REFRESH_TIME_MSG + 1;
    public final static int HIDE_OPREATE_BAR_MSG = BEGIN_MEDIA_MSG + 1;
    public final static int HIDE_PROGRESS_BAR_MSG = HIDE_OPREATE_BAR_MSG + 1;
    public final static int REFRESH_TIME = 1000;


    public static final String ARGUMENTS_NAME = "arg";
    public static final int LOCAL_VIDEO = 0;
    public static final int LOCAL_AUDIO = 1;
    public static final int LOCAL_FILE = 2;

    public static final String MEIDA_NAME_STR = "dttv.app.media_name";

    public static final String[] gEqulizerPresets = {
            "Normal",
            "Classical",
            "Dance",
            "Flat",
            "Folk",
            "Heavy Metal",
            "Hip Hop",
            "Jazz",
            "Rock"
    };

    public static final int FILTER_SIZE = 1 * 1024 * 1024;// 1MB
    public static final int FILTER_DURATION = 1 * 60 * 1000;// 1分钟


    // setting
    public static final String KEY_SETTING_FILTER_AUDIO_FILTER = "key_checkbox_setting_audio_filter";
    public static final String KEY_SETTING_FILTER_VIDEO_FILTER = "key_checkbox_setting_video_filter";
    public static final String KEY_SETTING_DECODER_TYPE = "key_listpreference_setting_decoder_type";
    public static final String KEY_SETTING_DECODER_TYPE_SOFT = "0";
    public static final String KEY_SETTING_DECODER_TYPE_HW = "1";
    public static final String KEY_SETTING_BROWSER_MODE ="key_listpreference_filebrowser_display";
    public static final String KEY_SETTING_DISPLAY_MODE ="key_listpreference_setting_display_mode";

}
