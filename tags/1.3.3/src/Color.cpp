/// @file
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @author  Ivan Vucica
/// @version 1.31
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php
#include <stdio.h>

#include <hltypes/hstring.h>
#include <hltypes/util.h>

#include "Color.h"

namespace april
{
	int hexstr_to_int(chstr s)
	{
		int i;
		sscanf(s.c_str(), "%x", &i);
		return i;
	}
	
	Color::Color()
	{
		this->r = 255;
		this->g = 255;
		this->b = 255;
		this->a = 255;
	}

	Color::Color(int r, int g, int b, int a)
	{
		this->set(r, g, b, a);
	}

	Color::Color(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
	{
		this->set(r, g, b, a);
	}

	Color::Color(unsigned int color)
	{
		this->set(color);
	}

	Color::Color(chstr hex)
	{
		this->set(hex);
	}

	Color::Color(Color color, unsigned char a)
	{
		this->set(color, a);
	}

	void Color::set(int r, int g, int b, int a)
	{
		this->r = (unsigned char)r;
		this->g = (unsigned char)g;
		this->b = (unsigned char)b;
		this->a = (unsigned char)a;
	}

	void Color::set(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
	{
		this->r = r;
		this->g = g;
		this->b = b;
		this->a = a;
	}

	void Color::set(unsigned int color)
	{
		this->r = (unsigned char)((color >> 24) & 0xFF);
		this->g = (unsigned char)((color >> 16) & 0xFF);
		this->b = (unsigned char)((color >> 8) & 0xFF);
		this->a = (unsigned char)(color & 0xFF);
	}

	void Color::set(chstr hex)
	{
		hstr value = hex;
		if (value(0, 2) != "0x")
		{
			value = "0x" + value;
		}
		if (value.size() != 8 && value.size() != 10)
		{
			throw "Color format must be either 0xRRGGBBAA or 0xRRGGBB";
		}
		this->r = (unsigned char)hexstr_to_int(value(2, 2));
		this->g = (unsigned char)hexstr_to_int(value(4, 2));
		this->b = (unsigned char)hexstr_to_int(value(6, 2));
		this->a = (value.size() == 10 ? (unsigned char)hexstr_to_int(value(8, 2)) : 255);
	}
	
	void Color::set(Color color, unsigned char a)
	{
		this->r = color.r;
		this->g = color.g;
		this->b = color.b;
		this->a = a;
	}

	hstr Color::hex(bool rgbOnly)
	{
		hstr result = hsprintf("%02X%02X%02X", this->r, this->g, this->b);
		if (!rgbOnly)
		{
			result += hsprintf("%02X", this->a);
		}
		return result;
	}

	Color::operator unsigned int() const
	{
		unsigned int i = 0;
		i |= this->r << 24;
		i |= this->g << 16;
		i |= this->b << 8;
		i |= this->a;
		return i;
	}
	
	bool Color::operator==(Color& other)
	{
		return (this->r == other.r && this->g == other.g && this->b == other.b && this->a == other.a);
	}

	bool Color::operator!=(Color& other)
	{
		return !(*this == other);
	}
	
	Color Color::operator+(Color& other)
	{
		Color result(*this);
		result += other;
		return result;
	}

	Color Color::operator-(Color& other)
	{
		Color result(*this);
		result -= other;
		return result;
	}

	Color Color::operator*(Color& other)
	{
		Color result(*this);
		result *= other;
		return result;
	}

	Color Color::operator/(Color& other)
	{
		Color result(*this);
		result /= other;
		return result;
	}

	Color Color::operator*(float value)
	{
		Color result(*this);
		result *= value;
		return result;
	}

	Color Color::operator/(float value)
	{
		Color result(*this);
		result /= value;
		return result;
	}

	Color Color::operator+=(Color& other)
	{
		this->r = hclamp(this->r + other.r, 0, 255);
		this->g = hclamp(this->g + other.g, 0, 255);
		this->b = hclamp(this->b + other.b, 0, 255);
		this->a = hclamp(this->a + other.a, 0, 255);
		return (*this);
	}

	Color Color::operator-=(Color& other)
	{
		this->r = hclamp(this->r - other.r, 0, 255);
		this->g = hclamp(this->g - other.g, 0, 255);
		this->b = hclamp(this->b - other.b, 0, 255);
		this->a = hclamp(this->a - other.a, 0, 255);
		return (*this);
	}

	Color Color::operator*=(Color& other)
	{
		this->r = hclamp((int)(this->r_f() * other.r), 0, 255);
		this->g = hclamp((int)(this->g_f() * other.g), 0, 255);
		this->b = hclamp((int)(this->b_f() * other.b), 0, 255);
		this->a = hclamp((int)(this->a_f() * other.a), 0, 255);
		return (*this);
	}

	Color Color::operator/=(Color& other)
	{
		this->r = hclamp((int)(this->r_f() / other.r), 0, 255);
		this->g = hclamp((int)(this->g_f() / other.g), 0, 255);
		this->b = hclamp((int)(this->b_f() / other.b), 0, 255);
		this->a = hclamp((int)(this->a_f() / other.a), 0, 255);
		return (*this);
	}

	Color Color::operator*=(float value)
	{
		this->r = hclamp((int)(this->r * value), 0, 255);
		this->g = hclamp((int)(this->g * value), 0, 255);
		this->b = hclamp((int)(this->b * value), 0, 255);
		this->a = hclamp((int)(this->a * value), 0, 255);
		return (*this);
	}

	Color Color::operator/=(float value)
	{
		this->r = hclamp((int)(this->r / value), 0, 255);
		this->g = hclamp((int)(this->g / value), 0, 255);
		this->b = hclamp((int)(this->b / value), 0, 255);
		this->a = hclamp((int)(this->a / value), 0, 255);
		return (*this);
	}
	
}
