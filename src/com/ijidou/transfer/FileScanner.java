package com.ijidou.transfer;

import java.io.File;
import java.io.FilenameFilter;
import java.util.ArrayList;
import java.util.List;

import android.os.Environment;
import android.util.Log;

public class FileScanner
{
	public static final String ROOT = Environment.getExternalStorageDirectory().getAbsolutePath();
	
	private List<File> fileList = new ArrayList<File> ();
	
	private void getFiles(String path) 
	{
		File file = new File(path);   
		File[] list = file.listFiles();
		if (list == null || list.length <= 0)
		{
			return;
		}
		
		for (int i = 0; i < list.length; i++)
		{
			if (list[i].isDirectory())
			{
				getFiles(list[i].getAbsolutePath());
			} 
			else if (list[i].isFile() && list[i].getName().endsWith("mp3"))
			{
				fileList.add(list[i]);
			}
		}

	}

	public List<File> getFiles()
	{
		getFiles(ROOT);
		for (int i = 0; i < fileList.size(); i++)
		{
			Log.v("file", fileList.get(i).getName());
		}
		
		return fileList;
	}
	
	class MideaFilter implements FilenameFilter
	{

		@Override
		public boolean accept(File dir, String filename)
		{
			// TODO Auto-generated method stub
			boolean ret = filename.endsWith(".mp3") || filename.endsWith(".mp4");    
            return ret;
		}
		
	}
}
