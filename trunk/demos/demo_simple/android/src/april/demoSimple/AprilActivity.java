package com.example.april.demoSimple;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import android.app.Activity;
import android.content.Context;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.view.MotionEvent;
import com.example.april.demoSimple.R;

public class AprilActivity extends Activity
{
	static
	{
		System.loadLibrary("demo_simple");
	}
	
	private GLSurfaceView glView;
	
	@Override
	protected void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);
		this.glView = new AprilGLSurfaceView(this);
		this.setContentView(this.glView);
	}
	
	@Override
	protected void onDestroy()
	{
		super.onDestroy();
		//AprilJNI.destroy();
	}
	
	@Override
	protected void onPause()
	{
		super.onPause();
		this.glView.onPause();
	}

	@Override
	protected void onResume()
	{
		super.onResume();
		this.glView.onResume();
	}

}

class AprilJNI
{
	public static native void init(String[] args);
	public static native void render();
	public static native void destroy();
	
}

class AprilGLSurfaceView extends GLSurfaceView
{
	private AprilRenderer renderer;
	
	public AprilGLSurfaceView(Context context)
	{
		super(context);
		this.renderer = new AprilRenderer();
		this.setRenderer(this.renderer);
	}

	public boolean onTouchEvent(final MotionEvent event)
	{
		if (event.getAction() == MotionEvent.ACTION_DOWN)
		{
			//this.nativePause();
		}
		return true;
	}

}

class AprilRenderer implements GLSurfaceView.Renderer
{
	public void onSurfaceCreated(GL10 gl, EGLConfig config)
	{
		String args[] = {System.getProperty("user.dir")}; // adding cwd
		AprilJNI.init(args);
	}

	public void onSurfaceChanged(GL10 gl, int w, int h)
	{
		//gl.glViewport(0, 0, w, h);
		//nativeResize(w, h);
	}
	
	public void onDrawFrame(GL10 gl)
	{
		AprilJNI.render();
	}
	
}
