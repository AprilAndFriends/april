package net.sourceforge.april.android;

// version 2.1

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import android.app.Activity;
import android.os.Build;
import android.util.DisplayMetrics;

public class Renderer implements android.opengl.GLSurfaceView.Renderer
{
	public void onSurfaceCreated(GL10 gl, EGLConfig config)
	{
		NativeInterface.onSurfaceCreated();
		if (!NativeInterface.Running)
		{
			NativeInterface.setVariables(NativeInterface.Activity, NativeInterface.SystemPath, NativeInterface.DataPath, NativeInterface.PackageName, NativeInterface.VersionCode, NativeInterface.ArchivePath);
			DisplayMetrics metrics = new DisplayMetrics();
			NativeInterface.Activity.getWindowManager().getDefaultDisplay().getMetrics(metrics);
			String args[] = {NativeInterface.ArchivePath}; // adding argv[0]
			int width = metrics.widthPixels;
			int height = metrics.heightPixels;
			if (height > width)
			{
				height = metrics.widthPixels;
				width = metrics.heightPixels;
			}
			// fixes problem with bottom 20 pixels being covered by Kindle Fire's menu
			if (Build.MANUFACTURER == "Amazon" && Build.MODEL == "Kindle Fire")
			{
				height -= 20;
			}
			NativeInterface.init(args, width, height);
			NativeInterface.Running = true;
		}
	}
	
	public void onSurfaceChanged(GL10 gl, int w, int h)
	{
	}
	
	public void onDrawFrame(GL10 gl)
	{
		if (!NativeInterface.render())
		{
			NativeInterface.Activity.finish();
		}
	}
	
}
