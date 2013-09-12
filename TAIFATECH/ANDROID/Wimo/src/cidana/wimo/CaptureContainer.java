package cidana.wimo;

import com.cmcc.wimo.Encoder;
import com.cmcc.wimo.ScreenCap;
import android.content.Context;

public class CaptureContainer {

	private Context mContext = null;
	private static CaptureContainer mContainer = null;
	private ScreenCap mHWScreencap = null;
	private Encoder mHWEncoder = null;

	private CaptureContainer(Context context)
	{
		mContext = context;

		mHWScreencap = new ScreenCap();
		mHWEncoder = new Encoder(MainActivity.mJPEG);
	}
	
	public static CaptureContainer getInstance(Context context)
	{
		if (mContainer==null)
		{
			mContainer = new CaptureContainer(context);
		}
		
		return mContainer;
	}
	
	public ScreenCap getHWScrnCap()
	{
		return mHWScreencap;
	}
	
	public Encoder getHWEncoder()
	{
		return mHWEncoder;
	}
}
