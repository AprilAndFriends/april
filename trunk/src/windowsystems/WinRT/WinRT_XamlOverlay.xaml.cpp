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
using namespace Windows::UI::Xaml::Markup;

// the textbox is manually created, because on some hardware the software keyboard won't appear if a static XAML textbox is used
#define XAML_TEXT_BOX "<TextBox \
	xmlns=\"http://schemas.microsoft.com/winfx/2006/xaml/presentation\" \
	xmlns:x=\"http://schemas.microsoft.com/winfx/2006/xaml\" \
	x:Name=\"keyboardTextbox\" \
	IsHitTestVisible=\"False\" \
	IsTextPredictionEnabled=\"False\" \
	Background=\"{x:Null}\" \
	BorderBrush=\"{x:Null}\" \
	Foreground=\"{x:Null}\" \
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
		this->keyboardTextbox = nullptr;
	}

	void WinRT_XamlOverlay::showKeyboard()
	{
		this->hideKeyboard(); // first destroy the already existing textbox
		this->keyboardTextbox = (TextBox^)_loadFromXaml(XAML_TEXT_BOX);
		if (this->keyboardTextbox != nullptr)
		{
			this->Children->Append(this->keyboardTextbox);
			this->keyboardTextbox->Focus(FocusState::Programmatic);
		}
	}
	
	void WinRT_XamlOverlay::hideKeyboard()
	{
		if (this->Children->Size > 0)
		{
			unsigned int index = -1;
			if (this->Children->IndexOf(this->keyboardTextbox, &index))
			{
				this->Children->RemoveAt(index);
			}
			this->keyboardTextbox->IsEnabled = false;
			this->keyboardTextbox = nullptr;
		}
	}
	
}
#endif
