/// @file
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @version 3.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php
/// 
/// @section DESCRIPTION
/// 
/// Defines an OpenGLES1 specific texture.

#ifdef _OPENGLES1
#ifndef APRIL_OPENGLES1_TEXTURE_H
#define APRIL_OPENGLES1_TEXTURE_H

#ifdef __APPLE__
#include <TargetConditionals.h>
#endif
#if (TARGET_OS_MAC) && !(TARGET_OS_IPHONE)
#include <OpenGL/gl.h>
#elif (TARGET_OS_IPHONE)
#include <OpenGLES/ES1/gl.h>
#elif defined(_OPENGLES)
#include <GLES/gl.h>
#else
#include <GL/gl.h>
#endif

#include "OpenGLES_Texture.h"

namespace april
{
	class OpenGLES1_RenderSystem;

	class OpenGLES1_Texture : public OpenGLES_Texture
	{
	public:
		friend class OpenGLES1_RenderSystem;
		
		OpenGLES1_Texture(chstr filename);
		OpenGLES1_Texture(int w, int h, unsigned char* rgba);
		OpenGLES1_Texture(int w, int h, Format format, Type type, Color color = Color::Clear);
		~OpenGLES1_Texture();
		bool load();
		void unload();

		bool isLoaded();

		void clear();
		Color getPixel(int x, int y);
		void setPixel(int x, int y, Color color);
		void fillRect(int x, int y, int w, int h, Color color);
		void blit(int x, int y, Texture* texture, int sx, int sy, int sw, int sh, unsigned char alpha = 255);
		void blit(int x, int y, unsigned char* data, int dataWidth, int dataHeight, int dataBpp, int sx, int sy, int sw, int sh, unsigned char alpha = 255);
		void write(int x, int y, unsigned char* data, int dataWidth, int dataHeight, int dataBpp);
		void stretchBlit(int x, int y, int w, int h, Texture* texture, int sx, int sy, int sw, int sh, unsigned char alpha = 255);
		void stretchBlit(int x, int y, int w, int h, unsigned char* data,int dataWidth, int dataHeight, int dataBpp, int sx, int sy, int sw, int sh, unsigned char alpha = 255);
		void rotateHue(float degrees);
		void saturate(float factor);
		bool copyPixelData(unsigned char** output);

	protected:
		GLuint textureId;
		unsigned char* manualBuffer;

		void _setCurrentTexture();

	};

}

#endif
#endif
