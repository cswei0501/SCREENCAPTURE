package com.cmcc.wimo;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.lang.reflect.Constructor;
import java.lang.reflect.Method;
import dalvik.system.DexClassLoader;
import android.content.Context;
import android.os.Environment;
import android.util.Log;
import java.lang.reflect.InvocationTargetException;

import com.cmcc.wimo.WiMoInterface.WimoControlCallbackPort;

public class WiMoControl {
	private static final String TAG = "WiMoControl";
	
	private String 
	mWimoJARPath = null;

	private Context mContext = null;
	
	public static final int mWimo1 = 1,	mWimo2 = 2,	mWimoGame = 3;
	public static final int TYPE_MSG = 0, TYPE_DIALOG = 1;
	public static final int STOP = 0, EXIT = 1;
	public static final int STATUS_WIMO_READY = 1, STATUS_WIMO_CONNECTING = 2, STATUS_WIMO_CONNECTED = 3;
	
	public static final int PMSG_WIMO_FAILED = 0, PMSG_WIMO_SUCCESS = 1, PMSG_WIMO_SUSPEND = 2;
	public static final int
	// play relate message
			PMSG_DETECT_TIMER = 1,
			PMSG_CONN_WIMO_SUCCESS = 2, PMSG_CONN_WIMO_FAILED = 3, PMSG_CONN_WIMO_SUSPEND = 4, PMSG_RSSI_SIGNAL_WEAK = 11, PMSG_SHOW_SPECIAL_DIALOG = 12;


	public static final int DIALOG_WLAN_TURNED_OFF = 10, DIALOG_WLAN_LOST = 11, DIALOG_WLAN_WEAK_SIGNAL = 12, DIALOG_WLAN_CONNECTED_OTHER = 13,
			DIALOG_CLEAR_INFO_ALL = 29, DIALOG_NET_ERROR = 30;

	public static final int RGB565 = 0, ARGB4444 = 1, ARGB8888 = 2, YUV420 = 3, YUV422 = 4, JPEG = 5, TS = 6;

	public static final int ROTATION_0 = 0, ROTATION_90 = 90, ROTATION_180 = 180, ROTATION_270 = 270;
	
	public static Object 
	objCiRob = null;
	public static Method mtdSetTVRotate = null;
	
	private final int 
	mStop = 0,
	mExit = 1;
	
	private Method
	mtdInit = null,
	mtdUnInit = null,
	mtdStartWimo = null,
	mtdStopWimo = null,
	mtdCheckFb0 = null,
	mtdSendVidBuf = null,
	mtdSendAudBuf = null,
	mtdAddCallback = null;
	
	/** constructor for wimo1.0, wimo2.0, game
	* mContext: object
	* mode:
	*	mWimo1 = 1,
	*	mWimo2 = 2,
	*	mWimoGame = 3;
	*/
	public WiMoControl(Context context, int mode)
	{
		mContext = context;
		checkJARforSDK(mode);
	}
	
