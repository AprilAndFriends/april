/// @file
/// @author  Kresimir Spes
/// @author  Ivan Vucica
/// @author  Boris Mikic
/// @version 3.3
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef _OPENGL1
#include <hltypes/hplatform.h>
#ifndef __APPLE__
#include <gl/GL.h>
#else
#include <OpenGL/gl.h>
#endif

#include <hltypes/hlog.h>
#include <hltypes/hstring.h>

#include "april.h"
#include "Color.h"
#include "Image.h"
#include "OpenGL1_Texture.h"
#include "OpenGL1_RenderSystem.h"

#define APRIL_OGL1_RENDERSYS ((OpenGL1_RenderSystem*)april::rendersys)

namespace april
{
	OpenGL1_Texture::OpenGL1_Texture() : OpenGL_Texture()
	{
	}

	OpenGL1_Texture::~OpenGL1_Texture()
	{
	}

	bool OpenGL1_Texture::copyPixelData(unsigned char** output, Image::Format format)
	{
		if (this->data != NULL)
		{
			return Texture::copyPixelData(output);
		}
		if (!this->isLoaded())
		{
			return false;
		}
		int size = this->getByteSize();
		Image::Format nativeFormat = april::rendersys->getNativeTextureFormat(this->format);
		glBindTexture(GL_TEXTURE_2D, this->textureId);
		APRIL_OGL1_RENDERSYS->currentState.textureId = APRIL_OGL1_RENDERSYS->deviceState.textureId = this->textureId;
		if (!Image::needsConversion(nativeFormat, format)) // to avoid unnecessary copying
		{
			if (*output == NULL)
			{
				*output = new unsigned char[size];
			}
			glGetTexImage(GL_TEXTURE_2D, 0, this->glFormat, GL_UNSIGNED_BYTE, *output);
			return true;
		}
		unsigned char* temp = new unsigned char[size];
		glGetTexImage(GL_TEXTURE_2D, 0, this->glFormat, GL_UNSIGNED_BYTE, temp);
		bool result = Image::convertToFormat(this->width, this->height, temp, nativeFormat, output, format);
		delete [] temp;
		return result;
	}

}

#endif
