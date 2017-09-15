package dttv.app.utils;

import dttv.app.PipTestActivity;
import dttv.app.VideoPlayerActivity;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;

public class PlayerUtil {
    private static PlayerUtil instance;

    private PlayerUtil() {
    }

    public static PlayerUtil getInstance() {
        if (instance == null)
            instance = new PlayerUtil();
        return instance;
    }


    public void beginToPlayer(Context cr, String uri, String name, int type) {

        Intent intent = new Intent();
        intent.putExtra(Constant.FILE_MSG, uri);
        intent.putExtra(Constant.FILE_TYPE, type);
        intent.putExtra(Constant.MEIDA_NAME_STR, name);

        // Fixme: select videoplayer - Android MediaPlayer | dtplayer(with glsurfaceview) | dtplayer(with surfaceview)

        // use VideoPlayerActivity
         intent.setClass(cr, VideoPlayerActivity.class);
         cr.startActivity(intent);

        // use PipTestActivity
        //intent.setClass(cr, PipTestActivity.class);
        //cr.startActivity(intent);
    }
}