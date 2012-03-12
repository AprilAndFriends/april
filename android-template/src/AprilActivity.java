package net.sourceforge.april;

// version 1.51

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import android.app.Activity;
import android.content.Context;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.util.DisplayMetrics;
import android.view.inputmethod.InputMethodManager;
import android.view.KeyEvent;
import android.view.MotionEvent;

class AprilJNI
{
	public static String ApkPath = "";
	public static String SystemPath = ".";
	public static AprilActivity Activity = null;
	public static boolean Running = false;
	public static native void init(Object activity, String[] args, String path, int width, int height);
	public static native boolean render();
	public static native void destroy();
	public static native boolean onQuit();
	public static native void onMouseDown(float x, float y, int button);
	public static native void onMouseUp(float x, float y, int button);
	public static native void onMouseMove(float x, float y);
	public static native boolean onKeyDown(int keyCode, int charCode);
	public static native boolean onKeyUp(int keyCode);
	public static native void onFocusChange(boolean focused);
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
	private boolean viewAlreadySet = false;
	
	@Override
	protected void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);
		AprilJNI.ApkPath = this.getPackageResourcePath();
		AprilJNI.SystemPath = this.getFilesDir().getAbsolutePath();
		AprilJNI.Activity = this;
		this.setDefaultKeyMode(DEFAULT_KEYS_DISABLE);
		this.glView = new AprilGLSurfaceView(this);
		AprilJNI.activityOnCreate();
	}
	
	@Override
	protected void onStart()
	{
		super.onStart();
		// this allows for a proper splash screen
		if (!this.viewAlreadySet)
		{
			this.setContentView(this.glView);
			this.viewAlreadySet = true;
		}
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
	public boolean dispatchKeyEvent(KeyEvent event)
	{
		if (event.getKeyCode() == KeyEvent.KEYCODE_BACK && AprilJNI.onQuit())
		{
			this.finish();
			return false;    
		}
        return super.dispatchKeyEvent(event);
	}
	
	@Override
	public void onWindowFocusChanged(boolean hasFocus)
	{
		super.onWindowFocusChanged(hasFocus);
		if (!this.isFinishing())
		{
			AprilJNI.onFocusChange(hasFocus);
		}
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
		queueEvent
		(
			new Runnable()
			{
				public void run()
				{
					if (event.getAction() == MotionEvent.ACTION_DOWN)
					{
						AprilJNI.onMouseDown(event.getX(), event.getY(), 0);
					}
					else if (event.getAction() == MotionEvent.ACTION_UP)
					{
						AprilJNI.onMouseUp(event.getX(), event.getY(), 0);
					}
					else if (event.getAction() == MotionEvent.ACTION_MOVE)
					{
						AprilJNI.onMouseMove(event.getX(), event.getY());
					}
				}
			}
		);
		return true;
	}	
	/*
    public boolean onTouchEvent(final MotionEvent event)
	{
        if (event.getAction() == MotionEvent.ACTION_DOWN)
		{
			AprilJNI.onMouseDown(event.getX(), event.getY(), 0);
        }
        else if (event.getAction() == MotionEvent.ACTION_UP)
		{
			AprilJNI.onMouseUp(event.getX(), event.getY(), 0);
        }
        else if (event.getAction() == MotionEvent.ACTION_MOVE)
		{
			AprilJNI.onMouseMove(event.getX(), event.getY());
        }
        return true;
    }
	*/
	
}

class AprilRenderer implements GLSurfaceView.Renderer
{
	public void onSurfaceCreated(GL10 gl, EGLConfig config)
	{
		AprilJNI.onSurfaceCreated();
		if (!AprilJNI.Running)
		{
			DisplayMetrics metrics = new DisplayMetrics();
			AprilJNI.Activity.getWindowManager().getDefaultDisplay().getMetrics(metrics);
			String args[] = {AprilJNI.ApkPath}; // adding argv[0]
			AprilJNI.init(AprilJNI.Activity, args, AprilJNI.SystemPath, metrics.widthPixels, metrics.heightPixels);
			AprilJNI.Running = true;
		}
	}
	
	public void onSurfaceChanged(GL10 gl, int w, int h)
	{
		//gl.glViewport(0, 0, w, h);
		//nativeResize(w, h);
	}
	
	public void onDrawFrame(GL10 gl)
	{
		if (!AprilJNI.render())
		{
			AprilJNI.Activity.finish();
		}
	}
	
}
