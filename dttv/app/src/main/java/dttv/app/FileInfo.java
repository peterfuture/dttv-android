package dttv.app;

import java.io.File;

public class FileInfo {

    public static boolean isVideo(String filename) {
        String name = filename.toLowerCase();
        for (String ext : video_extensions) {
            if (name.endsWith(ext))
                return true;
        }
        return false;
    }

    public static boolean isMusic(String filename) {
        String name = filename.toLowerCase();
        for (String ext : music_extensions) {
            if (name.endsWith(ext))
                return true;
        }
        return false;
    }

    public static boolean isPhoto(String filename) {
        String name = filename.toLowerCase();
        for (String ext : photo_extensions) {
            if (name.endsWith(ext))
                return true;
        }
        return false;
    }

    public static boolean isApk(String filename) {
        String name = filename.toLowerCase();
        if (name.endsWith(".apk"))
            return true;
        return false;
    }

    public static boolean isHtm(String filename) {
        String name = filename.toLowerCase();
        if (name.endsWith(".htm") || name.endsWith(".shtml") || name.endsWith(".bin"))
            return true;
        return false;
    }

    public static boolean isPdf(String filename) {
        String name = filename.toLowerCase();
        if (name.endsWith(".pdf"))
            return true;
        return false;
    }


    /* file type extensions */
    //video
    public static final String[] video_extensions = {
            ".3gp",
            ".3g2",
            ".divx",
            ".h264",
            ".h265",
            ".avi",
            ".m2ts",
            ".mkv",
            ".mov",
            ".mp4",
            ".mpg",
            ".mpeg",
            ".rm",
            ".rmvb",
            ".wmv",
            ".ts",
            ".tp",
            ".dat",
            ".vob",
            ".flv",
            ".bit",
            ".vc1",
            ".m4v",
            ".f4v",
            ".asf",
            ".lst",
            ".mts",
            ".webm",
            ".mpe",
            ".pmp",
            ".mvc",
       /* "" */
    };
    //music
    private static final String[] music_extensions = {
            ".mp3",
            ".wma",
            ".m4a",
            ".aac",
            ".ape",
            ".mp2",
            ".ogg",
            ".flac",
            ".alac",
            ".wav",
            ".mid",
            ".xmf",
            ".mka",
            ".aiff",
            ".aifc",
            ".aif",
            ".pcm",
            ".adpcm"
    };
    //photo
    private static final String[] photo_extensions = {
            ".jpg",
            ".jpeg",
            ".bmp",
            ".tif",
            ".tiff",
            ".png",
            ".gif",
            ".giff",
            ".jfi",
            ".jpe",
            ".jif",
            ".jfif",
            ".mpo",
            ".3dg",
            "3dp"
    };

    public static boolean isPlain(String filename) {
        String name = filename.toLowerCase();
        for (String ext : plain_extensions) {
            if (name.endsWith(ext))
                return true;
        }
        return false;
    }

    public static final String[] plain_extensions = {
            ".txt",
            ".c",
            ".cpp",
            ".java",
            ",conf",
            ".h",
            ".log",
            ".rc",
    };

    public static String getFileType(File file) {
        String typeStr = "application/*";
        String filename = file.getName();

        if (isVideo(filename))
            typeStr = "video/*";
        else if (isMusic(filename))
            typeStr = "audio/*";
        else if (isPhoto(filename))
            typeStr = "image/*";
        else if (isApk(filename))
            typeStr = "application/vnd.android.package-archive";
        else if (isHtm(filename))
            typeStr = "text/html";
        else if (isPdf(filename))
            typeStr = "application/pdf";
        else if (isPlain(filename)) {
            typeStr = "text/plain";
        } else {
            typeStr = "application/*";
        }
        return typeStr;
    }

    public static String getFileSizeStr(long length) {
        int sub_index = 0;
        String sizeStr = "";
        if (length >= 1073741824) {
            sub_index = (String.valueOf((float) length / 1073741824)).indexOf(".");
            sizeStr = ((float) length / 1073741824 + "000").substring(0, sub_index + 3) + " GB";
        } else if (length >= 1048576) {
            sub_index = (String.valueOf((float) length / 1048576)).indexOf(".");
            sizeStr = ((float) length / 1048576 + "000").substring(0, sub_index + 3) + " MB";
        } else if (length >= 1024) {
            sub_index = (String.valueOf((float) length / 1024)).indexOf(".");
            sizeStr = ((float) length / 1024 + "000").substring(0, sub_index + 3) + " KB";
        } else if (length < 1024) {
            sizeStr = String.valueOf(length) + " B";
        }
        return sizeStr;
    }

    public static Object getFileTypeIcon(String filename) {
        if (isMusic(filename)) {
            return R.drawable.fileop_type_music;
        } else if (isPhoto(filename)) {
            return R.drawable.fileop_type_photo;
        } else if (isVideo(filename)) {
            return R.drawable.fileop_type_video;
        } else if (isApk(filename)) {
            return R.drawable.fileop_type_apk;
        } else
            return R.drawable.fileop_type_file;
    }

}
         
