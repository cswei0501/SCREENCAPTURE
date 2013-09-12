package cidana.wimo;

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
import android.os.Environment;
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
import com.cmcc.wimo.WiMoControl;
import com.cmcc.wimo.WiMoControl.WimoControlCallback;

import dalvik.system.DexClassLoader;

public class MainActivity extends Activity {
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
	
	private CaptureContainer mCapContainer = null;
	private ScreenCap mHWScreencap = null;
	private Encoder mHWEncoder = null;
	private AudioCap mHWAudioCap = null;
	private AudioRecord audioRecord = null;
	private WiMoControl mWimoControl = null;
	
	private static int
	mSDKNum = 15,
	mSelectLibs = 0;
	
	private static String
	mLenovo_Model = "Lenovo S899t";
		
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

	public byte[] mEncodedBuf = null;
	public byte[] mAudioData = null;
	
	private int
	mWimo1 = 1,
	mWimo2 = 2,
	mWimoGame = 3;
	
	private int 
	mCapVideoMode = -1,
	mCapAudioMode = -1;

	private final int 
	mPmsgType = 0,
	mDialogType = 1;
	
	private final int
	HW_VIDEO_INTERFACE = 0,
	SW_VIDEO_FRAMEBUFFER = 1,
	SW_VIDEO_SYSTEM = 2,
	TS_STREAM_INTERFACE = 3,
	LNV_VIDEO_INTERFACE = 4,
	HS_VIDEO_INTERFACE = 5,
	ZTE_VIDEO_INTERFACE = 6,
	GAME_VIDEO_INTERFACE = 7;

	private final int
	AUDIO_METHOD_DEFAULT = 0,
	AUDIO_METHOD_SPEAKER = 1,
	AUDIO_METHOD_TS = 2,
	AUDIO_METHOD_LNV = 3,
	AUDIO_METHOD_HS = 4,
	AUDIO_METHOD_ZTE = 5,
	AUDIO_METHOD_GAME = 6;
	
	public static final int
	CURRENT_TV_ROTATE_0 = 0,
	CURRENT_TV_ROTATE_90 = 1,//CW:clockwise
	CURRENT_TV_ROTATE_270 = 2;//CCW:anticlockwise
	
	private final int
	ROTATION_0 = 0,
	ROTATION_90 = 90,
	ROTATION_180 = 180,
	ROTATION_270 = 270;
	
	private String 
	mModelVersion = null,
	mAppName = null;
		
	private final int
	RGB565 = 0,
	ARGB4444 = 1,
	ARGB8888 = 2,
	YUV420 = 3,
	YUV422 = 4,
	JPEG = 5,
	TS = 6;
	
	private final int
	PCM = 0,
	AAC = 1;
	
	private final int 
	mStop = 0,
	mExit = 1;
	
	private static final int
	ROTATE_0 = 0,
	ROTATE_90 = 90,
	ROTATE_180 = 180,
	ROTATE_270 =270;

	public static final int
	mJPEG = 0,
	mH264 = 1,
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
	PMSG_WIMO_FAILED = 0,
	PMSG_WIMO_SUCCESS = 1,
	PMSG_WIMO_SUSPEND = 2;
	
	private final int 
    // play relate message
	PMSG_DETECT_TIMER = 1,
	PMSG_CONN_WIMO_SUCCESS = 2,
	PMSG_CONN_WIMO_FAILED = 3,
	PMSG_CONN_WIMO_SUSPEND = 4,
	PMSG_AUTO_GOTO_HOME = 10,
	PMSG_RSSI_SIGNAL_WEAK = 11,
	PMSG_SHOW_SPECIAL_DIALOG = 12;
	
