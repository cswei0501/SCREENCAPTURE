package cidana.wimo;


import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.os.Bundle;
import android.text.method.LinkMovementMethod;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.TextView;

public class AboutDialog extends Activity {

	private String mModelVersion = "";
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		super.onCreate(savedInstanceState);
		
		Intent intent = getIntent();
		//mModelVersion = intent.getStringExtra("tVersion");
		setContentView(R.layout.about_dlg);
		
		initUI();
	}
	
	private void initUI()
	{
		TextView tvVersion = (TextView)findViewById(R.id.tv_version);
		TextView tVersion = (TextView)findViewById(R.id.t_version);
		Button btnOK = (Button)findViewById(R.id.btn_ok);
		
		TextView tvCidana = (TextView) findViewById(R.id.tv_cidanaweb);
		tvCidana.setMovementMethod(LinkMovementMethod.getInstance());
		
		tVersion.setText(mModelVersion);
		tvVersion.setText(getVersionName(AboutDialog.this));
		btnOK.setOnClickListener(new OnClickListener(){

			@Override
			public void onClick(View v) {
				// TODO Auto-generated method stub
				finish();
			}});
	}
	
	public static int getVersionCode(Context context) {
		   PackageManager pm = context.getPackageManager();
		   try {
		      PackageInfo pi = pm.getPackageInfo(context.getPackageName(), 0);
		      return pi.versionCode;
		   } catch (NameNotFoundException ex) {}
		   return 0;
	}
	
	public static String getVersionName(Context context) {
		   PackageManager pm = context.getPackageManager();
		   try {
		      PackageInfo pi = pm.getPackageInfo(context.getPackageName(), 0);
		      return pi.versionName;
		   } catch (NameNotFoundException ex) {}
		   return "";
	}

}
