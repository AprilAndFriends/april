/************************************************************************************\
This source file is part of the Awesome Portable Rendering Interface Library         *
For latest info, see http://libapril.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes                                                     *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#ifndef APRIL_IMAGE_SOURCE_H
#define APRIL_IMAGE_SOURCE_H

#include <hltypes/hstring.h>

#include "aprilExport.h"

namespace april
{
	class Color;
	
	class aprilExport ImageSource
	{
	public:
		ImageSource();
		~ImageSource();
		
		unsigned int getImageId() { return mImageId; };
		Color getPixel(int x, int y);
		void setPixel(int x, int y, Color c);
		Color getInterpolatedPixel(float x, float y);
		void copyPixels(void* output, int format);
		
		unsigned char* data;
		int w;
		int h;
		int bpp;
		int format;
		
		int compressedLength;
		
	protected:
		unsigned int mImageId;
		
	};
	
	ImageSource* loadImage(chstr filename);
	
}

#endif
