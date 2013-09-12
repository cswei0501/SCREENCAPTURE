package cidana.wimo2;

import java.io.DataOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.lang.reflect.Constructor;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import java.util.Timer;
import java.util.TimerTask;

import cidana.screencap.AutoCheckLib;
import cidana.screencap.Capture;
import cidana.screencap.Capture.WimoCheckListener;
import android.app.Activity;
import android.app.ActivityManager;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.graphics.drawable.AnimationDrawable;
import android.graphics.drawable.Drawable;
import android.media.AudioFormat;
import android.media.AudioRecord;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.NetworkInfo.State;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;
import android.os.Build;
import android.os.Build.VERSION;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.provider.Settings;
import android.util.Log;
import android.view.Gravity;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuItem;
import android.view.Surface;
import android.view.View;
import android.view.Window;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.ScrollView;
import android.widget.TextView;
import android.widget.Toast;

import com.cmcc.wimo.AudioCap;
import com.cmcc.wimo.ScreenCap;
import com.cmcc.wimo.Encoder;

import dalvik.system.DexClassLoader;

public class MainActivity extends Activity implements WimoCheckListener{
	private static final String TAG = "MainActivity";
    /** Called when the activity is first created. */
	
	private final static int
		MENU_UPGRADE	= Menu.FIRST,
		MENU_EXIT		= Menu.FIRST +1,
		MENU_ABOUT		= Menu.FIRST +2,
		MENU_HELP		= Menu.FIRST +3;
	
	private final static int 
		REQUEST_CODE_WIFI_SETTING = 0,
		REQUEST_CODE_SEL_DEVICE = 1;
		
	private boolean mIsExit = false;
	
	private PopMenu mCustomPopMenu = null;
	
	private CiAlertDialog mInfoDlg = null;
	private HashMap<Integer, Dialog> mInfoDlgMaps = new HashMap<Integer, Dialog>();
	
	private String mUsefulSSID;
	private String mCurrentSSID;
	
	private boolean mIsRssiWeak = false;
	private CaptureContainer mCapContainer = null;
	private Capture mScreencap = null;
	private ScreenCap mHWScreencap = null;
	private Encoder mHWEncoder = null;
	private AudioCap mHWAudioCap = null;
	private AudioRecord audioRecord = null;
	
//	private WFDStreamConfig mStreamTS = null;
	private Object objTS = null;
	private Method actionStartTS = null;
	private Method actionStopTS = null;
	private AutoCheckLib mAutoChecklib = null;

	private Button mBtnWimo = null;
	private Button mBtnWifi = null;
	private TextView mTvWifi = null;
	private ImageView mSignalView = null;
	
	//private ScrollView mGuiderContainers = null;
	private NotificationManager mWiMoConnNotify;
	private int 
	mStatus = -1,
	mDisconnectedStatus = -1;
	private byte[] mRGBbuf = null;
	
	private WIFIReceiver mReceiver = null;
	private IntentFilter mIntentFilter = null;
	private final int 
    // play relate message
    STATUS_NO_WIFI = 1,
    STATUS_WIFI = 2,
    //STATUS_WIMO = 3,
    STATUS_WIMO_CONNECTING = 4,
    STATUS_WIMO_CONNECTED = 5;
    
	public static final int 
	ERROR = -1,
	SUCCESS = 0;

	private int 
	mCapVideoMode = -1,
	mCapAudioMode = -1;
	
	public String 
	mModelVersion = null,
	mAppName = null;
	
	private static final int
	ROTATE_0 = 0,
	ROTATE_90 = 90,
	ROTATE_180 = 180,
	ROTATE_270 =270;

	private static final int
	PIX_FMT_RGBA_8888 = 1,
	PIX_FMT_RGB_565 = 4,
	PIX_FMT_BGRA_8888 = 5;

	public static final int
	JPEG = 0,
	H264 = 1,
	mQuality = 85;

	private final int
		BUFFERSIZE = 1024*768*4;
	
	private int
	mFormat = 0,
	mWidth = 0,
	mHeight = 0,
	mStride = 0,
	mSize = 0,
	mEncodedSize = 0;

	//audio capture
	private int 
	ulSamplesPerSec = 44100,
	bitRate = AudioFormat.ENCODING_PCM_16BIT,
	nChannel = AudioFormat.CHANNEL_CONFIGURATION_STEREO;
	
	public int 	
	WIMO_SPEAKER = 88;
	private boolean
	bIsRunning = false;
	
	
	private final int 
    // play relate message
		PMSG_DETECT_TIMER = 1,
		PMSG_CONN_WIMO_SUCCESS = 2,
		PMSG_CONN_WIMO_FAILED = 3,
		PMSG_CONN_WIMO_SUSPEND = 4,
		PMSG_AUTO_GOTO_HOME = 10,
		PMSG_RSSI_SIGNAL_WEAK = 11,
		PMSG_SHOW_SPECIAL_DIALOG = 12;
	private int mScrnOrient = Surface.ROTATION_0;
	
	private int mDlgToShow_Id = -1;
	private static final int 
	DIALOG_EXIT_MESSAGE = 1,
	DIALOG_CONNECT_WIMO = 2,
	DIALOG_CONNECT_WIFI = 3,
	DIALOG_WLAN_TURNED_OFF = 10,
	DIALOG_WLAN_LOST = 11,
	DIALOG_WLAN_WEAK_SIGNAL = 12,
	DIALOG_WLAN_CONNECTED_OTHER = 13,
	DIALOG_WIMO_CONNECTING_TIMEOUT = 20,
	DIALOG_WIMO_CONNECT_BREAK = 21,
	DIALOG_WIMO_CONNECT_SUSPEND = 22,
	DIALOG_OBTAIN_FB0_FAILED = 23,
	DIALOG_NET_ERROR = 30;
	private Timer timer = null;
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
        
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        
        setContentView(R.layout.main);
        
        if (isFirstRun())
    	{
    		Intent intent = new Intent();
    		intent.setClass(MainActivity.this, UsageGuiderActivity.class);
    		startActivity(intent);
    		saveFirstRun(false);
    	}
        /*mHandler.post(new Runnable() {
			
			@Override
			public void run() {
				// TODO Auto-generated method stub
				if (isFirstRun())
		    	{
		    		Intent intent = new Intent();
		    		intent.setClass(MainActivity.this, UsageGuiderActivity.class);
		    		startActivity(intent);
		    		saveFirstRun(false);
		    	}
			}
		});*/

        Process process = null;
        DataOutputStream os = null;
        Runtime runtime = null;
		int ret = ERROR;

        try {
            runtime = Runtime.getRuntime();
            process = runtime.exec("su");
            //or whatever command
            //get the runtime process and open  a output stream for writing.
            os = new DataOutputStream(process.getOutputStream());
            //write command.
            os.writeBytes("chmod 664 /dev/graphics/fb0\n");
            //ZTE audio should read /dev/pcm-pipe, other device is not
            //os.writeBytes("chmod 664 /dev/pcm-pipe\n");
            //flush it.
            os.flush();
            os.writeBytes("exit\n");
            os.flush();
            os.close();
        } catch (IOException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }	
        
        mCapContainer = CaptureContainer.getInstance(MainActivity.this);
        mScreencap = mCapContainer.getScrnCap();//new Capture();
		//mHWScreencap = mCapContainer.getHWScrnCap();//new ScreenCap();
		//mHWEncoder = mCapContainer.getHWEncoder();//new Encoder(JPEG); 
		mAutoChecklib = mCapContainer.getAutoCheckLib();//new AutoCheckLib(); 
        
