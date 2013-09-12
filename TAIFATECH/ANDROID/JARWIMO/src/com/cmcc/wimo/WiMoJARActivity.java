package com.cmcc.wimo;

import com.cmcc.wimo.WiMoInterface.WimoControlCallbackPort;

import android.app.Activity;
import android.os.Bundle;
import android.util.Log;

public class WiMoJARActivity extends Activity {
    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);
        
        
        WiMoPortForSDK mWiMoControl = new WiMoPortForSDK(this, 1);
        mWiMoControl.addCallbackPort(this, mWimoControlCallback);
    }
    
    
    private WimoControlCallbackPort mWimoControlCallback = new WimoControlCallbackPort() {
		@Override
		public int controlCallbackPort(int nVal, int nType)
		{
			Log.i("mWimoControlCallback", "nVal = "+nVal);
			
			return 0;
		}
	};
}