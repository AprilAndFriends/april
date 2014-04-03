package com.example.april.android.demoTicTacToe;

public class DemoTicTacToe extends com.googlecode.april.android.Activity
{
	static
	{
		System.loadLibrary("demo_tictactoe");
	}
	
	@Override
	protected void onCreate(android.os.Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);
		this.forceArchivePath(com.googlecode.april.android.NativeInterface.ApkPath); // forces APK as archive file
	}
	
}
