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

#include <gtypes/Vector2.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hmap.h>
#include <hltypes/hplatform.h>
#include <hltypes/hstring.h>

#include "Platform.h"
#include "RenderSystem.h"
#include "Window.h"
#include "WinRT_View.h"

using namespace Windows::Graphics::Display;
using namespace Windows::UI::Popups;

namespace april
{
	SystemInfo getSystemInfo()
	{
		// TODO
		static SystemInfo info;
		if (info.locale == "")
		{
			// number of CPU cores
			SYSTEM_INFO w32info;
			GetNativeSystemInfo(&w32info);
			info.cpuCores = w32info.dwNumberOfProcessors;
			// TODO
			// RAM size
			info.ram = 1024;
			// display resolution
			int width = (int)april::WinRT::View->getCoreWindow()->Bounds.Width;
			int height = (int)april::WinRT::View->getCoreWindow()->Bounds.Height;
			info.displayResolution.set((float)width, (float)height);
			// display DPI
			info.displayDpi = (int)DisplayProperties::LogicalDpi;
			// other
			info.locale = "en"; // TODO
		}
		// TODO
		if (info.maxTextureSize == 0 && april::rendersys != NULL)
		{
			info.maxTextureSize = april::rendersys->_getMaxTextureSize();
		}
		return info;
	}

	DeviceType getDeviceType()
	{
		return DEVICE_WINDOWS_8;
	}

	static void(*currentCallback)(MessageBoxButton) = NULL;

	void _messageBoxResult(int button)
	{
		switch (button)
		{
		case IDOK:
			if (currentCallback != NULL)
			{
				(*currentCallback)(AMSGBTN_OK);
			}
			break;
		case IDYES:
			if (currentCallback != NULL)
			{
				(*currentCallback)(AMSGBTN_YES);
			}
			break;
		case IDNO:
			if (currentCallback != NULL)
			{
				(*currentCallback)(AMSGBTN_NO);
			}
			break;
		case IDCANCEL:
			if (currentCallback != NULL)
			{
				(*currentCallback)(AMSGBTN_CANCEL);
			}
			break;
		}
	}

	void messageBox_platform(chstr title, chstr text, MessageBoxButton buttonMask, MessageBoxStyle style,
		hmap<MessageBoxButton, hstr> customButtonTitles, void(*callback)(MessageBoxButton))
	{
		currentCallback = callback;
		_HL_HSTR_TO_PSTR_DEF(text);
		_HL_HSTR_TO_PSTR_DEF(title);
		MessageDialog^ dialog = ref new MessageDialog(ptext, ptitle);
		MessageBoxButton button = AMSGBTN_OK;
		UICommandInvokedHandler^ commandHandler = ref new UICommandInvokedHandler(
			[](IUICommand^ command)
		{
			_messageBoxResult((int)command->Id);
		});
		hstr ok;
		hstr yes;
		hstr no;
		hstr cancel;
		_makeButtonLabels(&ok, &yes, &no, &cancel, buttonMask, customButtonTitles);
		_HL_HSTR_TO_PSTR_DEF(ok);
		_HL_HSTR_TO_PSTR_DEF(yes);
		_HL_HSTR_TO_PSTR_DEF(no);
		_HL_HSTR_TO_PSTR_DEF(cancel);

		if ((buttonMask & AMSGBTN_OK) && (buttonMask & AMSGBTN_CANCEL))
		{
			dialog->Commands->Append(ref new UICommand(pok, commandHandler, IDOK));
			dialog->Commands->Append(ref new UICommand(pcancel, commandHandler, IDCANCEL));
			dialog->DefaultCommandIndex = 0;
			dialog->CancelCommandIndex = 1;
		}
		else if ((buttonMask & AMSGBTN_YES) && (buttonMask & AMSGBTN_NO) && (buttonMask & AMSGBTN_CANCEL))
		{
			dialog->Commands->Append(ref new UICommand(pyes, commandHandler, IDYES));
			dialog->Commands->Append(ref new UICommand(pno, commandHandler, IDNO));
			dialog->Commands->Append(ref new UICommand(pcancel, commandHandler, IDCANCEL));
			dialog->DefaultCommandIndex = 0;
			dialog->CancelCommandIndex = 2;
		}
		else if (buttonMask & AMSGBTN_OK)
		{
			dialog->Commands->Append(ref new UICommand(pcancel, commandHandler, IDOK));
			dialog->DefaultCommandIndex = 0;
			dialog->CancelCommandIndex = 0;
		}
		else if ((buttonMask & AMSGBTN_YES) && (buttonMask & AMSGBTN_NO))
		{
			dialog->Commands->Append(ref new UICommand(pyes, commandHandler, IDYES));
			dialog->Commands->Append(ref new UICommand(pno, commandHandler, IDNO));
			dialog->DefaultCommandIndex = 0;
			dialog->CancelCommandIndex = 1;
		}
		dialog->ShowAsync();
	}

}
#endif
#endif
