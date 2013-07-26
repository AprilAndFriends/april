/// @file
/// @author  Boris Mikic
/// @version 3.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef _WINRT_WINDOW

#include <hltypes/hlog.h>
#include <hltypes/hplatform.h>
#include <hltypes/hresource.h>
#include <hltypes/hstring.h>

#include "april.h"
#include "Platform.h"
#include "Window.h"
#include "WinRT_View.h"
#include "WinRT_Window.h"

using namespace Windows::ApplicationModel;
using namespace Windows::Foundation;
using namespace Windows::UI::ViewManagement;

namespace april
{
	void (*WinRT::Init)(const harray<hstr>&) = NULL;
	void (*WinRT::Destroy)() = NULL;
	harray<hstr> WinRT::Args;
	WinRT_XamlApp^ WinRT::App = nullptr;
	WinRT_XamlInterface^ WinRT::Interface = nullptr;
	
	/*
	void WinRT_View::Initialize(_In_ CoreApplicationView^ applicationView)
	{
		applicationView->Activated +=
			ref new TypedEventHandler<CoreApplicationView^, IActivatedEventArgs^>(this, &WinRT_View::OnActivated);
	}
	
	void WinRT_View::Uninitialize()
	{
	}
	
	void WinRT_View::SetWindow(_In_ CoreWindow^ window)
	{
		// assign events
	}
	
	void WinRT_View::Load(_In_ Platform::String^ entryPoint)
	{
		this->filled = false;
		this->snapped = false;
	}
	
	void WinRT_View::Run()
	{
		this->scrollHorizontal = false;
		this->filled = false;
		this->snapped = false;
		this->mouseMoveMessagesCount = 0;
		hresource::setCwd(normalize_path(hstr::from_unicode(Package::Current->InstalledLocation->Path->Data())));
		hresource::setArchive("");
		WinRT::View = this;
		(*WinRT::Init)(WinRT::Args);
		april::window->enterMainLoop();
		(*WinRT::Destroy)();
		WinRT::View = nullptr;
	}
	
	void WinRT_View::OnActivated(_In_ CoreApplicationView^ applicationView, _In_ IActivatedEventArgs^ args)
	{
		CoreWindow::GetForCurrentThread()->Activate();
		this->filled = false;
		this->snapped = false;
	}
	
	void WinRT_View::updateViewState()
	{
		bool newFilled = (ApplicationView::Value == ApplicationViewState::Filled);
		if (!this->filled && newFilled)
		{
			hlog::write(april::logTag, "Handling filled view override...");
		}
		this->filled = newFilled;
		bool newSnapped = (ApplicationView::Value == ApplicationViewState::Snapped);
		if (!this->snapped && newSnapped)
		{
			hlog::write(april::logTag, "Handling snapped view override...");
		}
		this->snapped = newSnapped;
	}
	
	void WinRT_View::checkEvents()
	{
		CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);
	}

	*/
	
}
#endif
