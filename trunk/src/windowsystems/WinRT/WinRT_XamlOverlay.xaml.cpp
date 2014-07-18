/// @file
/// @version 3.5
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#ifdef _WINRT_WINDOW
#include "pch.h"

#include <hltypes/hlog.h>
#include <hltypes/hstring.h>

#include "april.h"
#include "WinRT_XamlOverlay.xaml.h"

using namespace Windows::UI::Xaml;

namespace april
{
	WinRT_XamlOverlay::WinRT_XamlOverlay()
	{
		this->InitializeComponent();
		this->hideKeyboard();
	}

	void WinRT_XamlOverlay::showKeyboard()
	{
		// this is required, because this method can be called from a touch-down event which will not display the keyboard
		this->hideKeyboard();
		this->Children->Append(this->keyboardTextbox);
		this->keyboardTextbox->Focus(FocusState::Programmatic);
	}
	
	void WinRT_XamlOverlay::hideKeyboard()
	{
		if (this->Children->Size > 0)
		{
			this->Children->RemoveAt(0);
		}
	}
	
}
#endif
