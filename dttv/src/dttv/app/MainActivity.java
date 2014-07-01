package dttv.app;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import android.view.Menu;
import android.view.View;

public class MainActivity extends Activity {
	public final static String EXTRA_MESSAGE = "com.example.simpleplayer.MESSAGE";
	public final static String FILE_MSG = "dttp.app.MainActivity.URL";
	private final String LOGTAG = "DTTV-FileBrowser";
	
	private final int REQUEST_CODE_PICK_DIR = 1;
	private final int REQUEST_CODE_PICK_FILE = 2;
	
	@Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);        
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.main, menu);
        return true;
    }
    
    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        String result = data.getExtras().getString("com.example.simpleplayer.filePathRet");
        Log.d(LOGTAG, result);
        
        //call playactivity  
        Intent intent = new Intent(this, PlayActivity.class);
        intent.putExtra(FILE_MSG, result);
        startActivity(intent);
    }
    
    //Choose file to play
    public void ChooseFile(View view) {
    	Log.d(LOGTAG, "Start Chooseing File to play");
    	// do something
    	Intent fileExploreIntent = new Intent(
    			dttv.app.FileBrowserActivity.INTENT_ACTION_SELECT_FILE,
				null,
				this,
				dttv.app.FileBrowserActivity.class
				);
		fileExploreIntent.putExtra(dttv.app.FileBrowserActivity.startDirectoryParameter, Environment.getExternalStorageDirectory());
		startActivityForResult(fileExploreIntent,REQUEST_CODE_PICK_FILE);	
    }
    
    public void PlayerStart(View view) {
    	Log.d(LOGTAG, "Start play");
    		
    }
    
    public void PlayerPause(View view) {
    	Log.d(LOGTAG, "Pause play");
    		
    }
    
    public void PlayerResume(View view) {
    	Log.d(LOGTAG, "Resume play");
    		
    }
    
    public void PlayerStop(View view) {
    	Log.d(LOGTAG, "Stop play");
    		
    }
  
    
}