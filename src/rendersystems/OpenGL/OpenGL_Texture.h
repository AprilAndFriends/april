/// @file
/// @version 4.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Defines a generic OpenGL texture.

#ifdef _OPENGL
#ifndef APRIL_OPENGL_TEXTURE_H
#define APRIL_OPENGL_TEXTURE_H

#include "Texture.h"

namespace april
{
	class OpenGL_RenderSystem;
	class OpenGLC_RenderSystem;
	class OpenGLES_RenderSystem;

	class OpenGL_Texture : public Texture
	{
	public:
		friend class OpenGL_RenderSystem;
		friend class OpenGLC_RenderSystem;
		friend class OpenGLES_RenderSystem;

		OpenGL_Texture(bool fromResource);
		~OpenGL_Texture();

	protected:
		unsigned int textureId;
		int glFormat;
		int internalFormat;

		void _setCurrentTexture(bool forceUpdate = false);

		bool _deviceCreateTexture(unsigned char* data, int size, Type type);
		bool _deviceDestroyTexture();
		void _assignFormat();

		Lock _tryLockSystem(int x, int y, int w, int h);
		bool _unlockSystem(Lock& lock, bool update);
		bool _uploadToGpu(int sx, int sy, int sw, int sh, int dx, int dy, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat);

		void _uploadPotSafeData(unsigned char* data);
		void _uploadPotSafeClearData();

	};

}
#endif
#endif
