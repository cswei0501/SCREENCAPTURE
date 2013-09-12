package com.cmcc.wimo;

import java.io.File;
import java.lang.reflect.Constructor;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.Timer;
import java.util.TimerTask;

import dalvik.system.DexClassLoader;
import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.NetworkInfo.State;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.Surface;

import com.cmcc.wimo.WiMoInterface.WimoControlCallbackPort;
import com.cmcc.wimo.WiMoCapture.WimoCheckListener;

public class WiMoPortForSDK {
	private static final String TAG = "WiMoPortForSDK";
	
	private Context mContext = null;
	private Object objTS = null;
	private Method 
		actionStartTS = null,
		actionStopTS = null;
	
	private AutoCheckLib mAutoChecklib = null;
	private WIFIReceiver mReceiver = null;
	private WimoControlCallbackPort mControlCallback = null;
	private WiMoCapture mWimoCapture = null;	
	private IntentFilter mIntentFilter = null;
	
	private final int 
	    // play relate message
	    STATUS_WIMO_READY = 1,
	    STATUS_WIMO_CONNECTING = 2,
	    STATUS_WIMO_CONNECTED = 3;
	
	private int 
		mStatus = STATUS_WIMO_READY;
	
	private int
		mWimo1 = 1,
		mWimo2 = 2,
		mWimoGame = 3;
	
	private String mUsefulSSID;
	private String mCurrentSSID;
		
	private final int 
		mPmsgType = 0,
		mDialogType = 1;

	private boolean mIsRssiWeak = false;
	
	private int 
		mCapVideoMode = -1,
		mCapAudioMode = -1;
	
	private final int
		PMSG_WIMO_FAILED = 0,
		PMSG_WIMO_SUCCESS = 1,
		PMSG_WIMO_SUSPEND = 2,
		PMSG_WIMO2_RUNNING = 9;

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
	
	private final String 
		mWimoPath = "/mnt/sdcard/wimo/";

	
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
	
	private int 
		mScrnOrient = Surface.ROTATION_0,
		mScrnOrientPrev = -1;
	
	private Timer timer = null;
	private TimerTask mRefTimeTask = new TimerTask(){   
        public void run() {   
            Message message = new Message();       
            message.what = PMSG_DETECT_TIMER;       
            mHandler.sendMessage(message);
        }      
    };
    
	/** constructor for wimo1.0, wimo2.0, game
	* mContext: object
	* mode:
	*	mWimo1 = 1,
	*	mWimo2 = 2,
	*	mWimoGame = 3;
	*/
	public WiMoPortForSDK(Context context, int mode)
	{
		mControlCallback = null;
		mContext = context;
		mStatus = STATUS_WIMO_READY;
		
		mWimoCapture = new WiMoCapture(mContext);
		mAutoChecklib = new AutoCheckLib();

		if(mode == mWimo1 && mAutoChecklib != null)
		{
			Log.d(TAG, "Running wimo1.0 interfaece!\n");
			if(AutoCheckLib.SW_INTERFACE_LIB == mAutoChecklib.getCapLibStatus())
			{
				mCapVideoMode = SW_VIDEO_FRAMEBUFFER;		
			}
			else if(AutoCheckLib.LENOVO_INTERFACE_LIB == mAutoChecklib.getCapLibStatus())
			{
				mCapVideoMode = LNV_VIDEO_INTERFACE;
				mCapAudioMode = AUDIO_METHOD_LNV;
			}
			else if(AutoCheckLib.HISENSE_INTERFACE_LIB == mAutoChecklib.getCapLibStatus())
			{
				mCapVideoMode = HS_VIDEO_INTERFACE;
				mCapAudioMode = AUDIO_METHOD_HS;	
			}
			else if(AutoCheckLib.ZTE_INTERFACE_LIB == mAutoChecklib.getCapLibStatus())
			{
				mCapVideoMode = ZTE_VIDEO_INTERFACE;
				mCapAudioMode = AUDIO_METHOD_ZTE;
			}
		}
		else if(mode == mWimo2)
		{
			Log.d(TAG, "Running wimo2.0 interfaece!\n");
			checkJARforWFD();
			if(objTS != null)
			{
				mCapVideoMode = mWimoCapture.TS_STREAM_INTERFACE;
				mCapAudioMode = mWimoCapture.AUDIO_METHOD_TS;	
			}else
				Log.d(TAG, "dynamic load wimo2.0 lib failed!\n");
		}
		else if(mode == mWimoGame)
		{
			Log.d(TAG, "Running wimo game interfaece!\n");
			mCapVideoMode = mWimoCapture.GAME_VIDEO_INTERFACE;
			mCapAudioMode = mWimoCapture.AUDIO_METHOD_GAME;
		}
       
		initControlPort(mCapVideoMode, mCapAudioMode);
     	timer = new Timer();
     	timer.schedule(mRefTimeTask, 20, 20);

        mIntentFilter = new IntentFilter();
        //mIntentFilter.addAction(ConnectivityManager.CONNECTIVITY_ACTION); 
        mIntentFilter.addAction(WifiManager.NETWORK_IDS_CHANGED_ACTION);
        mIntentFilter.addAction(WifiManager.NETWORK_STATE_CHANGED_ACTION);
        mIntentFilter.addAction(WifiManager.RSSI_CHANGED_ACTION);
        mIntentFilter.addAction(WifiManager.WIFI_STATE_CHANGED_ACTION);
        mReceiver = (WIFIReceiver) new WIFIReceiver();

        ((Activity)mContext).registerReceiver(mReceiver,mIntentFilter);
    }
	
