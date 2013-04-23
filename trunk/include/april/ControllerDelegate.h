/// @file
/// @author  Boris Mikic
/// @version 3.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php
/// 
/// @section DESCRIPTION
/// 
/// Defines a delegate for the controller input callbacks.

#ifndef APRIL_CONTROLLER_DELEGATE_H
#define APRIL_CONTROLLER_DELEGATE_H

// TDODO - due to a bug in the GCC 4.6 toolset, linking won't work if the members are defined in a separate source file
#ifdef _ANDROID
#include <hltypes/hlog.h>
#include <hltypes/hstring.h>
#endif

#include "aprilExport.h"
#include "Keys.h"

namespace april
{
	class aprilExport ControllerDelegate
	{
	public:
#ifndef _ANDROID
		ControllerDelegate();
		virtual ~ControllerDelegate();

		virtual void onButtonDown(april::Button buttonCode) {}
		virtual void onButtonUp(april::Button buttonCode) {}
		// TODO - analog triggers and analog sticks
#else
		ControllerDelegate()
		{
		}

		virtual ~ControllerDelegate()
		{
		}

		virtual void onButtonDown(Button buttonCode)
		{
			hlog::debug(april::logTag, "Event onButtonDown() was not implemented.");
		}

		virtual void onButtonUp(Button buttonCode)
		{
			hlog::debug(april::logTag, "Event onButtonUp() was not implemented.");
		}
#endif

	};

}
#endif
