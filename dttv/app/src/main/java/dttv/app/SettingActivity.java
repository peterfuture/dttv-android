package dttv.app;

import dttv.app.utils.Constant;

import android.annotation.SuppressLint;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.preference.CheckBoxPreference;
import android.preference.ListPreference;
import android.preference.Preference;
import android.preference.Preference.OnPreferenceChangeListener;
import android.preference.Preference.OnPreferenceClickListener;
import android.preference.PreferenceActivity;
import android.preference.PreferenceFragment;
import android.preference.PreferenceManager;
import android.widget.Toast;

@SuppressLint("NewApi")
public class SettingActivity extends PreferenceActivity {
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        //addPreferencesFromResource(R.xml.setting_preference);
        getFragmentManager().beginTransaction().replace(android.R.id.content,
                new MyPreferenceFragment()).commit();
    }

    public static class MyPreferenceFragment extends PreferenceFragment implements OnPreferenceChangeListener, OnPreferenceClickListener {
        ListPreference list_decoder_type;
        CheckBoxPreference list_display_mode;

        @Override
        public void onCreate(Bundle savedInstanceState) {
            super.onCreate(savedInstanceState);
            addPreferencesFromResource(R.xml.setting_preference);
            list_decoder_type = (ListPreference) findPreference("key_setting_decoder_list");
            list_decoder_type.setOnPreferenceChangeListener(this);
            list_decoder_type.setOnPreferenceClickListener(this);

            list_display_mode = (CheckBoxPreference) findPreference("key_setting_display_mode");
        }

        @Override
        public boolean onPreferenceChange(Preference preference, Object newValue) {
            if (Constant.KEY_SETTING_DECODER_TYPE.equals(preference.getKey())) {
                list_decoder_type.setSummary(newValue.toString());
                list_decoder_type.setValue(newValue.toString());
                //取得属于整个应用程序的SharedPreferences
                SharedPreferences settings = PreferenceManager.getDefaultSharedPreferences(getActivity());
                SharedPreferences.Editor editor = settings.edit();
                editor.putString(Constant.KEY_SETTING_DECODER_TYPE, newValue.toString());
                editor.commit();
                return true;
            }

            if (Constant.KEY_SETTING_DISPLAY_MODE.equals(preference.getKey())) {
                return true;
            }

            return false;
        }

        @Override
        public boolean onPreferenceClick(Preference preference) {
            if (Constant.KEY_SETTING_DECODER_TYPE.equals(preference.getKey())) {
                String nowValue = list_decoder_type.getSummary().toString();
                //list_decoder_type.setDefaultValue(nowValue);
                list_decoder_type.setValue(nowValue);
                return true;
            }

            if (Constant.KEY_SETTING_DISPLAY_MODE.equals(preference.getKey())) {
                return true;
            }

            return false;
        }
    }
}
