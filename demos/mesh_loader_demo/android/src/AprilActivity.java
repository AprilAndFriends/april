package net.sourceforge.april;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import android.app.Activity;
import android.content.Context;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.view.MotionEvent;

class AprilJNI
{
	public static String ApkPath;
	public static native void init(String[] args);
	public static native void render();
	public static native void destroy();
	public static native void onMouseDown(float x, float y, int button);
	public static native void onMouseUp(float x, float y, int button);
	public static native void onMouseMove(float x, float y);
	
}

public class AprilActivity extends Activity
{
	private GLSurfaceView glView;
	
	@Override
	protected void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);
		AprilJNI.ApkPath = this.getPackageResourcePath();
		this.glView = new AprilGLSurfaceView(this);
		this.setContentView(this.glView);
	}
	
	@Override
	protected void onDestroy()
	{
		super.onDestroy();
		AprilJNI.destroy();
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

}

class AprilRenderer implements GLSurfaceView.Renderer
{
	public void onSurfaceCreated(GL10 gl, EGLConfig config)
	{
		String args[] = {AprilJNI.ApkPath}; // adding argv[0]
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
