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
/// Defines a delegate for all event callbacks. One delegate to rule them all.

#ifndef APRIL_EVENT_DELEGATE_H
#define APRIL_EVENT_DELEGATE_H

#include "aprilExport.h"
#include "InputDelegate.h"
#include "UpdateDelegate.h"
#include "SystemDelegate.h"

namespace april
{
	class aprilExport EventDelegate : public InputDelegate, public UpdateDelegate, public SystemDelegate
	{
	public:
		EventDelegate();
		~EventDelegate();

	};

}
#endif
