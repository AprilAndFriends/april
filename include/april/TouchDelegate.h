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
/// Defines a delegate for the touch input callbacks.

#ifndef APRIL_TOUCH_DELEGATE_H
#define APRIL_TOUCH_DELEGATE_H

#include <gtypes/Vector2.h>
#include <hltypes/harray.h>

#include "aprilExport.h"

namespace april
{
	class aprilExport TouchDelegate
	{
	public:
		TouchDelegate();
		virtual ~TouchDelegate();

		virtual void onTouch(const harray<gvec2>& touches);

	};

}
#endif
