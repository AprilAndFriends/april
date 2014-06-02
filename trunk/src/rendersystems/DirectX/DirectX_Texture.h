/// @file
/// @version 3.4
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Defines a generic DirectX texture.

#ifdef _DIRECTX
#ifndef APRIL_DIRECTX_TEXTURE_H
#define APRIL_DIRECTX_TEXTURE_H

#include "Texture.h"

namespace april
{
	class DirectX_Texture : public Texture
	{
	public:
		DirectX_Texture(bool fromResource);
		~DirectX_Texture();

		bool _uploadToGpu(int sx, int sy, int sw, int sh, int dx, int dy, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat);

	};

}
#endif
#endif
