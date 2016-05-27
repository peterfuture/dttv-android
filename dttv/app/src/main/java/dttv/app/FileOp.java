package dttv.app;

import java.io.Closeable;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.FileChannel;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.List;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.net.Uri;
import android.os.Bundle;
import android.os.Environment;
import android.os.Message;
import android.os.StatFs;
import android.util.Log;

import dttv.app.FileBrowserDatabase.FileMarkCursor;
import dttv.app.utils.FileUtils;

public class FileOp {
    public static File copying_file = null;
    public static boolean copy_cancel = false;
    public static boolean switch_mode = false;
    public static boolean IsBusy = false;
    public static String source_path = null;
    public static String target_path = null;
    private static final String ROOT_PATH = "/storage";
    private static final String NAND_PATH = Environment.getExternalStorageDirectory().getPath();
    private static final String SD_PATH = "/storage/sdcard0/external_sdcard";
    private static final String USB_PATH = "/storage/sdcard0/usbdrive";
    private static final String SATA_PATH = "/storage/sdcard0/sata";

    public static void SetMode(boolean value) {
        switch_mode = value;
    }

    public static boolean GetMode() {
        return switch_mode;
    }

    public static FileOpTodo file_op_todo = FileOpTodo.TODO_NOTHING;

    public static enum FileOpTodo {
        TODO_NOTHING,
        TODO_CPY,
        TODO_CUT
    }

    public static enum FileOpReturn {
        SUCCESS,
        ERR,
        ERR_NO_FILE,
        ERR_DEL_FAIL,
        ERR_CPY_FAIL,
        ERR_CUT_FAIL,
        ERR_PASTE_FAIL
    }

    /**
     * getFileSizeStr
     */
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

