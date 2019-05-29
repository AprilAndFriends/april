/// @file
/// @version 5.2
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#ifdef _UWP
#define __HL_INCLUDE_PLATFORM_HEADERS
#include <gtypes/Vector2.h>
#include <hltypes/hdir.h>
#include <hltypes/hlog.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hmap.h>
#include <hltypes/hmutex.h>
#include <hltypes/hplatform.h>
#include <hltypes/hstring.h>

#include "Application.h"
#include "april.h"
#include "Platform.h"
#include "RenderSystem.h"
#include "Window.h"
#include "UWP.h"
#include "UWP_Window.h"

using namespace Windows::Foundation::Collections;
using namespace Windows::Graphics::Display;
using namespace Windows::Storage;
using namespace Windows::UI::Core;
using namespace Windows::UI::Popups;
using namespace Windows::UI::ViewManagement;

namespace april
{
	// this handling and updating of SystemInfo is required to be called on the proper thread only
	extern SystemInfo info;

	void _updateSystemInfo(SystemInfo& info)
	{
		DisplayInformation^ displayInfo = DisplayInformation::GetForCurrentView();
		// display DPI
		info.displayDpi = displayInfo->LogicalDpi;
		if (info.displayDpi < 0.01f)
		{
			static bool displayDpiLogged = false;
			if (info.displayDpi < 0.01f)
			{
				if (!displayDpiLogged)
				{
					hlog::warn(logTag, "Cannot get logical display DPI, defaulting to 96.");
				}
				info.displayDpi = 96.0f;
			}
			displayDpiLogged = true;
		}
#ifdef _UWP_WINDOW
		// display resolution
		int width = displayInfo->ScreenWidthInRawPixels;
		int height = displayInfo->ScreenHeightInRawPixels;
		// these orientations are not supported in APRIL, but Windows allows them anyway even if the manifest says that they aren't supported
		if (height > width && (displayInfo->CurrentOrientation == DisplayOrientations::Portrait ||
			displayInfo->CurrentOrientation == DisplayOrientations::PortraitFlipped))
		{
			hswap(width, height);
		}
		info.displayResolution.set(width, height);
#endif
	}

	void _updateSystemInfo()
	{
		_updateSystemInfo(info);
	}

	void _setupSystemInfo_platform(SystemInfo& info)
	{
		if (info.locale == "")
		{
			info.name = "winrt";
			info.deviceName = "UWP-device";
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
#ifndef _WINPHONE // TODOuwp - needs implementation if WinPhone 10 will be supported in the future
			// pure UWP can't retrieve this information so some arbitrary value is used
#ifndef _ARM
			info.ram = 2048;
#else
			info.ram = 1536;
#endif
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
			info.osVersion.set(10, 0);
			_updateSystemInfo(info);
		}
	}

	hstr _getPackageName_platform()
	{
		return _HL_PSTR_TO_HSTR(Windows::ApplicationModel::Package::Current->Id->FamilyName);
	}

	hstr _getUserDataPath_platform()
	{
		return hdir::systemize(_HL_PSTR_TO_HSTR(ApplicationData::Current->RoamingFolder->Path));
	}
	
	int64_t _getRamConsumption_platform()
	{
		// TODOuwp - implement this
		hlog::warn(logTag, "Cannot use getRamConsumption() on this platform.");
		return (int64_t)0;
	}	
	
	void _getNotchOffsets_platform(gvec2i& topLeft, gvec2i& bottomRight, bool landscape)
	{
		topLeft.set(0, 0);
		bottomRight.set(0, 0);
	}

	bool _openUrl_platform(chstr url)
	{
		hlog::write(logTag, "Opening URL: " + url);
		Windows::System::Launcher::LaunchUriAsync(ref new Windows::Foundation::Uri(_HL_HSTR_TO_PSTR(url)));
		return true;
	}

	static void(*_currentDialogCallback)(MessageBoxButton) = NULL;

	static void _showMessageBoxResult(int button)
	{
		switch (button)
		{
		case IDOK:
			Application::messageBoxCallback(MessageBoxButton::Ok);
			break;
		case IDYES:
			Application::messageBoxCallback(MessageBoxButton::Yes);
			break;
		case IDNO:
			Application::messageBoxCallback(MessageBoxButton::No);
			break;
		case IDCANCEL:
			Application::messageBoxCallback(MessageBoxButton::Cancel);
			break;
		default:
			Application::messageBoxCallback(MessageBoxButton::Ok);
			break;
		}
	}

	static harray<DispatchedHandler^> messageBoxQueue;
	static hmutex messageBoxQueueMutex;

	void _showMessageBox_platform(const MessageBoxData& data)
	{
		DispatchedHandler^ handler = ref new DispatchedHandler(
			[data]()
		{
			Platform::String^ ptitle = _HL_HSTR_TO_PSTR(data.title);
			Platform::String^ ptext = _HL_HSTR_TO_PSTR(data.text);
			MessageDialog^ dialog = ref new MessageDialog(ptext, ptitle);
			UICommandInvokedHandler^ commandHandler = ref new UICommandInvokedHandler(
				[](IUICommand^ command)
			{
				_showMessageBoxResult((int)command->Id);
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
			_makeButtonLabels(&ok, &yes, &no, &cancel, data.buttons, data.customButtonTitles);
			_HL_HSTR_TO_PSTR_DEF(ok);
			_HL_HSTR_TO_PSTR_DEF(yes);
			_HL_HSTR_TO_PSTR_DEF(no);
			_HL_HSTR_TO_PSTR_DEF(cancel);
			if (data.buttons == MessageBoxButton::OkCancel)
			{
				dialog->Commands->Append(ref new UICommand(pok, commandHandler, IDOK));
				dialog->Commands->Append(ref new UICommand(pcancel, commandHandler, IDCANCEL));
				dialog->DefaultCommandIndex = 0;
				dialog->CancelCommandIndex = 1;
			}
			else if (data.buttons == MessageBoxButton::YesNoCancel)
			{
				dialog->Commands->Append(ref new UICommand(pyes, commandHandler, IDYES));
				dialog->Commands->Append(ref new UICommand(pno, commandHandler, IDNO));
				dialog->Commands->Append(ref new UICommand(pcancel, commandHandler, IDCANCEL));
				dialog->DefaultCommandIndex = 0;
				dialog->CancelCommandIndex = 2;
			}
			else if (data.buttons == MessageBoxButton::Ok)
			{
				dialog->Commands->Append(ref new UICommand(pok, commandHandler, IDOK));
				dialog->DefaultCommandIndex = 0;
				dialog->CancelCommandIndex = 0;
			}
			else if (data.buttons == MessageBoxButton::YesNo)
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
			hlog::warn(logTag, "messagebox() on UWP called \"recursively\"! Queueing to UI thread now...");
			messageBoxQueueMutex.lock();
			messageBoxQueue += handler;
			messageBoxQueueMutex.unlock();
		}
	}

}
#endif
