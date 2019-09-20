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
/// Defines a generic OpenGL texture.

#ifdef _OPENGL
#ifndef APRIL_OPENGL_TEXTURE_H
#define APRIL_OPENGL_TEXTURE_H

#include "OpenGL_RenderSystem.h" // for GL header inclusion
#include "Texture.h"

// the memory warning message will likely print "volatile", but that's normal
#define SAFE_TEXTURE_UPLOAD_CHECK(glError, call) \
	if (glError == GL_OUT_OF_MEMORY) \
	{ \
		if (!_preventRecursion) \
		{ \
			OpenGL_Texture::_preventRecursion = true; \
			hlog::warnf(logTag, "Not enough VRAM for %s! Calling low memory warning.", this->_getInternalName().cStr()); \
			april::window->handleLowMemoryWarning(); \
			OpenGL_Texture::_preventRecursion = false; \
			this->_setCurrentTexture(); \
			call; \
			glError = glGetError(); \
		} \
		if (glError == GL_OUT_OF_MEMORY) \
		{ \
			hlog::error(logTag, "Failed to upload texture data: Not enough VRAM!"); \
		} \
	}

namespace april
{
	class OpenGL_RenderSystem;
	class OpenGLES_RenderSystem;

	class OpenGL_Texture : public Texture
	{
	public:
		friend class OpenGL_RenderSystem;
		friend class OpenGLES_RenderSystem;

		OpenGL_Texture(bool fromResource);

		void* getBackendId() const;
		
	protected:
		unsigned int textureId;
		int glFormat;
		int internalFormat;
		int internalType;

		void _setCurrentTexture();

		bool _deviceCreateTexture(unsigned char* data, int size);
		bool _deviceDestroyTexture();
		void _assignFormat();

		Lock _tryLockSystem(int x, int y, int w, int h);
		bool _unlockSystem(Lock& lock, bool update);
		bool _uploadToGpu(int sx, int sy, int sw, int sh, int dx, int dy, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat);

		void _uploadPotSafeData(unsigned char* data);
		void _uploadPotSafeClearData();

		static bool _preventRecursion;

	};

}
#endif
#endif
