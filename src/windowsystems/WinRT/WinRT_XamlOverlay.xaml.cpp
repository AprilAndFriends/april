/// @file
/// @version 4.0
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
using namespace Windows::UI::Xaml::Markup;

// the textbox is manually created, because on some hardware the software keyboard won't appear if a static XAML textbox is used
#define XAML_TEXT_BOX "<TextBox \
	xmlns=\"http://schemas.microsoft.com/winfx/2006/xaml/presentation\" \
	xmlns:x=\"http://schemas.microsoft.com/winfx/2006/xaml\" \
	x:Name=\"keyboardTextBox\" \
	IsHitTestVisible=\"False\" \
	IsTextPredictionEnabled=\"False\" \
	Background=\"{x:Null}\" \
	BorderBrush=\"{x:Null}\" \
	Foreground=\"{x:Null}\" \
	Opacity=\"0\"/>"
#define XAML_BUTTON "<Button \
	xmlns=\"http://schemas.microsoft.com/winfx/2006/xaml/presentation\" \
	xmlns:x=\"http://schemas.microsoft.com/winfx/2006/xaml\" \
	x:Name=\"keyboardButton\" \
	Opacity=\"0\"/>"

namespace april
{
	static UIElement^ _loadFromXaml(chstr xaml)
	{
		hstr hXaml = xaml;
		if ((unsigned char)hXaml[0] == 0xEF && (unsigned char)hXaml[1] == 0xBB && (unsigned char)hXaml[2] == 0xBF) // remove BOM
		{
			hXaml = hXaml(3, -1);
		}
		return (UIElement^)XamlReader::Load(_HL_HSTR_TO_PSTR(hXaml));
	}

	WinRT_XamlOverlay::WinRT_XamlOverlay()
	{
		this->InitializeComponent();
		this->keyboardTextBox = (TextBox^)_loadFromXaml(XAML_TEXT_BOX);
		this->Children->Append(this->keyboardTextBox);
		this->Children->Append(_loadFromXaml(XAML_BUTTON));
	}

	void WinRT_XamlOverlay::showKeyboard()
	{
		this->hideKeyboard(); // first hide the already existing textbox
		this->keyboardTextBox->Focus(FocusState::Programmatic);
	}
	
	void WinRT_XamlOverlay::hideKeyboard()
	{
		this->keyboardTextBox->IsEnabled = false;
		this->keyboardTextBox->IsEnabled = true;
	}
	
}
#endif
