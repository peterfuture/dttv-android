package dttv.app.utils;

import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

public class MultiMediaTypeUtil {
    Class<?> mMediaFile, mMediaFileType;
    Method getFileTypeMethod, isAudioFileTypeMethod, isVideoFileTypeMethod, isImageFileTypeMethod;
    String methodName = "getBoolean";
    String getFileType = "getFileType";
    String isAudioFileType = "isAudioFileType";
    String isVideoFileType = "isVideoFileType";
    String isImageFileType = "isImageFileType";

    Field fileType;

    public void initReflect() {
        try {
            mMediaFile = Class.forName("android.media.MediaFile");
            mMediaFileType = Class.forName("android.media.MediaFile$MediaFileType");
            fileType = mMediaFileType.getField("fileType");

            getFileTypeMethod = mMediaFile.getMethod(getFileType, String.class);

            isAudioFileTypeMethod = mMediaFile.getMethod(isAudioFileType, int.class);
            isVideoFileTypeMethod = mMediaFile.getMethod(isVideoFileType, int.class);
            isImageFileTypeMethod = mMediaFile.getMethod(isImageFileType, int.class);

        } catch (NoSuchMethodException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        } catch (NoSuchFieldException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        } catch (ClassNotFoundException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
    }

    public int getMediaFileType(String path) {
        int type = 0;
        try {
            Object obj = getFileTypeMethod.invoke(mMediaFile, path);
            if (obj == null) {
                type = -1;
            } else {
                type = fileType.getInt(obj);
            }
        } catch (IllegalAccessException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        } catch (IllegalArgumentException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        } catch (InvocationTargetException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
        return type;
    }

    public boolean isVideoFile(int fileType) {
        boolean isVideoFile = false;
        try {
            isVideoFile = (Boolean) isAudioFileTypeMethod.invoke(mMediaFile, fileType);
        } catch (IllegalAccessException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        } catch (IllegalArgumentException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        } catch (InvocationTargetException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
        return isVideoFile;
    }

    public boolean isAudioFile(int fileType) {
        boolean isAudioFile = false;
        try {
            isAudioFile = (Boolean) isAudioFileTypeMethod.invoke(mMediaFile, fileType);
        } catch (IllegalAccessException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        } catch (IllegalArgumentException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        } catch (InvocationTargetException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
        return isAudioFile;
    }

    public boolean isImageFile(int fileType) {
        boolean isImgeFile = false;
        try {
            isImgeFile = (Boolean) isImageFileTypeMethod.invoke(mMediaFile, fileType);
        } catch (IllegalAccessException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        } catch (IllegalArgumentException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        } catch (InvocationTargetException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
        return isImgeFile;
    }
}
