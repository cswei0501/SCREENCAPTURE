package cidana.wimo;

import java.io.BufferedOutputStream;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.net.HttpURLConnection;
import java.net.MalformedURLException;
import java.net.URI;
import java.net.URISyntaxException;
import java.net.URL;
import java.text.DecimalFormat;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.apache.http.HttpEntity;
import org.apache.http.HttpHost;
import org.apache.http.HttpResponse;
import org.apache.http.NameValuePair;
import org.apache.http.StatusLine;
import org.apache.http.client.ClientProtocolException;
import org.apache.http.client.HttpClient;
import org.apache.http.client.entity.UrlEncodedFormEntity;
import org.apache.http.client.methods.HttpGet;
import org.apache.http.client.methods.HttpPost;
import org.apache.http.conn.params.ConnRoutePNames;
import org.apache.http.entity.StringEntity;
import org.apache.http.impl.client.DefaultHttpClient;
import org.apache.http.message.BasicNameValuePair;
import org.apache.http.params.BasicHttpParams;
import org.apache.http.params.HttpConnectionParams;
import org.apache.http.params.HttpParams;
import org.apache.http.protocol.HTTP;
import org.json.JSONObject;
import org.json.JSONTokener;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.os.StatFs;
import android.util.Base64;
import android.util.Log;
import android.view.Gravity;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.ProgressBar;
import android.widget.TextView;
import android.widget.Toast;

public class UpgradeDialog extends Activity {
	private final String TAG = UpgradeDialog.class.getSimpleName();
	private TextView mTvTitle = null;
	private TextView mTvCheckVersion = null;
	private TextView mTvVersion = null;
	private Button mBtnUpgrade = null;
	private Button mBtnCancel = null;
	private Button mBtnCancelSigle = null;
	private LinearLayout mCheckVersionView = null;
	private LinearLayout mDispVersionView = null;
	private LinearLayout mCmriView = null;
	private LinearLayout mUpgradeProgView = null;
	private ProgressBar mProgressBar  = null; 
	private TextView mTvPercent = null;
	private TextView mTvDownloaded = null;
	private View mOkCancelView = null;
	private View mCancelView = null;
 
	
	private static final int 
		SOCKET_TIMEOUT = 10000,
		CONNECT_TIMEOUT = 10000;
	private final int
		PMSG_GET_UPGRADE_FAILED = 0,
		PMSG_GET_UPGRADE_SUCCESS = 1,
		PMSG_ERROR_NOSDCARD = 2,
		PMSG_ERROR_APK_SOURCE = 3,
		PMSG_ERROR_SD_NOMEMORY = 4,
		PMSG_DOWNLOAD_FAILED = 5,
		PMSG_DOWNLOAD_SUCCESS = 6;
	
	private boolean mNeedStopUpgrade = false;
	private String mDownApkPath = null;
	private static final String SAVE_APK_NAME = "wimo_upgrade.apk";
	private static final String APP_NAME = "wimo1.0_cidana_tx_cellphone";
	private static final String UPGRADE_URL = "http://wimo.hotpotpro.com/TingDong/appupdate";
	
	private UpgradeResponseParam mResponseParam = new UpgradeResponseParam();
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		super.onCreate(savedInstanceState);
		
		//Intent intent = getIntent();
		//APP_NAME = intent.getStringExtra("appName");
		setContentView(R.layout.upgrade_dlg);
		
		initUI();
		
