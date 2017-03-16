package dttv.app.widget;


import android.content.Context;
import android.os.Handler;
import android.text.Editable;
import android.text.TextUtils;
import android.text.TextWatcher;
import android.view.MotionEvent;
import android.view.View;
import android.view.WindowManager;
import android.view.inputmethod.InputMethodManager;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Toast;

import java.io.File;

import dttv.app.AppApplication;
import dttv.app.FileInfo;
import dttv.app.R;
import dttv.app.utils.Constant;
import dttv.app.utils.PlayerUtil;

public class AddUrlDialog extends BaseDialog {
    private Context context;
    private EditText ed_coupon_no;
    private Button dialog_button_ok;
    private Button dialog_button_sdp;
    private Handler mHandler;
    private String couponNo;

    private InputMethodManager mImm;

    private TextWatcher watcher=new TextWatcher() {
        @Override
        public void onTextChanged(CharSequence s, int arg1, int arg2, int arg3) {
            if (TextUtils.isEmpty(ed_coupon_no.getText().toString())) {
                dialog_button_ok.setEnabled(false);
            }else {
                dialog_button_ok.setEnabled(true);
            }
        }

        @Override
        public void beforeTextChanged(CharSequence arg0, int arg1, int arg2,
                                      int arg3) {
        }

        @Override
        public void afterTextChanged(Editable arg0) {
        }
    };

    private AddUrlDialog(Context context, int style) {
        super(context, style);
        this.context=context;
    }

    public AddUrlDialog(Context context) {
        this(context, R.style.dialog_two_style);
        getWindow().setSoftInputMode(WindowManager.LayoutParams.SOFT_INPUT_ADJUST_PAN |
                WindowManager.LayoutParams.SOFT_INPUT_STATE_VISIBLE);
        setContentView(R.layout.dialog_add_coupon);
        mImm = (InputMethodManager) context.getSystemService(Context.INPUT_METHOD_SERVICE);
        findView();
        initView();
        this.show();
    }

    private void findView() {
        ed_coupon_no=(EditText) findViewById(R.id.ed_coupon_no);
        dialog_button_ok=(Button) findViewById(R.id.dialog_button_ok);
        dialog_button_sdp = (Button)findViewById(R.id.dialog_button_sdp);
        ed_coupon_no.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View v, MotionEvent event) {
                return false;
            }
        });

    }

    private void initView() {
        if (TextUtils.isEmpty(ed_coupon_no.getText().toString())) {
            dialog_button_ok.setEnabled(false);
        }else {
            dialog_button_ok.setEnabled(true);
        }
        ed_coupon_no.addTextChangedListener(watcher);
        dialog_button_ok.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (TextUtils.isEmpty(ed_coupon_no.getText())) {
                    Toast.makeText(context,"请输入地址",Toast.LENGTH_SHORT).show();
                } else {
                    String uri = ed_coupon_no.getEditableText().toString();
                    String name = "UDP播放";
                    PlayerUtil.getInstance().beginToPlayer(context, uri, name, Constant.LOCAL_VIDEO);
                    closeDialog();
                }
            }
        });

        dialog_button_sdp.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                File file = new File(AppApplication.filePath+"/tower.sdp");
                if (!file.exists()){
                    Toast.makeText(context,"sdp文件写入失败",Toast.LENGTH_SHORT).show();
                }else{
                    String uri = file.getAbsolutePath();
                    String name = "UDP播放SDP";
                    PlayerUtil.getInstance().beginToPlayer(context, uri, name, Constant.LOCAL_VIDEO);
                    closeDialog();
                }
            }
        });
    }

    public void closeDialog(){
        mImm.hideSoftInputFromWindow(ed_coupon_no.getWindowToken(), 0);
        if(null != this && isShowing()){
            dismiss();
        }
    }
}
