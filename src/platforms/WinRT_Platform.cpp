/// @file
/// @version 3.5
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#if defined(_WINRT) && !defined(_OPENKODE)
#include <gtypes/Vector2.h>
#include <hltypes/hdir.h>
#include <hltypes/hlog.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hmap.h>
#include <hltypes/hmutex.h>
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
using namespace Windows::UI::ViewManagement;

namespace april
{
	extern SystemInfo info;
	
	SystemInfo getSystemInfo()
	{
		if (info.locale == "")
		{
			info.name = "winrt";
#ifdef _ARM
			info.architecture = "ARM";
#else
			info.architecture = "x86";
#endif
			// number of CPU cores
			SYSTEM_INFO w32info;
			GetNativeSystemInfo(&w32info);
			info.cpuCores = w32info.dwNumberOfProcessors;
			// RAM size
#ifndef _WINP8
			// pure WinRT does not seem to be able to retrieve this information
			info.ram = 1024;
#else
			// WinP8 reports 75% of the RAM's size as available
			info.ram = (int)(Windows::System::MemoryManager::AppMemoryUsageLimit / (1024 * 1024) * 4 / 3); // in MB
#endif
			// other
			info.locale = "en"; // default is "en"
			IIterator<Platform::String^>^ it = Windows::Globalization::ApplicationLanguages::Languages->First();
			if (it->HasCurrent)
			{
				hstr fullLocale = _HL_PSTR_TO_HSTR(it->Current);
				if (fullLocale.contains("-"))
				{
					fullLocale.split("-", info.locale, info.localeVariant);
				}
				else if (fullLocale.contains("_"))
				{
					fullLocale.split("_", info.locale, info.localeVariant);
				}
				else
				{
					info.locale = fullLocale;
				}
				info.locale = info.locale.lowered();
				info.localeVariant = info.localeVariant.uppered();
			}
			info.osVersion = 8.1f;
		}
		DisplayInformation^ displayInfo = DisplayInformation::GetForCurrentView();
		// display DPI
		info.displayDpi = displayInfo->RawDpiY;
#ifdef _WINRT_WINDOW
		// display resolution
		float dpiRatio = WinRT::getDpiRatio();
		CoreWindow^ window = CoreWindow::GetForCurrentThread();
		int width = hround(window->Bounds.Width * dpiRatio);
		int height = hround(window->Bounds.Height * dpiRatio);
		// these orientations are not supported in APRIL, but Windows allows them anyway even if the manifest says that they aren't supported
		if (displayInfo->CurrentOrientation == DisplayOrientations::Portrait ||
			displayInfo->CurrentOrientation == DisplayOrientations::PortraitFlipped)
		{
			hswap(width, height);
		}
		if (info.displayResolution.y == 0.0f)
		{
			info.displayResolution.set((float)width, (float)height);
		}
		else
		{
			info.displayResolution.x = hmax((float)width, info.displayResolution.x);
		}
#endif
		return info;
	}

	hstr getPackageName()
	{
		return _HL_PSTR_TO_HSTR(Windows::ApplicationModel::Package::Current->Id->FamilyName);
	}

	hstr getUserDataPath()
	{
		return hdir::systemize(_HL_PSTR_TO_HSTR(ApplicationData::Current->RoamingFolder->Path));
	}
	
	static void(*currentCallback)(MessageBoxButton) = NULL;

	void _messageBoxResult(int button)
	{
		switch (button)
		{
		case IDOK:
			if (currentCallback != NULL)
			{
				(*currentCallback)(MESSAGE_BUTTON_OK);
			}
			break;
		case IDYES:
			if (currentCallback != NULL)
			{
				(*currentCallback)(MESSAGE_BUTTON_YES);
			}
			break;
		case IDNO:
			if (currentCallback != NULL)
			{
				(*currentCallback)(MESSAGE_BUTTON_NO);
			}
			break;
		case IDCANCEL:
			if (currentCallback != NULL)
			{
				(*currentCallback)(MESSAGE_BUTTON_CANCEL);
			}
			break;
		default:
			hlog::error(april::logTag, "Unknown message box callback: " + hstr(button));
			break;
		}
	}

	static harray<DispatchedHandler^> messageBoxQueue;
	static hmutex messageBoxQueueMutex;

	void messageBox_platform(chstr title, chstr text, MessageBoxButton buttonMask, MessageBoxStyle style,
		hmap<MessageBoxButton, hstr> customButtonTitles, void(*callback)(MessageBoxButton))
	{
		DispatchedHandler^ handler = ref new DispatchedHandler(
			[title, text, buttonMask, style, customButtonTitles, callback]()
		{
			currentCallback = callback;
			_HL_HSTR_TO_PSTR_DEF(text);
			_HL_HSTR_TO_PSTR_DEF(title);
			MessageDialog^ dialog = ref new MessageDialog(ptext, ptitle);
			UICommandInvokedHandler^ commandHandler = ref new UICommandInvokedHandler(
				[](IUICommand^ command)
			{
				_messageBoxResult((int)command->Id);
				DispatchedHandler^ handler = nullptr;
				messageBoxQueueMutex.lock();
				if (messageBoxQueue.size() > 0)
				{
					handler = messageBoxQueue.removeFirst();
				}
				messageBoxQueueMutex.unlock();
				if (handler != nullptr)
				{
					CoreWindow::GetForCurrentThread()->Dispatcher->RunAsync(CoreDispatcherPriority::Normal, handler);
				}
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

			if ((buttonMask & MESSAGE_BUTTON_OK) && (buttonMask & MESSAGE_BUTTON_CANCEL))
			{
				dialog->Commands->Append(ref new UICommand(pok, commandHandler, IDOK));
				dialog->Commands->Append(ref new UICommand(pcancel, commandHandler, IDCANCEL));
				dialog->DefaultCommandIndex = 0;
				dialog->CancelCommandIndex = 1;
			}
			else if ((buttonMask & MESSAGE_BUTTON_YES) && (buttonMask & MESSAGE_BUTTON_NO) && (buttonMask & MESSAGE_BUTTON_CANCEL))
			{
				dialog->Commands->Append(ref new UICommand(pyes, commandHandler, IDYES));
				dialog->Commands->Append(ref new UICommand(pno, commandHandler, IDNO));
				dialog->Commands->Append(ref new UICommand(pcancel, commandHandler, IDCANCEL));
				dialog->DefaultCommandIndex = 0;
				dialog->CancelCommandIndex = 2;
			}
			else if (buttonMask & MESSAGE_BUTTON_OK)
			{
				dialog->Commands->Append(ref new UICommand(pok, commandHandler, IDOK));
				dialog->DefaultCommandIndex = 0;
				dialog->CancelCommandIndex = 0;
			}
			else if ((buttonMask & MESSAGE_BUTTON_YES) && (buttonMask & MESSAGE_BUTTON_NO))
			{
				dialog->Commands->Append(ref new UICommand(pyes, commandHandler, IDYES));
				dialog->Commands->Append(ref new UICommand(pno, commandHandler, IDNO));
				dialog->DefaultCommandIndex = 0;
				dialog->CancelCommandIndex = 1;
			}
			dialog->ShowAsync();
		});
		try
		{
			handler->Invoke();
		}
		catch (Platform::AccessDeniedException^ e)
		{
			hlog::warn(april::logTag, "messagebox() on WinRT called \"recursively\"! Queueing to UI thread now...");
			messageBoxQueueMutex.lock();
			messageBoxQueue += handler;
			messageBoxQueueMutex.unlock();
		}
	}

}
#endif
