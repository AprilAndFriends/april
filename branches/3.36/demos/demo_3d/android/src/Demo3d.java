package com.april.demo3d;

public class Demo3d extends com.april.Activity
{
	static
	{
		System.loadLibrary("demo_3d");
	}
	
	@Override
	protected void onCreate(android.os.Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);
		this.forceArchivePath(com.april.NativeInterface.ApkPath); // forces APK as archive file
	}
	
}
