package com.googlecode.april.android.demoSimple;

public class DemoSimple extends com.googlecode.april.android.Activity
{
	static
	{
		System.loadLibrary("demo_simple");
	}
	
	@Override
	protected void onCreate(android.os.Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);
		this.forceArchivePath(com.googlecode.april.android.NativeInterface.ApkPath); // forces APK as archive file
	}
	
}
