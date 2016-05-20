package dttv.app.utils;

import java.io.File;
import java.io.IOException;

import android.net.Uri;
import android.text.TextUtils;

/**
 * @author shihx1
 * @aim handle file
 */
public class FileUtil {

    public static String getCanonical(File file) {
        if (file == null)
            return null;
        try {
            return file.getCanonicalPath();
        } catch (IOException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
            return file.getAbsolutePath();
        }
    }

    public static String getPath(String uri) {
        if (TextUtils.isEmpty(uri))
            return null;
        if (uri.startsWith("file://") && uri.length() > 7) {
            return Uri.decode(uri.substring(7));
        }
        return Uri.decode(uri);
    }

    public static String getName(String uri) {
        String path = getPath(uri);
        if (path != null)
            return new File(path).getName();
        return null;
    }

    public static void deleteDir(File file) {
        if (file.exists() && file.isDirectory()) {
            for (File f : file.listFiles()) {
                if (f.isDirectory())
                    deleteDir(f);
                f.delete();
            }
            file.delete();
        }
    }
}
