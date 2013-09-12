package com.cmcc.wimo;

import android.util.Log;

public class ScreenCap {
	private static int mLibStatus = 0;

	static {
		try{
			System.loadLibrary("screenCap");
		}catch (UnsatisfiedLinkError ule){
			Log.d("test","WARNING: Could not load library!\n");
			setCapLibStatus(-1);
		}
	}	

	public static void setCapLibStatus(int mLib){mLibStatus = mLib;};
	public int getCapLibStatus(){return this.mLibStatus;};
	
	public native int update();
	public native int update(int reqWidth, int reqHeight);
	public native byte[] getPixels(int rotationDegree);
	public native int getWidth();
	public native int getHeight();
	public native int getFormat();
	public native int getStride();
	public native int getSize();
}
