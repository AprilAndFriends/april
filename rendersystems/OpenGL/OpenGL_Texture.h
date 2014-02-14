/// @file
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @version 3.3
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php
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

	class OpenGL_Texture : public Texture
	{
	public:
		friend class OpenGL_RenderSystem;

		OpenGL_Texture();
		~OpenGL_Texture();
		void unload();

		bool isLoaded();

		bool clear();
		Color getPixel(int x, int y);
		bool setPixel(int x, int y, Color color);
		bool fillRect(int x, int y, int w, int h, Color color);
		bool copyPixelData(unsigned char** output, Image::Format format);
		bool write(int sx, int sy, int sw, int sh, int dx, int dy, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat);
		bool write(int sx, int sy, int sw, int sh, int dx, int dy, Texture* texture);
		bool writeStretch(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat);
		bool writeStretch(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, Texture* texture);
		bool blit(int sx, int sy, int sw, int sh, int dx, int dy, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat, unsigned char alpha = 255);
		bool blit(int sx, int sy, int sw, int sh, int dx, int dy, Texture* texture, unsigned char alpha = 255);
		bool blitStretch(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat, unsigned char alpha = 255);
		bool blitStretch(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, Texture* texture, unsigned char alpha = 255);
		bool rotateHue(int x, int y, int w, int h, float degrees);
		bool saturate(int x, int y, int w, int h, float factor);
		bool invert(int x, int y, int w, int h);
		bool insertAlphaMap(unsigned char* srcData, Image::Format srcFormat, unsigned char median, int ambiguity);
		bool insertAlphaMap(Texture* texture, unsigned char median, int ambiguity);

	protected:
		unsigned int textureId;
		int glFormat;
		int internalFormat;

		void _setCurrentTexture();

		bool _createInternalTexture(unsigned char* data, int size, Type type);
		void _assignFormat();

		bool _uploadDataToGpu(int x, int y, int w, int h);


	};

}
#endif
#endif
