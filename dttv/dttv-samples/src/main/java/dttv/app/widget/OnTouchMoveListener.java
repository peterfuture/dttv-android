package dttv.app.widget;

import android.view.MotionEvent;

public interface OnTouchMoveListener {
    void onTouchMoveUp(float posX);

    void onTouchMoveDown(float posX);

    void onTouch(MotionEvent event);
}
