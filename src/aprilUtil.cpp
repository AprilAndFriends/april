/// @file
/// @version 4.3
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include <gtypes/Rectangle.h>
#include <gtypes/Vector2.h>
#include <gtypes/Vector3.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hmap.h>

#include "aprilUtil.h"

#define HSTR_SEPARATOR ','

namespace april
{
	HL_ENUM_CLASS_DEFINE(RenderOperation,
	(
		HL_ENUM_DEFINE_VALUE(RenderOperation, TriangleList, 0);
		HL_ENUM_DEFINE_VALUE(RenderOperation, TriangleStrip, 1);
		HL_ENUM_DEFINE_VALUE(RenderOperation, LineList, 2);
		HL_ENUM_DEFINE_VALUE(RenderOperation, LineStrip, 3);
		HL_ENUM_DEFINE_VALUE(RenderOperation, PointList, 4);
		HL_ENUM_DEFINE_VALUE(RenderOperation, TriangleFan, 5);
	));

	HL_ENUM_CLASS_DEFINE(BlendMode,
	(
		HL_ENUM_DEFINE(BlendMode, Alpha);
		HL_ENUM_DEFINE(BlendMode, Add);
		HL_ENUM_DEFINE(BlendMode, Subtract);
		HL_ENUM_DEFINE(BlendMode, Overwrite);
	));

	HL_ENUM_CLASS_DEFINE(ColorMode,
	(
		HL_ENUM_DEFINE(ColorMode, Multiply);
		HL_ENUM_DEFINE(ColorMode, AlphaMap);
		HL_ENUM_DEFINE(ColorMode, Lerp);
	));

	void rgbToHsl(unsigned char r, unsigned char g, unsigned char b, float* h, float* s, float* l)
	{
		unsigned char min = (unsigned char)hmin(hmin(r, g), b);
		unsigned char max = (unsigned char)hmax(hmax(r, g), b);
		float delta = (float)(max - min);
		*h = *s = 0.0f;
		*l = ((int)max + (int)min) * 0.001960784f;
		if (delta > 0.0f)
		{
			if (*l > 0.0f && *l < 1.0f)
			{
				*s = (delta * 0.003921569f) / (*l < 0.5f ? (2.0f * *l) : (2.0f - 2.0f * *l));
			}
			if (max == r)
			{
				*h = ((int)g - (int)b) / delta;
				if (g < b)
				{
					*h += 6.0f;
				}
			}
			else if (max == g)
			{
				*h += ((int)b - (int)r) / delta + 2.0f;
			}
			else if (max == b)
			{
				*h += ((int)r - (int)g) / delta + 4.0f;
			}
			*h *= 0.16666667f;
		}
	}

	static inline float _colorHueToRgb(float p, float q, float h)
	{ 
		h = (h < 0.0f ? h + 1.0f : ((h > 1.0f) ? h - 1.0f : h));
		if (h * 6.0f < 1.0f)
		{
			return p + (q - p) * h * 6.0f;
		}
		if (h * 2.0f < 1.0f)
		{
			return q;
		}
		if (h * 3.0f < 2.0f)
		{
			return (p + (q - p) * (0.6666667f - h) * 6.0f);
		}
		return p;
	}

	void hslToRgb(float h, float s, float l, unsigned char* r, unsigned char* g, unsigned char* b)
	{
		if (s == 0.0f)
		{
			*r = *g = *b = (unsigned char)(l * 255);
			return;
		}
		float q = (l < 0.5f ? l * (1.0f + s) : l + s - l * s);
		float p = l * 2.0f - q;
		*r = (unsigned char)hround(255.0f * _colorHueToRgb(p, q, h + 0.3333333f));
		*g = (unsigned char)hround(255.0f * _colorHueToRgb(p, q, h));
		*b = (unsigned char)hround(255.0f * _colorHueToRgb(p, q, h - 0.3333333f));
	}
	
	hstr generateName(chstr prefix)
	{
		if (prefix != "")
		{
			std::ustring characters = prefix.uStr();
			if (hbetweenII(characters[(int)characters.size() - 1], (unsigned int)'0', (unsigned int)'9'))
			{
				throw Exception("Called april::generateName() with an illegal string, cannot end with a number character: " + prefix);
			}
		}
		static hmap<hstr, int> counters;
		int count = counters[prefix] + 1;
		counters[prefix] = count;
		return prefix.replaced(".", "_") + hstr(count);
	}

	hstr gvec2ToHstr(gvec2 vector)
	{
		return hsprintf("%f%c%f", vector.x, HSTR_SEPARATOR, vector.y);
	}

	hstr gvec3ToHstr(gvec3 vector)
	{
		return hsprintf("%f%c%f%c%f", vector.x, HSTR_SEPARATOR, vector.y, HSTR_SEPARATOR, vector.z);
	}

	hstr grectToHstr(grect rect)
	{
		return hsprintf("%f%c%f%c%f%c%f", rect.x, HSTR_SEPARATOR, rect.y, HSTR_SEPARATOR, rect.w, HSTR_SEPARATOR, rect.h);
	}

	gvec2 hstrToGvec2(chstr string)
	{
		harray<hstr> data = string.split(HSTR_SEPARATOR);
		if (data.size() != 2)
		{
			throw Exception("Cannot convert string '" + string + "' to gtypes::Vector2.");
		}
		return gvec2(data[0].trimmed(), data[1].trimmed());
	}

	gvec3 hstrToGvec3(chstr string)
	{
		harray<hstr> data = string.split(HSTR_SEPARATOR);
		if (data.size() != 3)
		{
			throw Exception("Cannot convert string '" + string + "' to gtypes::Vector3.");
		}
		return gvec3(data[0].trimmed(), data[1].trimmed(), data[2].trimmed());
	}

	grect hstrToGrect(chstr string)
	{
		harray<hstr> data = string.split(HSTR_SEPARATOR);
		if (data.size() != 4)
		{
			throw Exception("Cannot convert string '" + string + "' to gtypes::Rectangle.");
		}
		return grect(data[0].trimmed(), data[1].trimmed(), data[2].trimmed(), data[3].trimmed());
	}

}
