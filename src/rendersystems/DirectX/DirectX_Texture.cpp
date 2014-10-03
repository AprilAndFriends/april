/// @file
/// @version 3.5
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#ifdef _DIRECTX
#include "DirectX_Texture.h"

namespace april
{
	DirectX_Texture::DirectX_Texture(bool fromResource) : Texture(fromResource)
	{
	}

	DirectX_Texture::~DirectX_Texture()
	{
	}

	bool DirectX_Texture::_uploadToGpu(int sx, int sy, int sw, int sh, int dx, int dy, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat)
	{
		Lock lock = this->_tryLockSystem(dx, dy, sw, sh);
		if (lock.failed)
		{
			return false;
		}
		bool result = true;
		if (srcData != lock.data)
		{
			Image::write(sx, sy, sw, sh, lock.x, lock.y, srcData, srcWidth, srcHeight, srcFormat, lock.data, lock.dataWidth, lock.dataHeight, lock.format);
		}
		this->_unlockSystem(lock, true);
		return result;
	}

}
#endif
