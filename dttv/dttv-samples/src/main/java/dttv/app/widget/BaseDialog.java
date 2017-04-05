package dttv.app.widget;

import android.app.Dialog;
import android.content.Context;
import android.view.Gravity;
import android.view.Window;
import android.view.WindowManager;

public class BaseDialog extends Dialog {
    private static int default_width = 360; // 默认宽度
    private static int default_height = 200;// 默认高度

    public BaseDialog(Context context, int layout, int style) {
        this(context, default_width, default_height, layout, style);
    }

    public BaseDialog(Context context, int style) {
        this(context, default_width, default_height, style);
    }

    public BaseDialog(Context context, int width, int height, int style) {
        super(context, style);
        setCanceledOnTouchOutside(false);
        Window window = getWindow();
        WindowManager.LayoutParams params = window.getAttributes();
        params.height = width;//WindowManager.LayoutParams.WRAP_CONTENT;
        params.width = height;//WindowManager.LayoutParams.MATCH_PARENT;
        // }
        params.gravity = Gravity.CENTER;
        window.setAttributes(params);
    }


    public BaseDialog(Context context, int width, int height, int layout,
                      int style) { // boolean bMyHeight, int heightDimen
        super(context, style);
        try {
            setContentView(layout);
        } catch (OutOfMemoryError e) {
            System.gc();
        } catch (Exception exception) {
            System.gc();
        }

        setCanceledOnTouchOutside(false);
        Window window = getWindow();
        WindowManager.LayoutParams params = window.getAttributes();
        params.height = WindowManager.LayoutParams.WRAP_CONTENT;
        params.width = WindowManager.LayoutParams.MATCH_PARENT;
        // }
        params.gravity = Gravity.CENTER;
        window.setAttributes(params);
    }

    public BaseDialog(Context context, int width, int height, int layout,
                      int style, int heightDimen) {
        super(context, style);
        try {
            setContentView(layout);
        } catch (OutOfMemoryError e) {
            System.gc();
        } catch (Exception exception) {
            System.gc();
        }

        setCanceledOnTouchOutside(false);
        Window window = getWindow();
        WindowManager.LayoutParams params = window.getAttributes();
        params.height = context.getResources().getDimensionPixelSize(
                heightDimen);
        params.gravity = Gravity.CENTER;
        window.setAttributes(params);
    }


}
