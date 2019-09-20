package com.april;

/// @version 5.2

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLContext;
import javax.microedition.khronos.egl.EGLDisplay;
import javax.microedition.khronos.egl.EGLSurface;
import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.opengles.GL10;

import android.app.Activity;
import android.graphics.Rect;
import android.view.View;
import android.view.ViewTreeObserver;

public class Renderer implements android.opengl.GLSurfaceView.Renderer
{
	public void onSurfaceCreated(GL10 unused, EGLConfig config)
	{
		NativeInterface.onSurfaceCreated();
		if (!NativeInterface.running)
		{
			NativeInterface.setVariables(NativeInterface.dataPath, NativeInterface.archivePath);
			String args[] = {NativeInterface.apkPath}; // adding argv[0]
			NativeInterface.init(args);
			NativeInterface.running = true;
			// needed for keyboard height
			NativeInterface.activity.getWindow().getDecorView().getViewTreeObserver().addOnGlobalLayoutListener(new ViewTreeObserver.OnGlobalLayoutListener()
			{
				@Override
				public void onGlobalLayout()
				{
					View view = NativeInterface.aprilActivity.getView();
					Rect r = new Rect();
					view.getWindowVisibleDisplayFrame(r);
					float heightRatio = 1.0f - (float)(r.bottom - r.top) / view.getRootView().getHeight();
					// on some devices heightRatio is weird and gives 0.0666667 or something like that
					if (heightRatio < 0.1f)
					{
						heightRatio = 0.0f;
					}
					NativeInterface.onVirtualKeyboardChanged((heightRatio >= 0.15f), heightRatio);
				}
			});
		}
	}
	
	public void onSurfaceChanged(GL10 unused, int w, int h)
	{
	}
	
	public void onDrawFrame(GL10 unused)
	{
		for (int i = 0; i < NativeInterface.aprilActivity.callbacksOnDrawFrame.size(); ++i)
		{
			NativeInterface.aprilActivity.callbacksOnDrawFrame.get(i).execute();
		}
		if (!NativeInterface.update())
		{
			NativeInterface.activity.finish();
		}
	}
	
	public void swapBuffers()
	{
		EGL10 egl = (EGL10)EGLContext.getEGL();
		EGLDisplay display = egl.eglGetCurrentDisplay();
		EGLSurface surface = egl.eglGetCurrentSurface(EGL10.EGL_READ);
		egl.eglSwapBuffers(display, surface);
	}
	
}
