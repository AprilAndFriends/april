/// @file
/// @version 5.2
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Defines a touches event.

#ifndef APRIL_TOUCHES_EVENT_H
#define APRIL_TOUCHES_EVENT_H

#include <hltypes/harray.h>
#include <gtypes/Vector2.h>

#include "aprilExport.h"

namespace april
{
	/// @brief Defines touch-interface input event data.
	class aprilExport TouchesEvent
	{
	public:
		/// @brief Active touch pointers.
		harray<gvec2f> touches;
		
		/// @brief Constructor.
		/// @param[in] touches Active touch pointers.
		TouchesEvent(const harray<gvec2f>& touches);
		
	};

}
#endif
