/************************************************************************************\
This source file is part of the Awesome Portable Rendering Interface Library         *
For latest info, see http://libapril.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes, Boris Mikic                                        *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#ifndef APRIL_COLOR_H
#define APRIL_COLOR_H

#include <hltypes/hstring.h>

#include "aprilExport.h"

namespace april
{
	class aprilExport Color
	{
	public:
		unsigned char r;
		unsigned char g;
		unsigned char b;
		unsigned char a;
		
		Color();
		Color(int _r, int _g, int _b, int _a = 255);
		Color(unsigned char _r, unsigned char _g, unsigned char _b, unsigned char _a = 255);
		Color(unsigned int color);
		Color(chstr hex);
		Color(Color color, unsigned char _a);

		void set(int _r, int _g, int _b, int _a = 255);
		void set(unsigned char _r, unsigned char _g, unsigned char _b, unsigned char _a = 255);
		void set(unsigned int color);
		void set(chstr hex);
		void set(Color color, unsigned char _a);

		float r_f() { return r / 255.0f; }
		float g_f() { return g / 255.0f; }
		float b_f() { return b / 255.0f; }
		float a_f() { return a / 255.0f; }
		
		hstr hex(bool rgbOnly = false); // careful when using rgbOnly!
		
		operator unsigned int() const;
		
		bool operator==(Color& other);
		bool operator!=(Color& other);
		
		Color operator+(Color& other);
		Color operator-(Color& other);
		Color operator*(Color& other);
		Color operator/(Color& other);
		Color operator*(float value);
		Color operator/(float value);
		Color operator+=(Color& other);
		Color operator-=(Color& other);
		Color operator*=(Color& other);
		Color operator/=(Color& other);
		Color operator*=(float value);
		Color operator/=(float value);
		
		// predefined colors
		#define RED			Color(255,   0,   0)
		#define GREEN		Color(  0, 255,   0)
		#define BLUE		Color(  0,   0, 255)
		#define YELLOW		Color(255, 255,   0)
		#define MANGENTA	Color(255,   0, 255)
		#define CYAN		Color(  0, 255, 255)
		#define ORANGE		Color(255, 127,   0)
		#define PINK		Color(255,   0, 127)
		#define TEAL		Color(  0, 255, 127)
		#define NEON		Color(127, 255,   0)
		#define PURPLE		Color(127,   0, 255)
		#define AQUA		Color(  0, 127, 255)
		#define WHITE		Color(255, 255, 255)
		#define GREY		Color(127, 127, 127)
		#define BLACK		Color(  0,   0,   0)
		#define CLEAR		Color(  0,   0,   0,   0)
		
	};

}

#endif
