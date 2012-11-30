/// @file
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @version 2.5
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

		static Color Red;
		static Color Green;
		static Color Blue;
		static Color Yellow;
		static Color Magenta;
		static Color Cyan;
		static Color Orange;
		static Color Pink;
		static Color Teal;
		static Color Neon;
		static Color Purple;
		static Color Aqua;
		static Color White;
		static Color Grey;
		static Color Black;
		static Color Clear;
		static Color Blank;
		
	};

}

// DEPRECATED
#define APRIL_COLOR_RED april::Color::Red
#define APRIL_COLOR_GREEN april::Color::Green
#define APRIL_COLOR_BLUE april::Color::Blue
#define APRIL_COLOR_YELLOW april::Color::Yellow
#define APRIL_COLOR_MAGENTA april::Color::Magenta
#define APRIL_COLOR_CYAN april::Color::Cyan
#define APRIL_COLOR_ORANGE april::Color::Orange
#define APRIL_COLOR_PINK april::Color::Pink
#define APRIL_COLOR_TEAL april::Color::Teal
#define APRIL_COLOR_NEON april::Color::Neon
#define APRIL_COLOR_PURPLE april::Color::Purple
#define APRIL_COLOR_AQUA april::Color::Aqua
#define APRIL_COLOR_WHITE april::Color::White
#define APRIL_COLOR_GREY april::Color::Grey
#define APRIL_COLOR_BLACK april::Color::Black
#define APRIL_COLOR_CLEAR april::Color::Clear
#define APRIL_COLOR_BLANK april::Color::Blank
		
#endif
