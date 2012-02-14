package com.example.april.demoSimple;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import android.app.Activity;
import android.content.Context;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.view.MotionEvent;
import android.widget.TextView;
import com.example.april.demoSimple.R;

public class AprilActivity extends Activity
{
	static
	{
		//System.loadLibrary("gnustl_shared");
		System.loadLibrary("demo_simple");
    }
	
    @Override
    protected void onCreate(Bundle savedInstanceState)
	{
        super.onCreate(savedInstanceState);
		setContentView(R.layout.main);
		TextView myTextField = (TextView)findViewById(R.id.myTextField);
		myTextField.setText(":V");
		//System.loadLibrary("gnustl_shared");
		/*
		try
		{
			System.loadLibrary("demo_simple");
		}
		catch (java.lang.Error e)
		{
			//TextView myTextField = (TextView)findViewById(R.id.myTextField);
			myTextField.setText(e.getMessage());
		}
		//*/
		try
		{
			myTextField.setText(stringFromJNICPP());
		}
		catch (java.lang.Error e)
		{
			//TextView myTextField = (TextView)findViewById(R.id.myTextField);
			myTextField.setText(e.getMessage());
		}
		/*
        mGLView = new DemoGLSurfaceView(this);
        setContentView(mGLView);
		*/
    }
	
	public native String stringFromJNICPP();
	
	/*
	@Override
    protected void onPause()
	{
        super.onPause();
        mGLView.onPause();
    }

    @Override
    protected void onResume()
	{
        super.onResume();
        mGLView.onResume();
    }

    private GLSurfaceView mGLView;
	*/
}
/*
class DemoGLSurfaceView extends GLSurfaceView {
    public DemoGLSurfaceView(Context context) {
        super(context);
        mRenderer = new DemoRenderer();
        setRenderer(mRenderer);
    }

    public boolean onTouchEvent(final MotionEvent event) {
        if (event.getAction() == MotionEvent.ACTION_DOWN) {
            nativePause();
        }
        return true;
    }

    DemoRenderer mRenderer;

    private static native void nativePause();
}

class DemoRenderer implements GLSurfaceView.Renderer {
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        nativeInit();
    }

    public void onSurfaceChanged(GL10 gl, int w, int h) {
        //gl.glViewport(0, 0, w, h);
        nativeResize(w, h);
    }

    public void onDrawFrame(GL10 gl) {
        nativeRender();
    }

    private static native void nativeInit();
    private static native void nativeResize(int w, int h);
    private static native void nativeRender();
    private static native void nativeDone();
}
*/
