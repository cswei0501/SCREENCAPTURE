package com.cmcc.wimo;

import android.util.Log;

public class AudioCap {
	private static final String TAG = "AudioCap";
	private static int mLibStatus = 0;

	static {
		try{
			System.loadLibrary("audioCap");
		}catch (UnsatisfiedLinkError ule){
			Log.d(TAG,"WARNING: Could not load audio library!\n");
			setAudCapStatus(-1);
		}
	}	

	public static void setAudCapStatus(int mLib){mLibStatus = mLib;};
	public int getAudCapStatus(){return mLibStatus;};
	
}
