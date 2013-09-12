package cidana.wimo;

import android.app.Dialog;
import android.content.Context;
import android.graphics.Color;
import android.os.Bundle;
import android.view.View;
import android.view.Window;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.TextView;

public class CiAlertDialog extends Dialog {

	private TextView mTvTitle = null;
	private TextView mTvMessage = null;
	private LinearLayout mButtonContainer = null;
	private Context mContext;
	
	 
	
	public CiAlertDialog(Context context) {
		super(context, R.style.Theme_CiAlertDialog);
		// TODO Auto-generated constructor stub
		mContext = context;
		
		setContentView(R.layout.alert_dlg); 
		initUI();
		
	}

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		super.onCreate(savedInstanceState);
 	 
	}
	
	private void initUI()
	{
		mTvTitle = (TextView)findViewById(R.id.tv_title);
		mTvMessage = (TextView)findViewById(R.id.tv_msg);
		mButtonContainer = (LinearLayout)findViewById(R.id.button_container);
	}

	@Override
	protected void onStop() {
		// TODO Auto-generated method stub
		super.onStop();
	}
	
	 

	@Override
	public void setTitle(CharSequence title) {
		// TODO Auto-generated method stub
		super.setTitle(title);
		mTvTitle.setText(title);
	}

	@Override
	public void setTitle(int titleId) {
		// TODO Auto-generated method stub
		super.setTitle(titleId);
		mTvTitle.setText(titleId);
	}
 
	public void setMessage(CharSequence msg) {
		// TODO Auto-generated method stub
		mTvMessage.setText(msg);
	}
 
	public void setMessage(int msgId) {
		// TODO Auto-generated method stub
		mTvMessage.setText(msgId);
	}
	
	public void setPositiveButton(int textId, final View.OnClickListener listener) {
       Button btnPositive = new Button(mContext);
       btnPositive.setBackgroundResource(R.drawable.button_common);
       btnPositive.setText(textId);
       btnPositive.setTextColor(Color.rgb(0xff, 0xff, 0xff));
       btnPositive.setTextSize(18);
       btnPositive.setOnClickListener(new View.OnClickListener(){

		@Override
		public void onClick(View v) {
			// TODO Auto-generated method stub
			if (listener!=null)
				listener.onClick(v);
			dismiss();
		}
    	   
       });
       if (mButtonContainer.getChildCount()>0)
       {
    	   LinearLayout.LayoutParams param = new LinearLayout.LayoutParams(LinearLayout.LayoutParams.WRAP_CONTENT,LinearLayout.LayoutParams.WRAP_CONTENT);
    	   param.leftMargin = getValidPixels(10);
    	   mButtonContainer.addView(btnPositive, param);
       }
       else
    	   mButtonContainer.addView(btnPositive);
       
    }
	
	public void setNegativeButton(int textId, final View.OnClickListener listener) {
		Button btnNegative = new Button(mContext);
		btnNegative.setBackgroundResource(R.drawable.button_common);
		btnNegative.setText(textId);
		btnNegative.setTextColor(Color.rgb(0xff, 0xff, 0xff));
		btnNegative.setTextSize(18);
		btnNegative.setOnClickListener(new View.OnClickListener(){

			@Override
			public void onClick(View v) {
				// TODO Auto-generated method stub
				if (listener!=null)
					listener.onClick(v);
				dismiss();
			}
	    	   
	       });
       
		if (mButtonContainer.getChildCount()>0)
	       {
	    	   LinearLayout.LayoutParams param = new LinearLayout.LayoutParams(LinearLayout.LayoutParams.WRAP_CONTENT,LinearLayout.LayoutParams.WRAP_CONTENT);
	    	   param.leftMargin = getValidPixels(10);
	    	   mButtonContainer.addView(btnNegative, param);
	       }
	       else
	    	   mButtonContainer.addView(btnNegative);
    }
	
	 
	private int getValidPixels(float valueDips)
	  	{
	  		return (int)(valueDips * mContext.getResources().getDisplayMetrics().density + 0.5f); // 0.5f for rounding
	  		
	  	}
}
