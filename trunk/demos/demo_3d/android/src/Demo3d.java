package com.example.april.android.demo3d;

public class Demo3d extends com.googlecode.april.android.Activity
{
	static
	{
		System.loadLibrary("demo_3d");
	}
	
	@Override
	protected void onCreate(android.os.Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);
		this.forceArchivePath(com.googlecode.april.android.NativeInterface.ApkPath); // forces APK as archive file
	}
	
}
