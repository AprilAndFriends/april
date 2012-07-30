package net.sourceforge.april.android;

// version 2.1

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import android.app.Dialog;
import android.content.pm.ActivityInfo;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager.NameNotFoundException;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.view.KeyEvent;
import android.view.View;

public class Activity extends android.app.Activity
{
	private GLSurfaceView glView = null;
	
	public void forceArchivePath(String archivePath) // use this code in your Activity to force APK as archive file
	{
		NativeInterface.ArchivePath = archivePath;
	}
	
	public View getView()
	{
		return this.glView;
	}
	
	@Override
	protected void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);
		NativeInterface.Activity = this;
		NativeInterface.PackageName = this.getPackageName();
		NativeInterface.DataPath = Environment.getExternalStorageDirectory().getAbsolutePath() + "/Android/obb/" + NativeInterface.PackageName;
		NativeInterface.SystemPath = this.getFilesDir().getAbsolutePath();
		try
		{
			PackageInfo info = this.getPackageManager().getPackageInfo(NativeInterface.PackageName, 0);
			NativeInterface.VersionCode = Integer.toString(info.versionCode);
			NativeInterface.ApkPath = info.applicationInfo.sourceDir;
		}
		catch (NameNotFoundException e)
		{
		}
		// cheap auto-rotation
		if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.GINGERBREAD)
		{
			int orientation = this.getRequestedOrientation();
			if (orientation == ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE)
			{
				this.setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_SENSOR_LANDSCAPE);
			}
			else if (orientation == ActivityInfo.SCREEN_ORIENTATION_PORTRAIT)
			{
				this.setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_SENSOR_PORTRAIT);
			}
		}
		// creating a GL surface view
		this.glView = new GLSurfaceView(this);
		this.setContentView(this.glView);
		// focusing this view allows proper input processing
		this.glView.requestFocus();
		this.glView.requestFocusFromTouch();
		NativeInterface.activityOnCreate();
	}
	
	@Override
	protected void onStart()
	{
		super.onStart();
		NativeInterface.activityOnStart();
	}

	@Override
	protected void onResume()
	{
		super.onResume();
		NativeInterface.activityOnResume();
		this.glView.onResume();
	}
	
	@Override
	protected void onPause()
	{
		this.glView.onPause();
		NativeInterface.activityOnPause();
		super.onPause();
	}
	
	@Override
	protected void onStop()
	{
		NativeInterface.activityOnStop();
		super.onStop();
	}
	
	@Override
	public void onDestroy()
	{
		NativeInterface.activityOnDestroy();
		NativeInterface.destroy();
		super.onDestroy();
		System.runFinalizersOnExit(true);
		System.exit(0);
	}
	
	@Override
	protected void onRestart()
	{
		super.onRestart();
		NativeInterface.activityOnRestart();
	}
	
	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event)
	{
		return NativeInterface.onKeyDown(event.getKeyCode(), event.getUnicodeChar());
	}
	
	@Override
	public boolean onKeyUp(int keyCode, KeyEvent event)
	{
		return NativeInterface.onKeyUp(event.getKeyCode());
	}
	
	@Override
	public void onLowMemory()
	{
		NativeInterface.onLowMemory();
		super.onLowMemory();
	}
	
	protected Dialog onCreateDialog(int id)
	{
		return NativeInterface.DialogBuilder.create();
	}
	
}
