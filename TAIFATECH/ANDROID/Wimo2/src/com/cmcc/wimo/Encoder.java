package com.cmcc.wimo;

import android.util.Log;

public class Encoder {
	private static int mLibStatus = 0;

	static {
		try{
			System.loadLibrary("Encoder");
		}catch (UnsatisfiedLinkError ule){
			Log.d("test","WARNING: Could not load Encoder library!\n");
			setEncLibStatus(-1);
		}
	}	

	public static void setEncLibStatus(int mLib){mLibStatus = mLib;};
	public int getEncLibStatus(){return this.mLibStatus;};

	public Encoder(int codec){};
	public native int setCodec(int codec);
	public native int encode(byte[] rgbBuf,int pixelFormat,int width,int height,int quality,byte[] encodedBuf);
}
