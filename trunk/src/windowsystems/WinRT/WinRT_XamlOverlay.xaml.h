/// @file
/// @version 3.4
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Defines a WinRT XAML Overlay for keyboard handling.

#ifdef _WINRT_WINDOW
#ifndef APRIL_WINRT_XAML_OVERLAY_H
#define APRIL_WINRT_XAML_OVERLAY_H

#include "WinRT_XamlOverlay.g.h" // auto-generated file

using namespace Windows::UI::Xaml::Controls;

namespace april
{
    [Windows::Foundation::Metadata::WebHostHidden]
    public ref class WinRT_XamlOverlay sealed
    {
    public:
        WinRT_XamlOverlay();

		void showKeyboard();
		void hideKeyboard();

    };

}
#endif
#endif
