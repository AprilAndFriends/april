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
/// Defines a generic WinRT Interface.

#ifdef _WINRT_WINDOW
#ifndef APRIL_IWINRT_H
#define APRIL_IWINRT_H

#include <hltypes/hstring.h>

namespace april
{
	public interface class IWinRT
    {
    public:
		virtual void unassignWindow();
		virtual void refreshCursor();
		virtual bool canSuspendResume();
		virtual void updateViewState();
		virtual void checkEvents();
		virtual void showKeyboard();
		virtual void hideKeyboard();

    };
}
#endif
#endif
