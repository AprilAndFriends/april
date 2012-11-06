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

#include "WinRT_View.h"

namespace april
{
	void WinRT_View::Initialize(_In_ CoreApplicationView^ applicationView)
	{
	}
	
	void WinRT_View::Uninitialize()
	{
	}
	
	void WinRT_View::SetWindow(_In_ CoreWindow^ window)
	{
	}
	
	void WinRT_View::Load(_In_ Platform::String^ entryPoint)
	{
	}
	
	void WinRT_View::Run()
	{
	}
	
	void WinRT_View::OnActivated(_In_ CoreApplicationView^ applicationView, _In_ IActivatedEventArgs^ args)
	{
	}
	
	void WinRT_View::OnWindowSizeChanged(_In_ CoreWindow^ sender, _In_ WindowSizeChangedEventArgs^ args)
	{
	}
	
}
#endif
#endif