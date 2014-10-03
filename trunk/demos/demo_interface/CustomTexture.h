/// @file
/// @version 3.5
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#ifndef CUSTOM_TEXTURE_H
#define CUSTOM_TEXTURE_H

#include <april/Texture.h>

class CustomRenderSystem;

class CustomTexture : public april::Texture
{
public:
	friend class CustomRenderSystem;

	CustomTexture(bool fromResource);
	~CustomTexture();

protected:
	unsigned int textureId;
	int glFormat;

	bool _createInternalTexture(unsigned char* data, int size, Type type);
	void _destroyInternalTexture();
	void _assignFormat();

	Lock _tryLockSystem(int x, int y, int w, int h);
	bool _unlockSystem(Lock& lock, bool update);
	bool _uploadToGpu(int sx, int sy, int sw, int sh, int dx, int dy, unsigned char* srcData, int srcWidth, int srcHeight, april::Image::Format srcFormat);

	void _uploadClearData();

};
#endif
