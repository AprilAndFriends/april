package net.sourceforge.april;

// version 2.0

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.pm.ActivityInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.opengl.GLSurfaceView;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.util.DisplayMetrics;
import android.view.inputmethod.BaseInputConnection;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputConnection;
import android.view.inputmethod.InputMethodManager;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.View;

class AprilJNI
{
	public static AprilActivity Activity = null;
	public static boolean Running = false;
	public static String ArchivePath = "";
	public static String SystemPath = ".";
	public static String DataPath = ".";
	public static String PackageName = "";
	public static String VersionCode = "0";
	public static AlertDialog.Builder DialogBuilder = null;
	
	public static native void setVariables(Object activity, String systemPath, String sharedPath, String packageName, String versionCode, String forceArchivePath);
	public static native void init(String[] args, int width, int height);
	public static native boolean render();
	public static native void destroy();
	public static native void onTouch(int type, float x, float y, int index);
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
	
	public static native void onDialogOk();
	public static native void onDialogYes();
	public static native void onDialogNo();
	public static native void onDialogCancel();
	
	public static void showMessageBox(String title, String text, String ok, String yes, String no, String cancel, int iconId)
	{
		AprilJNI.DialogBuilder = new AlertDialog.Builder(AprilJNI.Activity);
		AprilJNI.DialogBuilder.setTitle(title != null ? title : "");
		AprilJNI.DialogBuilder.setMessage(text != null ? text : "");
		if (ok != null)
		{
			AprilJNI.DialogBuilder.setPositiveButton(ok, new AprilDialogOkListener());
		}
		else
		{
			if (yes != null)
			{
				AprilJNI.DialogBuilder.setPositiveButton(yes, new AprilDialogYesListener());
			}
			if (no != null)
			{
				AprilJNI.DialogBuilder.setNegativeButton(no, new AprilDialogNoListener());
			}
		}
		if (cancel != null)
		{
			AprilJNI.DialogBuilder.setNeutralButton(cancel, new AprilDialogCancelListener());
			AprilJNI.DialogBuilder.setCancelable(true);
			AprilJNI.DialogBuilder.setOnCancelListener(new AprilDialogOnCancelListener());
		}
		else
		{
			AprilJNI.DialogBuilder.setCancelable(false);
		}
		switch (iconId)
		{
		case 1:
			AprilJNI.DialogBuilder.setIcon(android.R.drawable.ic_dialog_info);
			break;
		case 2:
			AprilJNI.DialogBuilder.setIcon(android.R.drawable.ic_dialog_alert);
			break;
		default:
			break;
		}
		AprilJNI.Activity.runOnUiThread(new Runnable()
		{
			public void run()
			{
				AprilJNI.Activity.showDialog(0);
			}
		});
	}
	
}

public class AprilActivity extends Activity
{
	private AprilGLSurfaceView glView = null;
	
	public void forceArchivePath(String archivePath) // use this code in your Activity to force APK as archive file
	{
		AprilJNI.ArchivePath = archivePath;
	}
	
	public View getView()
	{
		return this.glView;
	}
	
