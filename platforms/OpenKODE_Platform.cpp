/// @file
/// @author  Boris Mikic
/// @version 3.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef _OPENKODE
#include <KD/kd.h>

#ifdef __APPLE__
#include <TargetConditionals.h>
#endif

#include <gtypes/Vector2.h>
#include <hltypes/hlog.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hmap.h>
#include <hltypes/hplatform.h>
#include <hltypes/hstring.h>

#include "april.h"
#ifdef _EGL
#include "egl.h"
#endif
#include "Platform.h"
#include "RenderSystem.h"
#include "Window.h"

namespace april
{
	SystemInfo getSystemInfo()
	{
		static SystemInfo info;
		if (info.locale == "")
		{
			// number of CPU cores
			// TODOkd
			info.cpuCores = 1;
			// RAM size
#if TARGET_IPHONE_SIMULATOR
			info.ram = 1024;
#else
			int ram;
			kdQueryAttribi(KD_ATTRIB_RAM, (KDint*)&ram);
			info.ram = ram / 1048576; // in MB
#endif

			// display resolution
			int width = 0;
			int height = 0;
			kdQueryAttribi(KD_ATTRIB_WIDTH, (KDint*)&width);
			kdQueryAttribi(KD_ATTRIB_HEIGHT, (KDint*)&height);
			info.displayResolution.set((float)width, (float)height);
			// display DPI
			kdQueryAttribi(KD_ATTRIB_DPI, (KDint*)&info.displayDpi);
			// other
			info.locale = hstr(kdGetLocale());
			if (info.locale == "")
			{
				info.locale = "en"; // default is "en"
			}
			else if (info.locale.utf8_size() > 2)
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
		return DEVICE_OPENKODE;
	}

	hstr getPackageName()
	{
		return hstr(kdGetenv("KD_APP_ID"));
	}

	hstr getUserDataPath()
	{
		return "data";
	}
	
	void messageBox_platform(chstr title, chstr text, MessageBoxButton buttonMask, MessageBoxStyle style,
		hmap<MessageBoxButton, hstr> customButtonTitles, void(*callback)(MessageBoxButton))
	{
		hstr ok;
		hstr yes;
		hstr no;
		hstr cancel;
		_makeButtonLabels(&ok, &yes, &no, &cancel, buttonMask, customButtonTitles);
		const char* buttons[4] = {"", "", "", NULL};
		MessageBoxButton resultButtons[4] = {AMSGBTN_NULL, AMSGBTN_NULL, AMSGBTN_NULL, AMSGBTN_NULL};
		int indexCancel = -1;
		if ((buttonMask & AMSGBTN_OK) && (buttonMask & AMSGBTN_CANCEL))
		{
			buttons[0] = ok.c_str();
			buttons[1] = cancel.c_str();
			resultButtons[0] = AMSGBTN_OK;
			resultButtons[1] = AMSGBTN_CANCEL;
			indexCancel = 1;
		}
		else if ((buttonMask & AMSGBTN_YES) && (buttonMask & AMSGBTN_NO) && (buttonMask & AMSGBTN_CANCEL))
		{
			buttons[0] = yes.c_str();
			buttons[1] = no.c_str();
			buttons[2] = cancel.c_str();
			resultButtons[0] = AMSGBTN_YES;
			resultButtons[1] = AMSGBTN_NO;
			resultButtons[2] = AMSGBTN_CANCEL;
			indexCancel = 2;
		}
		else if (buttonMask & AMSGBTN_OK)
		{
			buttons[0] = ok.c_str();
			resultButtons[0] = AMSGBTN_OK;
			indexCancel = 0;
		}
		else if ((buttonMask & AMSGBTN_YES) && (buttonMask & AMSGBTN_NO))
		{
			buttons[0] = yes.c_str();
			buttons[1] = no.c_str();
			resultButtons[0] = AMSGBTN_YES;
			resultButtons[1] = AMSGBTN_NO;
			indexCancel = 1;
		}
		int index = kdShowMessage(title.c_str(), text.c_str(), buttons);
		if (index == -1)
		{
			index = indexCancel;
		}
		if (callback != NULL && index >= 0)
		{
			(*callback)(resultButtons[index]);
		}
	}

}
#endif
