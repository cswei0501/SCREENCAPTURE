package cidana.wimo2;

import cidana.screencap.AutoCheckLib;
import cidana.screencap.Capture;

import com.cmcc.wimo.Encoder;
import com.cmcc.wimo.ScreenCap;

import android.content.Context;

public class CaptureContainer {

	private Context mContext = null;
	private static CaptureContainer mContainer = null;
	private Capture mScreencap = null;
	private ScreenCap mHWScreencap = null;
	private Encoder mHWEncoder = null;
	private AutoCheckLib mAutoCheckLib = null;
	
	private CaptureContainer(Context context)
	{
		mContext = context;
		
		mScreencap = new Capture();
		mHWScreencap = new ScreenCap();
		mHWEncoder = new Encoder(MainActivity.JPEG);
		mAutoCheckLib = new AutoCheckLib();
		//
	}
	
	public static CaptureContainer getInstance(Context context)
	{
		if (mContainer==null)
		{
			mContainer = new CaptureContainer(context);
		}
		
		return mContainer;
	}
	
	
	public Capture getScrnCap()
	{
		return mScreencap;
	}
	
	public ScreenCap getHWScrnCap()
	{
		return mHWScreencap;
	}
	
	public Encoder getHWEncoder()
	{
		return mHWEncoder;
	}

	public AutoCheckLib getAutoCheckLib() {
		// TODO Auto-generated method stub
		return mAutoCheckLib;
	}
}
