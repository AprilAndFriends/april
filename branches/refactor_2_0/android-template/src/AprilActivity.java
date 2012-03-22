package net.sourceforge.april;

// version 1.52

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import android.app.Activity;
import android.content.Context;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.os.Environment;
import android.util.DisplayMetrics;
import android.view.inputmethod.InputMethodManager;
import android.view.KeyEvent;
import android.view.MotionEvent;

class AprilJNI
{
	public static AprilActivity Activity = null;
	public static boolean Running = false;
	public static String ArchivePath = "";
	public static String SystemPath = ".";
	public static String SharedPath = ".";
	public static String PackageName = "";
	public static String VersionCode = "0";
	
	public static native void setVariables(Object activity, String systemPath, String sharedPath, String packageName, String versionCode, String forceArchivePath);
	public static native void init(String[] args, int width, int height);
	public static native boolean render();
	public static native void destroy();
	public static native void onMouseDown(float x, float y, int button);
	public static native void onMouseUp(float x, float y, int button);
	public static native void onMouseMove(float x, float y);
	public static native boolean onKeyDown(int keyCode, int charCode);
	public static native boolean onKeyUp(int keyCode);
	public static native void onLowMemory();
	public static native void onSurfaceCreated();
	
	public static native void activityOnCreate();
	public static native void activityOnStart();
	public static native void activityOnResume();
	public static native void activityOnPause();
	public static native void activityOnStop();
	public static native void activityOnDestroy();
	public static native void activityOnRestart();
	
}

public class AprilActivity extends Activity
{
	private AprilGLSurfaceView glView = null;
	
	public void forceArchivePath(String archivePath) // use this code in your Activity to force APK as archive file
	{
		AprilJNI.ArchivePath = archivePath;
	}
	
	@Override
	protected void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);
		AprilJNI.Activity = this;
		AprilJNI.SharedPath = Environment.getExternalStorageDirectory().getAbsolutePath();
		AprilJNI.SystemPath = this.getFilesDir().getAbsolutePath();
		AprilJNI.PackageName = this.getPackageName();
		try
		{
			AprilJNI.VersionCode = Integer.toString(this.getPackageManager().getPackageInfo(AprilJNI.PackageName, 0).versionCode);
		}
		catch (NameNotFoundException e)
		{
		}
		this.glView = new AprilGLSurfaceView(this);
		this.setContentView(this.glView);
		AprilJNI.activityOnCreate();
	}
	
	@Override
	protected void onStart()
	{
		super.onStart();
		AprilJNI.activityOnStart();
	}

	@Override
	protected void onResume()
	{
		super.onResume();
		AprilJNI.activityOnResume();
		this.glView.onResume();
	}
	
	@Override
	protected void onPause()
	{
		this.glView.onPause();
		AprilJNI.activityOnPause();
		super.onPause();
	}
	
	@Override
	protected void onStop()
	{
		AprilJNI.activityOnStop();
		super.onStop();
	}
	
	@Override
	public void onDestroy()
	{
		AprilJNI.activityOnDestroy();
		AprilJNI.destroy();
		super.onDestroy();
		System.runFinalizersOnExit(true);
		System.exit(0);
	}
	
	@Override
	protected void onRestart()
	{
		super.onRestart();
		AprilJNI.activityOnRestart();
	}
	
	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event)
	{
		return AprilJNI.onKeyDown(event.getKeyCode(), event.getUnicodeChar());
	}
	
	@Override
	public boolean onKeyUp(int keyCode, KeyEvent event)
	{
		return AprilJNI.onKeyUp(event.getKeyCode());
	}
	
	@Override
	public void onLowMemory()
	{
		AprilJNI.onLowMemory();
		super.onLowMemory();
	}
	
}

class AprilGLSurfaceView extends GLSurfaceView
{
	private AprilRenderer renderer;
	
	public AprilGLSurfaceView(Context context)
	{
		super(context);
		this.setEGLConfigChooser(false);
		this.renderer = new AprilRenderer();
		this.setRenderer(this.renderer);
	}
	
	public boolean onTouchEvent(final MotionEvent event)
	{
		this.queueEvent
		(
			new Runnable()
			{
				public void run()
				{
					final int action = event.getAction();
					switch (action & MotionEvent.ACTION_MASK)
					{
					case MotionEvent.ACTION_DOWN:
						AprilJNI.onMouseDown(event.getX(), event.getY(), 0);
						break;
					case MotionEvent.ACTION_UP:
						AprilJNI.onMouseUp(event.getX(), event.getY(), 0);
						break;
					case MotionEvent.ACTION_MOVE:
						AprilJNI.onMouseMove(event.getX(), event.getY());
						break;
					// this part should handle multi touch properly
					case MotionEvent.ACTION_POINTER_DOWN:
						AprilJNI.onMouseDown(event.getX(), event.getY(), 0);
						break;
					case MotionEvent.ACTION_POINTER_UP:
						AprilJNI.onMouseUp(event.getX(), event.getY(), 0);
						break;
					}
				}
			}
		);
		return true;
	}
	
}

class AprilRenderer implements GLSurfaceView.Renderer
{
	public void onSurfaceCreated(GL10 gl, EGLConfig config)
	{
		AprilJNI.onSurfaceCreated();
		if (!AprilJNI.Running)
		{
			AprilJNI.setVariables(AprilJNI.Activity, AprilJNI.SystemPath, AprilJNI.SharedPath, AprilJNI.PackageName, AprilJNI.VersionCode, AprilJNI.ArchivePath);
			DisplayMetrics metrics = new DisplayMetrics();
			AprilJNI.Activity.getWindowManager().getDefaultDisplay().getMetrics(metrics);
			String args[] = {AprilJNI.ArchivePath}; // adding argv[0]
			AprilJNI.init(args, metrics.widthPixels, metrics.heightPixels);
			AprilJNI.Running = true;
		}
	}
	
	public void onSurfaceChanged(GL10 gl, int w, int h)
	{
	}
	
	public void onDrawFrame(GL10 gl)
	{
		if (!AprilJNI.render())
		{
			AprilJNI.Activity.finish();
		}
	}
	
}
