package cidana.wimo2;

import java.util.ArrayList;

import android.app.Activity;
import android.content.Context;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.View.OnClickListener;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.TextView;

public class SelectDeviceDialog extends Activity {
	
	private ListView mLvDevices = null;
	private DeviceListAdapter mDeviceAdapter = null;
	private ArrayList<Object> mDeviceList = null;
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		super.onCreate(savedInstanceState);
		
		setContentView(R.layout.select_device_dlg);
		
		fillDevice();
		initUI();
		
	}
	
	private void initUI()
	{
		mLvDevices = (ListView)findViewById(R.id.lv_devices);
	 
		mDeviceAdapter = new DeviceListAdapter(SelectDeviceDialog.this);
		mLvDevices.setAdapter(mDeviceAdapter);
		mLvDevices.setOnItemClickListener(mDeviceItemClickListener);
	}
	
	private void fillDevice()
	{
		mDeviceList = new ArrayList<Object>();
		for (int i=0;i<5;i++)
		{
			DeviceInfo device = new DeviceInfo();
			if (i==1)
				device.type = 0;
			else
				device.type = 1;
			device.name = getResources().getString(R.string.string_dev_name) + i;
			
			mDeviceList.add(device);
		}
	}
	
	private OnItemClickListener mDeviceItemClickListener = new OnItemClickListener()
	{

		@Override
		public void onItemClick(AdapterView<?> arg0, View arg1, int arg2,
				long arg3) {
			// TODO Auto-generated method stub
			DeviceInfo device = (DeviceInfo) mDeviceAdapter.getItem(arg2);
			setResult(Activity.RESULT_OK);
			finish();
		}
		
	};
	
	private class DeviceListAdapter extends BaseAdapter {
        private LayoutInflater mInflater;
        
        

        public DeviceListAdapter(Context context) {
 
	        // Cache the LayoutInflate to avoid asking for a new one each time.
	        mInflater = LayoutInflater.from(context);
	        
	    }
	
	    /**
	     * The number of items in the list is determined by the number of speeches
	     * in our array.
	     *
	     * @see android.widget.ListAdapter#getCount()
	     */
	    public int getCount() {
	        return (mDeviceList==null)?0:mDeviceList.size();
	    }
	
	    /**
	     * Since the data comes from an array, just returning the index is
	     * sufficent to get at the data. If we were using a more complex data
	     * structure, we would return whatever object represents one row in the
	     * list.
	     *
	     * @see android.widget.ListAdapter#getItem(int)
	     */
	    public Object getItem(int position) {
	        return mDeviceList.get(position);
	    }
	
	    /**
	     * Use the array index as a unique id.
	     *
	     * @see android.widget.ListAdapter#getItemId(int)
	     */
	    public long getItemId(int position) {
	        return position;
	    }
	
	    /**
	     * Make a view to hold each row.
	     *
	     * @see android.widget.ListAdapter#getView(int, android.view.View,
	     *      android.view.ViewGroup)
	     */
	    public View getView(int position, View convertView, ViewGroup parent) {
	        // A ViewHolder keeps references to children views to avoid unneccessary calls
	        // to findViewById() on each row.
	        ViewHolder holder;
	
	        // When convertView is not null, we can reuse it directly, there is no need
	        // to reinflate it. We only inflate a new View when the convertView supplied
	        // by ListView is null.
	        if (convertView == null) {
	            convertView = mInflater.inflate(R.layout.device_item, null);
	
	            // Creates a ViewHolder and store references to the two children views
	            // we want to bind data to.
	            holder = new ViewHolder();
	            
	            holder.deviceIcon = (ImageView) convertView.findViewById(R.id.img_device);
	            holder.deviceName = (TextView) convertView.findViewById(R.id.tv_device);
	           
	            
	            convertView.setTag(holder);
	        } else {
	            // Get the ViewHolder back to get fast access to the TextView
	            // and the ImageView.
	            holder = (ViewHolder) convertView.getTag();
	        }
	         	
	        // Bind the data efficiently with the holder.
	        DeviceInfo device = (DeviceInfo) mDeviceList.get(position);
	        if (device.type==0)
	        	holder.deviceIcon.setBackgroundResource(R.drawable.icon_pc);
	        else
	        	holder.deviceIcon.setBackgroundResource(R.drawable.icon_router);
	        holder.deviceName.setText(device.name);
	
	        return convertView;
	    }
	
	    class ViewHolder {
	    	ImageView deviceIcon;
	        TextView deviceName;
	    }

		@Override
		public void notifyDataSetChanged() {
			// TODO Auto-generated method stub
			super.notifyDataSetChanged();
			
		}
	}
	
	private class DeviceInfo
	{
		int type;
		String name;
		String ip;
	}
}