		mCheckVersionView.setVisibility(View.VISIBLE);
		mDispVersionView.setVisibility(View.GONE);
		mCmriView.setVisibility(View.VISIBLE);
		mUpgradeProgView.setVisibility(View.GONE);
		
		
		Bundle bundle = null;
		if(getIntent()!=null)
			bundle = getIntent().getExtras();
		if(bundle!=null && bundle.getBoolean("upgrad_need_update")){
			mResponseParam.need_update = true;
			mResponseParam.app_build = bundle.getLong("upgrad_app_build");
			mResponseParam.app_date = bundle.getString("upgrad_app_date");
			mResponseParam.app_desc = bundle.getString("upgrad_app_desc");
			mResponseParam.app_name = bundle.getString("upgrad_app_name");
			mResponseParam.app_url = bundle.getString("upgrad_app_url");
			mResponseParam.app_version = bundle.getString("upgrad_app_version");
			
			mCheckVersionView.setVisibility(View.GONE);
    		mDispVersionView.setVisibility(View.VISIBLE);
    		mCmriView.setVisibility(View.VISIBLE);
    		mUpgradeProgView.setVisibility(View.GONE);
    		showNewVersion();
		}else{
			getUpgradeInfo(UPGRADE_URL);
		}
		
	}
	
	private void initUI()
	{	
		mTvTitle = (TextView)findViewById(R.id.tv_title); 
		mTvCheckVersion = (TextView)findViewById(R.id.tv_checkversion);
		mTvVersion = (TextView)findViewById(R.id.tv_newversion);
		mBtnUpgrade = (Button)findViewById(R.id.btn_upgrade);
		mBtnCancel = (Button)findViewById(R.id.btn_cancel);
		mBtnCancelSigle = (Button)findViewById(R.id.btn_cancel_sigle);
		mCheckVersionView = (LinearLayout)findViewById(R.id.checkversion_container); 
		mDispVersionView = (LinearLayout)findViewById(R.id.dispversion_container); 
		mCmriView = (LinearLayout)findViewById(R.id.cmri_container); 
		mUpgradeProgView = (LinearLayout)findViewById(R.id.upgrade_container); 
		mOkCancelView = findViewById(R.id.okcancel_container); 
		mCancelView = findViewById(R.id.cancel_container); 
		
		mProgressBar = (ProgressBar)findViewById(R.id.progress); 
		mTvPercent = (TextView)findViewById(R.id.tv_percentage); 
		mTvDownloaded = (TextView)findViewById(R.id.tv_downloaded); 
		  
 
		mBtnUpgrade.setOnClickListener(mBtnUpgradeOnClick);
		mBtnCancel.setOnClickListener(mBtnCancelOnClick);
		mBtnCancelSigle.setOnClickListener(mBtnCancelOnClick);
		
		mResponseParam.need_update = false;
		mTvVersion.setText(getVersionName(UpgradeDialog.this));
		mTvTitle.setText(getResources().getString(R.string.string_menu_upgrade));//检测新版本
	}
	
	private OnClickListener mBtnUpgradeOnClick = new OnClickListener()
	{

		@Override
		public void onClick(View v) {
			// TODO Auto-generated method stub
			mNeedStopUpgrade = false;
			mBtnUpgrade.setEnabled(false);
			if (mResponseParam.need_update)
			{
				new DownloadThread(mResponseParam.app_url).execute();
			}
		}
	};
	
	private OnClickListener mBtnCancelOnClick = new OnClickListener()
	{

		@Override
		public void onClick(View v) {
			// TODO Auto-generated method stub
			exitUpgrade();
		}
	};
	
	@Override
	protected void onDestroy() {
		// TODO Auto-generated method stub
		super.onDestroy();
	}
	
	private void exitUpgrade()
	{
		mNeedStopUpgrade = true;
		finish();
	}
	
	public static long getVersionCode(Context context) {
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
	
	private String getB64Auth (String login, String pass) {
	    String source=login+":"+pass;
	    String ret="Basic "+Base64.encodeToString(source.getBytes(),Base64.URL_SAFE|Base64.NO_WRAP);
	    return ret;
     }
	
	
	private void getUpgradeInfo(final String strurl) {
		
		new Thread() {
			public void run() {
				HttpParams  httpParameters = new BasicHttpParams();// Set the timeout in milliseconds until a connection is established.   
	            HttpConnectionParams.setConnectionTimeout(httpParameters, CONNECT_TIMEOUT);// Set the default socket timeout (SO_TIMEOUT) // in milliseconds which is the timeout for waiting for data.   
	            HttpConnectionParams.setSoTimeout(httpParameters, SOCKET_TIMEOUT);
	            
				HttpClient client = new DefaultHttpClient(httpParameters);
				HttpPost httppost = new HttpPost(strurl);
				HttpResponse response;
 
				try {
					
					 NetworkInfo networkInfo = ((ConnectivityManager)getSystemService(Context.CONNECTIVITY_SERVICE))  
                     .getActiveNetworkInfo();  
		             // 如果是使用的运营商网络   
		             if (networkInfo.getType() == ConnectivityManager.TYPE_MOBILE) {  
		                 // 获取默认代理主机ip   
		                 String host = android.net.Proxy.getDefaultHost();  
		                 // 获取端口   
		                 int port = android.net.Proxy.getDefaultPort();  
		                 if (host != null && port != -1) {  
		            
		                     HttpHost proxy = new HttpHost(host, port);  
		                     client.getParams().setParameter(ConnRoutePNames.DEFAULT_PROXY,  
		                             proxy); 
		                 }   
		             }  
		             
		            //添加http头信息
					/*httppost.addHeader("Authorization", getB64Auth("cidana","cidana")); 
					//httppost.addHeader("Content-Type", "application/json");*/
					/*httppost.setHeader("Accept", "application/json");
					httppost.setHeader("Content-type", "application/json");
					httppost.setHeader("User-Agent", "Apache-HttpClient/4.1 (java 1.5)");*/

					JSONObject obj = new JSONObject();
					obj.put("app_build", getVersionCode(getApplicationContext()));
					obj.put("app_name", APP_NAME);
					obj.put("platform", "android");
					 
					List <NameValuePair> params=new ArrayList<NameValuePair>();
				    params.add(new BasicNameValuePair("app",obj.toString()));
    
				    //发出HTTP request
					httppost.setEntity(new UrlEncodedFormEntity(params,HTTP.UTF_8));
 
					response = client.execute(httppost);
 
					HttpEntity entity = response.getEntity();
					StringBuilder sb = new StringBuilder();
					if (response.getStatusLine().getStatusCode()==200)
					{
						if ( entity != null) {
							BufferedReader reader = new BufferedReader(new InputStreamReader(entity.getContent(), "UTF-8"), 8192);
					 
							String line = null;
							while ((line = reader.readLine()) != null) {
								sb.append(line);//(line + "\n");
							}
							reader.close();
						}
	 
						JSONObject object = new JSONObject(sb.toString());//(JSONObject) new JSONTokener(sb.toString()).nextValue();
					 
						mResponseParam.need_update = object.getBoolean("need_update");
						if (mResponseParam.need_update)
						{
							mResponseParam.app_name = object.getString("app_name");
							mResponseParam.app_build = object.getLong("app_build");
							mResponseParam.app_date = object.getString("app_date");
							mResponseParam.app_desc = object.getString("app_desc");
							mResponseParam.app_version = object.getString("app_version");
							mResponseParam.app_url = object.getString("app_url");
						}
				 
					}
					else
					{
						mHandler.sendEmptyMessage(PMSG_GET_UPGRADE_FAILED);
						return;
					}
				}catch (Exception e) {
					mHandler.sendEmptyMessage(PMSG_GET_UPGRADE_FAILED);
					e.printStackTrace();
					return;
				} 
				mHandler.sendEmptyMessage(PMSG_GET_UPGRADE_SUCCESS);
			}

		}.start();

	}
	
	public static void IsNeedUpgrade(final Context context) {

		new Thread() {
			public void run() {
				HttpParams  httpParameters = new BasicHttpParams();// Set the timeout in milliseconds until a connection is established.   
	            HttpConnectionParams.setConnectionTimeout(httpParameters, CONNECT_TIMEOUT);// Set the default socket timeout (SO_TIMEOUT) // in milliseconds which is the timeout for waiting for data.   
	            HttpConnectionParams.setSoTimeout(httpParameters, SOCKET_TIMEOUT);
	            
				HttpClient client = new DefaultHttpClient(httpParameters);
				HttpPost httppost = new HttpPost(UPGRADE_URL);
				HttpResponse response;
 
				try {
					
					 NetworkInfo networkInfo = ((ConnectivityManager)(context.getSystemService(Context.CONNECTIVITY_SERVICE)))  
                     .getActiveNetworkInfo();  
		             // 如果是使用的运营商网络   
		             if (networkInfo.getType() == ConnectivityManager.TYPE_MOBILE) {  
		                 // 获取默认代理主机ip   
		                 String host = android.net.Proxy.getDefaultHost();  
		                 // 获取端口   
		                 int port = android.net.Proxy.getDefaultPort();  
		                 if (host != null && port != -1) {  
		            
		                     HttpHost proxy = new HttpHost(host, port);  
		                     client.getParams().setParameter(ConnRoutePNames.DEFAULT_PROXY,  
		                             proxy); 
		                      
		                 }   
		             }  
		             
					JSONObject obj = new JSONObject();
					obj.put("app_build", getVersionCode(context));
					obj.put("app_name", APP_NAME);
					obj.put("platform", "android");
					 
					List <NameValuePair> params=new ArrayList<NameValuePair>();
				    params.add(new BasicNameValuePair("app",obj.toString()));
    
				    //发出HTTP request
					httppost.setEntity(new UrlEncodedFormEntity(params,HTTP.UTF_8));
 
					response = client.execute(httppost);
 
					HttpEntity entity = response.getEntity();
					StringBuilder sb = new StringBuilder();
					if (response.getStatusLine().getStatusCode()==200)
					{
						if ( entity != null) {
							BufferedReader reader = new BufferedReader(new InputStreamReader(entity.getContent(), "UTF-8"), 8192);
					 
							String line = null;
							while ((line = reader.readLine()) != null) {
								sb.append(line);//(line + "\n");
							}
							reader.close();
						}
	 
						JSONObject object = new JSONObject(sb.toString());//(JSONObject) new JSONTokener(sb.toString()).nextValue();
					 
						UpgradeResponseParam responseParam = new UpgradeResponseParam();
						
						responseParam.need_update = object.getBoolean("need_update");
						if (responseParam.need_update)
						{
							responseParam.app_name = object.getString("app_name");
							responseParam.app_build = object.getLong("app_build");
							responseParam.app_date = object.getString("app_date");
							responseParam.app_desc = object.getString("app_desc");
							responseParam.app_version = object.getString("app_version");
							responseParam.app_url = object.getString("app_url");
							
							if(context!=null && context instanceof MainActivity)
							{
								((MainActivity)context).showUpgradDialog(responseParam);
							}
						}else{
							Log.i("UpgradeDialog", " Do not need upgrade!");
						}
						
					}
					else
					{
						Log.i("UpgradeDialog", "upgrade failed!");
						//mHandler.sendEmptyMessage(PMSG_GET_UPGRADE_FAILED);
						return;
					}
				}catch (Exception e) {
					Log.i("UpgradeDialog", "upgrade failed!");
					//mHandler.sendEmptyMessage(PMSG_GET_UPGRADE_FAILED);
					e.printStackTrace();
					return;
				} 
				//mHandler.sendEmptyMessage(PMSG_GET_UPGRADE_SUCCESS);
			}
		}.start();
	}
	
	private void showMessage(String msg)
	{
		Toast toast = Toast.makeText(UpgradeDialog.this, msg, Toast.LENGTH_SHORT);
		toast.setGravity(Gravity.CENTER, 0, 0);
		toast.show();
	}
	
	private void showNewVersion()
	{
		if (mResponseParam.need_update)//(mResponseParam.app_build>getVersionCode(UpgradeDialog.this))
		{
			mTvTitle.setText(getResources().getString(R.string.string_version_upgrade));//版本升级
			mTvCheckVersion.setText(getResources().getString(R.string.string_find_new_version));//检测到有新版本
			mTvVersion.setText(mResponseParam.app_version);
			
			mCancelView.setVisibility(View.GONE);
			mOkCancelView.setVisibility(View.VISIBLE);
		}
		else
		{
			//mTvCheckVersion.setText("当前已是最新版本");
			//mTvVersion.setText(getVersionName(UpgradeDialog.this));
			
			showMessage(getResources().getString(R.string.string_prompt_lastest_version));//当前已是最新版本！
			exitUpgrade();
		}
	}
 
	private final Handler mHandler = new Handler(){
		@Override
        public void handleMessage(Message msg) {   
			
			if (mNeedStopUpgrade)
				return;
			
            switch (msg.what){
                case PMSG_GET_UPGRADE_FAILED:
                	showMessage(getResources().getString(R.string.string_detect_version_failed));//检测版本失败，请检查网络或更新服务器！
                	mCheckVersionView.setVisibility(View.GONE);
            		mDispVersionView.setVisibility(View.VISIBLE);
            		mCmriView.setVisibility(View.VISIBLE);
            		mUpgradeProgView.setVisibility(View.GONE);
            		exitUpgrade();
                	break;
                case PMSG_GET_UPGRADE_SUCCESS:
                	mCheckVersionView.setVisibility(View.GONE);
            		mDispVersionView.setVisibility(View.VISIBLE);
            		mCmriView.setVisibility(View.VISIBLE);
            		mUpgradeProgView.setVisibility(View.GONE);
            		showNewVersion();
                	break;
                case PMSG_ERROR_NOSDCARD:
                	mBtnUpgrade.setEnabled(true);
                	showMessage(getResources().getString(R.string.string_need_sdcard));//更新失败，请插入SD卡！
                	mCheckVersionView.setVisibility(View.GONE);
            		mDispVersionView.setVisibility(View.VISIBLE);
            		mCmriView.setVisibility(View.VISIBLE);
            		mUpgradeProgView.setVisibility(View.GONE);
            		exitUpgrade();
                	break;
                case PMSG_ERROR_APK_SOURCE:
                	mBtnUpgrade.setEnabled(true);
                	showMessage(getResources().getString(R.string.string_server_error));//更新失败，请检查更新服务器错误！
                	mCheckVersionView.setVisibility(View.GONE);
            		mDispVersionView.setVisibility(View.VISIBLE);
            		mCmriView.setVisibility(View.VISIBLE);
            		mUpgradeProgView.setVisibility(View.GONE);
            		exitUpgrade();
                	break;
                case PMSG_ERROR_SD_NOMEMORY:
                	mBtnUpgrade.setEnabled(true);
                	showMessage(getResources().getString(R.string.string_sdcard_no_mem));//更新失败，SD卡空间不足！
                	mCheckVersionView.setVisibility(View.GONE);
            		mDispVersionView.setVisibility(View.VISIBLE);
            		mCmriView.setVisibility(View.VISIBLE);
            		mUpgradeProgView.setVisibility(View.GONE);
            		exitUpgrade();
                	break;
                case PMSG_DOWNLOAD_FAILED:
                	mBtnUpgrade.setEnabled(true);
                	showMessage(getResources().getString(R.string.string_v_network_error)); //更新失败，请检查网络！
                	mCheckVersionView.setVisibility(View.GONE);
            		mDispVersionView.setVisibility(View.VISIBLE);
            		mCmriView.setVisibility(View.VISIBLE);
            		mUpgradeProgView.setVisibility(View.GONE);
            		exitUpgrade();
                	break;
                case PMSG_DOWNLOAD_SUCCESS:
                	installApk();
                	exitUpgrade();
                	break;
            }
		}
	};
	
	private class DownloadThread extends AsyncTask<Void, Integer, Integer>
    {
		String mDownUrl = "";
		
		public DownloadThread(final String url) {
			mDownUrl = url;
		}
		
		@Override
		protected Integer doInBackground(Void... params) {
			Log.d(TAG,"doInBackground");
			BufferedOutputStream bout=null;
		    FileOutputStream outStream = null;
		    InputStream inStream = null;
		    boolean success = false;
		    
		    HttpParams  httpParameters = new BasicHttpParams();// Set the timeout in milliseconds until a connection is established.   
            HttpConnectionParams.setConnectionTimeout(httpParameters, CONNECT_TIMEOUT);// Set the default socket timeout (SO_TIMEOUT) // in milliseconds which is the timeout for waiting for data.   
            HttpConnectionParams.setSoTimeout(httpParameters, SOCKET_TIMEOUT);
            
			HttpClient client = new DefaultHttpClient(httpParameters);
			HttpGet request = new HttpGet();
 
			try {
 
		    	String downloadDir = getDownloadDir();
		    	if (null==downloadDir)
		    	{
					mHandler.sendEmptyMessage(PMSG_ERROR_NOSDCARD);
					return 0;
		    	}
 
		    	NetworkInfo networkInfo = ((ConnectivityManager)getSystemService(Context.CONNECTIVITY_SERVICE))  
                .getActiveNetworkInfo();  
	             // 如果是使用的运营商网络   
	             if (networkInfo.getType() == ConnectivityManager.TYPE_MOBILE) {  
	                 // 获取默认代理主机ip   
	                 String host = android.net.Proxy.getDefaultHost();  
	                 // 获取端口   
	                 int port = android.net.Proxy.getDefaultPort();  
	                 if (host != null && port != -1) {  
	                     // 封装代理連接主机IP与端口号。   
	                     HttpHost proxy = new HttpHost(host, port);  
	                     client.getParams().setParameter(ConnRoutePNames.DEFAULT_PROXY,  
	                             proxy); 
	             
	                 }  
	             }  
	             
	            request.setURI(new URI(mDownUrl));
	            HttpResponse response = client.execute(request);
			    StatusLine status = response.getStatusLine();
			   
			    HttpEntity entity = response.getEntity();
			    long downloadedSize = 0;
			    long totalSize = entity.getContentLength();
			    
			    if (totalSize<0)
			    {
			    	mHandler.sendEmptyMessage(PMSG_ERROR_APK_SOURCE);
					return 0;
			    }
			    
			    if (getFreeSizeOfSd(downloadDir)<totalSize+1024)
		    	{
		    		mHandler.sendEmptyMessage(PMSG_ERROR_SD_NOMEMORY);
					return 0;
		    	}
 
			    mDownApkPath = downloadDir+SAVE_APK_NAME;
			    if (entity != null) {
			    	inStream = entity.getContent();
					byte buf[] = new byte[512 * 1024];
					int numBytesRead;
					
					outStream =  new FileOutputStream(mDownApkPath);
					
					bout = new BufferedOutputStream(
							outStream);
					do {
						numBytesRead = inStream.read(buf);
						if (numBytesRead > 0) {
							bout.write(buf, 0, numBytesRead);
							downloadedSize += numBytesRead;
							int percent = (int)((downloadedSize*100)/totalSize);
							publishProgress(percent, (int)downloadedSize, (int)totalSize);
						}
						if (mNeedStopUpgrade)
						{
							return 0;
						}
						 
					} while (numBytesRead > 0);
 
			    }
			    success = true;
			} catch (Exception e) {
				 // TODO Auto-generated catch block
				e.printStackTrace();
				mHandler.sendEmptyMessage(PMSG_DOWNLOAD_FAILED);
			}finally{
				try {
					if (null!=inStream)
					{
						inStream.close();
						inStream = null;
					}
					if (null!=bout)
					{
						bout.flush();
						bout.close();
					}
					if (outStream!=null)
						outStream.close();
				} catch (IOException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
				 
				if (success)
					mHandler.sendEmptyMessage(PMSG_DOWNLOAD_SUCCESS);
			}
			return null;
		}

		@Override
		protected void onProgressUpdate(Integer... values) {
			//to do
			mProgressBar.setProgress(values[0]);
			mTvPercent.setText(Integer.toString(values[0])+"%") ; 
			mTvDownloaded.setText(getFormatSize(values[1])+"/"+getFormatSize(values[2]));
			super.onProgressUpdate(values);
		}
		
		@Override  
		protected void onPreExecute() {  
			mCmriView.setVisibility(View.GONE);
    		mUpgradeProgView.setVisibility(View.VISIBLE);
		}  

		@Override 
		protected void onPostExecute(Integer result) {  
			Log.d(TAG,"BootProgress onPostExecute");
		 
			mCmriView.setVisibility(View.VISIBLE);
    		mUpgradeProgView.setVisibility(View.GONE);
	        
	        super.onPostExecute(result);
	    } 
	}
	
	private String getFormatSize(int size)
	{
		String retStr = "";
		try {
			DecimalFormat format1 = new DecimalFormat("#.00");
			//String premium = format1.format(mNewPolicyInfo.premiumAmount);
			
			if (size>1024*1024)
			{
				retStr = format1.format(size/(1024.0f*1024.0f));
				retStr += "MB";
			}
			else if (size>1024)
			{
				retStr = format1.format(size/1024.0f);
				retStr += "KB";
			}
			else{
				retStr = format1.format(size);
				retStr += "B";
			}
			return retStr;
		} catch (Exception e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
			return "0B";
		}
	}
 
	/**
     *  
     * @param url
     */
	private void installApk(){
		File apkfile = new File(mDownApkPath);
        if (!apkfile.exists()) {
            return;
        }    
        Intent intent = new Intent(Intent.ACTION_VIEW);
        intent.setDataAndType(Uri.parse("file://" + apkfile.toString()), "application/vnd.android.package-archive"); 
        startActivity(intent);
	
	}
	
	private String getDownloadDir()
    {
		File sdDir = null; 
		String sdPath = null;
		boolean sdCardExist = Environment.getExternalStorageState()
		                    .equals(android.os.Environment.MEDIA_MOUNTED);
		if(sdCardExist)   //use sd card
		{                               
		  sdDir = Environment.getExternalStorageDirectory();
		  String sdroot = sdDir.getAbsolutePath() ;
		  sdPath = sdroot+"/wimo/";
		  
		  if (createFolder(sdPath))
		  { 
			  return sdPath;
		  }
		  else
		  {
			  return null;
		  }
		}
		
		return sdPath;
    }
	
	private boolean createFolder(String folder)
    {
    	try  {
    		File recDir  =   new  File(folder);
    		if ( !recDir.exists() )  
    		{
    			boolean  creadok  =  recDir.mkdirs();
    			if (creadok)  {
    				return true;
    			} else  {
     
    				return false;
    			} 
    		} 
    		else if (!recDir.canWrite() || recDir.isFile())
    			return false;
    	} catch (Exception e)  {
    		e.printStackTrace();
    		System.out.println(e);
    		return false ;
    	} 
    	
    	return true;
    }
	
	public long getFreeSizeOfSd(String sdpath)
	{
		 
		File sdDir = new File(sdpath);
		if (!sdDir.canWrite()||sdDir.isFile())
			return 0;
		
		StatFs statfs=new StatFs(sdpath); 
		if (statfs==null)
			return 0;
		//get block size, in bytes
		long blocSize=statfs.getBlockSize(); 
		//get BLOCK count 
		long totalBlocks=statfs.getBlockCount(); 
		//Available Block count
		long availaBlock=statfs.getAvailableBlocks(); 

		return availaBlock*blocSize;
	}
 
	public static class UpgradeResponseParam
	{
		public boolean need_update;//是否需要更新，
		public String app_name;    // app的名称，
		public long app_build;     // app的build号，
		public String app_date;    //app的发布日期，（如2012-04-27）
		public String app_desc;    // app的版本说明，
		public String app_version; // app的版本号，
		public String app_url;     //app的下载URL，
	}

}
