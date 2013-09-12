package com.cmcc.wimo;

import android.content.Context;
import android.util.Log;

public class WiMoCapture
{    
	private static final String TAG = "WiMoCapture";
	private static int mLibStatus = 0;
	private static String mWimoLibPath = null;
	private String libPath = null;
	
	private Context mContext = null;
	
	public WiMoCapture(Context context)
	{
		mContext = context;
		mWimoLibPath = GetAppLibPath()+"libciwimo.so";
		try{
			System.load(mWimoLibPath);
		}catch (UnsatisfiedLinkError ule){
			Log.d(TAG,"WARNING: Capture Could not load screencapture library!\n");
			setCapStatus(-1);
			ule.printStackTrace();
		}
	}
	
	private String GetAppLibPath()
	{
		String filePath = mContext.getApplicationContext().getFilesDir().getAbsolutePath();
		int slashPos = filePath.lastIndexOf('/');
		libPath = filePath.substring(0, slashPos + 1)+"lib/";

		return libPath;
	}

	public byte[] mEncodedBuf = null;
	public byte[] mAudioData = null;
	
	public int cValPointer;
	public int udpPort = 0;
	public long ipAddr = 0;

	public final int
		HW_VIDEO_INTERFACE = 0,
		SW_VIDEO_FRAMEBUFFER = 1,
		SW_VIDEO_SCREENSHOT = 2,
		TS_STREAM_INTERFACE = 3,
		LNV_VIDEO_INTERFACE = 4,
		HS_VIDEO_INTERFACE = 5,
		ZTE_VIDEO_INTERFACE = 6,
		GAME_VIDEO_INTERFACE = 7,
		HAOLIAN_VIDEO_INTERFACE = 8,
		XIAOMI2_VIDEO_INTERFACE = 9;
	
	public final int
		AUDIO_METHOD_DEFAULT = 0,
		AUDIO_METHOD_SPEAKER = 1,
		AUDIO_METHOD_TS = 2,
		AUDIO_METHOD_LNV = 3,
		AUDIO_METHOD_HS = 4,
		AUDIO_METHOD_ZTE = 5,
		AUDIO_METHOD_GAME = 6,
		AUDIO_METHOD_HAOLIAN = 7,
		AUDIO_METHOD_XIAOMI2 = 8;
	
	public final int
		CURRENT_TV_ROTATE_0 = 0,
		CURRENT_TV_ROTATE_90 = 1,//CW:clockwise
		CURRENT_TV_ROTATE_270 = 2;//CCW:anticlockwise
	
	public final int
		ROTATION_0 = 0,
		ROTATION_90 = 90,
		ROTATION_180 = 180,
		ROTATION_270 = 270;
	
	public final int
		RGB565 = 0,
		ARGB4444 = 1,
		ARGB8888 = 2,
		YUV420 = 3,
		YUV422 = 4,
		JPEG = 5,
		TS = 6;
	
	public final int
		PCM = 0,
		AAC = 1;
	
	public final int
		SCREENSHOTOFF = 0,
		SCREENSHOTON = 1;
	
	public native int jniInit(int videoMode, int audioMode);
	public native int jniUnInit();
	public native int startWiMo();
	public native int stopWiMo();
	public native int sendVideoData(int dataType, byte[] data, int size, int width, int height, int rotate); 
	public native int sendAudioData(int dataType, byte[] data, int size, int sampleRate, int bitRate, int channels);
	public native int setTVRotate(int nTvRotate);
	public native int setVideoRotate(int nRotate);
	public native int checkFb0(); // return value: 0, means fb0 readable; !0, means fb0 can't readable.
	public native int notifyScreenshotCalled(int shotCalled); // shotCalled: 0,means off; 1,means using screen shot method
	
	public int javaCallback(int nOpen) {
		//Log.d(TAG,"javaCallback");
		if(mListener!=null)
			mListener.onCheckChange(nOpen);
		return 0;
	}

	private WimoCheckListener mListener = null;

	public interface WimoCheckListener {
		public void onCheckChange(int nOpen);
	};

	public void setWimoCheckListener(WimoCheckListener listener) {
		mListener = listener;
	}
	public static void setCapStatus(int mLib){mLibStatus = mLib;};
	public int getCapStatus(){return mLibStatus;};
}
