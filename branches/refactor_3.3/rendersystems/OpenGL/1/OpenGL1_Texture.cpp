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

	Color OpenGL1_Texture::getPixel(int x, int y)
	{
		if (this->data != NULL)
		{
			return Texture::getPixel(x, y);
		}
		Color result = april::Color::Clear;
		Image::Format format = april::rendersys->getNativeTextureFormat(GL_NATIVE_FORMAT);
		unsigned char* pixels = NULL;
		if (this->copyPixelData(&pixels, format)) // it's not possible to get just one pixel on OpenGL so the entire texture has to be retrieved (expensive!)
		{
			result = Image::getPixel(x, y, pixels, this->width, this->height, format);
			delete [] pixels;
		}
		return result;
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
		glBindTexture(GL_TEXTURE_2D, this->textureId);
		APRIL_OGL1_RENDERSYS->currentState.textureId = APRIL_OGL1_RENDERSYS->deviceState.textureId = this->textureId;
		if (!Image::needsConversion(GL_NATIVE_FORMAT, format)) // to avoid unnecessary copying
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
		bool result = Image::convertToFormat(this->width, this->height, temp, GL_NATIVE_FORMAT, output, format);
		delete [] temp;
		return result;
	}

}

#endif
