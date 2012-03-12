/// @file
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @version 1.51
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php
/// 
/// @section DESCRIPTION
/// 
/// Defines a generic render system.

#ifndef APRIL_H
#define APRIL_H

#include <hltypes/hstring.h>
#include <hltypes/harray.h>

#include "aprilExport.h"

namespace april
{
	extern hstr systemPath;

	aprilFnExport void setLogFunction(void (*fnptr)(chstr));
	aprilFnExport void init();
	aprilFnExport void createRenderSystem(chstr options);
	aprilFnExport void createRenderTarget(int w, int h, bool fullscreen, chstr title);
	aprilFnExport void destroy();
	aprilFnExport void addTextureExtension(chstr extension);
	aprilFnExport harray<hstr> getTextureExtensions();
	aprilFnExport void setTextureExtensions(const harray<hstr>& exts);
	aprilFnExport void log(chstr message, chstr prefix = "[april] ");
	
}

#endif
