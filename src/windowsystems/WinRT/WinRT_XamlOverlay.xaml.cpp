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
#include "WinRT_Window.h"
#include "WinRT_XamlOverlay.xaml.h"

using namespace Windows::Foundation;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Markup;

// the textbox is manually created, because on some hardware the software keyboard won't appear if a static XAML textbox is used
#define XAML_TEXT_BOX "<TextBox \
	xmlns=\"http://schemas.microsoft.com/winfx/2006/xaml/presentation\" \
	xmlns:x=\"http://schemas.microsoft.com/winfx/2006/xaml\" \
	x:Name=\"keyboardTextBox\" \
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
			hXaml = hXaml(3, hXaml.size() - 3);
		}
		return (UIElement^)XamlReader::Load(_HL_HSTR_TO_PSTR(hXaml));
	}

	WinRT_XamlOverlay::WinRT_XamlOverlay()
	{
		this->InitializeComponent();
		this->keyboardButton = (::Button^)_loadFromXaml(XAML_BUTTON);
		this->keyboardTextBox = (TextBox^)_loadFromXaml(XAML_TEXT_BOX);
		this->CompositionScaleChanged += ref new TypedEventHandler<SwapChainPanel^, Object^>(
			this, &WinRT_XamlOverlay::OnCompositionScaleChanged);
	}

	void WinRT_XamlOverlay::OnCompositionScaleChanged(_In_ SwapChainPanel^ sender, _In_ Object^ args)
	{
		april::getSystemInfo(); // so the displayResolution value gets updated
		if (april::window != NULL)
		{
			// so the size is updated
			float dpiRatio = WinRT::getDpiRatio();
			int correctedWidth = hround(april::window->getWidth() * dpiRatio);
			int correctedHeight = hround(april::window->getHeight() * dpiRatio);
			((WinRT_Window*)april::window)->changeSize(correctedWidth, correctedHeight);
		}
	}

	void WinRT_XamlOverlay::showKeyboard()
	{
		this->hideKeyboard(); // first hide the already existing textbox
		this->Children->Append(this->keyboardButton);
		this->Children->Append(this->keyboardTextBox);
		this->keyboardTextBox->Focus(FocusState::Programmatic);
	}
	
	void WinRT_XamlOverlay::hideKeyboard()
	{
		unsigned int index = -1;
		if (this->Children->IndexOf(this->keyboardTextBox, &index))
		{
			this->Children->RemoveAt(index);
		}
		if (this->Children->IndexOf(this->keyboardButton, &index))
		{
			this->Children->RemoveAt(index);
		}
	}
	
}
#endif
