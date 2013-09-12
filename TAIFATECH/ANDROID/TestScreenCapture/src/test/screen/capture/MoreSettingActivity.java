package test.screen.capture;

import cidana.screencap.Capture;
import android.app.Activity;
import android.content.Intent;
import android.content.res.Configuration;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.provider.Settings;
import android.util.Log;
import android.view.Gravity;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.view.View.OnClickListener;
import android.widget.ImageButton;
import android.widget.Toast;

public class MoreSettingActivity extends Activity {
	/** Called when the activity is first created. */

	private ImageButton mRadioBtn1 = null;
	private ImageButton mRadioBtn2 = null;
	private ImageButton mRadioBtn3 = null;
	private ImageButton mRadioBtn4 = null;

	private ImageButton mReturnBtn = null;
	private ImageButton mAdvancedBtn = null;

	private int nCheck1 = 0;
	private int nCheck2 = 1;

	private static Capture mScreencap;

	private final int
	// play relate message
			PMSG_RELAYOUT = 1,
			PMSG_REFRESH = 2;

	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		requestWindowFeature(Window.FEATURE_NO_TITLE);
		getWindow().addFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN);

		mScreencap = TestScreenCaptureActivity.getScreenLibHandle();
		if (getResources().getConfiguration().orientation == Configuration.ORIENTATION_LANDSCAPE) {
			setContentView(R.layout.more_l);
			initHorUI();
		} else if (getResources().getConfiguration().orientation == Configuration.ORIENTATION_PORTRAIT) {
			setContentView(R.layout.more_p);
			initVerUI();
		}

		setRadioStatus();
	}

	private void setRadioStatus() {
		if (nCheck1 == 0) {
			mRadioBtn1.setBackgroundResource(R.drawable.radiobox_check);
			mRadioBtn2.setBackgroundResource(R.drawable.radiobox_nocheck);
		} else {
			mRadioBtn1.setBackgroundResource(R.drawable.radiobox_nocheck);
			mRadioBtn2.setBackgroundResource(R.drawable.radiobox_check);
		}

		if (nCheck2 == 0) {
			mRadioBtn3.setBackgroundResource(R.drawable.radiobox_check);
			mRadioBtn4.setBackgroundResource(R.drawable.radiobox_nocheck);
		} else {
			mRadioBtn3.setBackgroundResource(R.drawable.radiobox_nocheck);
			mRadioBtn4.setBackgroundResource(R.drawable.radiobox_check);
		}
	}

	private void initHorUI() {
		mRadioBtn1 = (ImageButton) findViewById(R.id.btn_radio1);
		mRadioBtn1.setOnClickListener(mRadio1ClickListener);
		mRadioBtn2 = (ImageButton) findViewById(R.id.btn_radio2);
		mRadioBtn2.setOnClickListener(mRadio2ClickListener);
		mRadioBtn3 = (ImageButton) findViewById(R.id.btn_radio3);
		mRadioBtn3.setOnClickListener(mRadio3ClickListener);
		mRadioBtn4 = (ImageButton) findViewById(R.id.btn_radio4);
		mRadioBtn4.setOnClickListener(mRadio4ClickListener);
		mReturnBtn = (ImageButton) findViewById(R.id.btn_close);
		mReturnBtn.setOnClickListener(mBtnReturnClickListener);
		mAdvancedBtn = (ImageButton) findViewById(R.id.btn_more);
		mAdvancedBtn.setOnClickListener(mBtnMoreClickListener);

		mRadioBtn3.setEnabled(false);
	}

	private void initVerUI() {
		mRadioBtn1 = (ImageButton) findViewById(R.id.btn_radio1);
		mRadioBtn1.setOnClickListener(mRadio1ClickListener);
		mRadioBtn2 = (ImageButton) findViewById(R.id.btn_radio2);
		mRadioBtn2.setOnClickListener(mRadio2ClickListener);
		mRadioBtn3 = (ImageButton) findViewById(R.id.btn_radio3);
		mRadioBtn3.setOnClickListener(mRadio3ClickListener);
		mRadioBtn4 = (ImageButton) findViewById(R.id.btn_radio4);
		mRadioBtn4.setOnClickListener(mRadio4ClickListener);
		mReturnBtn = (ImageButton) findViewById(R.id.btn_close);
		mReturnBtn.setOnClickListener(mBtnReturnClickListener);
		mAdvancedBtn = (ImageButton) findViewById(R.id.btn_more);
		mAdvancedBtn.setOnClickListener(mBtnMoreClickListener);

		mRadioBtn3.setEnabled(false);
	}

	private OnClickListener mBtnMoreClickListener = new OnClickListener() {
		@Override
		public void onClick(View v) {
			startActivity(new Intent(Settings.ACTION_WIRELESS_SETTINGS));
		}
	};

	private OnClickListener mRadio1ClickListener = new OnClickListener() {
		@Override
		public void onClick(View v) {
			if (nCheck1 == 1) {
				nCheck1 = 0;
				mRadioBtn1.setBackgroundResource(R.drawable.radiobox_check);
				mRadioBtn2.setBackgroundResource(R.drawable.radiobox_nocheck);

				if (mScreencap != null)
					mScreencap.setDisplayMode(1);
			}
		}
	};

	private OnClickListener mRadio2ClickListener = new OnClickListener() {
		@Override
		public void onClick(View v) {
			if (nCheck1 == 0) {
				nCheck1 = 1;
				mRadioBtn1.setBackgroundResource(R.drawable.radiobox_nocheck);
				mRadioBtn2.setBackgroundResource(R.drawable.radiobox_check);

				if (mScreencap != null)
					mScreencap.setDisplayMode(0);
			}
		}
	};

	private OnClickListener mRadio3ClickListener = new OnClickListener() {
		@Override
		public void onClick(View v) {
			if (nCheck2 == 1) {
				nCheck2 = 0;
				mRadioBtn3.setBackgroundResource(R.drawable.radiobox_check);
				mRadioBtn4.setBackgroundResource(R.drawable.radiobox_nocheck);

				if (mScreencap != null)
					mScreencap.setVideoRenderMode(1);
			}
		}
	};

	private OnClickListener mRadio4ClickListener = new OnClickListener() {
		@Override
		public void onClick(View v) {
			if (nCheck2 == 0) {
				nCheck2 = 1;
				mRadioBtn3.setBackgroundResource(R.drawable.radiobox_nocheck);
				mRadioBtn4.setBackgroundResource(R.drawable.radiobox_check);

				if (mScreencap != null)
					mScreencap.setVideoRenderMode(0);
			}
		}
	};

	private OnClickListener mBtnReturnClickListener = new OnClickListener() {
		@Override
		public void onClick(View v) {
			finish();
		}
	};

	@Override
	public void onConfigurationChanged(Configuration newConfig) {
		// TODO Auto-generated method stub

		Log.d("test", "onConfigurationChanged");
		if (getResources().getConfiguration().orientation == Configuration.ORIENTATION_LANDSCAPE) {
			setContentView(R.layout.more_l);
			initHorUI();
			if (mScreencap != null)
				mScreencap.setVideoRotate(true);
		} else if (getResources().getConfiguration().orientation == Configuration.ORIENTATION_PORTRAIT) {
			setContentView(R.layout.more_p);
			initVerUI();
			if (mScreencap != null)
				mScreencap.setVideoRotate(false);
		}

		setRadioStatus();
		mHandler.sendEmptyMessageDelayed(PMSG_REFRESH, 200);
		super.onConfigurationChanged(newConfig);
	}

	private final Handler mHandler = new Handler() {
		@Override
		public void handleMessage(Message msg) {

			switch (msg.what) {
			case PMSG_RELAYOUT:

				break;
			case PMSG_REFRESH:
				mRadioBtn1.invalidate();
				break;
			default:
				break;
			}

		}
	};
}