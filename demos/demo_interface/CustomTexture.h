/// @file
/// @version 4.0
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
	int internalFormat;

	void _setCurrentTexture();

	bool _deviceCreateTexture(unsigned char* data, int size);
	bool _deviceDestroyTexture();
	void _assignFormat();

	april::Texture::Lock _tryLockSystem(int x, int y, int w, int h);
	bool _unlockSystem(april::Texture::Lock& lock, bool update);
	bool _uploadToGpu(int sx, int sy, int sw, int sh, int dx, int dy, unsigned char* srcData, int srcWidth, int srcHeight, april::Image::Format srcFormat);

	void _uploadClearData();

};
#endif