	private int mDlgToShow_Id = -1;
	private static final int 
	DIALOG_EXIT_MESSAGE = 1,
	DIALOG_CONNECT_WIMO = 2,
	DIALOG_CONNECT_WIFI = 3,
	DIALOG_WLAN_TURNED_OFF = 10,
	DIALOG_WLAN_LOST = 11,
	DIALOG_WLAN_WEAK_SIGNAL = 12,
	DIALOG_WLAN_CONNECTED_OTHER = 13,
	DIALOG_WLAN_CONNECTED_EXCHANGE_WIMO_BUTTON = 14,
	DIALOG_WIMO_CONNECTING_TIMEOUT = 20,
	DIALOG_WIMO_CONNECT_BREAK = 21,
	DIALOG_WIMO_CONNECT_SUSPEND = 22,
	DIALOG_OBTAIN_FB0_FAILED = 23,
	DIALOG_CLEAR_INFO_ALL = 29,
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
 		mHWScreencap = mCapContainer.getHWScrnCap();//new ScreenCap();
		mHWEncoder = mCapContainer.getHWEncoder();//new Encoder(mJPEG);
		mWimoControl = new WiMoControl(MainActivity.this, mWimo1);
		
		if((Build.MODEL.equals(mLenovo_Model)) && mSDKNum == VERSION.SDK_INT)
		{
			WIMO_SPEAKER = 9;
			ulSamplesPerSec = 48000;
			mCapAudioMode = AUDIO_METHOD_LNV;
		}
		else if(mHWScreencap!=null && mHWEncoder!=null && 0==mHWScreencap.getCapLibStatus() && 0==mHWEncoder.getEncLibStatus())
		{
			ret = mHWScreencap.update();
			if(ret == SUCCESS)
			{
				mCapVideoMode = HW_VIDEO_INTERFACE;

				mRGBbuf = new byte[BUFFERSIZE];
				mEncodedBuf = new byte[BUFFERSIZE];
				
				mFormat = mHWScreencap.getFormat();
				mWidth = mHWScreencap.getWidth();
				mHeight = mHWScreencap.getHeight();
				mStride = mHWScreencap.getStride();
				mSize = mHWScreencap.getSize();

				Log.d(TAG,"mFormat:"+mFormat+",mWidth:"+mWidth+",mHeight:"+mHeight+",mStride:"+mStride+"mSize:"+mSize);
			}
		}
	
		if(mCapAudioMode != AUDIO_METHOD_LNV)
		{
			mHWAudioCap = new AudioCap();
			if(mHWAudioCap!=null && 0==mHWAudioCap.getAudCapStatus())
			{
				Thread thread = new AudioCapture();
				thread.start();
				//if(AudioCapture())
				mCapAudioMode = AUDIO_METHOD_DEFAULT;			
			}	
		}

		Integer retCh = 1;
		if(mWimoControl != null)
			retCh = mWimoControl.CheckFb0();
		
