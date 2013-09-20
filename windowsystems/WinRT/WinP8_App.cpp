/// @file
/// @author  Boris Mikic
/// @version 3.1
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#if defined(_WINRT_WINDOW) && defined(_WINP8)
#include <hltypes/hlog.h>
#include <hltypes/hplatform.h>
#include <hltypes/hresource.h>
#include <hltypes/hstring.h>

#include "april.h"
#include "DirectX11_RenderSystem.h"
#include "Platform.h"
#include "SystemDelegate.h"
#include "Window.h"
#include "WinP8_App.h"
#include "WinRT.h"
#include "WinRT_BaseApp.h"
#include "WinRT_Window.h"

using namespace Windows::ApplicationModel;
using namespace Windows::Foundation;
using namespace Windows::Graphics::Display;
using namespace Windows::Phone::UI::Input;
using namespace Windows::UI::ViewManagement;

#define DX11_RENDERSYS ((DirectX11_RenderSystem*)april::rendersys)

namespace april
{
	void WinP8_App::Initialize(_In_ CoreApplicationView^ applicationView)
	{
		this->app = ref new WinRT_BaseApp();
		DisplayProperties::AutoRotationPreferences = (DisplayOrientations::Landscape | DisplayOrientations::LandscapeFlipped);
		applicationView->Activated +=
			ref new TypedEventHandler<CoreApplicationView^, IActivatedEventArgs^>(this, &WinP8_App::OnActivated);
	}
	
	void WinP8_App::Uninitialize()
	{
	}
	
	void WinP8_App::SetWindow(_In_ CoreWindow^ window)
	{
		this->window = window;
		this->keyboardInterface = ref new WinP8_KeyboardInterface(window, this->app);
		CoreApplication::Suspending +=
			ref new EventHandler<SuspendingEventArgs^>(this->app, &WinRT_BaseApp::OnSuspend);
		CoreApplication::Resuming +=
			ref new EventHandler<Object^>(this->app, &WinRT_BaseApp::OnResume);
		DisplayProperties::OrientationChanged +=
			ref new DisplayPropertiesEventHandler(
				this, &WinP8_App::OnOrientationChanged);
		DisplayProperties::LogicalDpiChanged +=
			ref new DisplayPropertiesEventHandler(
				this, &WinP8_App::OnLogicalDpiChanged);
		this->app->assignEvents(window);
	}
	
	void WinP8_App::Load(_In_ Platform::String^ entryPoint)
	{
	}
	
	void WinP8_App::Run()
	{
		hresource::setCwd(normalize_path(hstr::from_unicode(Package::Current->InstalledLocation->Path->Data())));
		hresource::setArchive("");
		WinRT::Interface = this;
		(*WinRT::Init)(WinRT::Args);
		april::window->enterMainLoop();
		(*WinRT::Destroy)();
		WinRT::Interface = nullptr;
	}
	
	void WinP8_App::OnActivated(_In_ CoreApplicationView^ applicationView, _In_ IActivatedEventArgs^ args)
	{
		CoreWindow::GetForCurrentThread()->Activate();
	}
	
	void WinP8_App::OnOrientationChanged(_In_ Object^ sender)
	{
		DX11_RENDERSYS->updateOrientation();
	}
	
	void WinP8_App::OnLogicalDpiChanged(_In_ Object^ sender)
	{
		DX11_RENDERSYS->updateOrientation();
	}

	void WinP8_App::unassignWindow()
	{
	}

	void WinP8_App::setCursorVisible(bool value)
	{
	}
	
	bool WinP8_App::canSuspendResume()
	{
		return true;
	}

	void WinP8_App::updateViewState()
	{
	}

	void WinP8_App::checkEvents()
	{
		this->window->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);
		this->keyboardInterface->processEvents();
	}
	
	void WinP8_App::showKeyboard()
	{
		this->keyboardInterface->showKeyboard();
	}
	
	void WinP8_App::hideKeyboard()
	{
		this->keyboardInterface->hideKeyboard();
	}
	
}
#endif
