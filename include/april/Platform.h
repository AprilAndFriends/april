/// @file
/// @version 3.5
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Defines platform specific functionality.

#ifndef APRIL_PLATFORM_H
#define APRIL_PLATFORM_H

#include <gtypes/Vector2.h>
#include <hltypes/hmap.h>
#include <hltypes/hstring.h>

#include "aprilExport.h"

namespace april
{
	/// @brief Provides system information.
	struct aprilExport SystemInfo
	{
		/// @brief Name of the OS.
		hstr name;
		/// @brief CPU architecture.
		hstr architecture;
		/// @brief On how many bits the architecture operates (e.g. 32 bit or 64 bit).
		int architectureBits;
		/// @brief Version of the OS.
		float osVersion;
		/// @brief Number of logical CPU cores (NOT physical!).
		int cpuCores;
		/// @brief How many MB of RAM the system has installed.
		/// @note On some platforms this information is not available directly and might return some values that are note entirely accurate.
		int ram;
		/// @brief Current screen resolution.
		gvec2 displayResolution;
		/// @brief Current screen DPI.
		float displayDpi;
		/// @brief Locale code as per ISO 639
		hstr locale;
		/// @brief Locale script and region codes as per ISO 15924 (for scripts) and ISO 3166 (for regions).
		/// @note If available, the "script" (ISO 15924) will always be first and the region (ISO 3166) will always be second. In that case they are separated by a "-" (dash) character.
		hstr localeVariant;

		/// @note Basic constructor.
		SystemInfo();
		/// @note Destructor.
		~SystemInfo();
		
	};
	
	/// @brief Defines possible button combinations in a message box.
	enum MessageBoxButton
	{
		/// @brief Just an "OK" button.
		MESSAGE_BUTTON_OK = 1,
		/// @brief Just a "CANCEL" button.
		MESSAGE_BUTTON_CANCEL = 2,
		/// @brief Just a "YES" button.
		MESSAGE_BUTTON_YES = 4,
		/// @brief Just a "NO" button.
		MESSAGE_BUTTON_NO = 8,
		/// @brief An "OK" button with a "CANCEL" button.
		MESSAGE_BUTTON_OK_CANCEL = MESSAGE_BUTTON_OK | MESSAGE_BUTTON_CANCEL,
		/// @brief A "YES" button with a "NO" button.
		MESSAGE_BUTTON_YES_NO = MESSAGE_BUTTON_YES | MESSAGE_BUTTON_NO,
		/// @brief A "YES" button with a "NO" and a "CANCEL" button.
		MESSAGE_BUTTON_YES_NO_CANCEL = MESSAGE_BUTTON_YES_NO | MESSAGE_BUTTON_CANCEL

	};
	
	/// @brief Defines possible display styles of a message box.
	/// @note On some OSes this affects what icon is displayed within the message box.
	enum MessageBoxStyle
	{
		/// @brief The standard display.
		MESSAGE_STYLE_NORMAL = 0,
		/// @brief Special informational display.
		MESSAGE_STYLE_INFO = 1,
		/// @brief Display as warning.
		MESSAGE_STYLE_WARNING = 2,
		/// @brief Display as critical.
		MESSAGE_STYLE_CRITICAL = 3,
		/// @brief Display as question.
		MESSAGE_STYLE_QUESTION = 4,
		/// @brief Force modal message box.
		/// @note Modal means that the user cannot interact with the underlying window until the message box is closed.
		/// @note Not available on all OSes and some OSes have always-modal message boxes.
		MESSAGE_STYLE_MODAL = 8,
		/// @brief Close underlying application before showing the message box.
		/// @note This might not work the same way in all OSes.
		MESSAGE_STYLE_TERMINATE_ON_DISPLAY = 16

	};

	/// @brief Get current OS's info.
	/// @return Current OS's info.
	aprilFnExport SystemInfo getSystemInfo();
	/// @brief Get current app's "package name".
	/// @return Current app's "package name".
	/// @note This is not available on all platforms.
	/// @note "Package name" might be named differently on different OSes. e.g. Android names it "Package Name", iOS names it "Bundle ID", WinRT names it "Package Family Name".
	aprilFnExport hstr getPackageName();
	/// @brief Get current OS's user directory path.
	/// @return Current OS's user directory path.
	aprilFnExport hstr getUserDataPath();
	/// @brief Get current process' RAM consumption.
	/// @return Current process' RAM consumption.
	/// @note This is the RAM consumed by the entire process.
	aprilFnExport int64_t getRamConsumption();
	/// @brief Displays an OS message box.
	/// @param[in] title Text displayed in the title bar.
	/// @param[in] text Text displayed within the message box.
	/// @param[in] buttonMask Buttons which should be displayed.
	/// @param[in] style Style of the message box.
	/// @param[in] customButtonTitles Custom texts for the buttons.
	/// @param[in] callback Callback function to call once the message box was dismissed. Can be NULL if not needed.
	/// @note "customButtonTitles" may not work on all OSes.
	/// @note Depending on the OS, this function may return immediately. Write your code to assume that this function is asynchronous to avoid problems.
	/// @note Expect that "callback" will be called asynchrously once the message box is dismissed.
	aprilFnExport void messageBox(chstr title, chstr text, MessageBoxButton buttonMask = MESSAGE_BUTTON_OK, MessageBoxStyle style = MESSAGE_STYLE_NORMAL,
		hmap<MessageBoxButton, hstr> customButtonTitles = hmap<MessageBoxButton, hstr>(), void(*callback)(MessageBoxButton) = NULL);

	/// @brief Used internally only.
	void _makeButtonLabels(hstr* ok, hstr* yes, hstr* no, hstr* cancel,
		MessageBoxButton buttonMask, hmap<MessageBoxButton, hstr> customButtonTitles);
	/// @brief Used internally only.
	void messageBox_platform(chstr title, chstr text, MessageBoxButton buttonMask = MESSAGE_BUTTON_OK, MessageBoxStyle style = MESSAGE_STYLE_NORMAL,
		hmap<MessageBoxButton, hstr> customButtonTitles = hmap<MessageBoxButton, hstr>(), void(*callback)(MessageBoxButton) = NULL);

}

#endif
