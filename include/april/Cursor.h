/// @file
/// @version 5.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Defines a Cursor object.

#ifndef APRIL_CURSOR_H
#define APRIL_CURSOR_H

#include <hltypes/hltypesUtil.h>
#include <hltypes/hstring.h>

#include "aprilExport.h"

namespace april
{
	class Window;

	/// @brief Defines a cursor object.
	class aprilExport Cursor
	{
	public:
		friend class Window;

		/// @brief The filename of the cursor file.
		HL_DEFINE_GET(hstr, filename, Filename);
		
	protected:
		/// @brief The filename of the cursor file.
		hstr filename;
		/// @brief Whether loaded from a resource file or a normal file.
		bool fromResource;
		
		/// @brief Basic constructor.
		/// @param[in] fromResource Whether loaded from a resource file or a normal file.
		Cursor(bool fromResource);
		/// @brief Destructor.
		virtual ~Cursor();

		/// @brief Creates an internal curosr object from a filename.
		/// @param[in] filename The filename to load.
		/// @return True if loading was successful.
		virtual bool _create(chstr filename);

	};

}

#endif