	@Override
	protected void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);
		AprilJNI.Activity = this;
		AprilJNI.PackageName = this.getPackageName();
		AprilJNI.DataPath = Environment.getExternalStorageDirectory().getAbsolutePath() + "/Android/obb/" + AprilJNI.PackageName;
		AprilJNI.SystemPath = this.getFilesDir().getAbsolutePath();
		try
		{
			AprilJNI.VersionCode = Integer.toString(this.getPackageManager().getPackageInfo(AprilJNI.PackageName, 0).versionCode);
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
		this.glView = new AprilGLSurfaceView(this);
		this.setContentView(this.glView);
		// focusing this view allows proper input processing
		this.glView.requestFocus();
		this.glView.requestFocusFromTouch();
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
	
	protected Dialog onCreateDialog(int id)
	{
		return AprilJNI.DialogBuilder.create();
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
		// view has to be properly focusable to be able to process input
		this.setFocusable(true);
		this.setFocusableInTouchMode(true);
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
					final int index = (action & MotionEvent.ACTION_POINTER_INDEX_MASK) >> MotionEvent.ACTION_POINTER_INDEX_SHIFT;
					final int pointerCount = event.getPointerCount();
					switch (action & MotionEvent.ACTION_MASK)
					{
					case MotionEvent.ACTION_DOWN:
					case MotionEvent.ACTION_POINTER_DOWN: // handles multi-touch
						AprilJNI.onTouch(0, event.getX(index), event.getY(index), index);
						break;
					case MotionEvent.ACTION_UP:
					case MotionEvent.ACTION_POINTER_UP: // handles multi-touch
						AprilJNI.onTouch(1, event.getX(index), event.getY(index), index);
						break;
					case MotionEvent.ACTION_MOVE: // Android batches multitouch move events into a single move event
						for (int i = 0; i < pointerCount; i++)
						{
							AprilJNI.onTouch(2, event.getX(i), event.getY(i), i);
						}
						break;
					}
				}
			}
		);
		return true;
	}
	
	@Override 
	public InputConnection onCreateInputConnection(EditorInfo outAttributes)  // required for creation of soft keyboard
	{ 
		outAttributes.actionId = EditorInfo.IME_ACTION_DONE; 
		outAttributes.imeOptions = EditorInfo.IME_FLAG_NO_EXTRACT_UI; 
		return new AprilInputConnection(this, false);
	}
	
	@Override
	public boolean onCheckIsTextEditor() // required for creation of soft keyboard
	{
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
			AprilJNI.setVariables(AprilJNI.Activity, AprilJNI.SystemPath, AprilJNI.DataPath, AprilJNI.PackageName, AprilJNI.VersionCode, AprilJNI.ArchivePath);
			DisplayMetrics metrics = new DisplayMetrics();
			AprilJNI.Activity.getWindowManager().getDefaultDisplay().getMetrics(metrics);
			String args[] = {AprilJNI.ArchivePath}; // adding argv[0]
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
			AprilJNI.init(args, width, height);
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

class AprilInputConnection extends BaseInputConnection
{
	private final View view;
	private final KeyEvent delKeyDownEvent = new KeyEvent(KeyEvent.ACTION_DOWN, KeyEvent.KEYCODE_DEL);
	private final KeyEvent delKeyUpEvent = new KeyEvent(KeyEvent.ACTION_UP, KeyEvent.KEYCODE_DEL);
	
	public AprilInputConnection(View view, boolean fullEditor)
	{
		super(view, fullEditor);
		this.view = view;
	}
	
	@Override
	public boolean deleteSurroundingText(int leftLength, int rightLength)
	{
		// Ericsson Xperia devices don't send key events but call this method for some reason when handling the delete key
		this.view.onKeyDown(KeyEvent.KEYCODE_DEL, this.delKeyDownEvent);
		this.view.onKeyUp(KeyEvent.KEYCODE_DEL, this.delKeyUpEvent);
		return super.deleteSurroundingText(leftLength, rightLength);
	}
	
}

class AprilDialogOkListener implements DialogInterface.OnClickListener
{
	public void onClick(DialogInterface dialog, int id)
	{
		AprilJNI.onDialogOk();
	}
}

class AprilDialogYesListener implements DialogInterface.OnClickListener
{
	public void onClick(DialogInterface dialog, int id)
	{
		AprilJNI.onDialogYes();
	}
}

class AprilDialogNoListener implements DialogInterface.OnClickListener
{
	public void onClick(DialogInterface dialog, int id)
	{
		AprilJNI.onDialogNo();
	}
}

class AprilDialogCancelListener implements DialogInterface.OnClickListener
{
	public void onClick(DialogInterface dialog, int id)
	{
		AprilJNI.onDialogCancel();
	}
}

class AprilDialogOnCancelListener implements DialogInterface.OnCancelListener
{
	public void onCancel(DialogInterface dialog)
	{
		AprilJNI.onDialogCancel();
	}
}

