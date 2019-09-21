/// @file
/// @version 5.2
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
	OpenGLES_Texture::OpenGLES_Texture(bool fromResource) :
		OpenGL_Texture(fromResource),
		framebufferId(0)
	{
#ifdef __ANDROID__
		this->alphaTextureId = 0;
#endif
	}

	bool OpenGLES_Texture::_deviceCreateTexture(unsigned char* data, int size)
	{
		if (!OpenGL_Texture::_deviceCreateTexture(data, size))
		{
			return false;
		}
		if (this->type == Type::RenderTarget)
		{
			GL_SAFE_CALL(glGenFramebuffers, (1, &this->framebufferId));
			if (this->framebufferId == 0)
			{
				hlog::error(logTag, "Cannot create GL frame buffer for: " + this->_getInternalName());
				return false;
			}
			this->_setCurrentTexture();
			this->_uploadPotSafeClearData();
			unsigned int previousFramebufferId = 0;
			glGetIntegerv(GL_FRAMEBUFFER_BINDING, (GLint*)&previousFramebufferId);
			GL_SAFE_CALL(glBindFramebuffer, (GL_FRAMEBUFFER, this->framebufferId));
			GL_SAFE_CALL(glFramebufferTexture2D, (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, this->internalType, this->textureId, 0));
			GL_SAFE_CALL(glBindFramebuffer, (GL_FRAMEBUFFER, previousFramebufferId));
		}
		// data has to be uploaded right away if compressed texture
#ifdef _IOS
		if (this->dataFormat == GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG || this->dataFormat == GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG)
		{
			this->_setCurrentTexture();
			glCompressedTexImage2D(this->internalType, 0, this->dataFormat, this->width, this->height, 0, size, data);
			GLenum glError = glGetError();
			SAFE_TEXTURE_UPLOAD_CHECK(glError, glCompressedTexImage2D(this->internalType, 0, this->dataFormat, this->width, this->height, 0, size, data));
			this->firstUpload = false;
		}
#endif
#ifdef __ANDROID__
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
				GL_SAFE_CALL(glGenTextures, (1, &this->alphaTextureId));
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
					glCompressedTexImage2D(this->internalType, 0, GL_ETC1_RGB8_OES, this->width, this->height, 0, size, &data[size]);
					glError = glGetError();
					SAFE_TEXTURE_UPLOAD_CHECK(glError, glCompressedTexImage2D(this->internalType, 0, GL_ETC1_RGB8_OES, this->width, this->height, 0, size, &data[size]));
					this->alphaTextureId = this->textureId;
					this->textureId = originalTextureId;
				}
			}
			this->_setCurrentTexture();
			glCompressedTexImage2D(this->internalType, 0, GL_ETC1_RGB8_OES, this->width, this->height, 0, size, data);
			glError = glGetError();
			SAFE_TEXTURE_UPLOAD_CHECK(glError, glCompressedTexImage2D(this->internalType, 0, GL_ETC1_RGB8_OES, this->width, this->height, 0, size, data));
			this->firstUpload = false;
		}
#endif
		return true;
	}

	bool OpenGLES_Texture::_deviceDestroyTexture()
	{
		if (this->framebufferId != 0)
		{
			if (april::rendersys->canUseLowLevelCalls())
			{
				glDeleteFramebuffers(1, &this->framebufferId);
			}
			this->framebufferId = 0;
		}
#ifdef __ANDROID__
		if (this->alphaTextureId != 0)
		{
			if (april::rendersys->canUseLowLevelCalls())
			{
				glDeleteTextures(1, &this->alphaTextureId);
			}
			this->alphaTextureId = 0;
		}
#endif
		return OpenGL_Texture::_deviceDestroyTexture();
	}

	void OpenGLES_Texture::_assignFormat()
	{
#ifdef __ANDROID__
		if (this->type == Type::External)
		{
			this->internalType = GL_TEXTURE_EXTERNAL_OES;
		}
#endif
		OpenGL_Texture::_assignFormat();
	}

}
#endif
