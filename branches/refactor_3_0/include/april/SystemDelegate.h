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
/// Defines a delegate for special system callbacks.

#ifndef APRIL_SYSTEM_DELEGATE_H
#define APRIL_SYSTEM_DELEGATE_H

#include "aprilExport.h"

namespace april
{
	class aprilExport SystemDelegate
	{
	public:
		SystemDelegate();
		virtual ~SystemDelegate();

	};

}
#endif
