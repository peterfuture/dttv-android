package app.dttv.dttvlib.subtitle;

import android.graphics.Bitmap;

/**
 * author：peter_future on 2017/8/20.
 * Email：peter_future@outlook.com
 */
public class SubtitleFrame {
    private int mType = 0; // 0-string, 1-bitmap
    private int mStartTime=0;
    private int mEndTime=0;
    private String mStringContent = null;
    private Bitmap mBitMap = null;
    private int mSubSize = -1;
    private int mWidth = 0;
    private int mHeight = 0;

    public SubtitleFrame(String s,int start ,int end) {
        mStringContent = s;
        mStartTime = start;
        mEndTime = end;
        mType = 0;
    }

    public SubtitleFrame(Bitmap bitmap,int start ,int end) {
        mBitMap = bitmap;
        mStartTime = start;
        mEndTime = end;
        mType = 1;
    }

    public String getStringContent () {
        return mStringContent;
    }

    public Bitmap getBitMap() {
        return mBitMap;
    }

    public int getStartTime() {
        return mStartTime;
    }

    public int getEndTime() {
        return mEndTime;
    }

    public int getType() {
        return mType;
    }

    public int getWidth() {
        return mWidth;
    }

    public int getHeight() {
        return mHeight;
    }
}
