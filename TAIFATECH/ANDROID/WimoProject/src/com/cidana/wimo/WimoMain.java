package com.cidana.wimo;


import java.util.Timer;
import java.util.TimerTask;

import cidana.screencap.Capture;
import cidana.screencap.Capture.WimoCheckListener;
import android.app.Activity;
import android.app.ActivityManager;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;

import android.content.res.Configuration;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.preference.CheckBoxPreference;
import android.preference.Preference;
import android.preference.Preference.OnPreferenceChangeListener;
import android.preference.PreferenceActivity;
import android.provider.Settings;
import android.util.Log;
import android.view.KeyEvent;
import android.view.Surface;

public class WimoMain extends PreferenceActivity implements OnPreferenceChangeListener,
	WimoCheckListener{
    /** Called when the activity is first created. */
	
	private static Capture mScreencap = null;
	private CheckBoxPreference mEnableWimo;
	private final int 
    // play relate message
		PMSG_DETECT_TIMER = 1,
		PMSG_CHECK_CHECK = 2,
		PMSG_CHECK_UNCHECK = 3;
	private int mScrnOrient = Surface.ROTATION_0;
	private Timer timer;
	private static final int 
	DIALOG_EXIT_MESSAGE = 1;
	private final int
		ROTATION_0 = 0,
		ROTATION_90 = 1,
		ROTATION_180 = 2,
		ROTATION_270 =3;

	private TimerTask mRefTimeTask = new TimerTask(){   
  	  
        public void run() {   
            Message message = new Message();       
            message.what = PMSG_DETECT_TIMER;       
            mHandler.sendMessage(message);
        }   
           
    };
    
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        addPreferencesFromResource(R.xml.preferences_page);
        mEnableWimo = (CheckBoxPreference) findPreference("key_wimo_enable");
        mEnableWimo.setChecked(false);
        mEnableWimo.setSummary(R.string.wimo_close);
        
        mEnableWimo.setOnPreferenceChangeListener(this);
        mScreencap = new Capture();
        mScreencap.setWimoCheckListener(this);
        //mScreencap.initSC();
        
	 mScrnOrient = getWindowManager().getDefaultDisplay().getRotation();
 
        if(mScrnOrient == Surface.ROTATION_0)
    	 {
    		Log.d("test","screen rotate Surface.ROTATION_0");
    		if(mScreencap!=null)
    			mScreencap.setVideoRotate(ROTATION_0);
    	 }
    	 else if(mScrnOrient == Surface.ROTATION_90)
    	 {
    		Log.d("test","screen rotate Surface.ROTATION_90");
    		if(mScreencap!=null)
    			mScreencap.setVideoRotate(ROTATION_90);
    	 }
    	 else if(mScrnOrient == Surface.ROTATION_180)
    	 {
    		Log.d("test","screen rotate Surface.ROTATION_180");
    		if(mScreencap!=null)
    			mScreencap.setVideoRotate(ROTATION_180);
    	 }
    	 else if(mScrnOrient == Surface.ROTATION_270)
    	 {
    		Log.d("test","screen rotate Surface.ROTATION_270");
    		if(mScreencap!=null)
    			mScreencap.setVideoRotate(ROTATION_270);
    	 }
		 
     	 timer = new Timer();
     	 timer.schedule(mRefTimeTask, 20, 20);
     
     	 final IntentFilter filter = new IntentFilter();
     	 filter.addAction(Intent.ACTION_SCREEN_OFF);  
     	 filter.addAction(Intent.ACTION_SCREEN_ON);  
	 registerReceiver(mBatInfoReceiver, filter);

    }
    
    private final BroadcastReceiver mBatInfoReceiver = new BroadcastReceiver() {  
    	@Override  
    	public void onReceive(final Context context, final Intent intent) {  
    		final String action = intent.getAction();  
    		if(Intent.ACTION_SCREEN_ON.equals(action)){  
    			Log.d("test", "screen is on...");  
    		}else if(Intent.ACTION_SCREEN_OFF.equals(action)){  
	    	  	 Log.d("test", "screen is off...");
    		}  
    	}  
   }; 

    	
	@Override
	public boolean onPreferenceChange(Preference arg0, Object arg1) {
		// TODO Auto-generated method stub
		
		if (arg0.getKey().equals("key_wimo_enable")) {
			if((Boolean)arg1)
			{
				//if(mScreencap!=null)
				//	mScreencap.setVideoEnable(true);
				//mEnableWimo.setSummary(R.string.wimo_open);
				startActivityForResult(new Intent(Settings.ACTION_WIFI_SETTINGS), 0);
				mEnableWimo.setChecked(false);
			}
			else
			{
				if(mScreencap!=null)
				{
					mScreencap.setVideoEnable(false);
					mEnableWimo.setSummary(R.string.wimo_close);
      					mEnableWimo.setChecked(false);
				}
			}
			return true;
		}
		return false;
	}
	
	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event) {
		// TODO Auto-generated method stub

         	Log.d("test","keyCode = " + keyCode);
         	if (keyCode == KeyEvent.KEYCODE_BACK && event.getRepeatCount() == 0)
		{ 
			showDialog(DIALOG_EXIT_MESSAGE);
			return true;
		}
			
		return super.onKeyDown(keyCode, event);
	}
	
	
    private final Handler mHandler = new Handler(){
		@Override
        public void handleMessage(Message msg) {   
			
            switch (msg.what){
                case PMSG_DETECT_TIMER:
                	//Log.d("test","PMSG_DETECT_TIMER");
                	mHandler.removeMessages(PMSG_DETECT_TIMER);
                	
			int newScrnOrient = getWindowManager().getDefaultDisplay().getRotation();
                	//Log.d("test","newScrnOrient = " + newScrnOrient);
                	if(newScrnOrient != mScrnOrient)
                	{
                		Log.d("test","screen rotate");
                		
                		mScrnOrient = newScrnOrient;

			        if(mScrnOrient == Surface.ROTATION_0)
			    	 {
			    		Log.d("test","screen rotate Surface.ROTATION_0");
			    		if(mScreencap!=null)
			    			mScreencap.setVideoRotate(ROTATION_0);
			    	 }
			    	 else if(mScrnOrient == Surface.ROTATION_90)
			    	 {
			    		Log.d("test","screen rotate Surface.ROTATION_90");
			    		if(mScreencap!=null)
			    			mScreencap.setVideoRotate(ROTATION_90);
			    	 }
			    	 else if(mScrnOrient == Surface.ROTATION_180)
			    	 {
			    		Log.d("test","screen rotate Surface.ROTATION_180");
			    		if(mScreencap!=null)
			    			mScreencap.setVideoRotate(ROTATION_180);
			    	 }
			    	 else if(mScrnOrient == Surface.ROTATION_270)
			    	 {
			    		Log.d("test","screen rotate Surface.ROTATION_270");
			    		if(mScreencap!=null)
			    			mScreencap.setVideoRotate(ROTATION_270);
			    	 }
                	}
                	break;
                case PMSG_CHECK_CHECK:
                	Log.d("test","PMSG_CHECK_CHECK");
                	if(mEnableWimo!=null)
                		mEnableWimo.setChecked(true);
                	break;
                case PMSG_CHECK_UNCHECK:
                	Log.d("test","PMSG_CHECK_UNCHECK");
                	if(mEnableWimo!=null)
                		mEnableWimo.setChecked(false);
                	break;
                default:
                    break;
            }
	
		}
	};

	@Override
    protected Dialog onCreateDialog(int id) {
        switch (id) {
        case DIALOG_EXIT_MESSAGE:
        	return new AlertDialog.Builder(WimoMain.this)
            .setTitle("Quit")
            //.setMessage(R.string.msg_exit_app)
            .setPositiveButton("Ok", new DialogInterface.OnClickListener() {
                public void onClick(DialogInterface dialog, int whichButton) {

                    /* User clicked OK so do some stuff */
                	
        			ActivityManager am = (ActivityManager)getSystemService(Context.ACTIVITY_SERVICE);   
        	        	am.restartPackage(getPackageName()); 
        			finish();
        			android.os.Process.killProcess(android.os.Process.myPid());
                }
            })
            .setNegativeButton("Cancel", new DialogInterface.OnClickListener() {
                public void onClick(DialogInterface dialog, int whichButton) {

                    /* User clicked Cancel so do some stuff */
                }
            })
            .create();
        }
        
        return null;
    }
	
	@Override

	 public void onActivityResult(int requestCode, int resultCode, Intent data) {

		 super.onActivityResult(requestCode, resultCode, data);
		 Log.d("test","onActivityResult");
		 mEnableWimo.setChecked(false);
		 
		 if(mScreencap.initSC()==0)
		 {
			if(mScreencap.setVideoEnable(true)!=0)
			{
				Log.d("test", "set video enable true\n");
				return;
			}
			mEnableWimo.setChecked(true);
			mEnableWimo.setSummary(R.string.wimo_open);
		 }
			 
	 }

	@Override
	public void onCheckChange(int nOpen) {
		// TODO Auto-generated method stub
		if(nOpen==1)
			mHandler.sendEmptyMessage(PMSG_CHECK_CHECK);
		else if(nOpen==0)
			mHandler.sendEmptyMessage(PMSG_CHECK_UNCHECK);
	}
}
