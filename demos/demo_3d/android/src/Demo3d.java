package com.example.april.demo3d;

public class Demo3d extends net.sourceforge.april.AprilActivity
{
	static
	{
		System.loadLibrary("demo_3d");
	}
	
	@Override
	protected void onCreate(android.os.Bundle savedInstanceState)
	{
		this.forceArchivePath(this.getPackageResourcePath()); // forces APK as archive file
		super.onCreate(savedInstanceState);
	}
	
}
