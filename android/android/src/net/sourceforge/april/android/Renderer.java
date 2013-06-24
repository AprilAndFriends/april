package net.sourceforge.april.android;

// version 3.0

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLContext;
import javax.microedition.khronos.egl.EGLDisplay;
import javax.microedition.khronos.egl.EGLSurface;
import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.opengles.GL10;

import android.app.Activity;

public class Renderer implements android.opengl.GLSurfaceView.Renderer
{
	public void onSurfaceCreated(GL10 gl, EGLConfig config)
	{
		NativeInterface.onSurfaceCreated();
		if (!NativeInterface.Running)
		{
			NativeInterface.setVariables(NativeInterface.DataPath, NativeInterface.ArchivePath);
			String args[] = {NativeInterface.ApkPath}; // adding argv[0]
			NativeInterface.init(args);
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
	
	public void swapBuffers()
	{
		EGL10 egl = (EGL10)EGLContext.getEGL();
		EGLDisplay display = egl.eglGetCurrentDisplay();
		EGLSurface surface = egl.eglGetCurrentSurface(EGL10.EGL_DRAW);
		egl.eglSwapBuffers(display, surface);
	}
	
}