/// @file
/// @version 5.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#ifdef _OPENGLES
#include <hltypes/hlog.h>
#include <hltypes/hstring.h>

#include "april.h"
#include "Color.h"
#include "OpenGLES_RenderSystem.h"
#include "OpenGLES_Texture.h"

#define APRIL_OGLES_RENDERSYS ((OpenGLES_RenderSystem*)april::rendersys)

namespace april
{
	OpenGLES_Texture::OpenGLES_Texture(bool fromResource) : OpenGL_Texture(fromResource)
	{
#ifdef _ANDROID
		this->alphaTextureId = 0;
#endif
	}

	OpenGLES_Texture::~OpenGLES_Texture()
	{
	}

	bool OpenGLES_Texture::_deviceCreateTexture(unsigned char* data, int size, Type type)
	{
		if (!OpenGL_Texture::_deviceCreateTexture(data, size, type))
		{
			return false;
		}
		// data has to be uploaded right away if compressed texture
#ifdef _IOS
		if (this->dataFormat == GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG || this->dataFormat == GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG)
		{
			this->_setCurrentTexture();
			glCompressedTexImage2D(GL_TEXTURE_2D, 0, this->dataFormat, this->width, this->height, 0, size, data);
			GLenum glError = glGetError();
			SAFE_TEXTURE_UPLOAD_CHECK(glError, glCompressedTexImage2D(GL_TEXTURE_2D, 0, this->dataFormat, this->width, this->height, 0, size, data));
			this->firstUpload = false;
		}
#endif
#ifdef _ANDROID
		if ((this->dataFormat & GL_ETC1_RGB8_OES) == GL_ETC1_RGB8_OES)
		{
			if (!APRIL_OGLES_RENDERSYS->etc1Supported)
			{
				hlog::error(logTag, "Trying to use ETC1 textures, but system reported that they are not supported!");
				this->_deviceDestroyTexture();
				return false;
			}
			GLenum glError = 0;
			if (this->dataFormat == GL_ETCX_RGBA8_OES_HACK)
			{
				size /= 2;
				glGenTextures(1, &this->alphaTextureId);
				if (this->alphaTextureId == 0)
				{
					hlog::warn(logTag, "Could not create alpha texture hack: " + this->_getInternalName());
				}
				else
				{
					unsigned int originalTextureId = this->textureId;
					this->textureId = this->alphaTextureId;
					this->alphaTextureId = 0;
					this->_setCurrentTexture();
					glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_ETC1_RGB8_OES, this->width, this->height, 0, size, &data[size]);
					glError = glGetError();
					SAFE_TEXTURE_UPLOAD_CHECK(glError, glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_ETC1_RGB8_OES, this->width, this->height, 0, size, &data[size]));
					this->alphaTextureId = this->textureId;
					this->textureId = originalTextureId;
				}
			}
			this->_setCurrentTexture();
			glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_ETC1_RGB8_OES, this->width, this->height, 0, size, data);
			glError = glGetError();
			SAFE_TEXTURE_UPLOAD_CHECK(glError, glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_ETC1_RGB8_OES, this->width, this->height, 0, size, data));
			this->firstUpload = false;
		}
#endif
		return true;
	}

	bool OpenGLES_Texture::_deviceDestroyTexture()
	{
#ifdef _ANDROID
		if (this->alphaTextureId != 0)
		{
			glDeleteTextures(1, &this->alphaTextureId);
			this->alphaTextureId = 0;
		}
#endif
		return OpenGL_Texture::_deviceDestroyTexture();
	}

}
#endif