	/*for only wimo1.0*/
	public void initControlPort(int videoMode, int audioMode) 
	{
		// TODO Auto-generated method stub	
		if(mWimoCapture != null)
			mWimoCapture.cValPointer = mWimoCapture.jniInit(videoMode, audioMode);
	}

	/*for only wimo1.0*/
	public void unInitControlPort() 
	{
		// TODO Auto-generated method stub	
		if(mWimoCapture != null)
			mWimoCapture.jniUnInit();
		
		((Activity)mContext).unregisterReceiver(mReceiver);
	}
	
	/*for only wimo1.0*/
	public int CheckFb0Port()
	{
		int ret = -1;
		if(mWimoCapture != null)
			ret = mWimoCapture.checkFb0();
		
		return ret;
	}
	
	/** for wimo1.0, wimo2.0, game
	* start WiMo
	*/
	public void WiMoStartPort()
	{
		Log.d(TAG, "WiMoStartPort......\n");
		if(mWimoCapture != null){
			mStatus = STATUS_WIMO_CONNECTING;
			mWimoCapture.startWiMo();
		}
	}
	
	/** for wimo1.0, wimo2.0, game
	* stop WiMo
	*/
	public void WiMoStopPort()
	{ 
		Log.d(TAG, "WiMoStopPort......\n");
		if(mWimoCapture!=null && (mStatus==STATUS_WIMO_CONNECTED || mStatus==STATUS_WIMO_CONNECTING))
			mWimoCapture.stopWiMo();
	}
	
	/** for wimo1.0, game
	* set video data to SDK,
	* dataTpye: video¡¯s data type
	*	public final int
	*		RGB565 = 0,
	*		ARGB4444 = 1,
	*		ARGB8888 = 2,
	*		YUV420 = 3,
	*		YUV422 = 4,
	*		JPEG = 5,
	*		TS = 6;
	* buff_in: video data
	* buffSize: video data size
	* width: screen width
	* height: screen height
	* rotate: the screen rotate degree
	*	public final int
	*		ROTATION_0 = 0,
	*		ROTATION_90 = 90,
	*		ROTATION_180 = 180,
	*		ROTATION_270 = 270;
	*/
	public void SetVideoBufferPort(int dataType, byte[] buff_in, int buffSize, int width, int height, int rotate)
	{
		//Log.d(TAG, "SetVideoBufferPort......\n");
		if(mWimoCapture != null)
			mWimoCapture.sendVideoData(dataType, buff_in, buffSize, width, height, rotate);
	}
	
	/** for wimo1.0, game
	* dataType: audio data type,
	*	public final int
	*		PCM = 0,
	*		AAC = 1
	* buff_in: audio data
	* buffSize: audio data size
	* sampleRate: audio sample rate
	* bitrate: audio bit rate
	* channels: channel number
	*/
	public void SetAudioBufferPort(int dataType, byte[] buff_in, int buffSize, int sampleRate, int bitRate, int channels)
	{
		//Log.d(TAG, "SetAudioBufferPort......\n");
		if(mWimoCapture != null && buff_in != null)
			mWimoCapture.sendAudioData(dataType, buff_in, buffSize, sampleRate, bitRate, channels);
	}
	
	/** for wimo1.0
	* set TV rotate degree, it only for wimo1.0
	* nTVRotate: 
	*	public final int
	*		CURRENT_TV_ROTATE_0 = 0,
	*		CURRENT_TV_ROTATE_90 = 1,//CW:clockwise
	*		CURRENT_TV_ROTATE_270 = 2;//CCW:anticlockwise
	*/
	public void SetTVRotatePort(int nTVRotate)
	{
		Log.d(TAG, "SetTVRotatePort......\n");
		if(mWimoCapture != null)
			mWimoCapture.setTVRotate(nTVRotate);
	}
	
