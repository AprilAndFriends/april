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
		*h = *s = 0.0f;
		*l = (max + min) / 510.0f;
		if (delta > 0)
		{
			if (*l > 0.0f && *l < 1.0f)
			{
				*s = (delta / 255.0f) / (*l < 0.5f ? (2 * *l) : (2 - 2 * *l));
			}
			if (max == r)
			{
				*h = (g - b) / (float)delta;
				if (g < b)
				{
					*h += 6.0f;
				}
			}
			else if (max == g)
			{
				*h += (b - r) / (float)delta + 2.0f;
			}
			else if (max == b)
			{
				*h += (r - g) / (float)delta + 4.0f;
			}
			*h *= 0.16666667f;
		}
	}

	float _colorHueToRgb(float p, float q, float h)
	{ 
		h = (h < 0 ? h + 1 : ((h > 1) ? h - 1 : h));
		if (h * 6 < 1)
		{
			return p + (q - p) * h * 6;
		}
		if (h * 2 < 1)
		{
			return q;
		}
		if (h * 3 < 2)
		{
			return (p + (q - p) * (0.6666667f - h) * 6);
		}
		return p;
	}

	void hslToRgb(float h, float s, float l, unsigned char* r, unsigned char* g, unsigned char* b)
	{
		if (s == 0.0f)
		{
			*r = *g = *b = 255;
			return;
		}
		float q = (l < 0.5f ? l * (1 + s) : l + s - l * s);
		float p = l * 2 - q;
		*r = (unsigned char)hround(255.0f * _colorHueToRgb(p, q, h + 0.3333333f));
		*g = (unsigned char)hround(255.0f * _colorHueToRgb(p, q, h));
		*b = (unsigned char)hround(255.0f * _colorHueToRgb(p, q, h - 0.3333333f));
	}

}
