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
/// Defines a color object.

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
		Color(int r, int g, int b, int a = 255);
		Color(unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255);
		Color(unsigned int color);
		Color(chstr hex);
		Color(Color color, unsigned char a);

		void set(int r, int g, int b, int a = 255);
		void set(unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255);
		void set(unsigned int color);
		void set(chstr hex);
		void set(Color color, unsigned char a);

		float r_f() const { return r / 255.0f; }
		float g_f() const { return g / 255.0f; }
		float b_f() const { return b / 255.0f; }
		float a_f() const { return a / 255.0f; }
		
		hstr hex(bool rgbOnly = false) const; // careful when using rgbOnly!
		
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
		
	};

}

// predefined colors
#define APRIL_COLOR_RED april::Color(255, 0, 0)
#define APRIL_COLOR_GREEN april::Color(0, 255, 0)
#define APRIL_COLOR_BLUE april::Color(0, 0, 255)
#define APRIL_COLOR_YELLOW april::Color(255, 255, 0)
#define APRIL_COLOR_MAGENTA april::Color(255, 0, 255)
#define APRIL_COLOR_CYAN april::Color(0, 255, 255)
#define APRIL_COLOR_ORANGE april::Color(255, 127, 0)
#define APRIL_COLOR_PINK april::Color(255, 0, 127)
#define APRIL_COLOR_TEAL april::Color(0, 255, 127)
#define APRIL_COLOR_NEON april::Color(127, 255, 0)
#define APRIL_COLOR_PURPLE april::Color(127, 0, 255)
#define APRIL_COLOR_AQUA april::Color(0, 127, 255)
#define APRIL_COLOR_WHITE april::Color(255, 255, 255)
#define APRIL_COLOR_GREY april::Color(127, 127, 127)
#define APRIL_COLOR_BLACK april::Color(0, 0, 0)
#define APRIL_COLOR_CLEAR april::Color(0, 0, 0, 0)
#define APRIL_COLOR_BLANK april::Color(255, 255, 255, 0)
		
#endif
