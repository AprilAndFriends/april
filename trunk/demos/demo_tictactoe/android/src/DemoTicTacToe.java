package com.example.april.demoTicTacToe;

public class DemoTicTacToe extends net.sourceforge.april.AprilActivity
{
	static
	{
		System.loadLibrary("demo_tictactoe");
	}
	
	@Override
	protected void onCreate(android.os.Bundle savedInstanceState)
	{
		this.forceArchivePath(this.getPackageResourcePath()); // forces APK as archive file
		super.onCreate(savedInstanceState);
	}
	
}
