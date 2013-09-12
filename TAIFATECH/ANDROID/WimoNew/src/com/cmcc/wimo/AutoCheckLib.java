package com.cmcc.wimo;

import android.os.Build;
import android.os.Build.VERSION;
import android.util.Log;

public class AutoCheckLib {
	private static final String TAG = "AutoCheckLib";
	private static int
		mSelectLibs = 0;
	
	private static String
		mLenovo_Model_899t = "Lenovo S899t",
		mLenovo_Model_868t = "Lenovo S868t",
		mHisense_Model = "HS-T96",
		mZTE_Model_U970 = "ZTE U970",
		mZTE_Model_U930 = "ZTE U930",
		mXiaoMi2 = "MI 2",
		mMoto887 = "MT887";
	
	public static int 
		mSDKNum = 0,
		mSDKNum_9 = 9,
		mSDKNum_10 = 10,
		mSDKNum_14 = 14,
		mSDKNum_15 = 15,
		mSDKNum_16 = 16,
		mSDKNum_17 = 17;
	
	public static final int
		HW_DEFAULT_LIB = 0,
		TS_STREAM_INTERFACE_LIB = 1,
		SW_INTERFACE_LIB = 2,
		LENOVO_INTERFACE_LIB = 3,
		HISENSE_INTERFACE_LIB = 4,
		ZTE_INTERFACE_LIB = 5,
		HAOLIAN_INTERFACE_LIB = 6,
		XIAOMI2_INTERFACE_LIB = 7,
		MOTOROLA_887_INTERFACE = 8,
		SCREENSHOT_INTERFACE_LIB = 9;
	
	static {		
        if(mSDKNum_14==VERSION.SDK_INT || mSDKNum_15==VERSION.SDK_INT)
			mSDKNum = 15;
		else if(mSDKNum_16==VERSION.SDK_INT)
			mSDKNum = 16;
		else if(mSDKNum_17==VERSION.SDK_INT)
			mSDKNum = 17;
		else if(mSDKNum_9==VERSION.SDK_INT || mSDKNum_10==VERSION.SDK_INT)
			mSDKNum = 9;
		else
			mSDKNum = 9;
        
		if((mSDKNum_14==VERSION.SDK_INT || mSDKNum_15==VERSION.SDK_INT)
				&& (Build.MODEL.equals(mLenovo_Model_899t) || Build.MODEL.equals(mLenovo_Model_868t)))
		{
			Log.d(TAG,"the mode is Lenovo interface!\n");
			try{//load lenovo libs
				System.load("/system/lib/libhwjpeg.so");
				System.load("/system/lib/libgui.so");
				setCapLibStatus(LENOVO_INTERFACE_LIB);
			}catch (UnsatisfiedLinkError ule){
				Log.d(TAG,"WARNING: Capture Could not load libhwjpeg.so!\n");
				setCapLibStatus(SW_INTERFACE_LIB);
				ule.printStackTrace();
			}			
		}
		else if((Build.MODEL.equals(mHisense_Model)) && (mSDKNum_14==VERSION.SDK_INT || mSDKNum_15==VERSION.SDK_INT))
		{
			Log.d(TAG,"the mode is Hisense interface!\n");
			try{//load hisense libs
				System.load("/system/lib/libavcap.so");
				System.load("/system/lib/libgui.so");
				setCapLibStatus(HISENSE_INTERFACE_LIB);
			}catch (UnsatisfiedLinkError ule){
				Log.d(TAG,"WARNING: Capture Could not load libavcap.so!\n");
				setCapLibStatus(SW_INTERFACE_LIB);
				ule.printStackTrace();
			}			
		}
//		else if((Build.MODEL.equals(mZTE_Model_U970) || Build.MODEL.equals(mZTE_Model_U930)) && mSDKNum == VERSION.SDK_INT)
//		{
//			setCapLibStatus(ZTE_INTERFACE_LIB);
//			Log.d(TAG,"the mode is ZTE interface!\n");
//		}
//		else if(Build.MODEL.equals(mXiaoMi2))
//		{
//			setCapLibStatus(XIAOMI2_INTERFACE_LIB);
//			Log.d(TAG,"the mode is XiaoMi2 interface!\n");
//		}
		else if((Build.MODEL.equals(mMoto887)) && (mSDKNum_14==VERSION.SDK_INT || mSDKNum_15==VERSION.SDK_INT))
		{
			setCapLibStatus(SW_INTERFACE_LIB);
			Log.d(TAG,"the mode is SW interface!\n");
		}
		else if(mSDKNum_14 <= VERSION.SDK_INT)
		{
			setCapLibStatus(SCREENSHOT_INTERFACE_LIB);
			Log.d(TAG,"the mode is screen shot interface!\n");		
		}
		else
		{
			setCapLibStatus(SW_INTERFACE_LIB);
			Log.d(TAG,"the mode is SW interface!\n");
		}
	}
	public static void setCapLibStatus(int mLib){mSelectLibs = mLib;};
	public int getCapLibStatus(){return AutoCheckLib.mSelectLibs;};
}
