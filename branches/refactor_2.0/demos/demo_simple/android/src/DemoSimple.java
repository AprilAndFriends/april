package com.example.april.demoSimple;

public class DemoSimple extends net.sourceforge.april.AprilActivity
{
	static
	{
		System.loadLibrary("demo_simple");
	}
	
	@Override
	protected void onCreate(android.os.Bundle savedInstanceState)
	{
		this.forceArchivePath(this.getPackageResourcePath()); // forces APK as archive file
		super.onCreate(savedInstanceState);
	}
	
}