        Context context = MainActivity.this;
		Class classTS = null;
		Class[] params = new Class[2];
		params[0] = Long.TYPE;
		params[1] = Integer.TYPE;
		try {
			 final File wimoDexJar = new File("/system/framework/TIWiMoInterface.jar");
			 if (wimoDexJar.exists())
			 {
				 DexClassLoader cl = new DexClassLoader(wimoDexJar.toString(), wimoDexJar.getParentFile().getAbsolutePath(), 
						 null, context.getClassLoader());  
				 classTS = cl.loadClass("com.cmcc.wimo.WFDStreamConfig");
				 
				 Constructor ct = classTS.getConstructor(Context.class);
	//			 Object paramVal[] = new Object[1];
	//			 paramVal[0] = context;
				 objTS = ct.newInstance(context.getApplicationContext());
	             /* Class[] params = new Class[2];
	             params[0] = Integer.TYPE;
	             params[1] = Integer.TYPE;*/
	             actionStartTS = classTS.getMethod("WFDStreamStart", params);
	             //actionStartTS.invoke(objTS);    
	             
	             actionStopTS = classTS.getMethod("WFDStreamStop");
	             //actionStopTS.invoke(objTS);    
			 }
		} catch (Exception e)
		{
			e.printStackTrace();
		}
		//mStreamTS = new WFDStreamConfig(getApplicationContext());
		if(objTS != null)
		{
			mCapVideoMode = mScreencap.TS_STREAM_INTERFACE;
			mModelVersion = "";
			mAppName = "wimo2.0_cidana_tx_cellphone";
		}
		else if(AutoCheckLib.SW_INTERFACE_LIB == mAutoChecklib.getCapLibStatus())
		{
			mCapVideoMode = mScreencap.SW_VIDEO_FRAMEBUFFER;
			mModelVersion = "";
			mAppName = "wimo1.0_cidana_tx_cellphone";		
		}
		else if(AutoCheckLib.LENOVO_INTERFACE_LIB == mAutoChecklib.getCapLibStatus())
		{
			mCapVideoMode = mScreencap.LNV_VIDEO_INTERFACE;
			WIMO_SPEAKER = 9;
			ulSamplesPerSec = 48000;
			mCapAudioMode = mScreencap.AUDIO_METHOD_LNV;
			mModelVersion = "Lenovo 899t";
			mAppName = "wimo1.0_cidana_tx_cellphone_lenovo899t";
		}
		else if(AutoCheckLib.HISENSE_INTERFACE_LIB == mAutoChecklib.getCapLibStatus())
		{
			mCapVideoMode = mScreencap.HS_VIDEO_INTERFACE;
			mCapAudioMode = mScreencap.AUDIO_METHOD_HS;	
			mModelVersion = "Hisense T96";
			mAppName = "wimo1.0_cidana_tx_cellphone_hisenset96";
		}
		else if(AutoCheckLib.ZTE_INTERFACE_LIB == mAutoChecklib.getCapLibStatus())
		{
			mCapVideoMode = mScreencap.ZTE_VIDEO_INTERFACE;
			mCapAudioMode = mScreencap.AUDIO_METHOD_ZTE;
			mModelVersion = "ZTE U970/U930";
			mAppName = "wimo1.0_cidana_tx_cellphone_zteu970";
		}
		else if(mHWScreencap!=null && mHWEncoder!=null && 0==mHWScreencap.getCapLibStatus() && 0==mHWEncoder.getEncLibStatus())
		{
			ret = mHWScreencap.update();
			if(ret == SUCCESS)
			{
				mCapVideoMode = mScreencap.HW_VIDEO_INTERFACE;

				mRGBbuf = new byte[BUFFERSIZE];
				mScreencap.mEncodedBuf = new byte[BUFFERSIZE];
				
				mFormat = mHWScreencap.getFormat();
				mWidth = mHWScreencap.getWidth();
				mHeight = mHWScreencap.getHeight();
				mStride = mHWScreencap.getStride();
				mSize = mHWScreencap.getSize();

				Log.d("test","mFormat:"+mFormat+",mWidth:"+mWidth+",mHeight:"+mHeight+",mStride:"+mStride+"mSize:"+mSize);
			}
		}
	
		if(mCapAudioMode != mScreencap.AUDIO_METHOD_LNV)
		{
			mHWAudioCap = new AudioCap();
			if(mHWAudioCap!=null && 0==mHWAudioCap.getAudCapStatus())
			{
				Thread thread = new AudioCapture();
				thread.start();
				//if(AudioCapture())
				mCapAudioMode = mScreencap.AUDIO_METHOD_DEFAULT;			
			}	
		}

        mScreencap.setWimoCheckListener(this);
    	if(mScreencap.getCapStatus() == 0)
    	{
    	  	mScreencap.cValPointer = mScreencap.jniInit(mCapVideoMode, mCapAudioMode);        	
    	    
    		//Log.d("tst","VERSION.SDK:"+VERSION.SDK+"VERSION.SDK_int:"+VERSION.SDK_INT+" RELEASE:"+VERSION.RELEASE);
     
    		if(mScreencap.checkFb0()!=0)
    		{
                mHandler.postDelayed(new Runnable() {
    				@Override
    				public void run() {
    					Toast.makeText(MainActivity.this, R.string.string_obtain_fb0_error, Toast.LENGTH_SHORT).show();
    				}
    			}, 100);
    		}
    		
            switch(TvDialog.getTVOrientation(MainActivity.this.getApplicationContext()))
    		{
    		case 0:
    			mScreencap.setTVRotate(mScreencap.CURRENT_TV_ROTATE_0);
    			break;
    		case 1:
    			mScreencap.setTVRotate(mScreencap.CURRENT_TV_ROTATE_90);
    			break;
    		case 2:
    			mScreencap.setTVRotate(mScreencap.CURRENT_TV_ROTATE_270);
    			break;
    		} 		
    	}	
  
     	timer = new Timer();
     	timer.schedule(mRefTimeTask, 20, 20);
     	 
     	initUI();
        initUIByNetworkInfo();
        initPopMenu();
     
     	final IntentFilter filter = new IntentFilter();
     	filter.addAction(Intent.ACTION_SCREEN_OFF);  
     	filter.addAction(Intent.ACTION_SCREEN_ON);
	 
        mIntentFilter = new IntentFilter();
        //mIntentFilter.addAction(ConnectivityManager.CONNECTIVITY_ACTION); 
        mIntentFilter.addAction(WifiManager.NETWORK_IDS_CHANGED_ACTION);
        mIntentFilter.addAction(WifiManager.NETWORK_STATE_CHANGED_ACTION);
        mIntentFilter.addAction(WifiManager.RSSI_CHANGED_ACTION);
        mIntentFilter.addAction(WifiManager.WIFI_STATE_CHANGED_ACTION);
        mReceiver = (WIFIReceiver) new WIFIReceiver();
        
