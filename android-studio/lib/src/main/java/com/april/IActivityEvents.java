package com.april;

/// @version 5.2

import android.content.Intent;
import android.content.res.Configuration;
import android.os.Bundle;

public interface IActivityEvents
{
	public void registerOnCreate(ICallback1<Void, Bundle> callback);
	public void registerOnStart(ICallback<Void> ICallback);
	public void registerOnResume(ICallback<Void> ICallback);
	public void registerOnPause(ICallback<Void> ICallback);
	public void registerOnStop(ICallback<Void> ICallback);
	public void registerOnDestroy(ICallback<Void> ICallback);
	public void registerOnRestart(ICallback<Void> ICallback);
	public void registerOnActivityResult(ICallback3<Boolean, Integer, Integer, Intent> callback);
	public void registerOnNewIntent(ICallback1<Void, Intent> callback);
	public void registerOnBackPressed(ICallback<Boolean> ICallback);
	public void registerOnConfigurationChanged(ICallback1<Void, Configuration> callback);
	public void registerOnRequestPermissionsResult(ICallback3<Boolean, Integer, String[], Integer[]> callback);
	public void registerOnDrawFrame(ICallback<Void> callback);
	
}
