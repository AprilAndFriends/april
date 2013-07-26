/// @file
/// @author  Boris Mikic
/// @version 3.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef _WINRT_WINDOW
#include "pch.h"

#include "WinRT_XamlInterface.xaml.h"

using namespace Windows::UI::Xaml;

namespace april
{
	WinRT_XamlInterface::WinRT_XamlInterface()
	{
		this->InitializeComponent();
	}

	void WinRT_XamlInterface::showKeyboard()
	{
		this->keyboardTextbox->IsEnabled = true;
		this->keyboardTextbox->Focus(FocusState::Programmatic);
	}
	
	void WinRT_XamlInterface::hideKeyboard()
	{
		this->keyboardDisable->Focus(FocusState::Programmatic);
	}
	
}
#endif