        registerReceiver(mReceiver,mIntentFilter);

    }
    
	private void getScreenShot(){
		mScrnOrient = getWindowManager().getDefaultDisplay().getRotation();
		if(mScreencap!=null && mScreencap.getCapStatus()==0)
		{
			if(mScrnOrient == Surface.ROTATION_0){
				//Log.d("test","screen rotate Surface.ROTATION_0");
				mScreencap.setVideoRotate(mScreencap.ROTATION_0);
				if(mCapVideoMode == mScreencap.HW_VIDEO_INTERFACE)
				{
					if((mRGBbuf = mHWScreencap.getPixels(ROTATE_0)) != null)
					{
						mEncodedSize = mHWEncoder.encode(mRGBbuf, mFormat, mWidth, mHeight, mQuality, mScreencap.mEncodedBuf);
						mScreencap.sendVideoData(mScreencap.mEncodedBuf, mEncodedSize, mWidth, mHeight, mScreencap.ROTATION_0);	
					}
				}
			}
			else if(mScrnOrient == Surface.ROTATION_90)	{
				//Log.d("test","screen rotate Surface.ROTATION_90");
				mScreencap.setVideoRotate(mScreencap.ROTATION_90);
				if(mCapVideoMode == mScreencap.HW_VIDEO_INTERFACE)
				{
					if((mRGBbuf = mHWScreencap.getPixels(ROTATE_90)) != null)
					{
						mEncodedSize = mHWEncoder.encode(mRGBbuf, mFormat, mHeight, mWidth, mQuality, mScreencap.mEncodedBuf);
						mScreencap.sendVideoData(mScreencap.mEncodedBuf, mEncodedSize, mHeight, mWidth, mScreencap.ROTATION_90);	
					}
				}
			}
			else if(mScrnOrient == Surface.ROTATION_180){
				//Log.d("test","screen rotate Surface.ROTATION_180");
				mScreencap.setVideoRotate(mScreencap.ROTATION_180);
				if(mCapVideoMode == mScreencap.HW_VIDEO_INTERFACE)
				{
					if((mRGBbuf = mHWScreencap.getPixels(ROTATE_180)) != null)
					{
						mEncodedSize = mHWEncoder.encode(mRGBbuf, mFormat, mWidth, mHeight, mQuality, mScreencap.mEncodedBuf);
						mScreencap.sendVideoData(mScreencap.mEncodedBuf, mEncodedSize, mWidth, mHeight, mScreencap.ROTATION_180);	
					}
				}
			}
			else if(mScrnOrient == Surface.ROTATION_270){
				//Log.d("test","screen rotate Surface.ROTATION_270");
				mScreencap.setVideoRotate(mScreencap.ROTATION_270);
				if(mCapVideoMode == mScreencap.HW_VIDEO_INTERFACE)
				{
					if((mRGBbuf = mHWScreencap.getPixels(ROTATE_90)) != null)
					{
						mEncodedSize = mHWEncoder.encode(mRGBbuf, mFormat, mHeight, mWidth, mQuality, mScreencap.mEncodedBuf);
						mScreencap.sendVideoData(mScreencap.mEncodedBuf, mEncodedSize, mHeight, mWidth, mScreencap.ROTATION_270);
					}		
				}
			}		
		}
	}
    
	// This class extends Thread
	class audioThread extends Thread {
        // This method is called when the thread runs
        public void run() {
			android.os.Process.setThreadPriority(android.os.Process.THREAD_PRIORITY_URGENT_AUDIO);
			Log.d(TAG,"HW audio capture success!\n");
			int validSize;

			while(mScreencap!=null && mScreencap.getCapStatus() == 0 && bIsRunning)
			{
				if(mStatus==STATUS_WIMO_CONNECTED && audioRecord != null)
				{
					validSize = audioRecord.read(mScreencap.mAudioData, 0, mScreencap.mAudioData.length);
					if(validSize == 0)
						try {
							Thread.sleep(2);
							continue;
						} catch (InterruptedException e) {
							// TODO Auto-generated catch block
							e.printStackTrace();
						}
					//Log.v(TAG, "writeDateTOFile readsize =  "+validSize);					
					mScreencap.sendAudioData(mScreencap.mAudioData, validSize, ulSamplesPerSec, bitRate, nChannel);
					validSize = 0;
					try {
						Thread.sleep(10);
					} catch (InterruptedException e) {
						// TODO Auto-generated catch block
						e.printStackTrace();
					}				
				}
				else
				{
					try {
						Thread.sleep(500);
					} catch (InterruptedException e) {
						// TODO Auto-generated catch block
						e.printStackTrace();
					}
				}
			}
        }
	}
	
    //private boolean AudioCapture()
	class AudioCapture extends Thread 
    {
		public void run() {
	    	int nBufferLength = 0;
	    	Log.d(TAG,"start audio capture!\n");
	
			//get min buf len
			int minBufferLength = AudioRecord.getMinBufferSize(ulSamplesPerSec, nChannel, bitRate);
	
			//some device can't get min buffer length which means it can't accept current sampling rate, here we give up!
			if(minBufferLength <= 0)
				return;// false;
	
			nBufferLength = minBufferLength;
			Log.d(TAG,"nBufferLength:"+nBufferLength);
			//allocate audio buffer
			if(mScreencap != null && mScreencap.getCapStatus() == 0 && mScreencap.mAudioData == null)
			{
				mScreencap.mAudioData = new byte[nBufferLength];
			}
			//create a recorder
			//WIMO_SPEAKER.../android.media.MediaRecorder.AudioSource.WIMO_SPEAKER
			try{
				audioRecord = new AudioRecord(WIMO_SPEAKER, ulSamplesPerSec, nChannel,
						bitRate, 10*nBufferLength); //we should set buffer to as long as we can to reduce buffer overflow.
				audioRecord.startRecording();
	
			} catch (Exception e) {
		        // TODO Auto-generated catch block
				Log.d(TAG,"new AudioRecord failed\n");
		        e.printStackTrace();
		        //return false;
		    }
				
			bIsRunning = true;
			// Create and start the thread
			Thread thread = new audioThread();
			thread.start();	
			
			//return true;
		}
    }

	@Override
	protected void onDestroy() {
		// TODO Auto-generated method stub
    	Log.d(TAG, "onDestroy");
    	unregisterReceiver(mReceiver);
		super.onDestroy();
		
		if(mIsExit)
		{
			ActivityManager am = (ActivityManager)getSystemService(Context.ACTIVITY_SERVICE);   
			am.restartPackage(getPackageName()); 
			android.os.Process.killProcess(android.os.Process.myPid());
		}
	}

	@Override
    public void onStart() {
    	super.onStart();
    	Log.d(TAG, "onStart");
    	//registerReceiver(mReceiver,mIntentFilter);
    }
    @Override
    public void onStop() {
    	super.onStop();
    	Log.d(TAG, "onStop");
    	//unregisterReceiver(mReceiver);
    }
    private boolean mIsActivityShowing = false;
	@Override
	protected void onResume() {
		// TODO Auto-generated method stub
		super.onResume();
		mIsActivityShowing = true;
		Log.d(TAG, "onResume");
		
		if(mDlgToShow_Id != -1){
			Message msg = new Message();
			msg.what = PMSG_SHOW_SPECIAL_DIALOG;
			msg.arg1 = mDlgToShow_Id;
			mHandler.sendMessage( msg );
		}
	}
	@Override
	protected void onPause() {
		// TODO Auto-generated method stub
		mIsActivityShowing = false;
		Log.d(TAG, "onPause");
		
		super.onPause();
	}

	private void initUI() {
    	mBtnWimo = (Button) findViewById(R.id.btn_wimo);
    	mBtnWimo.setOnClickListener(mBtnWimoClickListener);
    	
    	mBtnWifi = (Button) findViewById(R.id.btn_wifi);
    	mBtnWifi.setOnClickListener(mBtnWifiClickListener);
    	
    	mTvWifi = (TextView) findViewById(R.id.tv_msg1);
    	
    	mSignalView = (ImageView) findViewById(R.id.logo_signal);
    }
	
    private OnClickListener mBtnWimoClickListener = new OnClickListener() {
		@Override
		public void onClick(View v) {
			
			if(mStatus==STATUS_WIMO_CONNECTED)
			{
				if(mHandler.hasMessages(PMSG_AUTO_GOTO_HOME))
					mHandler.removeMessages(PMSG_AUTO_GOTO_HOME);
				exitWimo();
				mStatus = STATUS_WIFI;
				
				mBtnWimo.setText(R.string.string_open_wimo);
				mBtnWimo.setBackgroundResource(R.drawable.button_start);
				mBtnWifi.setEnabled(true);
				
				showToast(R.string.string_wimo_connect_stop);
			}
			else if(mStatus == STATUS_WIFI)
			{
				if(mScreencap != null && mScreencap.getCapStatus() == 0 && mScreencap.checkFb0() ==0){
					//selectDevice();
					if(mScreencap!=null){
						//mScreencap.setSelectDevice(0);
						if(mScreencap.startWiMo() != 0) //开始连接wimo
							return;
						mStatus = STATUS_WIMO_CONNECTING;
						startSignalAnimation();
						//showCiDialog(DIALOG_CONNECT_WIMO);
						cleanToShowCiDialog(DIALOG_CONNECT_WIMO);
					}
				}else if(mCapVideoMode == mScreencap.SW_VIDEO_FRAMEBUFFER){
					cleanToShowCiDialog(DIALOG_OBTAIN_FB0_FAILED);
				}
			}
		}
	};
	
	private OnClickListener mBtnWifiClickListener = new OnClickListener() {
		@Override
		public void onClick(View v) {
			startActivityForResult(new Intent(Settings.ACTION_WIFI_SETTINGS), REQUEST_CODE_WIFI_SETTING);
		}
	};
	
	private void initPopMenu(){
		mCustomPopMenu = new PopMenu(this);
		mCustomPopMenu.setMenuItemClickListener(menuItemlistener);
	};
	
	private PopMenu.onClickMenuItemListener menuItemlistener = new PopMenu.onClickMenuItemListener() {
		
		public void onUpradeClickListener() {
			Intent intent = new Intent();
			intent.setClass(MainActivity.this, UpgradeDialog.class);
			intent.putExtra("appName", mAppName);
			startActivity(intent);
		}
		
		public void onHelpClickListener() {
			Intent intent = new Intent();
			intent.setClass(MainActivity.this, UsageGuiderActivity.class);
			startActivity(intent);
		}

		public void onExitClickListener() {
			cleanToShowCiDialog(DIALOG_EXIT_MESSAGE);
		}
		
		public void onAboutClickListener() {
			Intent intent = new Intent();
			intent.setClass(MainActivity.this, AboutDialog.class);
			intent.putExtra("tVersion", mModelVersion);
			startActivity(intent);
		}

		public void onTvClickListener() {
			// TODO Auto-generated method stub
			Intent intent = new Intent();
			intent.setClass(MainActivity.this, TvDialog.class);
			startActivity(intent);
		}
	};
	
    private void initUIByNetworkInfo()
    {
    	ConnectivityManager conMan = (ConnectivityManager) getSystemService(Context.CONNECTIVITY_SERVICE);
    	State wifi = conMan.getNetworkInfo(ConnectivityManager.TYPE_WIFI).getState();    
    	
    	
    	if(wifi.toString().equals("CONNECTED"))
    	{
    		WifiManager wifiMan = (WifiManager) getSystemService(Context.WIFI_SERVICE);
    		WifiInfo wifiInfo = wifiMan.getConnectionInfo();
    		mCurrentSSID = wifiInfo.getSSID();
    		String msg = getString(R.string.string_connect_to) +" "+ wifiInfo.getSSID();
        	mTvWifi.setText(msg);
        	
        	mBtnWimo.setEnabled(true);
    		mBtnWimo.setText(R.string.string_open_wimo);
    		mBtnWimo.setBackgroundResource(R.drawable.button_start);
    		
    		mBtnWifi.setEnabled(true);

    		mStatus = STATUS_WIFI;
    		
    		Log.d(TAG, "Wifi is connected now!");
    	}
    	else
    	{
    		String msg = getString(R.string.string_connect_to) +" "+ getString(R.string.string_null);
        	mTvWifi.setText(msg);
        	
        	mBtnWimo.setEnabled(false);
    		mBtnWimo.setText(R.string.string_open_wimo);
    		mBtnWimo.setBackgroundResource(R.drawable.button_start);
    		
    		mBtnWifi.setEnabled(true);
        	
    		mStatus = STATUS_NO_WIFI;
    		
    		mHandler.postDelayed(new Runnable() {
				
				@Override
				public void run() {
					// TODO Auto-generated method stub
					cleanToShowCiDialog(DIALOG_CONNECT_WIFI);
				}
			}, 100);
    		Log.d(TAG, "Wifi is not connected!");
    	}
    }
    
    /*private void handleConnectivityChangeAction(Intent intent)
    {
    	//String action = intent.getAction();
    	//if( !action.equals(ConnectivityManager.CONNECTIVITY_ACTION) )
    	//	return;
    	ConnectivityManager conMan = (ConnectivityManager) getSystemService(Context.CONNECTIVITY_SERVICE);
    	State networkState = conMan.getNetworkInfo(ConnectivityManager.TYPE_WIFI).getState();
    	WifiManager wm = (WifiManager) getSystemService(Context.WIFI_SERVICE);
		int wifiState = wm.getWifiState();
		//String currSSID = wm.getConnectionInfo().getSSID();
		//if(currSSID !=null);
		
		if(networkState == State.CONNECTED && wifiState == WifiManager.WIFI_STATE_ENABLED)
		{
			//has connected to some wifi network.
			clearAllInfoDialog();
    		//mGuiderContainers.setVisibility(View.GONE);
    		
    		WifiManager wifiMan = (WifiManager) getSystemService(Context.WIFI_SERVICE);
    		WifiInfo wifiInfo = wifiMan.getConnectionInfo();
    		mCurrentSSID = wifiInfo.getSSID();
    		if(mStatus==STATUS_WIMO_CONNECTED && mCurrentSSID!=null && mUsefulSSID!=null){
    			if(!mCurrentSSID.equals(mUsefulSSID))
    			{
    				exitWimo(); //断开wimo连接
    				
    				mStatus = STATUS_WIFI;
    				
    				showCiDialog(DIALOG_WLAN_CONNECTED_OTHER);
    			}
    		}
    		String msg = getString(R.string.string_connect_to) +" "+ wifiInfo.getSSID();
        	mTvWifi.setText(msg);
        	
        	mBtnWimo.setEnabled(true);
    		mBtnWimo.setText(R.string.string_open_wimo);
    		mBtnWimo.setBackgroundResource(R.drawable.button_start);
    		mBtnWifi.setEnabled(true);
    		//mBtnWifi.setText(R.string.string_switch_network);
        	if(mStatus == STATUS_WIMO_CONNECTING){
        		;
    		}
        	else if (mStatus == STATUS_WIMO_CONNECTED)
    		{
    			mBtnWimo.setText(R.string.string_stop);
        		mBtnWimo.setBackgroundResource(R.drawable.button_stop);
        		mBtnWifi.setEnabled(false);
    		}else{
    			mStatus = STATUS_WIFI;
    		}
		}
		
		
		if(networkState==State.DISCONNECTED
			&&(wifiState==WifiManager.WIFI_STATE_ENABLED || wifiState==WifiManager.WIFI_STATE_DISABLING))
		{
			if(mStatus == STATUS_WIMO_CONNECTED)
    		{
    			exitWimo(); //断开wimo连接
    			Log.d(TAG, "disconnected and then break connected WiMo!");
    		}else if(mStatus == STATUS_WIMO_CONNECTING){
    			// User clicked OK so do some stuff 
            	if(mScreencap!=null)
        			mScreencap.stopWiMo(); //取消wimo连接 cancel
            	stopSignalAnimation();
            	//mStatus = STATUS_WIFI;
    		}else if(mStatus == STATUS_WIFI){
    		}
    		mStatus = STATUS_NO_WIFI;
			if(mWiMoConnNotify != null)
    			mWiMoConnNotify.cancelAll();

			String msg = getString(R.string.string_connect_to) +" "+ getString(R.string.string_null);
        	mTvWifi.setText(msg);
    		mBtnWimo.setEnabled(false);
    		mBtnWimo.setText(R.string.string_open_wimo);
    		mBtnWimo.setBackgroundResource(R.drawable.button_start);
    		mBtnWifi.setEnabled(true);
    		//mBtnWifi.setText(R.string.string_open_wifi);
    		//mGuiderContainers.setVisibility(View.VISIBLE);


			if(wifiState==WifiManager.WIFI_STATE_ENABLED)
			{
				//disconnected the wifi network.
				clearAllInfoDialog();
				showCiDialog(DIALOG_WLAN_LOST);
				Log.d(TAG, "showCiDialog---DIALOG_WLAN_LOST -- "+ wm.isWifiEnabled());
			}
			if(wifiState==WifiManager.WIFI_STATE_DISABLING || wifiState==WifiManager.WIFI_STATE_DISABLED)
			{
				//close wifi switch
				clearAllInfoDialog();
				showCiDialog(DIALOG_WLAN_TURNED_OFF);
				Log.d(TAG, "showCiDialog---DIALOG_WLAN_TURNED_OFF  --  "+wm.isWifiEnabled());

			}
		}
    	
    }*/
    
    public class WIFIReceiver extends BroadcastReceiver {
    	@Override
    	public void onReceive(Context context, Intent intent) {
    		String action = intent.getAction();
    		ConnectivityManager conMan = (ConnectivityManager) getSystemService(Context.CONNECTIVITY_SERVICE);
        	State networkState = conMan.getNetworkInfo(ConnectivityManager.TYPE_WIFI).getState(); 
        	WifiManager wm = (WifiManager) getSystemService(Context.WIFI_SERVICE);
			int wifiState = wm.getWifiState();
			String mssid = wm.getConnectionInfo().getSSID();
			
			String showStr = action+"======"+networkState+"======Wifi_State:"+wifiState;
			if(networkState==State.CONNECTED && mssid!=null){
				showStr = showStr + mssid;
			}
        	Log.i(TAG, showStr);
        	
    		if(action.equals(WifiManager.RSSI_CHANGED_ACTION))
    		{
    			handleRSSIChangedAction();
    		}
    		else if(action.equals(WifiManager.NETWORK_STATE_CHANGED_ACTION))
    		{
    			handleNetworkStateChangedAction(intent);
    		}
    		else if(action.equals(WifiManager.WIFI_STATE_CHANGED_ACTION))
    		{
    			handleWifiStateChangedAction(intent);
    		}
    	}
    }
    
    private void handleNetworkStateChangedAction(Intent intent)
    {
    	WifiManager wm = (WifiManager) getSystemService(Context.WIFI_SERVICE);
    	if(!wm.isWifiEnabled())
    		return;
    	
    	NetworkInfo networkInfo = (NetworkInfo) intent.getParcelableExtra(WifiManager.EXTRA_NETWORK_INFO);
    	NetworkInfo.State networkState = networkInfo.getState();
    	//Log.i(TAG, "NetworkInfo.State==>"+networkState.toString());
    	
    	if(networkState==State.CONNECTED)
    	{
    		clearAllInfoDialog();
    		
    		WifiInfo wifiInfo = wm.getConnectionInfo();
    		mCurrentSSID = wifiInfo.getSSID();
    		if(mStatus==STATUS_WIMO_CONNECTED && mCurrentSSID!=null && mUsefulSSID!=null){
    			if(!mCurrentSSID.equals(mUsefulSSID))
    			{
    				Log.d(TAG, "mUsefulSSID stop wimo....\n");
    				exitWimo();
    				mStatus = STATUS_WIFI;
    				cleanToShowCiDialog(DIALOG_WLAN_CONNECTED_OTHER);
    			}
    		}
    		
    		String msg = getString(R.string.string_connect_to) +" "+ wifiInfo.getSSID();
        	mTvWifi.setText(msg);
        	mBtnWimo.setEnabled(true);
    		mBtnWimo.setText(R.string.string_open_wimo);
    		mBtnWimo.setBackgroundResource(R.drawable.button_start);
    		mBtnWifi.setEnabled(true);
        	if(mStatus == STATUS_WIMO_CONNECTING)
        	{
        		Log.d(TAG, "STATUS_WIMO_CONNECTING stop wimo....\n");
        		if(mScreencap!=null && mScreencap.getCapStatus()==0)
        			mScreencap.stopWiMo();
            	stopSignalAnimation();
            	mStatus = STATUS_WIFI;
            	showToast(R.string.string_connect_failed);
    		}
        	else if (mStatus == STATUS_WIMO_CONNECTED)
    		{
    			mBtnWimo.setText(R.string.string_stop);
        		mBtnWimo.setBackgroundResource(R.drawable.button_stop);
        		mBtnWifi.setEnabled(false);
    		}
        	else
        	{
    			mStatus = STATUS_WIFI;
    		}
    	}
    	else if(networkState==State.DISCONNECTED)
    	{
    		netDisconnected_UpdateUI();
    		
    		//WIFI connectivity has disconnected
    		cleanToShowCiDialog(DIALOG_WLAN_LOST);
    	}
    }
    
    private void handleWifiStateChangedAction(Intent intent)
    {
    	WifiManager wm = (WifiManager) getSystemService(Context.WIFI_SERVICE);
    	int prevWifiState = intent.getIntExtra(WifiManager.EXTRA_PREVIOUS_WIFI_STATE,
                WifiManager.WIFI_STATE_UNKNOWN);
    	int currWifiState = intent.getIntExtra(WifiManager.EXTRA_WIFI_STATE,
                WifiManager.WIFI_STATE_UNKNOWN);
    	
    	//Log.i(TAG, "currWifiState==>"+currWifiState+";prevWifiState==>"+prevWifiState);
    	
    	//WIFI is turned off
    	if(prevWifiState==WifiManager.WIFI_STATE_DISABLING 
    			&& currWifiState==WifiManager.WIFI_STATE_DISABLED)
    	{
    		netDisconnected_UpdateUI();
    		
    		//WIFI switch close
    		cleanToShowCiDialog(DIALOG_WLAN_TURNED_OFF);
    	}
    	
    	else if(prevWifiState==WifiManager.WIFI_STATE_ENABLING
    			&& currWifiState==WifiManager.WIFI_STATE_ENABLED)
    	{
    		clearAllInfoDialog();
    	}
    }
    
    private void netDisconnected_UpdateUI()
    {
    	if(mStatus == STATUS_WIMO_CONNECTED)
		{
    		Log.d(TAG, "netDisconnected_UpdateUI stop wimo....\n");
			exitWimo();
		}
		else if(mStatus == STATUS_WIMO_CONNECTING)
		{
        	if(mScreencap!=null && mScreencap.getCapStatus()==0)
    			mScreencap.stopWiMo();
        	stopSignalAnimation();
		}
		else if(mStatus == STATUS_WIFI)
		{}
		mStatus = STATUS_NO_WIFI;
		if(mWiMoConnNotify != null)
			mWiMoConnNotify.cancelAll();
		
		String msg = getString(R.string.string_connect_to) +" "+ getString(R.string.string_null);
    	mTvWifi.setText(msg);
		mBtnWimo.setEnabled(false);
		mBtnWimo.setText(R.string.string_open_wimo);
		mBtnWimo.setBackgroundResource(R.drawable.button_start);
		mBtnWifi.setEnabled(true);
    }
    
    private void handleRSSIChangedAction()
    {
    	WifiManager wm = (WifiManager) getSystemService(Context.WIFI_SERVICE);
		WifiInfo wInfo = wm.getConnectionInfo();

		int threshold = wInfo.getRssi();
		//Log.d(TAG, "RSSI value is -- "+threshold);
		if(mStatus==STATUS_WIMO_CONNECTED || mStatus==STATUS_WIMO_CONNECTING)
			if(threshold < -85) //the worst
			{
				if(!mIsRssiWeak)
				{
					mHandler.sendEmptyMessageDelayed(PMSG_RSSI_SIGNAL_WEAK, 5000);
					mIsRssiWeak = true;
					/*Toast.makeText(getApplicationContext(), "send PMSG_RSSI_SIGNAL_WEAK",
							Toast.LENGTH_SHORT).show();*/
				}
			} else {
				
				if(mIsRssiWeak){
					mHandler.removeMessages(PMSG_RSSI_SIGNAL_WEAK);
					mIsRssiWeak = false;
					/*Toast.makeText(getApplicationContext(), "remove PMSG_RSSI_SIGNAL_WEAK",
							Toast.LENGTH_SHORT).show();*/
				}
			}
		
    }
    
    

    @Override
	public boolean onKeyDown(int keyCode, KeyEvent event) {
		// TODO Auto-generated method stub
    	
    	if(mCustomPopMenu !=null)
    	{
    		if(keyCode == KeyEvent.KEYCODE_MENU){
    			if( mCustomPopMenu.isShowing() )
    				mCustomPopMenu.dismiss();
    			else
    				mCustomPopMenu.show();
    			return true;
    		}
    		else if(keyCode == KeyEvent.KEYCODE_BACK)
    		{
    			if(mCustomPopMenu.isShowing()) {
    				mCustomPopMenu.dismiss();
    				return true;
    			}
    		}
    	}

        
        if (keyCode == KeyEvent.KEYCODE_BACK && event.getRepeatCount() == 0)
		{
        	//showCiDialog(DIALOG_EXIT_MESSAGE);
        	cleanToShowCiDialog(DIALOG_EXIT_MESSAGE);
			return true;
		}
			
		return super.onKeyDown(keyCode, event);
	}
    
    @Override
	protected void onActivityResult(int requestCode, int resultCode, Intent data) {
		// TODO Auto-generated method stub
		super.onActivityResult(requestCode, resultCode, data);
		
		switch(requestCode)
		{
		case REQUEST_CODE_WIFI_SETTING:
			break;
		case REQUEST_CODE_SEL_DEVICE:
			if (resultCode==Activity.RESULT_OK)
			{
				/*if(mScreencap!=null && mScreencap.getCapStatus()==0){
					mScreencap.setSelectDevice(0);
					mScreencap.setVideoEnable(true); //开始连接wimo
					mStatus = STATUS_WIMO_CONNECTING;
					startSignalAnimation();
					showCiDialog(DIALOG_CONNECT_WIMO);
					
				}*/
			}
			break;
		}
	}
    
 
    protected void showCiDialog(int id) {
    	CiAlertDialog infoDlg = null;
        switch (id) {
        case DIALOG_EXIT_MESSAGE:
        	infoDlg = new CiAlertDialog(MainActivity.this);
        	infoDlg.setTitle(R.string.string_quit);
        	infoDlg.setMessage(R.string.string_quit_info);
        	infoDlg.setPositiveButton(R.string.string_ok, new View.OnClickListener() {

				public void onClick(View v) {
					// TODO Auto-generated method stub
					//User clicked OK so do some stuff               	
                	if(timer!=null){
                		timer.cancel();
                		timer = null;
                	}
                	if(mHandler!=null){
                		mHandler.removeMessages(PMSG_DETECT_TIMER);
                	}
                	if(audioRecord != null)
                	{
                		bIsRunning = false;
                		audioRecord.stop();
                		audioRecord.release();
                		audioRecord = null;
                	}
            		if(mScreencap!=null && mScreencap.getCapStatus()==0)
            		{
            			mScreencap.stopWiMo(); //断开wimo连接
            			mScreencap.jniUnInit();
            			mScreencap = null;
            		}
                	exitWimo();  			
                	
                	mIsExit = true;
                	finish();
				}
            });
        	infoDlg.setNegativeButton(R.string.string_cancel, new View.OnClickListener() {

				@Override
				public void onClick(View v) {
					// TODO Auto-generated method stub
					
				}
                
            });
        	infoDlg.show();
        	break;
        case DIALOG_CONNECT_WIMO:
        	mInfoDlg = new CiAlertDialog(MainActivity.this);
        	mInfoDlg.setMessage(R.string.string_dlg_message);
        	mInfoDlg.setTitle(R.string.string_dlg_title);
        	mInfoDlg.setPositiveButton(R.string.string_cancel, new View.OnClickListener() {

				@Override
				public void onClick(View v) {
					// TODO Auto-generated method stub
					// User clicked OK so do some stuff 
					Log.d(TAG, "DIALOG_CONNECT_WIMO stop wimo....\n");
                	if(mScreencap!=null && mScreencap.getCapStatus()==0)
            			mScreencap.stopWiMo();//cancel
                	stopSignalAnimation();
                	mStatus = STATUS_WIFI;
				}
                 
            });
        	mInfoDlg.setCancelable(false);
        	mInfoDlg.show();
        	Log.d(TAG, "Show dialog ==> DIALOG_CONNECT_WIMO");
        	mInfoDlgMaps.put(DIALOG_CONNECT_WIMO, mInfoDlg);      	
        	break;
        case DIALOG_CONNECT_WIFI:
        	mInfoDlg = new CiAlertDialog(MainActivity.this);
        	mInfoDlg.setTitle(R.string.string_prompt);
        	mInfoDlg.setMessage(R.string.wlan_conn_prompt);
        	mInfoDlg.setPositiveButton(R.string.string_turn_on, new View.OnClickListener() {

				@Override
				public void onClick(View v) {
					// TODO Auto-generated method stub
					startActivityForResult(new Intent(Settings.ACTION_WIFI_SETTINGS), REQUEST_CODE_WIFI_SETTING);
				}
                
            });
        	mInfoDlg.setNegativeButton(R.string.string_cancel, new View.OnClickListener() {
                public void onClick(View v) {

                     //User clicked Cancel so do some stuff 
                }
            });
        	mInfoDlg.show();
        	Log.d(TAG, "Show dialog ==> DIALOG_CONNECT_WIFI");
        	mInfoDlgMaps.put(DIALOG_CONNECT_WIFI, mInfoDlg);
        	break;
        	
        case DIALOG_WLAN_TURNED_OFF:
        	mInfoDlg =  new CiAlertDialog(MainActivity.this);
        	mInfoDlg.setTitle(R.string.string_connect_failed);
        	mInfoDlg.setMessage(R.string.wlan_turned_off);
        	mInfoDlg.setPositiveButton(R.string.string_ok, null);
        	mInfoDlg.show();
        	Log.d(TAG, "Show dialog ==> DIALOG_WLAN_TURNED_OFF");
        	mInfoDlgMaps.put(DIALOG_WLAN_TURNED_OFF, mInfoDlg);
        	break;
        case DIALOG_WLAN_LOST:
        	mInfoDlg = new CiAlertDialog(MainActivity.this);
        	mInfoDlg.setTitle(R.string.string_connect_failed);
        	mInfoDlg.setMessage(R.string.wlan_lost);
        	mInfoDlg.setPositiveButton(R.string.string_ok, null);
        	mInfoDlg.show();
        	Log.d(TAG, "Show dialog ==> DIALOG_WLAN_LOST");
        	mInfoDlgMaps.put(DIALOG_WLAN_LOST, mInfoDlg);
        	break;
        case DIALOG_WLAN_WEAK_SIGNAL:
        	mInfoDlg = new CiAlertDialog(MainActivity.this);
        	mInfoDlg.setTitle(R.string.string_connect_failed);
        	mInfoDlg.setMessage(R.string.wlan_coverage_area_out);
        	mInfoDlg.setPositiveButton(R.string.string_ok, null);
        	mInfoDlg.show();
        	Log.d(TAG, "Show dialog ==> DIALOG_WLAN_WEAK_SIGNAL");
        	mInfoDlgMaps.put(DIALOG_WLAN_WEAK_SIGNAL, mInfoDlg);
        	break;
        case DIALOG_WLAN_CONNECTED_OTHER:
        	mInfoDlg = new CiAlertDialog(MainActivity.this);
        	mInfoDlg.setTitle(R.string.string_connect_failed);
        	mInfoDlg.setMessage(R.string.wlan_connected_others);
        	mInfoDlg.setPositiveButton(R.string.string_ok, null);
        	mInfoDlg.show();
        	Log.d(TAG, "Show dialog ==> DIALOG_WLAN_CONNECTED_OTHER");
        	mInfoDlgMaps.put(DIALOG_WLAN_CONNECTED_OTHER, mInfoDlg);
        	break;
        case DIALOG_WIMO_CONNECTING_TIMEOUT:
        	mInfoDlg = new CiAlertDialog(MainActivity.this);
        	mInfoDlg.setTitle(R.string.wimo_connect_timeout);
        	mInfoDlg.setMessage(R.string.wimo_connect_timeout_info);
        	mInfoDlg.setPositiveButton(R.string.string_ok, null);
        	mInfoDlgMaps.put(DIALOG_WIMO_CONNECTING_TIMEOUT, mInfoDlg);
        	break;
        case DIALOG_WIMO_CONNECT_BREAK:
        	mInfoDlg = new CiAlertDialog(MainActivity.this);
        	mInfoDlg.setTitle(R.string.wimo_connect_break);
        	mInfoDlg.setMessage(R.string.wimo_connect_break);
        	mInfoDlg.setPositiveButton(R.string.string_ok, null);
        	mInfoDlg.show();
        	Log.d(TAG, "Show dialog ==> DIALOG_WIMO_CONNECT_BREAK");
        	mInfoDlgMaps.put(DIALOG_WIMO_CONNECT_BREAK, mInfoDlg);
        	break;
        case DIALOG_WIMO_CONNECT_SUSPEND:
        	mInfoDlg = new CiAlertDialog(MainActivity.this);
        	mInfoDlg.setTitle(R.string.wimo_connect_suspend_title);
        	mInfoDlg.setMessage(R.string.wimo_connect_suspend);
        	mInfoDlg.setPositiveButton(R.string.string_ok, null);
        	mInfoDlg.show();
        	Log.d(TAG, "Show dialog ==> DIALOG_WIMO_CONNECT_SUSPEND");
        	mInfoDlgMaps.put(DIALOG_WIMO_CONNECT_SUSPEND, mInfoDlg);
        	break;
        case DIALOG_NET_ERROR:
        	mInfoDlg = new CiAlertDialog(MainActivity.this);
        	mInfoDlg.setTitle(R.string.string_connect_failed);
        	mInfoDlg.setMessage(R.string.net_error_info);
        	mInfoDlg.setPositiveButton(R.string.string_setting, null);
			mInfoDlg.setNegativeButton(R.string.string_cancel, null);
			 
        	mInfoDlgMaps.put(DIALOG_NET_ERROR, mInfoDlg);
        	break;
        case DIALOG_OBTAIN_FB0_FAILED:
        	mInfoDlg = new CiAlertDialog(MainActivity.this);
        	mInfoDlg.setTitle(R.string.string_prompt);
        	mInfoDlg.setMessage(R.string.string_obtain_fb0_error);
        	mInfoDlg.setPositiveButton(R.string.string_ok, null);
        	mInfoDlg.show();
        	Log.d(TAG, "Show dialog ==> DIALOG_OBTAIN_FB0_FAILED");
        	mInfoDlgMaps.put(DIALOG_OBTAIN_FB0_FAILED, mInfoDlg);
        	break;
        }
        
    
        return ;
    }

	@Override
	protected void onPrepareDialog(int id, Dialog dialog) {
		// TODO Auto-generated method stub
		
		for(Iterator iter = mInfoDlgMaps.entrySet().iterator(); (iter.hasNext() && iter!=null);)
		{
			Map.Entry entry = (Map.Entry) iter.next();
			if(entry!=null && entry.getKey()!=null && entry.getValue()!=null)
			{
				Integer key = (Integer)entry.getKey();
				if(key == id)
					continue;
				
				Dialog dlg = (Dialog) entry.getValue();
				if(dlg.isShowing())
					dlg.dismiss();
			}
		}
		
		super.onPrepareDialog(id, dialog);
	}
	
	private void clearAllInfoDialog(){
		for(Iterator iter = mInfoDlgMaps.entrySet().iterator(); (iter.hasNext() && iter!=null);)
		{
			Map.Entry entry = (Map.Entry) iter.next();
			if(entry!=null && entry.getKey()!=null && entry.getValue()!=null)
			{
				Dialog dlg = (Dialog) entry.getValue();
				if(dlg.isShowing())
					dlg.dismiss();
			}
		}
		
		mDlgToShow_Id = -1;
	}
	
	private void cleanToShowCiDialog(int dlgId)
	{
		clearAllInfoDialog();
		
		if(mIsActivityShowing){
			showCiDialog(dlgId);
		}else{
			mDlgToShow_Id = dlgId;
			Log.i(TAG, "Toppest(unshow) dialog is ==> "+dlgId);
		}
	}

	@Override
	public void onCheckChange(int nOpen) {
		// TODO Auto-generated method stub
		if(nOpen==1)
		{
			Log.d(TAG, "onCheckChange [nOpen : " +nOpen +"], send PMSG_CONN_WIMO_SUCCESS");
			mHandler.sendEmptyMessageDelayed(PMSG_CONN_WIMO_SUCCESS, 0);
		}
		else if(nOpen==0)
		{
			Log.d(TAG, "onCheckChange [nOpen : " +nOpen +"], send PMSG_CONN_WIMO_FAILED");
			mHandler.sendEmptyMessageDelayed(PMSG_CONN_WIMO_FAILED, 0);
		}
		else if(nOpen==2)
		{
			Log.d(TAG, "onCheckChange [nOpen : " +nOpen +"], send PMSG_CONN_WIMO_SUSPEND");
			mHandler.sendEmptyMessageDelayed(PMSG_CONN_WIMO_SUSPEND, 0);
		}
	}
	

    private final BroadcastReceiver mBatInfoReceiver = new BroadcastReceiver() {  
    	@Override  
    	public void onReceive(final Context context, final Intent intent) {  
    		final String action = intent.getAction();  
    		if(Intent.ACTION_SCREEN_ON.equals(action)){  
    			Log.d(TAG, "screen is on...");  
    		}else if(Intent.ACTION_SCREEN_OFF.equals(action)){  
	    	  	 Log.d(TAG, "screen is off...");
    		}  
    	}  
   }; 
   
	
	private final Handler mHandler = new Handler(){
		@Override
        public void handleMessage(Message msg) {   
            switch (msg.what){
                case PMSG_DETECT_TIMER:
                	//Log.d(TAG,"PMSG_DETECT_TIMER");
                	mHandler.removeMessages(PMSG_DETECT_TIMER);
                	
        			if(mScreencap != null && mScreencap.getCapStatus() == 0 && mCapVideoMode == mScreencap.HW_VIDEO_INTERFACE)
        				mHWScreencap.update();
        			if(mStatus == STATUS_WIMO_CONNECTED)
        			{
        				//Log.d("test", "start screen shot....\n");
        				getScreenShot();
        			}

                	break;
                case PMSG_CONN_WIMO_SUCCESS:
                {
                	Log.d(TAG, "PMSG_CONN_WIMO_SUCCESS");
            		//if(mStreamTS!=null)
            		//	mStreamTS.WFDStreamStart();
                	
            		if(objTS != null)
            		{
            			try {
            				Log.d(TAG,"ipAddr:"+mScreencap.ipAddr+" udpPort:"+mScreencap.udpPort);
            				actionStartTS.invoke(objTS, mScreencap.ipAddr, mScreencap.udpPort);
            			} catch (NullPointerException e){
            				// TODO Auto-generated catch block
            				e.printStackTrace();
            			} catch (IllegalArgumentException e) {
            				// TODO Auto-generated catch block
            				e.printStackTrace();
            			} catch (IllegalAccessException e) {
            				// TODO Auto-generated catch block
            				e.printStackTrace();
            			} catch (InvocationTargetException e) {
            				// TODO Auto-generated catch block
            				e.printStackTrace();
            			}		
            		}
            		
                	WifiManager wifiMan = (WifiManager) getSystemService(Context.WIFI_SERVICE);
            		WifiInfo wifiInfo = wifiMan.getConnectionInfo();
            		mUsefulSSID = wifiInfo.getSSID();
            		mCurrentSSID = mUsefulSSID;
                	
                	Dialog dlg = mInfoDlgMaps.get(DIALOG_CONNECT_WIMO);
                	if(dlg != null && dlg.isShowing())
                		dlg.dismiss();
                	//stopSignalAnimation();
                	
                	mBtnWimo.setText(R.string.string_stop);
                	mBtnWimo.setBackgroundResource(R.drawable.button_stop);
                	//Toast.makeText(MainActivity.this,R.string.string_connected,Toast.LENGTH_SHORT).show();
            		showToast(R.string.string_connected);
                	
                	mDisconnectedStatus = mStatus = STATUS_WIMO_CONNECTED;
                	mBtnWifi.setEnabled(false);
                	
                	if(mWiMoConnNotify == null)
                		mWiMoConnNotify = (NotificationManager)getSystemService(NOTIFICATION_SERVICE);
                	mWiMoConnNotify.cancelAll();
                	String tickerText = MainActivity.this.getString(R.string.string_connected);
                	Notification notification = new Notification(
                			R.drawable.icon_wimo_statusbar, tickerText, System.currentTimeMillis() );
                	PendingIntent contentIntent = PendingIntent.getActivity(
                			MainActivity.this, 0, getIntent(), 0 );
                	String notifyInfo = MainActivity.this.getString(R.string.string_wimo_running);
                	notification.setLatestEventInfo(
                			MainActivity.this, "", notifyInfo, contentIntent );
                	mWiMoConnNotify.notify( R.drawable.icon_wimo_statusbar, notification );
                	
                	mHandler.sendEmptyMessageDelayed(PMSG_AUTO_GOTO_HOME, 2000);
                	
                	if(mCapAudioMode == mScreencap.AUDIO_METHOD_LNV)
                	{
                		//AudioCapture();
                		new Thread(new AudioCapture()).start();
                 	}
                }
                	break;
                case PMSG_CONN_WIMO_FAILED:
                {
                	Log.d(TAG, "PMSG_CONN_WIMO_FAILED");
            		//if(mStreamTS!=null && mStatus == STATUS_WIMO_CONNECTED)
            		//	mStreamTS.WFDStreamStop();
                	
            		if(objTS != null && mStatus == STATUS_WIMO_CONNECTED)
            		{
            			try {
            				actionStopTS.invoke(objTS);
            			} catch (NullPointerException e){
            				// TODO Auto-generated catch block
            				e.printStackTrace();
            			} catch (IllegalArgumentException e) {
            				// TODO Auto-generated catch block
            				e.printStackTrace();
            			} catch (IllegalAccessException e) {
            				// TODO Auto-generated catch block
            				e.printStackTrace();
            			} catch (InvocationTargetException e) {
            				// TODO Auto-generated catch block
            				e.printStackTrace();
            			}		
            		}
                	
                	if(audioRecord != null)
                	{
                		bIsRunning = false;
                		audioRecord.stop();
                		audioRecord.release();
                		audioRecord = null;
                	}
                	
                	Dialog dlg = mInfoDlgMaps.get(DIALOG_CONNECT_WIMO);
                	if(dlg != null && dlg.isShowing())
                		dlg.dismiss();
                	stopSignalAnimation();
                	
                	mBtnWimo.setText(R.string.string_open_wimo);
                	mBtnWimo.setBackgroundResource(R.drawable.button_start);
                	if(mDisconnectedStatus==STATUS_WIMO_CONNECTED)
                	{
                		cleanToShowCiDialog(DIALOG_WIMO_CONNECT_BREAK);
                	}
                	else
                	{
                		showToast(R.string.string_connect_failed);
                	}
                	mDisconnectedStatus = mStatus = STATUS_WIFI;
                	mBtnWifi.setEnabled(true);
                }
                	break;
                case PMSG_CONN_WIMO_SUSPEND:
                {
                	//if(mScreencap!=null && mScreencap.getCapStatus()==0)
            		//	mScreencap.stopWiMo(); //断开wimo连接
            		//if(mStreamTS!=null && mStatus == STATUS_WIMO_CONNECTED)
            		//	mStreamTS.WFDStreamStop();
                	
            		if(objTS != null && mStatus == STATUS_WIMO_CONNECTED)
            		{
            			try {
            				actionStopTS.invoke(objTS);
            			} catch (NullPointerException e){
            				// TODO Auto-generated catch block
            				e.printStackTrace();
            			} catch (IllegalArgumentException e) {
            				// TODO Auto-generated catch block
            				e.printStackTrace();
            			} catch (IllegalAccessException e) {
            				// TODO Auto-generated catch block
            				e.printStackTrace();
            			} catch (InvocationTargetException e) {
            				// TODO Auto-generated catch block
            				e.printStackTrace();
            			}		
            		}
            		                	
                	if(audioRecord != null)
                	{
                		bIsRunning = false;
                		audioRecord.stop();
                		audioRecord.release();
                		audioRecord = null;
                	}
                	
                	if(mWiMoConnNotify == null)
                		mWiMoConnNotify = (NotificationManager)getSystemService(NOTIFICATION_SERVICE);
                	mWiMoConnNotify.cancelAll();
                	String tickerText = MainActivity.this.getString(R.string.wimo_connect_suspend_title);
                	Notification notification = new Notification(
                			R.drawable.icon_wimo_statusbar_d, tickerText, System.currentTimeMillis() );
                	PendingIntent contentIntent = PendingIntent.getActivity(
                			MainActivity.this, 0, getIntent(), 0 );
                	String notifyInfo = MainActivity.this.getString(R.string.wimo_connect_suspend);
                	notification.setLatestEventInfo(
                			MainActivity.this, "", notifyInfo, contentIntent );
                	mWiMoConnNotify.notify( R.drawable.icon_wimo_statusbar, notification );
                	
                	
            		if(mStatus == STATUS_WIMO_CONNECTING || mStatus == STATUS_WIMO_CONNECTED)
            			stopSignalAnimation();
    				mStatus = STATUS_WIFI;
    				mBtnWimo.setText(R.string.string_open_wimo);
    				mBtnWimo.setBackgroundResource(R.drawable.button_start);
    				mBtnWifi.setEnabled(true);
    				
    				cleanToShowCiDialog(DIALOG_WIMO_CONNECT_SUSPEND);
                }
                	break;
                case PMSG_AUTO_GOTO_HOME:
                {
                	this.removeMessages(PMSG_AUTO_GOTO_HOME);
                	gotoHomePage();
                }
                	break;
                case PMSG_RSSI_SIGNAL_WEAK:
                {
                	mIsRssiWeak = false;
            		//if(mStreamTS!=null && mStatus == STATUS_WIMO_CONNECTED)
            		//	mStreamTS.WFDStreamStop();
            		
                	if(mStatus == STATUS_WIMO_CONNECTED)
            		{
						Log.d(TAG, "PMSG_RSSI_SIGNAL_WEAK stop wimo....\n");
                		if(objTS != null)
                		{
                			try {
                				actionStopTS.invoke(objTS);
                			} catch (NullPointerException e){
                				// TODO Auto-generated catch block
                				e.printStackTrace();
                			} catch (IllegalArgumentException e) {
                				// TODO Auto-generated catch block
                				e.printStackTrace();
                			} catch (IllegalAccessException e) {
                				// TODO Auto-generated catch block
                				e.printStackTrace();
                			} catch (InvocationTargetException e) {
                				// TODO Auto-generated catch block
                				e.printStackTrace();
                			}		
                		}
                		
            			exitWimo();
            			mUsefulSSID = "";
            			mBtnWimo.setEnabled(true);
            			mBtnWimo.setText(R.string.string_open_wimo);
                		mBtnWimo.setBackgroundResource(R.drawable.button_start);
                		mBtnWifi.setEnabled(true);
                		
                		cleanToShowCiDialog(DIALOG_WLAN_WEAK_SIGNAL);
            			
            			mStatus = STATUS_WIFI;
            		}
                	else if(mStatus == STATUS_WIMO_CONNECTING)
            		{
                    	if(mScreencap!=null && mScreencap.getCapStatus()==0)
                			mScreencap.stopWiMo();
                    	stopSignalAnimation();
                    	
                    	cleanToShowCiDialog(DIALOG_WLAN_WEAK_SIGNAL);
                    	
                    	mStatus = STATUS_WIFI;
            		}
                }
                	break;
                case PMSG_SHOW_SPECIAL_DIALOG:
                {
                	int dlgid = msg.arg1;
                	cleanToShowCiDialog(dlgid);
                }
                	break;
                	
                default:
                    break;
            }
	
		}
	};
	

	private void selectDevice()
	{
		Intent intent = new Intent();
		intent.setClass(MainActivity.this, SelectDeviceDialog.class);
		startActivityForResult(intent, REQUEST_CODE_SEL_DEVICE);
	}

	private void startSignalAnimation()
	{
		if(mSignalView != null)
		{
			mSignalView.setBackgroundResource(R.anim.signal_connectting);
			AnimationDrawable ad = (AnimationDrawable)mSignalView.getBackground();
			if( ad != null && !ad.isRunning() )
			{
				ad.start();
				Log.d(TAG, "start signal animation");
			}
		}
	}
	
	private void stopSignalAnimation()
	{
		if(mSignalView != null)
		{
			Drawable d = mSignalView.getBackground();
			if( d instanceof AnimationDrawable )
			{
				AnimationDrawable ad = (AnimationDrawable)mSignalView.getBackground();
				if(ad != null && ad.isRunning() )
				{
					ad.stop();
					Log.d(TAG, "stop signal animation");
				}
			}
			
			mSignalView.setBackgroundResource(R.drawable.logo_signal4);
		}
	}
	
	private void gotoHomePage()
	{
		Log.d(TAG, "Activity is going to Home page.");
		Intent intent = new Intent(Intent.ACTION_MAIN);
    	intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
    	intent.addCategory(Intent.CATEGORY_HOME);
    	this.startActivity(intent);
	}
	
	private void exitWimo(){
		//if(mStreamTS!=null && mStatus == STATUS_WIMO_CONNECTED)
		//	mStreamTS.WFDStreamStop();
		if(objTS != null && mStatus == STATUS_WIMO_CONNECTED)
		{
			try {
				actionStopTS.invoke(objTS);
			} catch (NullPointerException e){
				// TODO Auto-generated catch block
				e.printStackTrace();
			} catch (IllegalArgumentException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			} catch (IllegalAccessException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			} catch (InvocationTargetException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}		
		}
    	if(audioRecord != null)
    	{
    		bIsRunning = false;
    		audioRecord.stop();
    		audioRecord.release();
    		audioRecord = null;
    	}
		if(mScreencap!=null && mScreencap.getCapStatus()==0)
		{
			Log.d(TAG, "stop wimo and not quit!\n");
			mScreencap.stopWiMo();			
		}

		if(mWiMoConnNotify != null)
			mWiMoConnNotify.cancelAll();
		
		if(mStatus == STATUS_WIMO_CONNECTING || mStatus == STATUS_WIMO_CONNECTED)
			stopSignalAnimation();
	}
	
	private boolean IsNetWorkConnected()
	{
		ConnectivityManager conMan = (ConnectivityManager) getSystemService(Context.CONNECTIVITY_SERVICE);
    	State wifi = conMan.getNetworkInfo(ConnectivityManager.TYPE_WIFI).getState();
    	
    	Log.d(TAG, "The Wifi state is : "+ wifi.toString());
    	
    	return wifi.toString().equals("CONNECTED");
	}
	
	private boolean isFirstRun()
	{
		SharedPreferences sysPref = MainActivity.this.getApplicationContext().getSharedPreferences("sys_param", 0);
		return sysPref.getBoolean("is_firstrun", true);
	}
	
	public void saveFirstRun(boolean isFirst)
	{
		SharedPreferences.Editor editor;
		try {
			SharedPreferences sysPref = MainActivity.this.getApplicationContext().getSharedPreferences("sys_param", 0);
			editor = sysPref.edit();
			editor.putBoolean("is_firstrun", isFirst);
			editor.commit();
		} catch (Exception e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}
	
	private void showToast(int stringId){
		Toast toast = Toast.makeText(MainActivity.this, stringId, Toast.LENGTH_SHORT);
		toast.setGravity(Gravity.CENTER, 0, 0);
		toast.show();
	}
	
}
