package com.april.demoPix;

public class DemoPix extends com.april.Activity
{
	static
	{
		System.loadLibrary("demo_pix");
	}
	
	@Override
	protected void onCreate(android.os.Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);
		this.forceArchivePath(com.april.NativeInterface.apkPath); // forces APK as archive file
	}
	
}
