/// @file
/// @version 4.5
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Defines a touch event.

#ifndef APRIL_TOUCH_EVENT_H
#define APRIL_TOUCH_EVENT_H

#include <hltypes/harray.h>
#include <gtypes/Vector2.h>

#include "aprilExport.h"

namespace april
{
	/// @brief Defines touch-interface input event data.
	class aprilExport TouchEvent
	{
	public:
		/// @brief Active touch pointers.
		harray<gvec2> touches;
			
		/// @brief Basic constructor.
		TouchEvent();
		/// @brief Constructor.
		/// @param[in] touches Active touch pointers.
		TouchEvent(const harray<gvec2>& touches);
		
	};

}
#endif
