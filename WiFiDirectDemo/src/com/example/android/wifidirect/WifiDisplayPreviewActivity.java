package com.example.android.wifidirect;

import android.app.Activity;
import android.graphics.Color;
import android.graphics.Point;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.util.Log;
import android.util.TypedValue;
import android.view.Gravity;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.Window;
import android.view.WindowManager;
import android.widget.LinearLayout;
import android.widget.RelativeLayout;
import android.widget.TextView;
import android.widget.Toast;

public class WifiDisplayPreviewActivity extends Activity implements SurfaceHolder.Callback {
	static {
		System.loadLibrary("wfd");
		System.loadLibrary("wfd_jni");
		}

	private native int start(String remoteIP, Object thisSurface);
	private native int stop();
	private native final int setSurface(Object thisSurface);
	
	private String mSourceIP;
	private SurfaceView mPreview;
	private final String TAG = "WFD Preview";
	private EventHandler mEventHandler;
	
	private final int MSG_VIDEO_SIZE_CHANGED = 5;
	private final int MSG_PLAYBACK_COMPLETE = 2;
	private final int MSG_MEDIA_ERROR = 100;
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		
		getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, WindowManager.LayoutParams.FLAG_FULLSCREEN);
		requestWindowFeature(Window.FEATURE_NO_TITLE);
		setContentView(R.layout.video);
		
		Bundle bundle = getIntent().getExtras();
		if (bundle != null)
			mSourceIP = bundle.getString("SourceIP");
		
		mPreview = (SurfaceView) findViewById(R.id.surfaceView1);
		mPreview.getHolder().addCallback(this);
		mPreview.getHolder().setKeepScreenOn(true);
		mPreview.getHolder().setSizeFromLayout();
		
		Looper looper;
		if ((looper = Looper.myLooper()) != null) {
			mEventHandler = new EventHandler(looper);
		} else if ((looper = Looper.getMainLooper()) != null) {
			mEventHandler = new EventHandler(looper);
		} else {
			mEventHandler = null;
		}
	}

	protected void onDestroy() {
		super.onDestroy();
	}

	@Override
	public void surfaceCreated(SurfaceHolder holder) {
		if (mSourceIP != null)
		{
			start(mSourceIP, holder.getSurface());
		}
	}

	@Override
	public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
	}

	@Override
	public void surfaceDestroyed(SurfaceHolder holder) {
		stop();
	}
	
	protected void updateLayout(int videoWidth, int videoHeight) {
		if (videoWidth <= 0 || videoHeight <= 0) {
			return;
		}
		
		Point pt = new Point();
		getWindowManager().getDefaultDisplay().getSize(pt);
		int scrWidth = pt.x;
		int scrHeight = pt.y;
		Log.d(TAG, "layoutSurfaceOfHor scrWidth = " + scrWidth + ", scrHeight = " + scrHeight);
		RelativeLayout containerView = (RelativeLayout) this.findViewById(R.id.RelativeLayout1);
		RelativeLayout.LayoutParams param = new RelativeLayout.LayoutParams(
				RelativeLayout.LayoutParams.WRAP_CONTENT,
				RelativeLayout.LayoutParams.WRAP_CONTENT);
		
		if (scrWidth >= videoWidth && scrHeight >= videoHeight) {
			if ((videoWidth * scrHeight) == (scrWidth * videoHeight)) {
				param.leftMargin = 0;
				param.topMargin = 0;

				param.width = scrWidth;
				param.height = scrHeight;
			} else if ((videoWidth * scrHeight) > (scrWidth * videoHeight)) {
				param.leftMargin = 0;
				param.width = scrWidth;

				param.height = (scrWidth * videoHeight) / videoWidth;
				param.topMargin = (scrHeight - param.height) / 2;
			} else if ((videoWidth * scrHeight) < (scrWidth * videoHeight)) {
				param.topMargin = 0;
				param.height = scrHeight;

				param.width = (scrHeight * videoWidth) / videoHeight;
				param.leftMargin = (scrWidth - param.width) / 2;
			}

		} else {
			if ((scrWidth * videoHeight) > (videoWidth * scrHeight)) {
				param.topMargin = 0;
				param.height = scrHeight;

				param.width = (scrHeight * videoWidth) / videoHeight;
				param.leftMargin = (scrWidth - param.width) / 2;
			} else {
				param.leftMargin = 0;
				param.width = scrWidth;

				param.height = (scrWidth * videoHeight) / videoWidth;
				param.topMargin = (scrHeight - param.height) / 2;

			}
		}
		containerView.updateViewLayout(mPreview, param);
	}
	
	private class EventHandler extends Handler
    {
        public EventHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
        	switch(msg.what) {
        	case MSG_VIDEO_SIZE_CHANGED:
        		updateLayout(msg.arg1, msg.arg2);
        		break;
        	case MSG_MEDIA_ERROR:
        		Log.w(WiFiDirectActivity.TAG, String.format("Player error (%d,%d)", msg.arg1, msg.arg2));
        		stop();
        		finish();
        		break;
        	case MSG_PLAYBACK_COMPLETE:
        		Log.w(WiFiDirectActivity.TAG, "Player EOF");
        		LinearLayout toastLayout = new LinearLayout(WifiDisplayPreviewActivity.this);   
                toastLayout.setOrientation(LinearLayout.HORIZONTAL);   
                toastLayout.setGravity(Gravity.CENTER_VERTICAL);
                TextView tv_content = new TextView(WifiDisplayPreviewActivity.this);   
                tv_content.setText("Lost connection.\nPlease retry connection on source side.");   
                tv_content.setTextColor(Color.LTGRAY);
                tv_content.setTextSize(TypedValue.COMPLEX_UNIT_SP, 32);
                tv_content.setBackgroundColor(Color.TRANSPARENT);// Í¸Ã÷±³¾°   
                toastLayout.addView(tv_content);   
                
        		Toast toast = new Toast(WifiDisplayPreviewActivity.this);
        		toast.setGravity(Gravity.CENTER, 0, 0);
        		toast.setView(toastLayout);
        		toast.setDuration(5000);
        	//	toast.show();
        		stop();
        		finish();
        		break;
        	default:
        		break;
        	}
        }
    }
	
	private void onReceiveMessage(int msg, int arg1, int arg2) {
    	Message m = mEventHandler.obtainMessage(msg, arg1, arg2, null);
       	mEventHandler.sendMessage(m);
    }
}
