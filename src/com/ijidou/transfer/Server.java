package com.ijidou.transfer;

import java.net.InetAddress;
import java.net.NetworkInterface;
import java.net.SocketException;
import java.util.ArrayList;
import java.util.Enumeration;
import java.util.List;

import org.apache.http.conn.util.InetAddressUtils;
import org.json.JSONObject;

import android.util.Log;


public class Server 
{
	public static final String TAG = "server";
	
	private String clientIp;
	
	private int mObject;
	
	public native final void native_init();
	private native final void native_destroy();
	
	public native void native_listenHeartBeat(OnFindClientListener l);
	
	public native void native_receiveClientInfo(OnReceivedFromClientListener l);
	
	public native void native_sendtoClient(String ip, String data);
	
	private native void native_receiveFiles(String ip, String name, String newname);
	
	private OnInfoListener mOnInfoListener;
	
	private List<Record> mList = new ArrayList<Record> ();
	
	public Server()
	{
		native_init();
		listenHeartBeat();
		listentoClient();
	}
	
	private OnFindClientListener clientBroadCastListener = new OnFindClientListener()
	{

		@Override
		public void onFind(String ip)
		{
			// TODO Auto-generated method stub
			Log.v(TAG, "receive broadcast from client " + ip);
			clientIp = ip;
		}
		
	};
	
	private OnReceivedFromClientListener getClientInfoListener = new OnReceivedFromClientListener()
	{

		@Override
		public void onReceive(String info)
		{
			// TODO Auto-generated method stub
			Log.v(TAG, "receive info from client \n" + info);
			if ("[begin]".equals(info)) {
				mList.clear();
			} 
			else if ("[end]".equals(info))
			{
				if (mOnInfoListener != null)
				{
					mOnInfoListener.onInfo(mList);
				}
			}
			else if (info != null) 
			{
				try
				{
					JSONObject obj = new JSONObject(info);
					Record rec = new Record();
					rec.name = obj.optString("name");
					rec.path = obj.optString("path");
					
					mList.add(rec);
				}
				catch (Exception e)
				{
					
				}
			}
			
		}
		
	};
	
	private void listenHeartBeat() 
	{
		new Thread(
				new Runnable(){
					public void run() 
					{
						native_listenHeartBeat(clientBroadCastListener);
					}
				}
				).start();
	}
	
	private void listentoClient() 
	{
		Log.v(TAG, "listentoClient");
		new Thread(
				new Runnable(){
					public void run() 
					{
						native_receiveClientInfo(getClientInfoListener);
					}
				}
				).start();
	}
	
	public void sendInfo(String text)
	{
		if (clientIp == null)
		{
			return ;
		}
		
		native_sendtoClient(clientIp, text);
	}
	
	public void receiveFiles(final String name, final String newname)
	{
		if (clientIp == null)
		{
			return ;
		}
		
		new Thread(new Runnable()
		{
			public void run()
			{
				native_receiveFiles(clientIp, name, newname);
			}
		}).start();
	}
	
	public void setOnInfoListener(OnInfoListener lis)
	{
		mOnInfoListener = lis;
	}
	
	public static String getLocalHostIp()
    {
        String ipaddress = "";
        try
        {
            Enumeration<NetworkInterface> en = NetworkInterface.getNetworkInterfaces();
            while (en.hasMoreElements())
            {
                NetworkInterface nif = en.nextElement();
                Enumeration<InetAddress> inet = nif.getInetAddresses();
                while (inet.hasMoreElements())
                {
                    InetAddress ip = inet.nextElement();
                    if (!ip.isLoopbackAddress()
                            && InetAddressUtils.isIPv4Address(ip
                                    .getHostAddress()))
                    {
                        return ipaddress = ip.getHostAddress();
                    }
                }

            }
        }
        catch (SocketException e)
        {
            e.printStackTrace();
            Log.v(TAG, "get ip error " + e);
        }
        Log.v(TAG, "get server ip = " + ipaddress);
        
        return ipaddress;
    }	
	
	public interface OnReceivedFromClientListener
	{
		public void onReceive(String info);
	}
	
	public interface OnFindClientListener
	{
		public void onFind(String info);
	}
	
	public interface OnInfoListener
	{
		public void onInfo(List<Record> info);
	}
	
	public static class Record
	{
		String name;
		String path;
	}
}