 		Log.d(TAG,"retCh: "+retCh.intValue());
 		if(retCh.intValue()!=0)
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
			WiMoControl.SetTVRotate(CURRENT_TV_ROTATE_0);
			break;
		case 1:
			WiMoControl.SetTVRotate(CURRENT_TV_ROTATE_90);
			break;
		case 2:
			WiMoControl.SetTVRotate(CURRENT_TV_ROTATE_270);
			break;
		} 	

    	mWimoControl.addCallback(this, mWimoCallback);
     	timer = new Timer();
     	timer.schedule(mRefTimeTask, 20, 20);
     	 
     	initUI();
        initUIByNetworkInfo();
        initPopMenu();
     
     	final IntentFilter filter = new IntentFilter();
     	filter.addAction(Intent.ACTION_SCREEN_OFF);  
     	filter.addAction(Intent.ACTION_SCREEN_ON);
    }
	
	public WimoControlCallback mWimoCallback = new WimoControlCallback(){
	 	public int controlCallback(int nVal, int nType) {
			// TODO Auto-generated method stub
	 		if(nType == mPmsgType){
				if(nVal==PMSG_WIMO_SUCCESS)
				{
					Log.d(TAG, "controlCallback [nOpen : " +nVal+ "], send PMSG_CONN_WIMO_SUCCESS");
					mHandler.sendEmptyMessageDelayed(PMSG_CONN_WIMO_SUCCESS, 0);
				}
				else if(nVal==PMSG_WIMO_FAILED)
				{
					Log.d(TAG, "controlCallback [nOpen : " +nVal+ "], send PMSG_CONN_WIMO_FAILED");
					mHandler.sendEmptyMessageDelayed(PMSG_CONN_WIMO_FAILED, 0);
				}
				else if(nVal==PMSG_WIMO_SUSPEND)
				{
					Log.d(TAG, "controlCallback [nOpen : " +nVal+ "], send PMSG_CONN_WIMO_SUSPEND");
					mHandler.sendEmptyMessageDelayed(PMSG_CONN_WIMO_SUSPEND, 0);
				}else if(nVal==PMSG_RSSI_SIGNAL_WEAK){
					Log.d(TAG, "controlCallback [nOpen : " +nVal+ "], send PMSG_RSSI_SIGNAL_WEAK");
					mHandler.sendEmptyMessageDelayed(PMSG_RSSI_SIGNAL_WEAK, 0);
				}	
	 		}else if(nType == mDialogType){
	 			if(nVal==DIALOG_CLEAR_INFO_ALL){
	 				Log.d(TAG, "controlCallback [nOpen : " +nVal+ "], send DIALOG_CLEAR_INFO_ALL");
	 				clearAllInfoDialog();
	 			}else if(nVal==DIALOG_WLAN_TURNED_OFF){
	 				Log.d(TAG, "controlCallback [nOpen : " +nVal+ "], send DIALOG_WLAN_TURNED_OFF");
	 				netDisconnected_UpdateUI();
	 				cleanToShowCiDialog(DIALOG_WLAN_TURNED_OFF);
	 			}else if(nVal==DIALOG_WLAN_LOST){
	 				Log.d(TAG, "controlCallback [nOpen : " +nVal+ "], send DIALOG_WLAN_LOST");
	 				netDisconnected_UpdateUI();
	 				cleanToShowCiDialog(DIALOG_WLAN_LOST);
	 			}else if(nVal==DIALOG_WLAN_CONNECTED_OTHER){
	 				Log.d(TAG, "controlCallback [nOpen : " +nVal+ "], send DIALOG_WLAN_CONNECTED_OTHER");
    				exitWimo();
    				mStatus = STATUS_WIFI;
    				cleanToShowCiDialog(DIALOG_WLAN_CONNECTED_OTHER);
	 			}else if(nVal==	DIALOG_WLAN_CONNECTED_EXCHANGE_WIMO_BUTTON){
	 				Log.d(TAG, "controlCallback [nOpen : " +nVal+ "], send DIALOG_WLAN_CONNECTED_EXCHANGE_WIMO_BUTTON");
	 				changeWimoButton();
	 			}
	 		}
			return 0;
		}

		private void changeWimoButton() {
			// TODO Auto-generated method stub
          	WifiManager wifiMan = (WifiManager) getSystemService(Context.WIFI_SERVICE);
    		WifiInfo wifiInfo = wifiMan.getConnectionInfo();  		
    		String msg = getString(R.string.string_connect_to) +" "+ wifiInfo.getSSID();
        	mTvWifi.setText(msg);
        	mBtnWimo.setEnabled(true);
    		mBtnWimo.setText(R.string.string_open_wimo);
    		mBtnWimo.setBackgroundResource(R.drawable.button_start);
    		mBtnWifi.setEnabled(true);
        	if(mStatus == STATUS_WIMO_CONNECTING)
        	{
        		Log.d(TAG, "STATUS_WIMO_CONNECTING stop wimo....\n");
        		//if(mScreencap!=null && mScreencap.getCapStatus()==0)
        		//	mScreencap.stopWiMo();
           		if(mWimoControl!=null)
           			mWimoControl.WiMoStop(mStop);
           		
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
	};
    
	// This class extends Thread
	class audioThread extends Thread {
        // This method is called when the thread runs
        public void run() {
			android.os.Process.setThreadPriority(android.os.Process.THREAD_PRIORITY_URGENT_AUDIO);
			Log.d(TAG,"HW audio capture success!\n");
			int validSize;

			//while(mScreencap!=null && mScreencap.getCapStatus() == 0 && bIsRunning)
			while(bIsRunning)
			{
				if(mStatus==STATUS_WIMO_CONNECTED && audioRecord != null)
				{
					validSize = audioRecord.read(mAudioData, 0, mAudioData.length);
					if(validSize == 0)
						try {
							Thread.sleep(2);
							continue;
						} catch (InterruptedException e) {
							// TODO Auto-generated catch block
							e.printStackTrace();
						}
					//Log.v(TAG, "writeDateTOFile readsize =  "+validSize);	
					mWimoControl.SetAudioBuffer(PCM, mAudioData, validSize, ulSamplesPerSec, bitRate, nChannel);
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
			//if(mScreencap != null && mScreencap.getCapStatus() == 0 && mScreencap.mAudioData == null)
			if(mAudioData == null)
			{
				mAudioData = new byte[nBufferLength];
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
				if(mWimoControl != null && mWimoControl.CheckFb0() ==0){
					//selectDevice();
					//mScreencap.setSelectDevice(0);

					//if(mScreencap.startWiMo() != 0) //开始连接wimo
					//	return;
            		mWimoControl.WiMoStart();
					mStatus = STATUS_WIMO_CONNECTING;
					startSignalAnimation();
					//showCiDialog(DIALOG_CONNECT_WIMO);
					cleanToShowCiDialog(DIALOG_CONNECT_WIMO);
				}else if(mCapVideoMode == SW_VIDEO_FRAMEBUFFER){
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
			//intent.putExtra("appName", mAppName);
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
			//intent.putExtra("tVersion", mModelVersion);
			startActivity(intent);
		}

		public void onTvClickListener() {
			// TODO Auto-generated method stub
			Intent intent = new Intent();
			intent.setClass(MainActivity.this, TvDialog.class);
			startActivity(intent);
		}
	};
	
    private void netDisconnected_UpdateUI()
    {
    	if(mStatus == STATUS_WIMO_CONNECTED)
		{
    		Log.d(TAG, "netDisconnected_UpdateUI stop wimo....\n");
			exitWimo();
		}
		else if(mStatus == STATUS_WIMO_CONNECTING)
		{
        	//if(mScreencap!=null && mScreencap.getCapStatus()==0)
    		//	mScreencap.stopWiMo();

       		if(mWimoControl!=null)
       			mWimoControl.WiMoStop(mStop);
       		
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
                	/*
            		if(mScreencap!=null && mScreencap.getCapStatus()==0)
            		{
            			mScreencap.stopWiMo(); //断开wimo连接
            			mScreencap.jniUnInit();
            			mScreencap = null;
            		}*/
               		if(mWimoControl!=null)
            		{
            			mWimoControl.WiMoStop(mExit);
            			mWimoControl = null;
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
                	//if(mScreencap!=null && mScreencap.getCapStatus()==0)
            		//	mScreencap.stopWiMo();//cancel
               		if(mWimoControl!=null && mStatus==STATUS_WIMO_CONNECTING) //cancel
               			mWimoControl.WiMoStop(mStop);
               		
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

                	if(mCapVideoMode == HW_VIDEO_INTERFACE)
        				mHWScreencap.update();
                	break;
                case PMSG_CONN_WIMO_SUCCESS:
                {
                	Log.d(TAG, "PMSG_CONN_WIMO_SUCCESS");
            		
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
                	
                	if(mCapAudioMode == AUDIO_METHOD_LNV)
                	{
                		//AudioCapture();
                		new Thread(new AudioCapture()).start();
                 	}
                }
                	break;
                case PMSG_CONN_WIMO_FAILED:
                {
                	Log.d(TAG, "PMSG_CONN_WIMO_FAILED");
               		if(mWimoControl!=null && mStatus==STATUS_WIMO_CONNECTED)
               			mWimoControl.WiMoStop(mStop);
               		
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
                	if(mStatus == STATUS_WIMO_CONNECTED)
            		{
						Log.d(TAG, "PMSG_RSSI_SIGNAL_WEAK stop wimo....\n");                		
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
                   		if(mWimoControl!=null)
                   			mWimoControl.WiMoStop(mStop);             		
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
    	if(audioRecord != null)
    	{
    		bIsRunning = false;
    		audioRecord.stop();
    		audioRecord.release();
    		audioRecord = null;
    	}

   		if(mWimoControl!=null && (mStatus==STATUS_WIMO_CONNECTED || mStatus==STATUS_WIMO_CONNECTING))
		{
			Log.d(TAG, "stop wimo and not quit!\n");
   			mWimoControl.WiMoStop(mStop);
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
