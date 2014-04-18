/// @file
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @version 3.3
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Defines utility enums and structs.

#ifndef APRIL_UTIL_H
#define APRIL_UTIL_H

#include <gtypes/Vector3.h>
#include <hltypes/hltypesUtil.h>

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
		HL_INLINE PlainVertex() : gvec3(0.0f, 0.0f, 0.0f) { }
		HL_INLINE PlainVertex(float x, float y, float z) : gvec3(x, y, z) { }
		void operator=(const gvec3& v);

	};

	struct aprilExport ColoredVertex : public PlainVertex
	{
	public:
		unsigned int color;

		HL_INLINE ColoredVertex() : PlainVertex() { this->color = 0xFFFFFFFF; }
		HL_INLINE ColoredVertex(float x, float y, float z) : PlainVertex(x, y, z) { this->color = 0xFFFFFFFF; }
		HL_INLINE ColoredVertex(unsigned int color) : PlainVertex() { this->color = color; }
		HL_INLINE ColoredVertex(float x, float y, float z, unsigned int color) : PlainVertex(x, y, z) { this->color = color; }
		void operator=(const gvec3& v);

	};

	class aprilExport TexturedVertex : public PlainVertex
	{
	public:
		float u;
		float v;

		HL_INLINE TexturedVertex() : PlainVertex() { this->u = 0.0f; this->v = 0.0f; }
		HL_INLINE TexturedVertex(float x, float y, float z) : PlainVertex(x, y, z) { this->u = 0.0f; this->v = 0.0f; }
		HL_INLINE TexturedVertex(float u, float v) : PlainVertex() { this->u = u; this->v = v; }
		HL_INLINE TexturedVertex(float x, float y, float z, float u, float v) : PlainVertex(x, y, z) { this->u = u; this->v = v; }
		void operator=(const gvec3& v);

	};

	class aprilExport ColoredTexturedVertex : public ColoredVertex
	{
	public:
		float u;
		float v;

		HL_INLINE ColoredTexturedVertex() : ColoredVertex() { this->u = 0.0f; this->v = 0.0f; }
		HL_INLINE ColoredTexturedVertex(float x, float y, float z) : ColoredVertex(x, y, z) { this->u = 0.0f; this->v = 0.0f; }
		HL_INLINE ColoredTexturedVertex(float x, float y, float z, unsigned int color) : ColoredVertex(x, y, z, color) { this->u = 0.0f; this->v = 0.0f; }
		HL_INLINE ColoredTexturedVertex(float u, float v) : ColoredVertex() { this->u = u; this->v = v; }
		HL_INLINE ColoredTexturedVertex(float x, float y, float z, float u, float v) : ColoredVertex(x, y, z) { this->u = u; this->v = v; }
		HL_INLINE ColoredTexturedVertex(float x, float y, float z, unsigned int color, float u, float v) : ColoredVertex(x, y, z, color) { this->u = u; this->v = v; }
		void operator=(const gvec3& v);

	};
	
	class aprilExport ColoredTexturedNormalVertex : public ColoredTexturedVertex
	{
	public:
		gvec3 normal;

		HL_INLINE ColoredTexturedNormalVertex() : ColoredTexturedVertex() { this->normal.set(0.0f, 0.0f, 0.0f); }
		HL_INLINE ColoredTexturedNormalVertex(float x, float y, float z) : ColoredTexturedVertex(x, y, z) { this->normal.set(0.0f, 0.0f, 0.0f); }
		HL_INLINE ColoredTexturedNormalVertex(float x, float y, float z, float u, float v) : ColoredTexturedVertex(x, y, z, u, v) { this->normal.set(0.0f, 0.0f, 0.0f); }
		HL_INLINE ColoredTexturedNormalVertex(float x, float y, float z, unsigned int color, float u, float v) : ColoredTexturedVertex(x, y, z, color, u, v) { this->normal.set(0.0f, 0.0f, 0.0f); }
		HL_INLINE ColoredTexturedNormalVertex(gvec3 normal) : ColoredTexturedVertex(x, y, z) { this->normal = normal; }
		HL_INLINE ColoredTexturedNormalVertex(float x, float y, float z, gvec3 normal) : ColoredTexturedVertex(x, y, z) { this->normal = normal; }
		HL_INLINE ColoredTexturedNormalVertex(float x, float y, float z, float u, float v, gvec3 normal) : ColoredTexturedVertex(x, y, z, u, v) { this->normal = normal; }
		HL_INLINE ColoredTexturedNormalVertex(float x, float y, float z, unsigned int color, float u, float v, gvec3 normal) : ColoredTexturedVertex(x, y, z, color, u, v) { this->normal = normal; }
		void operator=(const gvec3& v);

	};
	
	class aprilExport TexturedNormalVertex : public TexturedVertex
	{
	public:
		gvec3 normal;

		HL_INLINE TexturedNormalVertex() : TexturedVertex() { this->normal.set(0.0f, 0.0f, 0.0f); }
		HL_INLINE TexturedNormalVertex(float x, float y, float z) : TexturedVertex(x, y, z) { this->normal.set(0.0f, 0.0f, 0.0f); }
		HL_INLINE TexturedNormalVertex(float x, float y, float z, float u, float v) : TexturedVertex(x, y, z, u, v) { this->normal.set(0.0f, 0.0f, 0.0f); }
		HL_INLINE TexturedNormalVertex(gvec3 normal) : TexturedVertex() { this->normal = normal; }
		HL_INLINE TexturedNormalVertex(float x, float y, float z, gvec3 normal) : TexturedVertex(x, y, z) { this->normal = normal; }
		HL_INLINE TexturedNormalVertex(float x, float y, float z, float u, float v, gvec3 normal) : TexturedVertex(x, y, z, u, v) { this->normal = normal; }
		void operator=(const gvec3& v);

	};
	
	class aprilExport ColoredNormalVertex : public ColoredVertex
	{
	public:
		gvec3 normal;

		HL_INLINE ColoredNormalVertex() : ColoredVertex() { this->normal.set(0.0f, 0.0f, 0.0f); }
		HL_INLINE ColoredNormalVertex(float x, float y, float z) : ColoredVertex(x, y, z) { this->normal.set(0.0f, 0.0f, 0.0f); }
		HL_INLINE ColoredNormalVertex(gvec3 normal) : ColoredVertex(x, y, z) { this->normal = normal; }
		HL_INLINE ColoredNormalVertex(float x, float y, float z, gvec3 normal) : ColoredVertex(x, y, z) { this->normal = normal; }
		void operator=(const gvec3& v);

	};

	aprilFnExport void rgbToHsl(unsigned char r, unsigned char g, unsigned char b, float* h, float* s, float* l);
	aprilFnExport void hslToRgb(float h, float s, float l, unsigned char* r, unsigned char* g, unsigned char* b);

}

#endif