    /**
     * getFileTypeImg
     */
    public static Object getFileTypeImg(String filename) {
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

    /**
     * get file type op
     */
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
    //video from layer
    public static final String[] video_extensions = {".3gp",
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
    private static final String[] music_extensions = {".mp3",
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
    private static final String[] photo_extensions = {".jpg",
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

    public static String CheckMediaType(File file) {
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

    public static int getThumbDeviceIcon(Context c, String device_name) {
        //String internal = c.getString(R.string.memory_device_str);
        //String sdcard = c.getString(R.string.sdcard_device_str);
        //String usb = c.getString(R.string.usb_device_str);
        if (device_name.equals("flash")) {
            return R.drawable.fileop_type_memory;
        } else if (device_name.equals("sata")) {
            return R.drawable.fileop_type_sata;
        } else if (device_name.equals("sdcard")) {
            return R.drawable.fileop_type_sdcard;
        } else if (device_name.equals("usb")) {
            return R.drawable.fileop_type_usb;
        }
        return R.drawable.fileop_type_unknown;
    }

    public static String getDeviceName(Context c, String filename) {
        String ret_str = null;
        /*if(filename.equals("/mnt/flash")){
    		ret_str = c.getString(R.string.memory_device_str);
    	}
    	else */
        if (filename.equals(SD_PATH)) {
            ret_str = c.getString(R.string.sdcard_device_str);
        } else if (filename.equals(USB_PATH)) {
            ret_str = c.getString(R.string.usb_device_str);
        }
        return ret_str;

    }

    public static String convertDeviceName(Context c, String name) {
        // TODO Auto-generated method stub
        String temp_name = null;
        String internal = c.getString(R.string.memory_device_str);
        String sdcard = c.getString(R.string.sdcard_device_str);
        String usb = c.getString(R.string.usb_device_str);
    		/*if(name.equals(internal))
    			temp_name="/mnt/flash";
    		else */
        if (name.equals(sdcard))
            temp_name = SD_PATH;
        else if (name.equals(usb))
            temp_name = USB_PATH;
        return temp_name;
    }

    public static boolean deviceExist(String string) {
        // TODO Auto-generated method stub
        if (Environment.getExternalStorageState().equals(Environment.MEDIA_MOUNTED)) {
            File sdCardDir = Environment.getExternalStorageDirectory();
        }
        return true;
    }

    public static String getShortName(String file) {

        File file_path = new File(file);
        String filename = file_path.getName();
        String temp_str = null;
        if (file_path.isDirectory()) {
            if (filename.length() > 18) {
                temp_str = String.copyValueOf((filename.toCharArray()), 0, 15);
                temp_str = temp_str + "...";

            } else {
                temp_str = filename;
            }

        } else {
            if (filename.length() > 18) {
                int index = filename.lastIndexOf(".");
                if ((index != -1) && (index < filename.length())) {
                    String suffix = String.copyValueOf((filename.toCharArray()), index, (filename.length() - index));
                    if (index >= 15) {
                        temp_str = String.copyValueOf((filename.toCharArray()), 0, 15);
                    } else {
                        temp_str = String.copyValueOf((filename.toCharArray()), 0, index);
                    }
                    temp_str = temp_str + "~" + suffix;
                } else {
                    temp_str = String.copyValueOf((filename.toCharArray()), 0, 15);
                    temp_str = temp_str + "~";
                }
            } else {
                temp_str = filename;
            }
        }

        return temp_str;

    }

    /**
     * check file sel status
     */
    public static boolean isFileSelected(String file_path, String cur_page) {
        if (cur_page.equals("list")) {
            if (FileBrowserActivity.mDataBase == null) return false;
            try {
                FileBrowserActivity.mCursor = FileBrowserActivity.mDataBase.getFileMarkByPath(file_path);
                if (FileBrowserActivity.mCursor.getCount() > 0) {
                    return true;
                }

            } finally {
                FileBrowserActivity.mCursor.close();
            }
        }
        return false;
    }

    public static Bitmap fitSizePic(File f) {
        
       /* Bitmap bitmap = null;
        BitmapFactory.Options opts = new BitmapFactory.Options(); 
        if(f.length()<20480){         //0-20k
          opts.inSampleSize = 1;
        }else if(f.length()<51200){   //20-50k
          opts.inSampleSize = 2;
        }else if(f.length()<307200){  //50-300k
          opts.inSampleSize = 4;
        }else if(f.length()<819200){  //300-800k
          opts.inSampleSize = 6;
        }else if(f.length()<1048576){ //800-1024k
          opts.inSampleSize = 8;
        }else{
          opts.inSampleSize = 10;
        }
        bitmap = BitmapFactory.decodeFile(f.getPath(), opts);*/
        BitmapFactory.Options options = new BitmapFactory.Options();
        options.inJustDecodeBounds = true;
        Bitmap bitmap = BitmapFactory.decodeFile(f.getPath(), options);

        int samplesize = (int) (options.outHeight / 96);
        if (samplesize <= 0) samplesize = 1;


        options.inSampleSize = samplesize;
        options.inJustDecodeBounds = false;
        bitmap = BitmapFactory.decodeFile(f.getPath(), options);

        return bitmap;
    }

    /**
     * update file sel status
     * 1: add to mark table 0: remove from mark table
     */
    public static void cleanFileMarks(String cur_page) {
        if (cur_page.equals("list")) {
            if (FileBrowserActivity.mDataBase != null) {
                FileBrowserDatabase.FileMarkCursor cc = null;
                try {
                    cc = FileBrowserActivity.mDataBase.getFileMark();
                    if (cc != null && cc.moveToFirst()) {
                        if (cc.getCount() > 0) {
                            for (int i = 0; i < cc.getCount(); i++) {
                                cc.moveToPosition(i);
                                String file_path = cc.getColFilePath();
                                if (file_path != null) {
                                    if (!new File(file_path).exists()) {
                                        FileBrowserActivity.mDataBase.deleteFileMark(file_path);
                                    }
                                }
                            }
                        }
                    }
                } finally {
                    if (cc != null) cc.close();
                }
            }
        }
    }

    public static void updateFileStatus(String file_path, int status, String cur_page) {
        if (cur_page.equals("list")) {
            if (FileBrowserActivity.mDataBase == null) return;
            if (status == 1) {
                try {
                    FileBrowserActivity.mCursor = FileBrowserActivity.mDataBase.getFileMarkByPath(file_path);
                    if (FileBrowserActivity.mCursor.getCount() <= 0) {
                        //Log.i(FileBrowserActivity.TAG, "add file: " + file_path);
                        FileBrowserActivity.mDataBase.addFileMark(file_path, 1);
                    }

                } finally {
                    FileBrowserActivity.mCursor.close();
                }
            } else {
                //Log.i(FileBrowserActivity.TAG, "remove file: " + file_path);
                FileBrowserActivity.mDataBase.deleteFileMark(file_path);
            }
        }
    }

    /**
     * cut/copy/paste/delete selected files
     */
    public static FileOpReturn cutSelectedFile() {
        return FileOpReturn.ERR;
    }

    public static FileOpReturn copySelectedFile() {
        return FileOpReturn.ERR;
    }

    private static void nioTransferCopy(File source, File target) {
        FileChannel in = null;
        FileChannel out = null;

        FileInputStream inStream = null;
        FileOutputStream outStream = null;

        try {
            inStream = new FileInputStream(source);
            outStream = new FileOutputStream(target);

            in = inStream.getChannel();
            out = outStream.getChannel();

            in.transferTo(0, in.size(), out);
        } catch (IOException e) {
            e.printStackTrace();
        } finally {
            close(inStream);
            close(in);
            close(outStream);
            close(out);
        }
    }

    private static void nioBufferCopy(File source, File target, String cur_page, int buf_size) {
        if (!source.exists() || !target.exists())
            return;

        FileChannel in = null;
        FileChannel out = null;

        FileInputStream inStream = null;
        FileOutputStream outStream = null;

        try {
            source_path = source.getPath();
            target_path = target.getPath();
            inStream = new FileInputStream(source);
            outStream = new FileOutputStream(target);

            in = inStream.getChannel();
            out = outStream.getChannel();

            ByteBuffer buffer = ByteBuffer.allocate(1024 * buf_size);
            long bytecount = 0;
            int byteread = 0;
            while (!copy_cancel && ((byteread = in.read(buffer)) != -1)) {
                if (copy_cancel) {
                    break;
                }
                buffer.flip();
                out.write(buffer);
                buffer.clear();
                bytecount += byteread;
                if (cur_page.equals("list")) {
                    FileBrowserActivity.mProgressHandler.sendMessage(Message.obtain(
                            FileBrowserActivity.mProgressHandler, 1, (int) (bytecount * 100 / source.length()), 0));
                }
            }
            source_path = null;
            target_path = null;
            out.force(true);
        } catch (IOException e) {
            e.printStackTrace();
        } finally {
            close(inStream);
            close(in);
            close(outStream);
            close(out);
        }
    }

    private static void close(Closeable closable) {
        if (closable != null) {
            try {
                closable.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }

    public static FileOpReturn pasteSelectedFile(String cur_page) {
        ArrayList<String> fileList = new ArrayList<String>();
        //long copy_time_start=0, copy_time_end = 0;
        //copying_file = null;
        copy_cancel = false;
        IsBusy = true;
        if ((file_op_todo != FileOpTodo.TODO_CPY) &&
                (file_op_todo != FileOpTodo.TODO_CUT)) {
            if (cur_page.equals("list")) {
                FileBrowserActivity.mProgressHandler.sendMessage(Message.obtain(
                        FileBrowserActivity.mProgressHandler, 5));
            }
            IsBusy = false;
            return FileOpReturn.ERR;
        }

        if (cur_page.equals("list")) {
            if (FileBrowserActivity.mCurrentPath.startsWith(ROOT_PATH)) {
                if (Environment.getExternalStorageState().equals(Environment.MEDIA_MOUNTED_READ_ONLY)) {
                    FileBrowserActivity.mProgressHandler.sendMessage(Message.obtain(
                            FileBrowserActivity.mProgressHandler, 7));
                    IsBusy = false;
                    return FileOpReturn.ERR;
                }
            }
            if (!checkCanWrite(FileBrowserActivity.mCurrentPath)) {
                FileBrowserActivity.mProgressHandler.sendMessage(Message.obtain(
                        FileBrowserActivity.mProgressHandler, 7));
                IsBusy = false;
                return FileOpReturn.ERR;
            }
        }
        try {
            if (cur_page.equals("list")) {
                FileBrowserActivity.mCursor = FileBrowserActivity.mDataBase.getFileMark();
                if (FileBrowserActivity.mCursor.getCount() > 0) {
                    for (int i = 0; i < FileBrowserActivity.mCursor.getCount(); i++) {
                        FileBrowserActivity.mCursor.moveToPosition(i);
                        fileList.add(FileBrowserActivity.mCursor.getColFilePath());
                    }
                }
            }

        } finally {
            if (cur_page.equals("list")) {
                FileBrowserActivity.mCursor.close();
            }
        }

        if (!fileList.isEmpty()) {
            if (cur_page.equals("list")) {
                FileBrowserActivity.mProgressHandler.sendMessage(Message.obtain(
                        FileBrowserActivity.mProgressHandler, 3));
            }

            String curPathBefCopy = null;
            String curPathAftCopy = null;
            if (cur_page.equals("list")) {
                curPathBefCopy = FileBrowserActivity.mCurrentPath;
            } else {
                Log.e("pasteSelectedFile", "curPathBefCopy=null error.");
                IsBusy = false;
                return FileOpReturn.ERR;
            }

            for (int i = 0; i < fileList.size(); i++) {
                String name = fileList.get(i);
                File file = new File(name);
                if (copy_cancel) {
                    if (cur_page.equals("list")) {
                        FileBrowserActivity.mProgressHandler.sendMessage(Message.obtain(
                                FileBrowserActivity.mProgressHandler, 9));
                    }
                    IsBusy = false;
                    return FileOpReturn.ERR;
                }

                if (file.exists()) {
                    //Log.i(FileBrowserActivity.TAG, "paste file: " + name);
                    if (cur_page.equals("list")) {
                        if (file.length() > checkFreeSpace(FileBrowserActivity.mCurrentPath)) {
                            FileBrowserActivity.mProgressHandler.sendMessage(Message.obtain(
                                    FileBrowserActivity.mProgressHandler, 8));
                            IsBusy = false;
                            return FileOpReturn.ERR;
                        }
                    }

                    if (file.isDirectory()) {
                        try {
                            int idx = -1;
                            idx = (FileBrowserActivity.mCurrentPath).indexOf(file.getPath());
                            if (idx != -1) {
                                if (cur_page.equals("list")) {
                                    FileBrowserActivity.mProgressHandler.sendMessage(Message.obtain(
                                            FileBrowserActivity.mProgressHandler, 11));
                                }
                                IsBusy = false;
                                return FileOpReturn.ERR;
                            } else {
                                //copying_file=new File(FileBrowserActivity.cur_path+file);
                                File file_new = null;
                                //Log.i(FileBrowserActivity.TAG, "copy and paste file: " + name);
                                if (cur_page.equals("list")) {
                                    file_new = new File(FileBrowserActivity.mCurrentPath + File.separator + file.getName());
                                }

                                if (file_new.exists()) {
                                    String date = new SimpleDateFormat("yyyyMMddHHmmss_")
                                            .format(Calendar.getInstance().getTime());
                                    if (cur_page.equals("list")) {
                                        file_new = new File(FileBrowserActivity.mCurrentPath + File.separator + date + file.getName());
                                    }
                                }

                                if (!file_new.exists()) {
                                    copying_file = file_new;
                                    FileUtils.setCurPage(cur_page);
                                    FileUtils.copyDirectoryToDirectory(file, new File(FileBrowserActivity.mCurrentPath));

                                    if (!copy_cancel) {
                                        if (file_op_todo == FileOpTodo.TODO_CUT)
                                            FileUtils.deleteDirectory(file);
                                    } else {
                                        if (file_new.exists())
                                            FileUtils.deleteDirectory(file_new);

                                        if (cur_page.equals("list")) {
                                            FileBrowserActivity.mProgressHandler.sendMessage(Message.obtain(
                                                    FileBrowserActivity.mProgressHandler, 9));
                                        }
                                        IsBusy = false;
                                        return FileOpReturn.ERR;
                                    }
                                }
                            }
                        } catch (Exception e) {
                            Log.e("Exception-FileOP", e.toString());
                        }
                    } else {
                        try {
                            File file_new = null;
                            //Log.i(FileBrowserActivity.TAG, "copy and paste file: " + name);
                            if (cur_page.equals("list")) {
                                file_new = new File(FileBrowserActivity.mCurrentPath + File.separator + file.getName());
                            }


                            if (file_new.exists()) {
                                String date = new SimpleDateFormat("yyyyMMddHHmmss_")
                                        .format(Calendar.getInstance().getTime());
                                if (cur_page.equals("list")) {
                                    file_new = new File(FileBrowserActivity.mCurrentPath + File.separator + date + file.getName());
                                }
                            }

                            if (!file_new.exists()) {
                                //Log.i(FileBrowserActivity.TAG, "copy to file: " + file_new.getPath());
                                file_new.createNewFile();
                                copying_file = file_new;
                                try {
                                    //copy_time_start = Calendar.getInstance().getTimeInMillis();
                                    if (file.length() < 1024 * 1024 * 10)
                                        nioBufferCopy(file, file_new, cur_page, 4);
                                    else if (file.length() < 1024 * 1024 * 100)
                                        nioBufferCopy(file, file_new, cur_page, 1024);
                                    else
                                        nioBufferCopy(file, file_new, cur_page, 1024 * 10);
                                    //nioTransferCopy(file, file_new);
                                    //copy_time_end = Calendar.getInstance().getTimeInMillis();
                                    if (!copy_cancel) {
                                        if (file_op_todo == FileOpTodo.TODO_CUT)
                                            file.delete();
                                    } else {
                                        if (file_new.exists())
                                            file_new.delete();

                                        if (cur_page.equals("list")) {
                                            FileBrowserActivity.mProgressHandler.sendMessage(Message.obtain(
                                                    FileBrowserActivity.mProgressHandler, 9));
                                        }
                                        IsBusy = false;
                                        return FileOpReturn.ERR;
                                    }
                                } catch (Exception e) {
                                    Log.e("Exception-FileOp", e.toString());
                                }
                            }

                        } catch (Exception e) {
                            Log.e("Exception-FileOp", e.toString());
                        }

                        if (cur_page.equals("list")) {
                            FileBrowserActivity.mProgressHandler.sendMessage(Message.obtain(
                                    FileBrowserActivity.mProgressHandler, 2, (i + 1) * 100 / fileList.size(), 0));
                        }
                    }
                }
            }

            //make sure current path is the destination path, otherwise indicate copy fail
            if (cur_page.equals("list")) {
                curPathAftCopy = FileBrowserActivity.mCurrentPath;
            } else {
                Log.e("pasteSelectedFile", "curPathAftCopy=null error.");
                IsBusy = false;
                return FileOpReturn.ERR;
            }

            //Log.i("wxl","curPathBefCopy:"+curPathBefCopy);
            //Log.i("wxl","curPathAftCopy:"+curPathAftCopy);

            if ((copy_cancel) || !(curPathBefCopy.equals(curPathAftCopy))) {
                if (copying_file.exists()) {
                    try {
                        if (copying_file.isDirectory()) {
                            FileUtils.deleteDirectory(copying_file);
                        } else {
                            copying_file.delete();
                        }
                    } catch (Exception e) {
                        Log.e("Exception when delete", e.toString());
                    }
                }

                if (cur_page.equals("list")) {
                    FileBrowserActivity.mProgressHandler.sendMessage(Message.obtain(FileBrowserActivity.mProgressHandler, 9));
                }

                IsBusy = false;
                return FileOpReturn.ERR;
            }

            Bundle data = new Bundle();
            Message msg;
            if (cur_page.equals("list")) {
                msg = Message.obtain(FileBrowserActivity.mProgressHandler, 4);
                data.putStringArrayList("file_name_list", fileList);
                msg.setData(data);
                FileBrowserActivity.mProgressHandler.sendMessage(msg);
                IsBusy = false;
                return FileOpReturn.SUCCESS;
            }

        } else {
            if (cur_page.equals("list")) {
                FileBrowserActivity.mProgressHandler.sendMessage(Message.obtain(
                        FileBrowserActivity.mProgressHandler, 5));
                IsBusy = false;
                return FileOpReturn.ERR;

            }
        }
        IsBusy = false;
        return FileOpReturn.ERR;
    }

    public static FileOpReturn deleteSelectedFile(String cur_page) {
        List<String> fileList = new ArrayList<String>();
        boolean IsDelSuccess = true;
        IsBusy = true;
        try {
            if (cur_page.equals("list")) {
                FileBrowserActivity.mCursor = FileBrowserActivity.mDataBase.getFileMark();
                if (FileBrowserActivity.mCursor.getCount() > 0) {
                    for (int i = 0; i < FileBrowserActivity.mCursor.getCount(); i++) {
                        FileBrowserActivity.mCursor.moveToPosition(i);
                        fileList.add(FileBrowserActivity.mCursor.getColFilePath());
                    }
                }

            }

        } finally {
            if (cur_page.equals("list")) {
                FileBrowserActivity.mCursor.close();

            }
        }

        if (!fileList.isEmpty()) {
            for (String name : fileList) {
                File file = new File(name);
                if (file.exists()) {
                    //Log.i(FileBrowserActivity.TAG, "delete file: " + name);
                    try {
                        if (file.canWrite()) {
                            if (file.isDirectory()) {
                                FileUtils.deleteDirectory(file);
                            } else {
                                IsDelSuccess = file.delete();
                            }
                        } else {
                            if (name.startsWith(ROOT_PATH)) {
                                if (!(Environment.getExternalStorageState().equals(Environment.MEDIA_MOUNTED_READ_ONLY))) {
                                    if (file.isDirectory()) {
                                        FileUtils.deleteDirectory(file);
                                    } else {
                                        IsDelSuccess = file.delete();
                                    }
                                }
                            }
                        }
                    } catch (Exception e) {
                        IsDelSuccess = false;
                        Log.e("Exception-FileOp", e.toString());
                    }
                }
            }
            IsBusy = false;
            if (IsDelSuccess)
                return FileOpReturn.SUCCESS;
            else
                return FileOpReturn.ERR_DEL_FAIL;
        } else {
            IsBusy = false;
            return FileOpReturn.ERR;
        }

    }

    public static long checkFreeSpace(String path) {
        long nSDFreeSize = 0;
        if (path != null) {
            StatFs statfs = new StatFs(path);

            long nBlocSize = statfs.getBlockSize();
            long nAvailaBlock = statfs.getAvailableBlocks();
            nSDFreeSize = nAvailaBlock * nBlocSize;
        }
        return nSDFreeSize;

    }

    public static boolean checkCanWrite(String path) {
        if (path != null) {
            File file = new File(path);
            if (file.exists() && file.canWrite())
                return true;
        }
        return false;

    }

    /*
    * get mark file name for function paste
    */
    public static String getMarkFileName(String cur_page) {
        String path = "\0";
        String name = "\0";
        int index = -1;

        if (cur_page.equals("list")) {
            FileBrowserActivity.mCursor = FileBrowserActivity.mDataBase.getFileMark();
            if (FileBrowserActivity.mCursor.getCount() > 0) {
                for (int i = 0; i < FileBrowserActivity.mCursor.getCount(); i++) {
                    FileBrowserActivity.mCursor.moveToPosition(i);
                    path = FileBrowserActivity.mCursor.getColFilePath();
                    index = path.lastIndexOf("/");
                    if (index >= 0) {
                        name += path.substring(index + 1);
                    }
                    name += "\n";
                }
            }
        }

        return name;
    }

    public static ArrayList<Uri> getMarkFilePathUri(String cur_page) {
        ArrayList<Uri> uris = new ArrayList<Uri>();
        String path = "\0";

        if (cur_page.equals("list")) {
            FileBrowserActivity.mCursor = FileBrowserActivity.mDataBase.getFileMark();
            if (FileBrowserActivity.mCursor.getCount() > 0) {
                for (int i = 0; i < FileBrowserActivity.mCursor.getCount(); i++) {
                    FileBrowserActivity.mCursor.moveToPosition(i);
                    path = FileBrowserActivity.mCursor.getColFilePath();
                    File f = new File(path);
                    if (!f.isDirectory()) {
                        uris.add(Uri.fromFile(f));
                    }
                }
            }
        }

        return uris;
    }

    /*
    * get mark file name for function rename
    */
    public static String getMarkFilePath(String cur_page) {
        String path = "\0";

        if (cur_page.equals("list")) {
            FileBrowserActivity.mCursor = FileBrowserActivity.mDataBase.getFileMark();
            if (FileBrowserActivity.mCursor.getCount() > 0) {
                for (int i = 0; i < FileBrowserActivity.mCursor.getCount(); i++) {
                    FileBrowserActivity.mCursor.moveToPosition(i);
                    path += FileBrowserActivity.mCursor.getColFilePath();
                    path += "\n";
                }
            }
        }

        return path;
    }

    /*
    * get single mark file name for function rename
    */
    public static String getSingleMarkFilePath(String cur_page) {
        String path = null;

        if (cur_page.equals("list")) {
            FileBrowserActivity.mCursor = FileBrowserActivity.mDataBase.getFileMark();
            if (FileBrowserActivity.mCursor.getCount() > 0) {
                if (FileBrowserActivity.mCursor.getCount() != 1) {
                    Log.e(FileBrowserActivity.TAG, "current FileBrowserActivity Cursor outof range, count:" + FileBrowserActivity.mCursor.getCount());
                } else {
                    FileBrowserActivity.mCursor.moveToFirst();
                    path = FileBrowserActivity.mCursor.getColFilePath();
                }
            }
        }

        Log.i(FileBrowserActivity.TAG, "path :" + path);
        return path;
    }

    public static boolean isPlain(String filename) {
        String name = filename.toLowerCase();
        for (String ext : plain_extensions) {
            if (name.endsWith(ext))
                return true;
        }
        return false;
    }

    public static final String[] plain_extensions = {".txt",
            ".c",
            ".cpp",
            ".java",
            ",conf",
            ".h",
            ".log",
            ".rc",
    };

    public static String pingIpAddr(String ip) {
        String ret = "idle";
        try {
            Process p = Runtime.getRuntime().exec("ping -c 1 " + ip);
            int status = p.waitFor();
            if (status == 0) {
                ret = "Pass";
            } else {
                ret = "Fail: IP addr not reachable";
            }
        } catch (IOException e) {
            ret = "Fail: IOException";
        } catch (InterruptedException e) {
            ret = "Fail: InterruptedException";
        }
        Log.i(FileBrowserActivity.TAG, "[pingIpAddr]ip:" + ip + ", ret:" + ret);
        return ret;
    }
}
         
