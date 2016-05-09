package dttv.app;

import dttv.app.utils.Constant;
import android.annotation.SuppressLint;
import android.content.SharedPreferences;
import android.os.Bundle;
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
		// TODO Auto-generated method stub
		super.onCreate(savedInstanceState);
		//addPreferencesFromResource(R.xml.setting_preference);
		getFragmentManager().beginTransaction().replace(android.R.id.content,
				new MyPreferenceFragment()).commit();
	}
	
	public static class MyPreferenceFragment extends PreferenceFragment implements OnPreferenceChangeListener,OnPreferenceClickListener{
		ListPreference decode_list;
		@Override
		public void onCreate(Bundle savedInstanceState) {
			// TODO Auto-generated method stub
			super.onCreate(savedInstanceState);
			addPreferencesFromResource(R.xml.setting_preference);
			decode_list = (ListPreference)findPreference("dt_setting_decode_list");
			decode_list.setOnPreferenceChangeListener(this);
			decode_list.setOnPreferenceClickListener(this);
		}

		@Override
		public boolean onPreferenceChange(Preference preference, Object newValue) {
			// TODO Auto-generated method stub
			if(Constant.DECODE_STYLE_KEY.equals(preference.getKey())){
				
				decode_list.setSummary(newValue.toString());
				decode_list.setValue(newValue.toString());
				
				//取得属于整个应用程序的SharedPreferences  
	            SharedPreferences settings = PreferenceManager.getDefaultSharedPreferences(getActivity());
	            SharedPreferences.Editor editor = settings.edit();
	            editor.putString(Constant.DECODE_STYLE_KEY, newValue.toString());
	            editor.commit();
			}
			return false;
		}

		@Override
		public boolean onPreferenceClick(Preference preference) {
			// TODO Auto-generated method stub
			if(Constant.DECODE_STYLE_KEY.equals(preference.getKey())){
				/*SharedPreferences settings = PreferenceManager.getDefaultSharedPreferences(getActivity());
				String result = settings.getString(Constant.DECODE_STYLE_KEY, "11");*/
				String nowValue = decode_list.getSummary().toString();
				//decode_list.setDefaultValue(nowValue);
				//Toast.makeText(getActivity(), nowValue, 1).show();
				decode_list.setValue(nowValue);
			}
			return false;
		}
	}
}
