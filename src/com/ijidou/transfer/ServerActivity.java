package com.ijidou.transfer;

import java.util.List;

import android.app.Activity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.ListView;
import android.widget.TextView;

import com.ijidou.transfer.Server.OnInfoListener;
import com.ijidou.transfer.Server.Record;

public class ServerActivity extends Activity {

	private Server server;
	private ListView mListView;
	private List<Record> datas;
	
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_test);
        
        Button btn = (Button) findViewById(R.id.send);
        btn.setOnClickListener(new OnClickListener()
		{
			
			@Override
			public void onClick(View v)
			{
				// TODO Auto-generated method stub
				server = new Server();
				server.setOnInfoListener(mOnInfoListener);
			}
		});
        
        
        mListView = (ListView) findViewById(R.id.list_view);
        
        mListView.setOnItemClickListener(new OnItemClickListener()
		{

			@Override
			public void onItemClick(AdapterView<?> parent, View view,
					int position, long id)
			{
				// TODO Auto-generated method stub
				server.receiveFiles(datas.get(position).path, "/sdcard/" + datas.get(position).name);
			}
		});
    }
    
    private OnInfoListener mOnInfoListener = new OnInfoListener()
	{

		@Override
		public void onInfo(final List<Record> info)
		{
			// TODO Auto-generated method stub
			datas = info;
			
			for (Record rec : info)
			{
				Log.v("server","=== " + rec.name);
			}
			mListView.post(new Runnable(){

				@Override
				public void run()
				{
					// TODO Auto-generated method stub
					mListView.setAdapter(new FileAdapter(info));
				}});
		}

	};
	
	
	class FileAdapter extends BaseAdapter
	{
		private List<Record> flist;

		FileAdapter(List<Record> list)
		{
			flist = list;
		}
		
		@Override
		public int getCount()
		{
			// TODO Auto-generated method stub
			return flist == null ? 0 : flist.size();
		}

		@Override
		public Object getItem(int position)
		{
			// TODO Auto-generated method stub
			return null;
		}

		@Override
		public long getItemId(int position)
		{
			// TODO Auto-generated method stub
			return 0;
		}

		@Override
		public View getView(int position, View convertView, ViewGroup parent)
		{
			// TODO Auto-generated method stub
			if (convertView == null)
			{
				convertView = new TextView(ServerActivity.this);
			}
			
			((TextView) convertView).setText(flist.get(position).name);
			
			return convertView;
		}
		
	}

}
