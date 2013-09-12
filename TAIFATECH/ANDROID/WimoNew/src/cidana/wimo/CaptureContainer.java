package cidana.wimo;


import com.cmcc.wimo.AutoCheckLib;
import com.cmcc.wimo.WiMoCapture;
import com.cmcc.wimo.Encoder;
import com.cmcc.wimo.ScreenCap;

import android.content.Context;

public class CaptureContainer {

	private Context mContext = null;
	private static CaptureContainer mContainer = null;
	private WiMoCapture mScreencap = null;
	private ScreenCap mHWScreencap = null;
	private Encoder mHWEncoder = null;
	private AutoCheckLib mAutoCheckLib = null;
	
	private CaptureContainer(Context context)
	{
		mContext = context;
		
		mScreencap = new WiMoCapture(mContext);
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
	
	
	public WiMoCapture getScrnCap()
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
