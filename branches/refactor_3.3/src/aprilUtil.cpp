/// @file
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @version 3.3
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include <gtypes/Vector3.h>
#include <hltypes/hltypesUtil.h>

#include "aprilUtil.h"

namespace april
{
	DisplayMode::DisplayMode(int width, int height, int refreshRate)
	{
		this->width = width;
		this->height = height;
		this->refreshRate = refreshRate;
	}

	bool DisplayMode::operator==(const DisplayMode& other) const
	{
		return (this->width == other.width && this->height == other.height && this->refreshRate == other.refreshRate);
	}

	bool DisplayMode::operator!=(const DisplayMode& other) const
	{
		return !(*this == other);
	}
	
	void PlainVertex::operator=(const gvec3& v)
	{
		this->x = v.x;
		this->y = v.y;
		this->z = v.z;
	}

	void ColoredVertex::operator=(const gvec3& v)
	{
		this->x = v.x;
		this->y = v.y;
		this->z = v.z;
	}

	void TexturedVertex::operator=(const gvec3& v)
	{
		this->x = v.x;
		this->y = v.y;
		this->z = v.z;
	}

	void ColoredTexturedVertex::operator=(const gvec3& v)
	{
		this->x = v.x;
		this->y = v.y;
		this->z = v.z;
	}

	void ColoredTexturedNormalVertex::operator=(const gvec3& v)
	{
		this->x = v.x;
		this->y = v.y;
		this->z = v.z;
	}

	void TexturedNormalVertex::operator=(const gvec3& v)
	{
		this->x = v.x;
		this->y = v.y;
		this->z = v.z;
	}

	void ColoredNormalVertex::operator=(const gvec3& v)
	{
		this->x = v.x;
		this->y = v.y;
		this->z = v.z;
	}

	void rgbToHsl(unsigned char r, unsigned char g, unsigned char b, float* h, float* s, float* l)
	{
		int min = hmin(hmin(r, g), b);
		int max = hmax(hmax(r, g), b);
		int delta = max - min;
		*l = (max + min) / 510.0f;
		*s = 0.0f;
		if (*l > 0.0f && *l < 1.0f)
		{
			*s = (delta / 255.0f) / (*l < 0.5f ? (2 * *l) : (2 - 2 * *l));
		}
		*h = 0.0f;
		if (delta > 0)
		{
			if (max == r)
			{
				*h += (g - b) / (float)delta;
			}
			if (max == g)
			{
				*h += 2 + (b - r) / (float)delta;
			}
			if (max == b)
			{
				*h += 4 + (r - g) / (float)delta;
			}
			*h /= 6;
		}
	}

	float _colorHueToRgb(float m1, float m2, float h)
	{ 
		h = (h < 0 ? h + 1 : ((h > 1) ? h - 1 : h));
		if (h * 6 < 1)
		{
			return m1 + (m2 - m1) * h * 6;
		}
		if (h * 2 < 1)
		{
			return m2;
		}
		if (h * 3 < 2)
		{
			return (m1 + (m2 - m1) * (0.6666667f - h) * 6);
		}
		return m1;
	}

	void hslToRgb(float h, float s, float l, unsigned char* r, unsigned char* g, unsigned char* b)
	{
		float m2 = (l <= 0.5f) ? l * (s + 1) : l + s - l * s;
		float m1 = l * 2 - m2;
		*r = (unsigned char)hround(255.0f * _colorHueToRgb(m1, m2, h + 0.3333333f));
		*g = (unsigned char)hround(255.0f * _colorHueToRgb(m1, m2, h));
		*b = (unsigned char)hround(255.0f * _colorHueToRgb(m1, m2, h - 0.3333333f));
	}

}
