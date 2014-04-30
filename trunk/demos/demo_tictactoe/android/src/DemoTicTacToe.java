package com.april.demoTicTacToe;

public class DemoTicTacToe extends com.april.Activity
{
	static
	{
		System.loadLibrary("demo_tictactoe");
	}
	
	@Override
	protected void onCreate(android.os.Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);
		this.forceArchivePath(com.april.NativeInterface.ApkPath); // forces APK as archive file
	}
	
}
