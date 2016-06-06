package dttv.app;

import dttv.app.utils.Constant;

import android.annotation.SuppressLint;
import android.content.SharedPreferences;
import android.net.Uri;
import android.os.Bundle;
import android.preference.CheckBoxPreference;
import android.preference.ListPreference;
import android.preference.Preference;
import android.preference.Preference.OnPreferenceChangeListener;
import android.preference.Preference.OnPreferenceClickListener;
import android.preference.PreferenceActivity;
import android.preference.PreferenceFragment;
import android.preference.PreferenceManager;
import android.util.Log;
import android.widget.Toast;

import com.google.android.gms.appindexing.Action;
import com.google.android.gms.appindexing.AppIndex;
import com.google.android.gms.common.api.GoogleApiClient;

@SuppressLint("NewApi")
public class SettingActivity extends PreferenceActivity {
    /**
     * ATTENTION: This was auto-generated to implement the App Indexing API.
     * See https://g.co/AppIndexing/AndroidStudio for more information.
     */
    private GoogleApiClient client;

    private static String TAG = "SettingActivity";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        //addPreferencesFromResource(R.xml.setting_preference);
        getFragmentManager().beginTransaction().replace(android.R.id.content,
                new MyPreferenceFragment()).commit();
        // ATTENTION: This was auto-generated to implement the App Indexing API.
        // See https://g.co/AppIndexing/AndroidStudio for more information.
        client = new GoogleApiClient.Builder(this).addApi(AppIndex.API).build();
    }

    @Override
    public void onStart() {
        super.onStart();

        // ATTENTION: This was auto-generated to implement the App Indexing API.
        // See https://g.co/AppIndexing/AndroidStudio for more information.
        client.connect();
        Action viewAction = Action.newAction(
                Action.TYPE_VIEW, // TODO: choose an action type.
                "Setting Page", // TODO: Define a title for the content shown.
                // TODO: If you have web page content that matches this app activity's content,
                // make sure this auto-generated web page URL is correct.
                // Otherwise, set the URL to null.
                Uri.parse("http://host/path"),
                // TODO: Make sure this auto-generated app URL is correct.
                Uri.parse("android-app://dttv.app/http/host/path")
        );
        AppIndex.AppIndexApi.start(client, viewAction);
    }

    @Override
    public void onStop() {
        super.onStop();

        // ATTENTION: This was auto-generated to implement the App Indexing API.
        // See https://g.co/AppIndexing/AndroidStudio for more information.
        Action viewAction = Action.newAction(
                Action.TYPE_VIEW, // TODO: choose an action type.
                "Setting Page", // TODO: Define a title for the content shown.
                // TODO: If you have web page content that matches this app activity's content,
                // make sure this auto-generated web page URL is correct.
                // Otherwise, set the URL to null.
                Uri.parse("http://host/path"),
                // TODO: Make sure this auto-generated app URL is correct.
                Uri.parse("android-app://dttv.app/http/host/path")
        );
        AppIndex.AppIndexApi.end(client, viewAction);
        client.disconnect();
    }

    public static class MyPreferenceFragment extends PreferenceFragment implements OnPreferenceChangeListener, OnPreferenceClickListener {
        ListPreference list_decoder_type;
        ListPreference list_browser_mode;
        ListPreference list_display_mode;

        @Override
        public void onCreate(Bundle savedInstanceState) {
            super.onCreate(savedInstanceState);
            addPreferencesFromResource(R.xml.setting_preference);

            list_browser_mode = (ListPreference) findPreference(Constant.KEY_SETTING_BROWSER_MODE);
            list_browser_mode.setOnPreferenceChangeListener(this);
            list_browser_mode.setOnPreferenceClickListener(this);

            list_decoder_type = (ListPreference) findPreference(Constant.KEY_SETTING_DECODER_TYPE);
            list_decoder_type.setOnPreferenceChangeListener(this);
            list_decoder_type.setOnPreferenceClickListener(this);

            list_display_mode = (ListPreference) findPreference(Constant.KEY_SETTING_DISPLAY_MODE);
            list_display_mode.setOnPreferenceChangeListener(this);
            list_display_mode.setOnPreferenceClickListener(this);
        }

        @Override
        public boolean onPreferenceChange(Preference preference, Object newValue) {

            if (Constant.KEY_SETTING_BROWSER_MODE.equals(preference.getKey())) {
                //list_browser_mode.setSummary(newValue.toString());
                list_browser_mode.setValue(newValue.toString());
                //取得属于整个应用程序的SharedPreferences
                SharedPreferences settings = PreferenceManager.getDefaultSharedPreferences(getActivity());
                SharedPreferences.Editor editor = settings.edit();
                editor.putString(Constant.KEY_SETTING_BROWSER_MODE, newValue.toString());
                editor.commit();
                Log.i(TAG, "Set browser mode: " + newValue.toString());
                return true;
            }

            if (Constant.KEY_SETTING_DECODER_TYPE.equals(preference.getKey())) {
                //list_decoder_type.setSummary(newValue.toString());
                list_decoder_type.setValue(newValue.toString());
                //取得属于整个应用程序的SharedPreferences
                SharedPreferences settings = PreferenceManager.getDefaultSharedPreferences(getActivity());
                SharedPreferences.Editor editor = settings.edit();
                editor.putString(Constant.KEY_SETTING_DECODER_TYPE, newValue.toString());
                editor.commit();
                Log.i(TAG, "Set decoder type: " + newValue.toString());
                return true;
            }

            if (Constant.KEY_SETTING_DISPLAY_MODE.equals(preference.getKey())) {
                //list_display_mode.setSummary(newValue.toString());
                list_display_mode.setValue(newValue.toString());
                //取得属于整个应用程序的SharedPreferences
                SharedPreferences settings = PreferenceManager.getDefaultSharedPreferences(getActivity());
                SharedPreferences.Editor editor = settings.edit();
                editor.putString(Constant.KEY_SETTING_DISPLAY_MODE, newValue.toString());
                editor.commit();
                Log.i(TAG, "Set display mode: " + newValue.toString());
                return true;
            }

            return false;
        }

        @Override
        public boolean onPreferenceClick(Preference preference) {
/*
            // sample
            if (Constant.KEY_SETTING_DECODER_TYPE.equals(preference.getKey())) {
                String nowValue = list_decoder_type.getSummary().toString();
                list_decoder_type.setDefaultValue(nowValue);
                list_decoder_type.setValue(nowValue);
                return true;
            }
*/
            return false;
        }
    }
}
