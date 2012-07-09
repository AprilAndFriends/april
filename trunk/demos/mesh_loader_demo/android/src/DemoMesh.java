package com.example.april.android.demoMesh;

public class DemoMesh extends net.sourceforge.april.android.Activity
{
	static
	{
		System.loadLibrary("demo_mesh");
	}
	
	@Override
	protected void onCreate(android.os.Bundle savedInstanceState)
	{
		this.forceArchivePath(this.getPackageResourcePath()); // forces APK as archive file
		super.onCreate(savedInstanceState);
	}
	
}
