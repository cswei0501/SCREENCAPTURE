package cidana.wimo2;

import android.app.Activity;
import android.content.Context;
import android.os.Bundle;
import android.view.GestureDetector;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.view.Window;
import android.view.GestureDetector.OnGestureListener;
import android.view.View.OnTouchListener;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.RadioGroup;
import android.widget.ViewFlipper;

public class UsageGuiderActivity extends Activity {

	private ViewFlipper mViewFlipper = null;
	private GestureDetector mGestureDetector;
	//private RadioGroup mPageGroup = null;
	private ImageView mImgNavigator = null;
	private LayoutInflater mInflater = null;
	private int mCurChildIndex = 0;
	private final int HORIZONTAL_FLING_THRESHOLD = 20;
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		super.onCreate(savedInstanceState);
		requestWindowFeature(Window.FEATURE_NO_TITLE);
		setContentView(R.layout.usage_guider);
		initUI();
	}
	
	private void initUI()
	{
		mInflater = (LayoutInflater) getSystemService(Context.LAYOUT_INFLATER_SERVICE);
		mGestureDetector = new GestureDetector(mGestureListener);
		mViewFlipper = (ViewFlipper)findViewById(R.id.flipper_usage);
		//mPageGroup = (RadioGroup)findViewById(R.id.page_group);
		mImgNavigator = (ImageView)findViewById(R.id.img_navigator);
		addViewToFlipper();
	}

	@Override
	protected void onDestroy() {
		// TODO Auto-generated method stub
		super.onDestroy();
	}

	@Override
	protected void onResume() {
		// TODO Auto-generated method stub
		super.onResume();
	}

	@Override
	protected void onStop() {
		// TODO Auto-generated method stub
		super.onStop();
	}
	
	private void addViewToFlipper()
	{
 
		int viewArray[] = {R.layout.usage_page1, R.layout.usage_page2,
				R.layout.usage_page3,R.layout.usage_page4};
		
		for (int i=0;i<viewArray.length;i++)
		{
			ViewGroup.LayoutParams param = new ViewGroup.LayoutParams(ViewGroup.LayoutParams.FILL_PARENT,ViewGroup.LayoutParams.FILL_PARENT);
			View view = mInflater.inflate(viewArray[i], null);
			
			if (viewArray.length-1==i) //the last page
			{
				Button btnStartEnjoy = (Button)view.findViewById(R.id.btn_enjoy);
				btnStartEnjoy.setOnClickListener(new OnClickListener()
				{

					@Override
					public void onClick(View v) {
						// TODO Auto-generated method stub
						finish();
					}
					
				});
			}

			mViewFlipper.addView(view, param); 
		}
	}
	
	OnGestureListener mGestureListener = new OnGestureListener()
	{
		@Override
		public boolean onDown(MotionEvent e) {
			// TODO Auto-generated method stub
			return true;
		}

		@Override
		public void onShowPress(MotionEvent e) {
			// TODO Auto-generated method stub
			
		}

		@Override
		public boolean onSingleTapUp(MotionEvent e) {
			// TODO Auto-generated method stub
			return false;
		}

		@Override
		public boolean onScroll(MotionEvent e1, MotionEvent e2, float distanceX,
				float distanceY) {
			// TODO Auto-generated method stub
			return false;
		}

		@Override
		public void onLongPress(MotionEvent e) {
			// TODO Auto-generated method stub
			
		}

		@Override
		public boolean onFling(MotionEvent e1, MotionEvent e2, float velocityX,
				float velocityY) {
			// TODO Auto-generated method stub
			
			int imageId[] = {R.drawable.navigator1, R.drawable.navigator2, R.drawable.navigator3, R.drawable.navigator4};
			
			if( e1.getX() > e2.getX() && (e1.getX()-e2.getX()) > HORIZONTAL_FLING_THRESHOLD ) //move to left 
			{  
				mViewFlipper.setInAnimation(UsageGuiderActivity.this, R.anim.slide_left_in);   
				mViewFlipper.setOutAnimation(UsageGuiderActivity.this, R.anim.slide_left_out);    
				
				mCurChildIndex = (mCurChildIndex<(mViewFlipper.getChildCount()-1))?mCurChildIndex++:0;
 
	        	mViewFlipper.showNext();
				
			}
			else if(e1.getX() < e2.getX() && (e2.getX()-e1.getX()) > HORIZONTAL_FLING_THRESHOLD ) 
			{   
	    		mViewFlipper.setInAnimation(UsageGuiderActivity.this, R.anim.slide_right_in);   
	        	mViewFlipper.setOutAnimation(UsageGuiderActivity.this, R.anim.slide_right_out);  
	        	
	        	 
	        	mViewFlipper.showPrevious();
	        	mCurChildIndex = (mCurChildIndex>0)?mCurChildIndex--:(mViewFlipper.getChildCount()-1);

			}
			else
				return false;
			
			try {
				int index = mViewFlipper.getDisplayedChild();
				mImgNavigator.setBackgroundResource(imageId[index]);
			} catch (Exception e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
			
			return true;  

		}
	};

	@Override
	public boolean onTouchEvent(MotionEvent event) {
		// TODO Auto-generated method stub
		//return super.onTouchEvent(event);
		
		return mGestureDetector.onTouchEvent(event);
	}

}
