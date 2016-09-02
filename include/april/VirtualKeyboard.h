/// @file
/// @version 4.1
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Defines an interface for implementation if a custom virtual keyboard is required.

#ifndef APRIL_VIRTUAL_KEYBOARD_H
#define APRIL_VIRTUAL_KEYBOARD_H

#include <hltypes/hltypesUtil.h>

#include "aprilExport.h"

namespace april
{
	/// @brief Defines an interface for implementation if a custom virtual keyboard is required.
	class aprilExport VirtualKeyboard
	{
	public:
		/// @brief Basic constructor.
		VirtualKeyboard();
		/// @brief Destructor.
		virtual ~VirtualKeyboard();

		HL_DEFINE_IS(visible, Visible);
		HL_DEFINE_GET(float, heightRatio, HeightRatio);

		void show();
		void hide();

		void draw();

	protected:
		bool visible;
		float heightRatio;

		virtual float _show() = 0;
		virtual bool _hide() = 0;

		virtual void _draw() = 0;


	};

}
#endif
