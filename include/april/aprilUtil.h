/// @file
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @version 3.0
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
		RENDER_OP_UNDEFINED = 0x7FFFFFFF
	};

	enum InputLayout
	{
		PLAIN = 1,
		TEXTURED = 2,
		COLORED = 3,
		COLORED_TEXTURED = 4,
		INPUT_LAYOUT_UNDEFINED = 0x7FFFFFFF
	};
	
	enum BlendMode
	{
		DEFAULT = 0,
		ALPHA_BLEND = 1,
		ADD = 2,
		SUBTRACT = 3,
		OVERWRITE = 4,
		BLEND_MODE_UNDEFINED = 0x7FFFFFFF
	};

	enum ColorMode
	{
		NORMAL = 0,
		MULTIPLY = 1,
		LERP = 2,
		ALPHA_MAP = 3,
		COLOR_MODE_UNDEFINED = 0x7FFFFFFF
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
		PlainVertex() : gvec3(0.0f, 0.0f, 0.0f) { }
		PlainVertex(float x, float y, float z) : gvec3(x, y, z) { }
		void operator=(const gvec3& v);

	};

	struct aprilExport ColoredVertex : public PlainVertex
	{
	public:
		unsigned int color;

		ColoredVertex() : PlainVertex() { this->color = 0xFFFFFFFF; }
		ColoredVertex(float x, float y, float z) : PlainVertex(x, y, z) { this->color = 0xFFFFFFFF; }
		void operator=(const gvec3& v);

	};

	class aprilExport TexturedVertex : public PlainVertex
	{
	public:
		float u;
		float v;

		TexturedVertex() : PlainVertex() { this->u = 0.0f; this->v = 0.0f; }
		TexturedVertex(float x, float y, float z) : PlainVertex(x, y, z) { this->u = 0.0f; this->v = 0.0f; }
		void operator=(const gvec3& v);

	};

	class aprilExport ColoredTexturedVertex : public ColoredVertex
	{
	public:
		float u;
		float v;

		ColoredTexturedVertex() : ColoredVertex() { this->u = 0.0f; this->v = 0.0f; }
		ColoredTexturedVertex(float x, float y, float z) : ColoredVertex(x, y, z) { this->u = 0.0f; this->v = 0.0f; }
		void operator=(const gvec3& v);

	};
	
	class aprilExport ColoredTexturedNormalVertex : public ColoredTexturedVertex
	{
	public:
		gvec3 normal;

		ColoredTexturedNormalVertex() : ColoredTexturedVertex() { this->normal.set(0.0f, 0.0f, 0.0f); }
		ColoredTexturedNormalVertex(float x, float y, float z) : ColoredTexturedVertex(x, y, z) { this->normal.set(0.0f, 0.0f, 0.0f); }
		void operator=(const gvec3& v);

	};
	
	class aprilExport TexturedNormalVertex : public TexturedVertex
	{
	public:
		gvec3 normal;

		TexturedNormalVertex() : TexturedVertex() { this->normal.set(0.0f, 0.0f, 0.0f); }
		TexturedNormalVertex(float x, float y, float z) : TexturedVertex(x, y, z) { this->normal.set(0.0f, 0.0f, 0.0f); }
		void operator=(const gvec3& v);

	};
	
	class aprilExport ColoredNormalVertex : public ColoredVertex
	{
	public:
		gvec3 normal;

		ColoredNormalVertex() : ColoredVertex() { this->normal.set(0.0f, 0.0f, 0.0f); }
		ColoredNormalVertex(float x, float y, float z) : ColoredVertex(x, y, z) { this->normal.set(0.0f, 0.0f, 0.0f); }
		void operator=(const gvec3& v);

	};

	aprilFnExport void rgbToHsl(unsigned char r, unsigned char g, unsigned char b, float* h, float* s, float* l);
	aprilFnExport void hslToRgb(float h, float s, float l, unsigned char* r, unsigned char* g, unsigned char* b);

}

#endif
