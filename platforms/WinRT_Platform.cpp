/// @file
/// @author  Boris Mikic
/// @version 3.14
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#if defined(_WINRT) && !defined(_OPENKODE)
#include <gtypes/Vector2.h>
#include <hltypes/hlog.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hmap.h>
#include <hltypes/hplatform.h>
#include <hltypes/hstring.h>

#include "april.h"
#include "Platform.h"
#include "RenderSystem.h"
#include "Window.h"
#include "WinRT.h"
#include "WinRT_Window.h"

using namespace Windows::Foundation::Collections;
using namespace Windows::Graphics::Display;
using namespace Windows::Storage;
using namespace Windows::UI::Popups;
#ifndef WINP8
using namespace Windows::UI::ViewManagement;
#endif

namespace april
{
	SystemInfo getSystemInfo()
	{
		static SystemInfo info;
		if (info.locale == "")
		{
			// number of CPU cores
			SYSTEM_INFO w32info;
			GetNativeSystemInfo(&w32info);
			info.cpuCores = w32info.dwNumberOfProcessors;
			// TODO - though, WinRT does not seem to be able to retrieve this information
			// RAM size
#ifndef _WINP8
			info.ram = 1024;
#else
			info.ram = 512;
#endif
			// display resolution
#ifdef _WINRT_WINDOW
			int width = (int)CoreWindow::GetForCurrentThread()->Bounds.Width;
			int height = (int)CoreWindow::GetForCurrentThread()->Bounds.Height;
#ifndef _WINP8
			if (ApplicationView::Value == ApplicationViewState::Filled)
			{
				width += WINRT_SNAPPED_VIEW_UNUSED;
			}
#endif
			info.displayResolution.set((float)hmax(width, height), (float)hmin(width, height));
#endif
			// display DPI
			info.displayDpi = (int)DisplayProperties::LogicalDpi;
			// other
			info.locale = "";
#ifndef _WINP8
			IIterator<Platform::String^>^ it = Windows::Globalization::ApplicationLanguages::Languages->First();
			if (it->HasCurrent)
			{
				info.locale = _HL_PSTR_TO_HSTR(it->Current).lower();
			}
#else
			unsigned long count = 0;
			unsigned long length = LOCALE_NAME_MAX_LENGTH;
			wchar_t locale[LOCALE_NAME_MAX_LENGTH] = {0};
			if (GetUserPreferredUILanguages(MUI_LANGUAGE_NAME, &count, locale, &length) && count > 0 && length > 0)
			{
				info.locale = hstr::from_unicode(locale).lower();
			}
#endif
			if (info.locale == "")
			{
				info.locale = "en"; // default is "en"
			}
			else if (info.locale == "pt_pt" || info.locale == "pt-pt")
			{
				info.locale = "pt-PT";
			}
			else if (info.locale.utf8_size() > 2 && info.locale != "pt-PT")
			{
				info.locale = info.locale.utf8_substr(0, 2);
			}
		}
		if (info.maxTextureSize == 0 && april::rendersys != NULL)
		{
			info.maxTextureSize = april::rendersys->getMaxTextureSize();
		}
		return info;
	}

	DeviceType getDeviceType()
	{
#ifndef _WINP8
		return DEVICE_WINDOWS_8;
#else
		return DEVICE_WINDOWS_PHONE_8;
#endif
	}

	hstr getPackageName()
	{
		return _HL_PSTR_TO_HSTR(Windows::ApplicationModel::Package::Current->Id->FamilyName);
	}

	hstr getUserDataPath()
	{
		return systemize_path(_HL_PSTR_TO_HSTR(ApplicationData::Current->RoamingFolder->Path));
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
#ifndef _WINP8
		currentCallback = callback;
		_HL_HSTR_TO_PSTR_DEF(text);
		_HL_HSTR_TO_PSTR_DEF(title);
		MessageDialog^ dialog = ref new MessageDialog(ptext, ptitle);
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
			dialog->Commands->Append(ref new UICommand(pok, commandHandler, IDOK));
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
#else
		// TODOp8 - if Microsoft ever decides to allow usage of basic features on Windows Phone 8, this has to be implemented
		hlog::error(april::logTag, "Windows Phone 8 does not support messageBox()!)");
#endif
	}

}
#endif
