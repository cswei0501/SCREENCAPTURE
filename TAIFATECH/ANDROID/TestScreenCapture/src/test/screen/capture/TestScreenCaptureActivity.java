package test.screen.capture;

import java.util.Timer;
import java.util.TimerTask;

import cidana.screencap.Capture;
import android.app.Activity;
import android.app.ActivityManager;
import android.content.Context;
import android.content.Intent;
import android.content.res.Configuration;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.Gravity;
import android.view.KeyEvent;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.view.View.OnClickListener;
import android.widget.ImageButton;
import android.widget.Toast;
import android.hardware.SensorEventListener;
import android.hardware.Sensor;
import android.hardware.SensorManager;
import 	android.hardware.SensorEvent;


public class TestScreenCaptureActivity extends Activity{
    /** Called when the activity is first created. */
	private ImageButton mRadioBtn1 = null;
	private ImageButton mRadioBtn2 = null;
	private ImageButton mMoreBtn = null;
	private ImageButton mCloseBtn = null;
	private int nCheck = 0;
	private int mScrnOrient = Configuration.ORIENTATION_PORTRAIT; 
	private static Capture mScreencap;
	
	private final int 
    // play relate message
	PMSG_DETECT_TIMER = 1;
	
	private Timer timer;
	
	private TimerTask mRefTimeTask = new TimerTask(){   
  	  
        public void run() {   
            Message message = new Message();       
            message.what = PMSG_DETECT_TIMER;       
            mHandler.sendMessage(message);     
        }   
           
    };
    
