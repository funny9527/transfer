package com.ijidou.transfer;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;

public class MainActivity extends Activity {

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        
        Button ser = (Button) findViewById(R.id.server);
        Button clt = (Button) findViewById(R.id.client);
        
        ser.setOnClickListener(new OnClickListener()
		{
			
			@Override
			public void onClick(View v)
			{
				// TODO Auto-generated method stub
				startActivity(new Intent(MainActivity.this, ServerActivity.class));
			}
		});
        
        clt.setOnClickListener(new OnClickListener()
		{
			
			@Override
			public void onClick(View v)
			{
				// TODO Auto-generated method stub
				startActivity(new Intent(MainActivity.this, ClientActivity.class));
			}
		});
    }

    static
	{
		System.loadLibrary("transfer");
	}

}
