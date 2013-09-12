package cidana.screencap;

import android.os.Build;
import android.os.Build.VERSION;
import android.util.Log;

public class AutoCheckLib {
	private static int
		mSDKNum = 15,
		mSelectLibs = 0;
	
	private static String
		mLenovo_Model = "Lenovo S899t",
		mHisense_Model = "HS-T96",
		mZTE_Model_U970 = "ZTE U970",
		mZTE_Model_U930 = "ZTE U930";
	
	public static final int
		HW_DEFAULT_LIB = 0,
		TS_STREAM_INTERFACE_LIB = 1,
		SW_INTERFACE_LIB = 2,
		LENOVO_INTERFACE_LIB = 3,
		HISENSE_INTERFACE_LIB = 4,
		ZTE_INTERFACE_LIB = 5;	
	
	static {
		if((Build.MODEL.equals(mLenovo_Model)) && mSDKNum == VERSION.SDK_INT)
		{
			Log.d("test","the mode is Lenovo interface!\n");
			try{//load lenovo libs
				System.load("/system/lib/libhwjpeg.so");
				System.load("/system/lib/libgui.so");
				setCapLibStatus(LENOVO_INTERFACE_LIB);
			}catch (UnsatisfiedLinkError ule){
				Log.d("test","WARNING: Capture Could not load libhwjpeg.so!\n");
				ule.printStackTrace();
			}			
		}
		else if((Build.MODEL.equals(mHisense_Model)) && mSDKNum == VERSION.SDK_INT)
		{
			Log.d("test","the mode is Hisense interface!\n");
			try{//load hisense libs
				System.load("/system/lib/libavcap.so");
				System.load("/system/lib/libgui.so");
				setCapLibStatus(HISENSE_INTERFACE_LIB);
			}catch (UnsatisfiedLinkError ule){
				Log.d("test","WARNING: Capture Could not load libavcap.so!\n");
				ule.printStackTrace();
			}			
		}
		else if((Build.MODEL.equals(mZTE_Model_U970) || Build.MODEL.equals(mZTE_Model_U930)) && mSDKNum == VERSION.SDK_INT)
		{
			setCapLibStatus(ZTE_INTERFACE_LIB);
			Log.d("test","the mode is ZTE interface!\n");
		}
		else
		{
			setCapLibStatus(SW_INTERFACE_LIB);
			Log.d("test","the mode is SW interface!\n");
		}
	}
	public static void setCapLibStatus(int mLib){mSelectLibs = mLib;};
	public int getCapLibStatus(){return AutoCheckLib.mSelectLibs;};
}
