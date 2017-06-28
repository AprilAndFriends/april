package com.april;

/// @version 4.4

import android.content.Intent;
import android.content.res.Configuration;
import android.os.Bundle;

public interface IActivityEvents
{
	public void registerOnCreate(Callback1<Void, Bundle> callback);
	public void registerOnStart(Callback<Void> callback);
	public void registerOnResume(Callback<Void> callback);
	public void registerOnPause(Callback<Void> callback);
	public void registerOnStop(Callback<Void> callback);
	public void registerOnDestroy(Callback<Void> callback);
	public void registerOnRestart(Callback<Void> callback);
	public void registerOnActivityResult(Callback3<Boolean, Integer, Integer, Intent> callback);
	public void registerOnNewIntent(Callback1<Void, Intent> callback);
	public void registerOnBackPressed(Callback<Boolean> callback);
	public void registerOnConfigurationChanged(Callback1<Void, Configuration> callback);
	
}
