package com.ijidou.transfer;

import java.io.File;
import java.util.List;

import org.json.JSONArray;
import org.json.JSONObject;

import android.util.Log;



public class Client 
{
	private int mObject;
	
	public static final String TAG = "client";
	
	private String serverIp;
	
	public native final void native_init();
	private native final void native_destroy();
	
	private native void native_sendHeartBeat();

	public native void native_listenHeartBeatBack(OnFindServerListener l);
	
	public native void native_sendtoServer(String ip, String data);
	
	private native void native_sendFiles();
	
	public Client()
	{
		native_init();
		sendHeartBeat();
		receiveHeartBeatBack();
	}
	
	public interface OnFindServerListener
	{
		public void onFind(String ip);
	}
	
	private OnFindServerListener onServerFeedBackListener = new OnFindServerListener()
	{

		@Override
		public void onFind(String ip)
		{
			// TODO Auto-generated method stub
			Log.v(TAG, "feed back from server " + ip);
			if (serverIp == null)
			{
			    serverIp = ip;
			    sendFileInfo();
			    sendFiles();
			}
		}
	};
	
	private void sendFileInfo()
	{
		new Thread(
				new Runnable(){
					public void run() 
					{
						FileScanner util = new FileScanner();
						List<File> list = util.getFiles();
						
						try
						{
							sendInfo("[begin]");
							for (File file : list)
							{
								JSONObject obj = new JSONObject();
								obj.put("name", file.getName());
								obj.put("path", file.getAbsolutePath());
								sendInfo(obj.toString());
							}
							sendInfo("[end]");
						}
						catch (Exception e)
						{
							
						}
					}
				}
				).start();
	}
	
	private void sendHeartBeat() 
	{
		new Thread(
				new Runnable(){
					public void run() 
					{
						native_sendHeartBeat();
					}
				}
				).start();
	}
	
	public void sendInfo(String text)
	{
		if (serverIp == null)
		{
			return ;
		}
		
		Log.v(TAG, "length in java = " + text.length());
		native_sendtoServer(serverIp, text);
	}
	
	public void sendFiles()
	{
		new Thread(new Runnable()
		{
			public void run()
			{
				native_sendFiles();
			}
		}
		).start();
	}
	
	private void receiveHeartBeatBack() 
	{
		new Thread(
				new Runnable(){
					public void run() 
					{
						native_listenHeartBeatBack(onServerFeedBackListener);
					}
				}
				).start();
	}
}