	/** for wimo1.0, wimo2.0, game
	* set video rotate
	* nRotate: 
	*	public final int
	*		ROTATION_0 = 0,
	*		ROTATION_90 = 90,
	*		ROTATION_180 = 180,
	*		ROTATION_270 = 270;
	*/
	public void SetVideoRotatePort(int nRotate)
	{
		//Log.d(TAG, "SetVideoRotatePort......\n");
		if(mWimoCapture != null)
			mWimoCapture.setVideoRotate(nRotate);
	}
	
	/**
	* register callback function
	*/
	public void addCallbackPort(Context context,WimoControlCallbackPort callback)
	{
		Log.d(TAG, "addCallbackPort......\n");
		mControlCallback = callback;
		if(mWimoCapture!=null){
			mWimoCapture.setWimoCheckListener(context, mWimoCheckListener);
		}
	}

	public WimoCheckListener mWimoCheckListener = new WimoCheckListener() {
		
		@Override
		public void onCheckChange(int nType) {
			// TODO Auto-generated method stub
			/*nOpen: 
			 * 0: failed,
			 * 1: successfully,
			 * 2: suspend,
			 * 9: wimo2.0, changed ip and port
			 * */
			Log.d(TAG, "entry onCheckChange nType: "+nType);
			if(mControlCallback !=null)
			{
    			try {
    				if(nType == PMSG_WIMO_FAILED || nType == PMSG_WIMO_SUSPEND)
    				{//sdk error
    					mStatus = STATUS_WIMO_READY;
    					if(objTS != null && actionStopTS != null)
    						actionStopTS.invoke(objTS);
    					Log.d(TAG, "wimo stopped or suspended!\n");
    				}
    				else if(nType == PMSG_WIMO2_RUNNING)
    				{//sdk successfully
    					mStatus = STATUS_WIMO_CONNECTED;
    					
        				Log.d(TAG,"ipAddr:"+mWimoCapture.ipAddr+" udpPort:"+mWimoCapture.udpPort);
        				if(objTS != null && actionStartTS != null)
        					actionStartTS.invoke(objTS, mWimoCapture.ipAddr, mWimoCapture.udpPort);
        				nType = PMSG_WIMO_SUCCESS;
    				}
    				else if(nType == PMSG_WIMO_SUCCESS)
    				{
    					mStatus = STATUS_WIMO_CONNECTED;
    					Log.d(TAG, "wimo connect successfully!\n");
    				}
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
				mControlCallback.controlCallbackPort(nType, mPmsgType);
			}
		}
	};
	
	private void getScreenShot(){
		try{
			mScrnOrient = ((Activity)mContext).getWindowManager().getDefaultDisplay().getRotation();
		}catch (NullPointerException e){
			e.printStackTrace();
		}catch (ClassCastException e){
			e.printStackTrace();
		}
		if(mWimoCapture != null && mScrnOrient != mScrnOrientPrev)
		{
			mScrnOrientPrev = mScrnOrient;
			if(mScrnOrient == Surface.ROTATION_0){
				//Log.d(TAG,"screen rotate Surface.ROTATION_0");
				mWimoCapture.setVideoRotate(mWimoCapture.ROTATION_0);
			}
			else if(mScrnOrient == Surface.ROTATION_90)	{
				//Log.d(TAG,"screen rotate Surface.ROTATION_90");
				mWimoCapture.setVideoRotate(mWimoCapture.ROTATION_90);
			}
			else if(mScrnOrient == Surface.ROTATION_180){
				//Log.d(TAG,"screen rotate Surface.ROTATION_180");
				mWimoCapture.setVideoRotate(mWimoCapture.ROTATION_180);
			}
			else if(mScrnOrient == Surface.ROTATION_270){
				//Log.d(TAG,"screen rotate Surface.ROTATION_270");
				mWimoCapture.setVideoRotate(mWimoCapture.ROTATION_270);
			}		
		}
	}
    
	private final Handler mHandler = new Handler(){
		@Override
        public void handleMessage(Message msg) {   
            switch (msg.what){
                case PMSG_DETECT_TIMER:
                	//Log.d(TAG,"PMSG_DETECT_TIMER");
                	mHandler.removeMessages(PMSG_DETECT_TIMER);
        			if(mStatus == STATUS_WIMO_CONNECTED)
        			{
        				//Log.d(TAG, "start screen shot....\n");
						getScreenShot();
        			}
                	break;

                case PMSG_RSSI_SIGNAL_WEAK:
                	mIsRssiWeak = false;
                	mControlCallback.controlCallbackPort(PMSG_RSSI_SIGNAL_WEAK, mPmsgType);
                	break;

                default:
                    break;
            }
		}
	};
	
    public class WIFIReceiver extends BroadcastReceiver {
    	@Override
    	public void onReceive(Context context, Intent intent) {
    		String action = intent.getAction();
    		ConnectivityManager conMan = (ConnectivityManager) ((Activity)mContext).getSystemService(Context.CONNECTIVITY_SERVICE);
        	State networkState = conMan.getNetworkInfo(ConnectivityManager.TYPE_WIFI).getState(); 
        	WifiManager wm = (WifiManager) ((Activity)mContext).getSystemService(Context.WIFI_SERVICE);
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
    	WifiManager wm = (WifiManager) ((Activity)mContext).getSystemService(Context.WIFI_SERVICE);
    	if(!wm.isWifiEnabled())
    		return;
    	
    	NetworkInfo networkInfo = (NetworkInfo) intent.getParcelableExtra(WifiManager.EXTRA_NETWORK_INFO);
    	NetworkInfo.State networkState = networkInfo.getState();
    	//Log.i(TAG, "NetworkInfo.State==>"+networkState.toString());
    	
    	if(networkState==State.CONNECTED)
    	{
    		//clearAllInfoDialog();
    		mControlCallback.controlCallbackPort(DIALOG_CLEAR_INFO_ALL, mDialogType);
    		
    		WifiInfo wifiInfo = wm.getConnectionInfo();
    		mCurrentSSID = wifiInfo.getSSID();
    		if(mStatus==STATUS_WIMO_CONNECTED && mCurrentSSID!=null && mUsefulSSID!=null){
    			if(!mCurrentSSID.equals(mUsefulSSID))
    			{
    				Log.d(TAG, "mUsefulSSID stop wimo....\n");
    				//exitWimo();
    				//mStatus = STATUS_WIFI;
    				mControlCallback.controlCallbackPort(DIALOG_WLAN_CONNECTED_OTHER, mDialogType);
    				//cleanToShowCiDialog(DIALOG_WLAN_CONNECTED_OTHER);
    			}
    		}
    		mControlCallback.controlCallbackPort(DIALOG_WLAN_CONNECTED_EXCHANGE_WIMO_BUTTON, mDialogType);
    	}
    	else if(networkState==State.DISCONNECTED)
    	{
    		//netDisconnected_UpdateUI();
    		mControlCallback.controlCallbackPort(DIALOG_WLAN_LOST, mDialogType);
    		//WIFI connectivity has disconnected
    		//cleanToShowCiDialog(DIALOG_WLAN_LOST);
    	}
    }
    
    private void handleWifiStateChangedAction(Intent intent)
    {
    	WifiManager wm = (WifiManager) ((Activity)mContext).getSystemService(Context.WIFI_SERVICE);
    	int prevWifiState = intent.getIntExtra(WifiManager.EXTRA_PREVIOUS_WIFI_STATE,
                WifiManager.WIFI_STATE_UNKNOWN);
    	int currWifiState = intent.getIntExtra(WifiManager.EXTRA_WIFI_STATE,
                WifiManager.WIFI_STATE_UNKNOWN);
    	
    	//Log.i(TAG, "currWifiState==>"+currWifiState+";prevWifiState==>"+prevWifiState);
    	
    	//WIFI is turned off
    	if(prevWifiState==WifiManager.WIFI_STATE_DISABLING 
    			&& currWifiState==WifiManager.WIFI_STATE_DISABLED)
    	{
    		//netDisconnected_UpdateUI();
    		mControlCallback.controlCallbackPort(DIALOG_WLAN_TURNED_OFF, mDialogType);
    		//WIFI switch close
    		//cleanToShowCiDialog(DIALOG_WLAN_TURNED_OFF);
    	}
    	
    	else if(prevWifiState==WifiManager.WIFI_STATE_ENABLING
    			&& currWifiState==WifiManager.WIFI_STATE_ENABLED)
    	{
    		//clearAllInfoDialog();
    		mControlCallback.controlCallbackPort(DIALOG_CLEAR_INFO_ALL, mDialogType);
    	}
    }
       
    private void handleRSSIChangedAction()
    {
    	WifiManager wm = (WifiManager) ((Activity)mContext).getSystemService(Context.WIFI_SERVICE);
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
    
	private void checkJARforWFD() {
		// TODO Auto-generated method stub
        Context context = mContext;
		Class classTS = null;
		Class[] params = new Class[2];
		params[0] = Long.TYPE;
		params[1] = Integer.TYPE;
		try {
			 final File wimoDexJar = new File("/system/framework/TIWiMoInterface.jar");
			 if (wimoDexJar.exists())
			 {
				 DexClassLoader cl = new DexClassLoader(wimoDexJar.toString(), mWimoPath, 
						 null, context.getClassLoader());  
				 classTS = cl.loadClass("com.cmcc.wimo.WFDStreamConfig");
				 Constructor ct = classTS.getConstructor(Context.class);

				 objTS = ct.newInstance(context.getApplicationContext());
	             actionStartTS = classTS.getMethod("WFDStreamStart", params);	             
	             actionStopTS = classTS.getMethod("WFDStreamStop"); 
			 }
		} catch (Exception e)
		{
			e.printStackTrace();
		}
	}
}
