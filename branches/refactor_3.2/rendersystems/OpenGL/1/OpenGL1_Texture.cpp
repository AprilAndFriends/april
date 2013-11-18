/// @file
/// @author  Kresimir Spes
/// @author  Ivan Vucica
/// @author  Boris Mikic
/// @version 3.1
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
#include "Image.h"
#include "OpenGL1_Texture.h"
#include "OpenGL1_RenderSystem.h"

#define APRIL_OGL1_RENDERSYS ((OpenGL1_RenderSystem*)april::rendersys)

namespace april
{
	OpenGL1_Texture::OpenGL1_Texture(chstr filename) : OpenGL_Texture(filename)
	{
	}

	OpenGL1_Texture::OpenGL1_Texture(int w, int h, unsigned char* rgba) : OpenGL_Texture(w, h, rgba)
	{
	}

	OpenGL1_Texture::OpenGL1_Texture(int w, int h, Format format, Type type, Color color) :
		OpenGL_Texture(w, h, format, type, color)
	{
	}

	OpenGL1_Texture::~OpenGL1_Texture()
	{
	}

	bool OpenGL1_Texture::copyPixelData(unsigned char** output)
	{
		this->load();
		glBindTexture(GL_TEXTURE_2D, this->textureId);
		APRIL_OGL1_RENDERSYS->state.textureId = APRIL_OGL1_RENDERSYS->deviceState.textureId = this->textureId;
		*output = new unsigned char[this->width * this->height * this->bpp];
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, *output);
		return true;
	}
	
}

#endif
