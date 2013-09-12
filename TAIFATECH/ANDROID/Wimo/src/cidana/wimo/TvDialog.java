package cidana.wimo;

import java.lang.reflect.InvocationTargetException;
import java.util.ArrayList;
import java.util.HashMap;

import com.cmcc.wimo.WiMoControl;

import android.app.Activity;
import android.content.Context;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.AdapterView;
import android.widget.Button;
import android.widget.ListView;
import android.widget.SimpleAdapter;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.Toast;

public class TvDialog extends Activity {
	private ListView mListView;
	private ArrayList<HashMap<String, Object>> mListItem;
	private SimpleAdapter mAdapter;
	private Button mBtnYes;
	private Button mBtnNo;
	private CaptureContainer mCapContainer = null;
	private int mListIndex = 0;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		super.onCreate(savedInstanceState);
		setContentView(R.layout.tv_dlg);

		mCapContainer = CaptureContainer.getInstance(TvDialog.this);
		initUI();
		mListItem = new ArrayList<HashMap<String, Object>>();

		HashMap<String, Object> map1 = new HashMap<String, Object>();
		HashMap<String, Object> map2 = new HashMap<String, Object>();
		HashMap<String, Object> map3 = new HashMap<String, Object>();

		map1.put("radio_img", R.drawable.list_radio_nomal);
		map2.put("radio_img", R.drawable.list_radio_nomal);
		map3.put("radio_img", R.drawable.list_radio_nomal);
		
		mListIndex = getTVOrientation(this.getApplicationContext());
		switch(mListIndex){
		case 1:
			map2.put("radio_img", R.drawable.list_radio_chosen);
			break;
		case 2:
			map3.put("radio_img", R.drawable.list_radio_chosen);
			break;
		default:
			map1.put("radio_img", R.drawable.list_radio_chosen);
			break;
		}

		map1.put("text",getString(R.string.install_way1));
		map2.put("text", getString(R.string.install_way2));
		map3.put("text", getString(R.string.install_way3));

		mListItem.add(map1);
		mListItem.add(map2);
		mListItem.add(map3);

		mAdapter = new SimpleAdapter(this, mListItem, R.layout.tv_dlg_item,
				new String[] { "radio_img", "text" }, new int[] {
						R.id.list_radio_img, R.id.item_text });

		mListView.setAdapter(mAdapter);
		mListView.setOnItemClickListener(installChoose);
	}

	private void initUI() {
		mListView = (ListView) findViewById(R.id.list);
		mBtnYes = (Button) findViewById(R.id.btn_ok);
		mBtnNo = (Button) findViewById(R.id.btn_no);

		if (mBtnYes != null)
			mBtnYes.setOnClickListener(btnYesAct);
		if (mBtnNo != null)
			mBtnNo.setOnClickListener(btnNoAct);
	}

	private OnClickListener btnYesAct = new OnClickListener() {

		@Override
		public void onClick(View v) {
			// TODO Auto-generated method stub

			switch(mListIndex)
			{
			case 0:
				WiMoControl.SetTVRotate(MainActivity.CURRENT_TV_ROTATE_0);
				break;
			case 1:
				WiMoControl.SetTVRotate(MainActivity.CURRENT_TV_ROTATE_90);
				break;
			case 2:
				WiMoControl.SetTVRotate(MainActivity.CURRENT_TV_ROTATE_270);
				break;
			}
				
			setTVOrientation(TvDialog.this.getApplicationContext(), mListIndex);
			
			finish();
		}
	};

	private OnClickListener btnNoAct = new OnClickListener() {

		@Override
		public void onClick(View v) {
			// TODO Auto-generated method stub
			finish();
		}
	};

	private OnItemClickListener installChoose = new OnItemClickListener() {

		@Override
		public void onItemClick(AdapterView<?> arg0, View arg1, int arg2,
				long arg3) {
			
			changeRadioImg(mListIndex, false);
			changeRadioImg(arg2, true);
			
			mListIndex = arg2;
		}

	};
	

	private void changeRadioImg(int selectedItem, boolean b) {

		SimpleAdapter adapter = mAdapter;

		HashMap<String, Object> map = (HashMap<String, Object>) adapter
				.getItem(selectedItem);

		if (b)
			map.put("radio_img", R.drawable.list_radio_chosen);
		else
			map.put("radio_img", R.drawable.list_radio_nomal);

		adapter.notifyDataSetChanged();

	}

	
	public static int getTVOrientation(Context context){
		SharedPreferences sysPref = context.getSharedPreferences("sys_param", 0);
		
		int ret = sysPref.getInt("tv_orientation", 0);
		if(ret<0 || ret > 2)
			ret = 0;
		
		return ret; 
	}
	
	public static void setTVOrientation(Context context, int ori)
	{
		SharedPreferences.Editor editor;
		try {
			SharedPreferences sysPref = context.getSharedPreferences("sys_param", 0);
			editor = sysPref.edit();
			editor.putInt("tv_orientation", ori);
			editor.commit();
		} catch (Exception e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}
}
