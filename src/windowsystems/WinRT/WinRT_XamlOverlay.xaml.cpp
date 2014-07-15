/// @file
/// @version 3.4
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#ifdef _WINRT_WINDOW
#include "pch.h"

#include "WinRT_XamlOverlay.xaml.h"

using namespace Windows::UI::Xaml;

namespace april
{
	WinRT_XamlOverlay::WinRT_XamlOverlay()
	{
		this->InitializeComponent();
	}

	void WinRT_XamlOverlay::showKeyboard()
	{
		this->keyboardTextbox->IsEnabled = true;
		this->keyboardTextbox->Focus(FocusState::Programmatic);
	}
	
	void WinRT_XamlOverlay::hideKeyboard()
	{
		this->keyboardDisable->Focus(FocusState::Programmatic);
	}
	
}
#endif
