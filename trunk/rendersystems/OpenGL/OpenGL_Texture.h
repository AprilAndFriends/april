/// @file
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @version 2.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php
/// 
/// @section DESCRIPTION
/// 
/// Defines an OpenGL specific texture.

#if defined(_OPENGL) || (_OPENGLES1)
#ifndef APRIL_OPENGL_TEXTURE_H
#define APRIL_OPENGL_TEXTURE_H

#ifdef __APPLE__
#include <TargetConditionals.h>
#endif
#if (TARGET_OS_MAC) && !(TARGET_OS_IPHONE)
#include <OpenGL/gl.h>
#elif (TARGET_OS_IPHONE)
#include <OpenGLES/ES1/gl.h>
#elif (_OPENGLES1)
#include <GLES/gl.h>
#else
#include <GL/gl.h>
#endif

#include "RenderSystem.h"

namespace april
{
	class OpenGL_RenderSystem;

	class OpenGL_Texture : public Texture
	{
	public:
		friend class OpenGL_RenderSystem;
		
		OpenGL_Texture(chstr filename);
		OpenGL_Texture(int w, int h, unsigned char* rgba);
		OpenGL_Texture(int w, int h, Format format, Type type, Color color = APRIL_COLOR_CLEAR);
		~OpenGL_Texture();
		bool load();
		void unload();

		bool isLoaded();

		void clear();
		Color getPixel(int x, int y);
		void setPixel(int x, int y, Color color);
		void fillRect(int x, int y, int w, int h, Color color);
		void blit(int x, int y, Texture* texture, int sx, int sy, int sw, int sh, unsigned char alpha = 255);
		void blit(int x, int y, unsigned char* data, int dataWidth, int dataHeight, int dataBpp, int sx, int sy, int sw, int sh, unsigned char alpha = 255);
		void stretchBlit(int x, int y, int w, int h, Texture* texture, int sx, int sy, int sw, int sh, unsigned char alpha = 255);
		void stretchBlit(int x, int y, int w, int h, unsigned char* data,int dataWidth, int dataHeight, int dataBpp, int sx, int sy, int sw, int sh, unsigned char alpha = 255);
		void rotateHue(float degrees);
		void saturate(float factor);
		bool copyPixelData(unsigned char** output);

	protected:
		GLuint textureId;
		unsigned char* manualBuffer;

	};

}

#endif
#endif
