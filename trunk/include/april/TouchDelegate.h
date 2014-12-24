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
	/// @brief Defines a delegate for the touch input callbacks.
	class aprilExport TouchDelegate
	{
	public:
		/// @brief Basic constructor.
		TouchDelegate();
		/// @brief Destructor.
		virtual ~TouchDelegate();

		/// @brief Called when the number of touches or their position changes.
		/// @param[in] touches A list of touch coordinates.
		virtual void onTouch(const harray<gvec2>& touches);

	};

}
#endif
