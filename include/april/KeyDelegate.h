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
/// Defines a delegate for the key input callbacks.

#ifndef APRIL_KEY_DELEGATE_H
#define APRIL_KEY_DELEGATE_H

#include "aprilExport.h"
#include "Keys.h"

namespace april
{
	/// @brief Defines a delegate for the key input callbacks.
	class aprilExport KeyDelegate
	{
	public:
		/// @brief Basic constructor.
		KeyDelegate();
		/// @brief Destructor.
		virtual ~KeyDelegate();

		/// @brief Called when a key is pressed.
		/// @param[in] keyCode The key's number code.
		virtual void onKeyDown(april::Key keyCode);
		/// @brief Called when a key is released.
		/// @param[in] keyCode The key's number code.
		virtual void onKeyUp(april::Key keyCode);
		/// @brief Called when a character input value is sent from the OS.
		/// @param[in] charCode The unicode character.
		virtual void onChar(unsigned int charCode);

	};

}
#endif