	/**for only wimo1.0**/
	public int CheckFb0()
	{
		int ret = -1;
		if(mtdCheckFb0 != null)
			try {
				ret = (Integer)mtdCheckFb0.invoke(objCiRob);
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
		
		return ret;
	}
	
	/** for wimo1.0, wimo2.0, game
	* start WiMo
	*/
	public void WiMoStart()
	{
		Log.d(TAG, "WiMoStart......\n");
        if(mtdStartWimo != null){
			try {
				mtdStartWimo.invoke(objCiRob);
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
	}
	
	/** for wimo1.0, wimo2.0, game
	* stop WiMo
	*/
	public void WiMoStop(int exit)
	{
		Log.d(TAG, "WiMoStop......\n");
		if(mtdStopWimo != null){
			try {
				mtdStopWimo.invoke(objCiRob);
				if(exit == mExit)
				{
					Log.d(TAG, "unInit WiMo......\n");
					mtdUnInit.invoke(objCiRob);					
				}
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
	}
	
	/** for wimo1.0, game
	* set video data to SDK,
	* dataTpye: video’s data type
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
	public void SetVideoBuffer(int dataType, byte[] buff_in, int buffSize, int width, int height, int rotate)
	{
		//Log.d(TAG, "SetVideoBuffer......\n");
        if(mtdSendVidBuf != null){
			try {
				mtdSendVidBuf.invoke(objCiRob, dataType, buff_in, buffSize, width, height, rotate);
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
	public void SetAudioBuffer(int dataType, byte[] buff_in, int buffSize, int sampleRate, int bitRate, int channels)
	{
		//Log.d(TAG, "SetAudioBuffer......\n");
		if(mtdSendAudBuf != null)
		{
			try {
				mtdSendAudBuf.invoke(objCiRob, dataType, buff_in, buffSize, sampleRate, bitRate, channels);
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
	}
	
	/** for wimo1.0
	* set TV rotate degree, it only for wimo1.0
	* nTVRotate: 
	*	public final int
	*		CURRENT_TV_ROTATE_0 = 0,
	*		CURRENT_TV_ROTATE_90 = 1,//CW:clockwise
	*		CURRENT_TV_ROTATE_270 = 2;//CCW:anticlockwise
	*/
	public static void SetTVRotate(int nTVRotate)
	{
		Log.d(TAG, "SetTVRotate......\n");
		if(mtdSetTVRotate != null)
		{
			try {
				mtdSetTVRotate.invoke(objCiRob, nTVRotate);
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
	}
	
	/** for wimo1.0, wimo2.0, game 
	* The UI need to implements the function controlCallback()
	*/
	private WimoControlCallback mControlCallback = null;
	public interface WimoControlCallback
	{
		/**callback message
		* nVal:
		*	public final int
		*		PMSG_WIMO_FAILED = 1,
		*		PMSG_WIMO_SUCCESS = 2,
		*		PMSG_WIMO_SUSPEND = 3;
		* If it is PMSG_WIMO_SUCCESS, WiMo has already connected with RX,
		* and start screen transmit;
		* If it is PMSG_WIMO_FAILED, WiMo connect failed or else reason;
		* If it is PMSG_WIMO_SUSPEND, oneself has already been kicked, and
		* another model or device will connect to RX;
		* nType: reserve
		*/
		public abstract int controlCallback(int nVal, int nType);
	};
	
	/**
	* register callback function
	*/
	public void addCallback(Context context,WimoControlCallback callback)
	{
		Log.d(TAG, "addCallback......\n");
		mControlCallback = callback;
		if(mtdAddCallback != null)
		{
			try {
				mtdAddCallback.invoke(objCiRob, context, mWimoCallbackPort);
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
	}
	
	public WimoControlCallbackPort mWimoCallbackPort = new WimoControlCallbackPort(){
	 	public int controlCallbackPort(int nVal, int nType) {
			// TODO Auto-generated method stub
			mControlCallback.controlCallback(nVal, nType);
			return 0;
		}	
	};
	
	public String GetAppLibPath(Context cnt)
	{
		String filePath = cnt.getApplicationContext().getFilesDir().getAbsolutePath();
		int slashPos = filePath.lastIndexOf('/');
		String libPath = filePath.substring(0, slashPos + 1)+"lib";
		Log.d(TAG, "lib path: "+libPath);
		
		return libPath;
	}
	
	/**
     * 将InputStream转换成byte数组
     * @param in InputStream
     * @return byte[]
     * @throws IOException
     */
    public byte[] InputStreamToByte(InputStream in) throws IOException
    {
	    ByteArrayOutputStream outStream = new ByteArrayOutputStream();
	    byte[] data = new byte[128];
	    int count = -1;
	    while((count = in.read(data,0,128)) != -1)
	        outStream.write(data, 0, count);
	         
	        data = null;
	    return outStream.toByteArray();
    }
	
    private int IsDirExist()
    {
    	String SDCardRoot = Environment.getExternalStorageDirectory().getAbsolutePath() + File.separator;
    	File file = new File(SDCardRoot + "wimo/" + File.separator);
    	if(!file.exists())
    		file.mkdir();  //如果不存在则创建
    	
    	mWimoJARPath = SDCardRoot + "wimo/ciWimo.jar";
    	
    	return 0;
    }
    
	private void checkJARforSDK(int mode) {
		// TODO Auto-generated method stub
	    Context context = mContext;
		Class classCiRob = null;
	    Class callbackCiRob = null;
		Class[] constructorCiRob = {Context.class, Integer.TYPE};
		Object[] argConCiRob = {context, Integer.valueOf(mode)};
		Class[] pamInit = {Integer.TYPE, Integer.TYPE};
		Class[] pamRotate = {Integer.TYPE};
		Class[] pamVidBuf = new Class[6]; 
		pamVidBuf[0] = Integer.TYPE;
		pamVidBuf[1] = byte[].class;
		pamVidBuf[2] = Integer.TYPE;
		pamVidBuf[3] = Integer.TYPE;
		pamVidBuf[4] = Integer.TYPE;
		pamVidBuf[5] = Integer.TYPE;
		Class[] pamAudBuf = {Integer.TYPE,byte[].class,Integer.TYPE,Integer.TYPE,Integer.TYPE,Integer.TYPE};
		Class[] pamCallback = new Class[2];
		pamCallback[0] = Context.class;
		pamCallback[1] = WimoControlCallbackPort.class;
	/* 
		mWimoJARPath = GetAppLibPath(context);
	    
		InputStream abpath = getClass().getResourceAsStream("/assets/ciWimo.jar");
		String path = null;
		try {
			path = new String(InputStreamToByte(abpath));
		} catch (IOException e1) {
			// TODO Auto-generated catch block
			e1.printStackTrace();
		}
		Log.d(TAG, "assets path: "+ path);
	*/	
		
		IsDirExist();
		
		Log.v(TAG, "mWimoJARPath: "+ mWimoJARPath);
	    try {	
			 final File wimoDexJar = new File(mWimoJARPath);
			 if (wimoDexJar.exists())
			 {
				 DexClassLoader cl = new DexClassLoader(wimoDexJar.toString(), context.getApplicationContext().getFilesDir().getAbsolutePath(), 
						 null, context.getClassLoader());  
 
				 classCiRob = cl.loadClass("com.cmcc.wimo.WiMoPortForSDK");
				 Constructor ct = classCiRob.getConstructor(constructorCiRob);
				 
				 objCiRob = ct.newInstance(argConCiRob);
				 mtdUnInit = classCiRob.getMethod("unInitControlPort");
	             mtdStartWimo = classCiRob.getMethod("WiMoStartPort");
	             mtdStopWimo = classCiRob.getMethod("WiMoStopPort");
	             mtdCheckFb0 = classCiRob.getMethod("CheckFb0Port");
	             mtdSendVidBuf = classCiRob.getMethod("SetVideoBufferPort", pamVidBuf);
	             mtdSendAudBuf = classCiRob.getMethod("SetAudioBufferPort", pamAudBuf);
	             mtdSetTVRotate = classCiRob.getMethod("SetTVRotatePort", pamRotate);
	             mtdAddCallback = classCiRob.getMethod("addCallbackPort", pamCallback); 
				 callbackCiRob = cl.loadClass("com.cmcc.wimo.WiMoInterface$WimoControlCallbackPort");
			 }
		} catch (Exception e){
			e.printStackTrace();
		}
	}	
}
