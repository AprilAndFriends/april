package com.example.april.demoMesh;

public class DemoMesh extends net.sourceforge.april.AprilActivity
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
