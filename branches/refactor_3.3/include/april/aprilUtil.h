/// @file
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @version 3.3
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
	enum RenderOperation
	{
		RO_TRIANGLE_LIST = 1,
		RO_TRIANGLE_STRIP = 2,
		RO_TRIANGLE_FAN = 3,
		RO_LINE_LIST = 4,
		RO_LINE_STRIP = 5,
		RO_POINT_LIST = 6,
		RO_UNDEFINED = 0x7FFFFFFF
	};

	enum BlendMode
	{
		BM_DEFAULT = 0,
		BM_ALPHA = 1,
		BM_ADD = 2,
		BM_SUBTRACT = 3,
		BM_OVERWRITE = 4,
		BM_UNDEFINED = 0x7FFFFFFF
	};

	enum ColorMode
	{
		CM_DEFAULT = 0,
		CM_MULTIPLY = 1,
		CM_LERP = 2,
		CM_ALPHA_MAP = 3,
		CM_UNDEFINED = 0x7FFFFFFF
	};

	DEPRECATED_ATTRIBUTE extern aprilExport RenderOperation TriangleList;
	DEPRECATED_ATTRIBUTE extern aprilExport RenderOperation TriangleStrip;
	DEPRECATED_ATTRIBUTE extern aprilExport RenderOperation TriangleFan;
	DEPRECATED_ATTRIBUTE extern aprilExport RenderOperation LineList;
	DEPRECATED_ATTRIBUTE extern aprilExport RenderOperation LineStrip;
	DEPRECATED_ATTRIBUTE extern aprilExport RenderOperation PointList;
	DEPRECATED_ATTRIBUTE extern aprilExport RenderOperation RENDER_OP_UNDEFINED;
	DEPRECATED_ATTRIBUTE extern aprilExport BlendMode DEFAULT;
	DEPRECATED_ATTRIBUTE extern aprilExport BlendMode ALPHA_BLEND;
	DEPRECATED_ATTRIBUTE extern aprilExport BlendMode ADD;
	DEPRECATED_ATTRIBUTE extern aprilExport BlendMode SUBTRACT;
	DEPRECATED_ATTRIBUTE extern aprilExport BlendMode OVERWRITE;
	DEPRECATED_ATTRIBUTE extern aprilExport BlendMode BLEND_MODE_UNDEFINED;
	DEPRECATED_ATTRIBUTE extern aprilExport ColorMode NORMAL;
	DEPRECATED_ATTRIBUTE extern aprilExport ColorMode MULTIPLY;
	DEPRECATED_ATTRIBUTE extern aprilExport ColorMode LERP;
	DEPRECATED_ATTRIBUTE extern aprilExport ColorMode ALPHA_MAP;
	DEPRECATED_ATTRIBUTE extern aprilExport ColorMode COLOR_MODE_UNDEFINED;
	
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
