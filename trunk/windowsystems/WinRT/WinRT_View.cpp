/// @file
/// @author  Boris Mikic
/// @version 2.5
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef _WIN32
#include <hltypes/hplatform.h>
#if _HL_WINRT

#include <windows.h>

#include <hltypes/hlog.h>
#include <hltypes/hresource.h>
#include <hltypes/hstring.h>

#include "Window.h"
#include "WinRT_View.h"
#include "WinRT_Window.h"

using namespace Windows::Foundation;
using namespace Windows::ApplicationModel;

namespace april
{
	void (*WinRT::Init)(const harray<hstr>&) = NULL;
	void (*WinRT::Destroy)() = NULL;
	harray<hstr> WinRT::Args;
	WinRT_View^ WinRT::View = nullptr;

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
		this->window = window;
        this->window->PointerCursor = ref new CoreCursor(CoreCursorType::Arrow, 0);
		this->window->SizeChanged +=
            ref new TypedEventHandler<CoreWindow^, WindowSizeChangedEventArgs^>(
                this, &WinRT_View::OnWindowSizeChanged);
	}
	
	void WinRT_View::Load(_In_ Platform::String^ entryPoint)
	{
	}

	void WinRT_View::Run()
	{
		hresource::setCwd(normalize_path(unicode_to_utf8(Package::Current->InstalledLocation->Path->Data())));
		hlog::write("APRIL", hresource::getCwd());
		hresource::setArchive("");
		WinRT::View = this;
		(*WinRT::Init)(WinRT::Args);
		april::window->enterMainLoop();
		(*WinRT::Destroy)();
	}

	void WinRT_View::OnActivated(_In_ CoreApplicationView^ applicationView, _In_ IActivatedEventArgs^ args)
	{
        CoreWindow::GetForCurrentThread()->Activate();
	}
	
	void WinRT_View::OnWindowSizeChanged(_In_ CoreWindow^ sender, _In_ WindowSizeChangedEventArgs^ args)
	{
        // TODO
	}

	void WinRT_View::checkEvents()
	{
		this->window->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);
	}
	
}
#endif
#endif