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

		/// @brief The cursor position in the current callback.
		HL_DEFINE_GETSET2(hmap, int, gvec2f, currentIndexedTouches, CurrentIndexedTouches);

		/// @brief Called when an indexed touch-down event happens.
		/// @param[in] index Index of the touch.
		virtual void onTouchDown(int index);
		/// @brief Called when an indexed touch-up event happens.
		/// @param[in] index Index of the touch.
		virtual void onTouchUp(int index);
		/// @brief Called when an indexed touch-move event happens.
		/// @param[in] index Index of the touch.
		virtual void onTouchMove(int index);
		/// @brief Called when an indexed touch-cancel event happens.
		/// @param[in] index Index of the touch.
		virtual void onTouchCancel(int index);
		/// @brief Called when the number of touches or their position changes.
		/// @param[in] touches A list of touch coordinates.
		/// @note This method isn't as reliable as other methods for processing touches, but it will remain available for legacy reasons.
		virtual void onTouch(const harray<gvec2f>& touches);

	protected:
		/// @brief The cursor position in the current callback.
		hmap<int, gvec2f> currentIndexedTouches;

	};

}
#endif
