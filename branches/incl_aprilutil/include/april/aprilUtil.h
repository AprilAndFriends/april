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
/// Defines utility enums and structs.

#ifndef APRIL_UTIL_H
#define APRIL_UTIL_H

#include <gtypes/Vector3.h>

#include "aprilExport.h"

namespace april
{
	// render operations
	enum RenderOp
	{
		TriangleList = 1,
		TriangleStrip = 2,
		TriangleFan = 3,
		LineList = 4,
		LineStrip = 5,
		PointList = 6,
	};
	
	enum BlendMode
	{
		DEFAULT = 0,
		ALPHA_BLEND = 1,
		ADD = 2,
		SUBTRACT = 3,
		OVERWRITE = 4
	};

	enum ColorMode
	{
		NORMAL = 0,
		MULTIPLY = 1,
		LERP = 2,
		ALPHA_MAP = 3
	};

	struct aprilExport DisplayMode
	{
		int width;
		int height;
		int refreshRate;

		bool operator==(const DisplayMode& other);
		bool operator!=(const DisplayMode& other);

	};
	
	struct aprilExport PlainVertex : public gvec3
	{
	public:
		void operator=(const gvec3& v);

	};

	struct aprilExport ColoredVertex : public PlainVertex
	{
	public:
		unsigned int color;

		void operator=(const gvec3& v);

	};

	class aprilExport TexturedVertex : public PlainVertex
	{
	public:
		float u;
		float v;

		void operator=(const gvec3& v);

	};

	class aprilExport ColoredTexturedVertex : public ColoredVertex
	{
	public:
		float u;
		float v;

		void operator=(const gvec3& v);

	};
	
	class aprilExport ColoredTexturedNormalVertex : public ColoredTexturedVertex
	{
	public:
		gvec3 normal;

		void operator=(const gvec3& v);

	};
	
	class aprilExport TexturedNormalVertex : public TexturedVertex
	{
	public:
		gvec3 normal;

		void operator=(const gvec3& v);

	};
	
	class aprilExport ColoredNormalVertex : public ColoredVertex
	{
	public:
		gvec3 normal;

		void operator=(const gvec3& v);

	};

	aprilFnExport void rgbToHsl(unsigned char r, unsigned char g, unsigned char b, float* h, float* s, float* l);
	aprilFnExport void hslToRgb(float h, float s, float l, unsigned char* r, unsigned char* g, unsigned char* b);

}

#endif
