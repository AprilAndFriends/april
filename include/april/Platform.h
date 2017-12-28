/// @file
/// @version 5.0
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
#include <hltypes/harray.h>
#include <hltypes/henum.h>
#include <hltypes/hmap.h>
#include <hltypes/hstring.h>
#include <hltypes/hversion.h>

#include "aprilExport.h"

namespace april
{
	/// @brief Provides system information.
	struct aprilExport SystemInfo
	{
		/// @brief Name of the OS.
		hstr name;
		/// @brief Device name
		hstr deviceName;
		/// @brief CPU architecture.
		hstr architecture;
		/// @brief On how many bits the architecture operates (e.g. 32 bit or 64 bit).
		int architectureBits;
		/// @brief Version of the OS.
		hversion osVersion;
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
	
	/// @class MessageBoxButton
	/// @brief Defines possible button combinations in a message box.
	HL_ENUM_CLASS_PREFIX_DECLARE(aprilExport, MessageBoxButton,
	(
		/// @var static const MessageBoxButton MessageBoxButton::Ok
		/// @brief Only an "OK" button.
		HL_ENUM_DECLARE(MessageBoxButton, Ok);
		/// @var static const MessageBoxButton MessageBoxButton::Cancel
		/// @brief Only a "CANCEL" button.
		HL_ENUM_DECLARE(MessageBoxButton, Cancel);
		/// @var static const MessageBoxButton MessageBoxButton::Yes
		/// @brief Only a "YES" button.
		HL_ENUM_DECLARE(MessageBoxButton, Yes);
		/// @var static const MessageBoxButton MessageBoxButton::No
		/// @brief Only a "NO" button.
		HL_ENUM_DECLARE(MessageBoxButton, No);
		/// @var static const MessageBoxButton MessageBoxButton::OkCancel
		/// @brief Mouse button was pressed.
		HL_ENUM_DECLARE(MessageBoxButton, OkCancel);
		/// @var static const MessageBoxButton MessageBoxButton::YesNo
		/// @brief Mouse button was pressed.
		HL_ENUM_DECLARE(MessageBoxButton, YesNo);
		/// @var static const MessageBoxButton MessageBoxButton::YesNoCancel
		/// @brief Mouse button was pressed.
		HL_ENUM_DECLARE(MessageBoxButton, YesNoCancel);

		/// @brief Checks if button "YES" is used.
		bool hasOk() const;
		/// @brief Checks if button "YES" is used.
		bool hasCancel() const;
		/// @brief Checks if button "YES" is used.
		bool hasYes() const;
		/// @brief Checks if button "YES" is used.
		bool hasNo() const;
	));

	/// @class MessageBoxStyle
	/// @brief Defines possible display styles of a message box.
	/// @note On some OSes this affects what icon is displayed within the message box.
	HL_ENUM_CLASS_PREFIX_DECLARE(aprilExport, MessageBoxStyle,
	(
		/// @var static const MessageBoxStyle MessageBoxStyle::Normal
		/// @brief The standard display.
		HL_ENUM_DECLARE(MessageBoxStyle, Normal);
		/// @var static const MessageBoxStyle MessageBoxStyle::Info
		/// @brief Special informational display.
		HL_ENUM_DECLARE(MessageBoxStyle, Info);
		/// @var static const MessageBoxStyle MessageBoxStyle::Warning
		/// @brief Display as warning.
		HL_ENUM_DECLARE(MessageBoxStyle, Warning);
		/// @var static const MessageBoxStyle MessageBoxStyle::Critical
		/// @brief Display as critical.
		HL_ENUM_DECLARE(MessageBoxStyle, Critical);
		/// @var static const MessageBoxStyle MessageBoxStyle::Question
		/// @brief Display as question.
		HL_ENUM_DECLARE(MessageBoxStyle, Question);
	));

	/// @brief Get current OS's info.
	/// @return Current OS's info.
	aprilFnExport SystemInfo getSystemInfo();
	/// @brief Get arguments with which the app was started.
	/// @return Arguments with which the app was started.
	aprilFnExport harray<hstr> getArgs();
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
	/// @brief Opens a URL.
	/// @param[in] url The URL to open.
	/// @return True if the system call was successful.
	/// @note The return value might be true even if a URL couldn't be opened due to problems, e.g. Internet connectivity.
	aprilFnExport bool openUrl(chstr url);
	/// @brief Displays an OS message box.
	/// @param[in] title Text displayed in the title bar.
	/// @param[in] text Text displayed within the message box.
	/// @param[in] buttons Buttons which should be displayed.
	/// @param[in] style Style of the message box.
	/// @param[in] customButtonTitles Custom texts for the buttons.
	/// @param[in] callback Callback function to call once the message box was dismissed. Can be NULL if not needed.
	/// @param[in] modal Force modal message box.
	/// @param[in] terminateOnDisplay Close underlying application before showing the message box.
	/// @note Modal means that the user cannot interact with the underlying window until the message box is closed. It's not available on all OSes and some OSes have always-modal message boxes.
	/// @note terminateOnDisplay may not work the same way in all OSes.
	/// @note "customButtonTitles" may not work on all OSes.
	/// @note Depending on the OS, this function may return immediately. Write your code to assume that this function is asynchronous to avoid problems.
	/// @note Expect that "callback" will be called asynchrously once the message box is dismissed.
	aprilFnExport void showMessageBox(chstr title, chstr text, MessageBoxButton buttons = MessageBoxButton::Ok, MessageBoxStyle style = MessageBoxStyle::Normal,
		hmap<MessageBoxButton, hstr> customButtonTitles = hmap<MessageBoxButton, hstr>(), void(*callback)(MessageBoxButton) = NULL, bool modal = false, bool terminateOnDisplay = false);

#ifndef DOXYGEN_SHOULD_SKIP_THIS
	HL_DEPRECATED("Deprecated API. Use showMessageBox() instead.")
		aprilFnExport void messageBox(chstr title, chstr text, MessageBoxButton buttons = MessageBoxButton::Ok, MessageBoxStyle style = MessageBoxStyle::Normal,
			hmap<MessageBoxButton, hstr> customButtonTitles = hmap<MessageBoxButton, hstr>(), void(*callback)(MessageBoxButton) = NULL, bool modal = false, bool terminateOnDisplay = false);
#endif

#ifndef DOXYGEN_SHOULD_SKIP_THIS
	void _makeButtonLabels(hstr* ok, hstr* yes, hstr* no, hstr* cancel, MessageBoxButton buttons, hmap<MessageBoxButton, hstr> customButtonTitles);
#endif

}
#endif