    private final Handler mHandler = new Handler(){
		@Override
        public void handleMessage(Message msg) {   
			
            switch (msg.what){
                case PMSG_DETECT_TIMER:
                	Log.d("test","PMSG_DETECT_TIMER");
                	mHandler.removeMessages(PMSG_DETECT_TIMER);
                	
                	int newScrnOrient = getResources().getConfiguration().orientation;
                	Log.d("test","newScrnOrient = " + newScrnOrient);
                	if(newScrnOrient != mScrnOrient)
                	{
                		Log.d("test","screen rotate");
                		
                		mScrnOrient = newScrnOrient;
                		if(newScrnOrient == Configuration.ORIENTATION_LANDSCAPE)
                		{
                			Log.d("test","screen rotate ORIENTATION_LANDSCAPE");
                			if(mScreencap!=null)
                        		mScreencap.setVideoRotate(true);
                		}
                		else if(mScrnOrient == Configuration.ORIENTATION_PORTRAIT)
                		{
                			Log.d("test","screen rotate ORIENTATION_PORTRAIT");
                			if(mScreencap!=null)
                        		mScreencap.setVideoRotate(false);
                		}
                	}
                	break;
                default:
                    break;
            }
	
		}
	};
	
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN);

        mScrnOrient = getResources().getConfiguration().orientation;
        if(mScrnOrient == Configuration.ORIENTATION_LANDSCAPE)
        {
        	setContentView(R.layout.main_l);
        	initHorUI();
        }
        else if(mScrnOrient == Configuration.ORIENTATION_PORTRAIT)
        {
        	setContentView(R.layout.main_p);
        	initVerUI();
        } 
         
        setRadioStatus();
        mScreencap = new Capture();
        Log.d("test","test");
        mScreencap.initSC();
        
        if(mScrnOrient == Configuration.ORIENTATION_LANDSCAPE)
		{
			Log.d("test","screen rotate ORIENTATION_LANDSCAPE");
			if(mScreencap!=null)
        		mScreencap.setVideoRotate(true);
		}
		else if(mScrnOrient == Configuration.ORIENTATION_PORTRAIT)
		{
			Log.d("test","screen rotate ORIENTATION_PORTRAIT");
			if(mScreencap!=null)
        		mScreencap.setVideoRotate(false);
		}
        
        timer = new Timer();
		timer.schedule(mRefTimeTask, 20, 20);
		
    }/*
    @Override
	protected void onPause() {
		// TODO Auto-generated method stub
		super.onPause();
		if(timer==null)
			timer = new Timer();
		timer.schedule(mRefTimeTask, 0, 1000);
	}
    
    @Override
	protected void onRestart() {
		// TODO Auto-generated method stub
		super.onRestart();
		timer.cancel();
		timer = null;
	}*/
    
    private void setRadioStatus() {
    	if(nCheck==0)
        {
        	mRadioBtn1.setBackgroundResource(R.drawable.radiobox_check);
			mRadioBtn2.setBackgroundResource(R.drawable.radiobox_nocheck);
        }
        else
        {
        	mRadioBtn1.setBackgroundResource(R.drawable.radiobox_nocheck);
			mRadioBtn2.setBackgroundResource(R.drawable.radiobox_check);
        }
    }
    private void initHorUI() {
    	mRadioBtn1 = (ImageButton)findViewById(R.id.btn_radio1);
    	mRadioBtn1.setOnClickListener(mRadio1ClickListener);
    	mRadioBtn2 = (ImageButton)findViewById(R.id.btn_radio2);
    	mRadioBtn2.setOnClickListener(mRadio2ClickListener);
    	mMoreBtn = (ImageButton)findViewById(R.id.btn_more);
    	mMoreBtn.setOnClickListener(mBtnMoreClickListener);
    	mCloseBtn = (ImageButton)findViewById(R.id.btn_close);
    	mCloseBtn.setOnClickListener(mBtnCloseClickListener);
    }
     
    private void initVerUI() {
    	mRadioBtn1 = (ImageButton)findViewById(R.id.btn_radio1);
    	mRadioBtn1.setOnClickListener(mRadio1ClickListener);
    	mRadioBtn2 = (ImageButton)findViewById(R.id.btn_radio2);
    	mRadioBtn2.setOnClickListener(mRadio2ClickListener);
    	mMoreBtn = (ImageButton)findViewById(R.id.btn_more);
    	mMoreBtn.setOnClickListener(mBtnMoreClickListener);
    	mCloseBtn = (ImageButton)findViewById(R.id.btn_close);
    	mCloseBtn.setOnClickListener(mBtnCloseClickListener);
    }
    
    public static Capture getScreenLibHandle() {
    	return mScreencap;
    }
    
    private OnClickListener mBtnCloseClickListener = new OnClickListener(){
		@Override
		public void onClick(View v)
		{
			ActivityManager am = (ActivityManager)getSystemService(Context.ACTIVITY_SERVICE);   
	        am.restartPackage(getPackageName()); 
			finish();
			android.os.Process.killProcess(android.os.Process.myPid());
		}
	};
	
    private OnClickListener mRadio1ClickListener = new OnClickListener(){
		@Override
		public void onClick(View v)
		{
			if(nCheck==1)
			{
				nCheck = 0;
				mRadioBtn1.setBackgroundResource(R.drawable.radiobox_check);
				mRadioBtn2.setBackgroundResource(R.drawable.radiobox_nocheck);
				if(mScreencap!=null)
					mScreencap.setVideoEnable(true);
			}
		}
	};
	
	private OnClickListener mRadio2ClickListener = new OnClickListener(){
		@Override
		public void onClick(View v)
		{
			if(nCheck==0)
			{
				nCheck = 1;
				mRadioBtn1.setBackgroundResource(R.drawable.radiobox_nocheck);
				mRadioBtn2.setBackgroundResource(R.drawable.radiobox_check);
				if(mScreencap!=null)
					mScreencap.setVideoEnable(false);
			}
		}
	};
	
	private OnClickListener mBtnMoreClickListener = new OnClickListener(){
		@Override
		public void onClick(View v)
		{
			Intent intent= new Intent();
			intent.setClass(TestScreenCaptureActivity.this, MoreSettingActivity.class);
			startActivity(intent);
		}
	};
	
	@Override
	public void onConfigurationChanged(Configuration newConfig) {
		// TODO Auto-generated method stub
		
		//mScrnOrient = newConfig.orientation;
		if(newConfig.orientation == Configuration.ORIENTATION_LANDSCAPE)
        {
        	setContentView(R.layout.main_l);
        	initHorUI();
        	//if(mScreencap!=null)
        	//	mScreencap.setVideoRotate(true);
        }
        else if(newConfig.orientation == Configuration.ORIENTATION_PORTRAIT)
        {
        	setContentView(R.layout.main_p);
        	initVerUI();
        	//if(mScreencap!=null)
        	//	mScreencap.setVideoRotate(false);
        } 
		setRadioStatus();
		super.onConfigurationChanged(newConfig);
	}
	
	
	
		@Override
	public boolean onKeyDown(int keyCode, KeyEvent event) {
		// TODO Auto-generated method stub

		if (keyCode == KeyEvent.KEYCODE_BACK && event.getRepeatCount() == 0) { 
			ActivityManager am = (ActivityManager)getSystemService(Context.ACTIVITY_SERVICE);   
	        am.restartPackage(getPackageName()); 
			finish();
			android.os.Process.killProcess(android.os.Process.myPid()); 
			return true;
		}
		 
		return super.onKeyDown(keyCode, event);
	}
}
