package cidana.wimo2;

import android.content.Context;
import android.graphics.Color;
import android.graphics.drawable.Drawable;
import android.graphics.drawable.ShapeDrawable;
import android.graphics.drawable.shapes.OvalShape;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnTouchListener;
import android.view.ViewGroup;
import android.widget.LinearLayout.LayoutParams;
import android.widget.PopupWindow;
import android.widget.RelativeLayout;
import android.widget.TableRow;

public class PopMenu {
	private static final String TAG = "PopMenu";
	
	private View mView ;
	private PopupWindow mPW;
	
	private View mMenuAbout = null;
	private View mMenuHelp = null;
	private View mMenuUpgrade = null;
	private View mMenuExit = null;
	private View mMenuTv = null;
	

	public PopMenu( Context context) {
		// TODO Auto-generated constructor stub
		LayoutInflater inflate = (LayoutInflater) context.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
		mView = (View) inflate.inflate(R.layout.popup_menu, null,false);
		
		mPW = new PopupWindow(mView, LayoutParams.FILL_PARENT, LayoutParams.WRAP_CONTENT, true);
		
		View rootView = (View)mView.findViewById(R.id.pop_menu_root);
		rootView.setOnClickListener(rootViewListener);
		mPW.setTouchable(true);
		Drawable d = context.getResources().getDrawable(R.drawable.bg_line);
		//ShapeDrawable dd = new ShapeDrawable(new OvalShape());
		//dd.getPaint().setColor(0xff3a3a3a);
        
		mPW.setBackgroundDrawable(d);
		mPW.setOutsideTouchable(true);
		/*mPW.setTouchInterceptor(new OnTouchListener()
		{

			@Override
			public boolean onTouch(View v, MotionEvent event) {
				// TODO Auto-generated method stub
				 if (event.getAction()==MotionEvent.ACTION_OUTSIDE)
				 {
					dismiss();
					return true;
				 }
				 
 
				return false;
			}
			
		});*/
		
		
		mMenuAbout = (View) mView.findViewById(R.id.menu_about);
		mMenuHelp = (View) mView.findViewById(R.id.menu_help);
		mMenuUpgrade = (View) mView.findViewById(R.id.menu_upgrade);
		mMenuExit = (View) mView.findViewById(R.id.menu_exit);
		mMenuTv = (View) mView.findViewById(R.id.menu_tv);
		mMenuAbout.setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View v) {
				if(mMenuItemListener != null)
					mMenuItemListener.onAboutClickListener();
				
				dismiss();
			}
		});
		mMenuTv.setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View v) {
				if(mMenuItemListener != null)
					mMenuItemListener.onTvClickListener();

				dismiss();
			}
		});
		mMenuHelp.setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View v) {
				if(mMenuItemListener != null)
					mMenuItemListener.onHelpClickListener();

				dismiss();
			}
		});
		mMenuUpgrade.setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View v) {
				if(mMenuItemListener != null)
					mMenuItemListener.onUpradeClickListener();

				dismiss();
			}
		});
		mMenuExit.setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View v) {
				if(mMenuItemListener != null)
					mMenuItemListener.onExitClickListener();

				dismiss();
			}
		});
	}
	
	private View.OnClickListener rootViewListener = new View.OnClickListener() {
		
		@Override
		public void onClick(View v) {
			// TODO Auto-generated method stub
			dismiss();
		}
	};
	
	public void show(){
		if(mPW !=null/* && mPW.isShowing()*/){
			//mPW.setAnimationStyle(anim.slide_in_left);
			mPW.setAnimationStyle(R.style.PopupAnimation);
			mPW.showAtLocation(mView, Gravity.BOTTOM, 0, 0);
			mPW.setFocusable(false);
			mPW.setTouchable(true);
			mPW.setOutsideTouchable(true);
			mPW.update();
		 
		}
	}
	
	public void dismiss(){
		if(mPW != null && mPW.isShowing()){
			//mPW.setAnimationStyle(anim.slide_out_right);
			mPW.dismiss();
		}
	}
	
	public boolean isShowing(){
		if(mPW != null && mPW.isShowing())
			return true;
		return false;
	}

	
	
	
	public interface onClickMenuItemListener{
		public void onAboutClickListener ();
		public void onHelpClickListener();
		public void onUpradeClickListener();
		public void onExitClickListener();
		public void onTvClickListener();
	}
	
	private onClickMenuItemListener mMenuItemListener = null;
	
	public void setMenuItemClickListener (onClickMenuItemListener listener){
		mMenuItemListener = listener;
	}
}
