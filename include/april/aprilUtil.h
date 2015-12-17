/// @file
/// @version 4.0
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

#include <gtypes/Rectangle.h>
#include <gtypes/Vector2.h>
#include <gtypes/Vector3.h>
#include <hltypes/hstring.h>

#include "aprilExport.h"

namespace april
{
	enum RenderOperation
	{
		/// @brief Undefined blend mode.
		RO_UNDEFINED = 0,
		/// @brief Triangle-list.
		RO_TRIANGLE_LIST = 1,
		/// @brief Triangle-strip.
		RO_TRIANGLE_STRIP = 2,
		/// @brief Triangle-fan.
		RO_TRIANGLE_FAN = 3,
		/// @brief Line-list.
		RO_LINE_LIST = 4,
		/// @brief Line-strip.
		RO_LINE_STRIP = 5,
		/// @brief Point-list.
		RO_POINT_LIST = 6,
	};

	enum BlendMode
	{
		/// @brief Default blend mode.
		BM_DEFAULT = 0,
		/// @brief Alpha blending.
		BM_ALPHA = 1,
		/// @brief Additive blending.
		BM_ADD = 2,
		/// @brief Subtractive blending.
		BM_SUBTRACT = 3,
		/// @brief Overwrite data.
		BM_OVERWRITE = 4,
	};

	enum ColorMode
	{
		/// @brief Default color mode.
		CM_DEFAULT = 0,
		/// @brief Multiply.
		CM_MULTIPLY = 1,
		/// @brief Alpha Map.
		CM_ALPHA_MAP = 2,
		/// @brief Linear Interpolation.
		CM_LERP = 3,
	};

	/// @brief Represents a plain vertex.
	class aprilExport PlainVertex
	{
	public:
		/// @brief The X-coordinate.
		float x;
		/// @brief The Y-coordinate.
		float y;
		/// @brief The Z-coordinate.
		float z;

		/// @brief Basic constructor.
		inline PlainVertex() :							x(0.0f), y(0.0f), z(0.0f) { }
		/// @brief Constructor.
		/// @param[in] x X-coordinate.
		/// @param[in] y Y-coordinate.
		/// @param[in] z Z-coordinate.
		inline PlainVertex(float x, float y, float z) :	x(x), y(y), z(z) { }
		/// @brief Constructor.
		/// @param[in] position Position of the vertex.
		inline PlainVertex(gvec3 position) :			x(position.x), y(position.y), z(position.z) { }

		/// @brief Assigns a position to this vertex.
		/// @param[in] x X-coordinate.
		/// @param[in] y Y-coordinate.
		/// @param[in] z Z-coordinate.
		inline void set(float x, float y, float z) { this->x = x; this->y = y; this->z = z; }
		/// @brief Assigns a gtypes::Vector3 position to this vertex.
		/// @param[in] position Position of the vector.
		inline void set(const gvec3& position) { this->x = position.x; this->y = position.y; this->z = position.z; }

	};

	/// @brief Represents a vertex with a color component.
	class aprilExport ColoredVertex : public PlainVertex
	{
	public:
		/// @brief The vertex color.
		/// @note This value is in native format, not RGBA MSB!
		unsigned int color;

		/// @brief Basic constructor.
		inline ColoredVertex() :												PlainVertex(), color(0xFFFFFFFF) { }
		/// @brief Constructor.
		/// @param[in] x X-coordinate.
		/// @param[in] y Y-coordinate.
		/// @param[in] z Z-coordinate.
		inline ColoredVertex(float x, float y, float z) :						PlainVertex(x, y, z), color(0xFFFFFFFF) { }
		/// @brief Constructor.
		/// @param[in] position Position of the vertex.
		inline ColoredVertex(gvec3 position) :									PlainVertex(position), color(0xFFFFFFFF) { }
		/// @brief Constructor.
		/// @param[in] color Color of the vertex.
		/// @note The 'color' value is in native format, not RGBA MSB!)
		inline ColoredVertex(unsigned int color) :								PlainVertex(), color(color) { }
		/// @brief Constructor.
		/// @param[in] x X-coordinate.
		/// @param[in] y Y-coordinate.
		/// @param[in] z Z-coordinate.
		/// @param[in] color Color of the vertex.
		/// @note The 'color' value is in native format, not RGBA MSB!)
		inline ColoredVertex(float x, float y, float z, unsigned int color) :	PlainVertex(x, y, z), color(color) { }
		/// @brief Constructor.
		/// @param[in] position Position of the vertex.
		/// @param[in] color Color of the vertex.
		/// @note The 'color' value is in native format, not RGBA MSB!)
		inline ColoredVertex(gvec3 position, unsigned int color) :				PlainVertex(position), color(color) { }

	};

	/// @brief Represents a vertex with a UV-coordinate component.
	class aprilExport TexturedVertex : public PlainVertex
	{
	public:
		/// @brief The texture U-coordinate.
		float u;
		/// @brief The texture V-coordinate.
		float v;

		/// @brief Basic constructor.
		inline TexturedVertex() :													PlainVertex(), u(0.0f), v(0.0f) { }
		/// @brief Constructor.
		/// @param[in] x X-coordinate.
		/// @param[in] y Y-coordinate.
		/// @param[in] z Z-coordinate.
		inline TexturedVertex(float x, float y, float z) :							PlainVertex(x, y, z), u(0.0f), v(0.0f) { }
		/// @brief Constructor.
		/// @param[in] position Position of the vertex.
		inline TexturedVertex(gvec3 position) :										PlainVertex(position), u(0.0f), v(0.0f) { }
		/// @brief Constructor.
		/// @param[in] u Texture U-coordinate.
		/// @param[in] v Texture V-coordinate.
		inline TexturedVertex(float u, float v) :									PlainVertex(), u(u), v(v) { }
		/// @brief Constructor.
		/// @param[in] textureCoordinate Texture coordinate of the vertex.
		inline TexturedVertex(gvec2 textureCoordinate) :							PlainVertex(), u(textureCoordinate.x), v(textureCoordinate.y) { }
		/// @brief Constructor.
		/// @param[in] x X-coordinate.
		/// @param[in] y Y-coordinate.
		/// @param[in] z Z-coordinate.
		/// @param[in] u Texture U-coordinate.
		/// @param[in] v Texture V-coordinate.
		inline TexturedVertex(float x, float y, float z, float u, float v) :		PlainVertex(x, y, z), u(u), v(v) { }
		/// @brief Constructor.
		/// @param[in] x X-coordinate.
		/// @param[in] y Y-coordinate.
		/// @param[in] z Z-coordinate.
		/// @param[in] textureCoordinate Texture coordinate of the vertex.
		inline TexturedVertex(float x, float y, float z, gvec2 textureCoordinate) :	PlainVertex(x, y, z), u(textureCoordinate.x), v(textureCoordinate.y) { }
		/// @brief Constructor.
		/// @param[in] position Position of the vertex.
		/// @param[in] u Texture U-coordinate.
		/// @param[in] v Texture V-coordinate.
		inline TexturedVertex(gvec3 position, float u, float v) :					PlainVertex(position), u(u), v(v) { }
		/// @brief Constructor.
		/// @param[in] position Position of the vertex.
		/// @param[in] textureCoordinate Texture coordinate of the vertex.
		inline TexturedVertex(gvec3 position, gvec2 textureCoordinate) :			PlainVertex(position), u(textureCoordinate.x), v(textureCoordinate.y) { }

	};

	/// @brief Represents a vertex with a UV-coordinate component and a color component.
	class aprilExport ColoredTexturedVertex : public ColoredVertex
	{
	public:
		/// @brief The texture U-coordinate.
		float u;
		/// @brief The texture V-coordinate.
		float v;

		/// @brief Basic constructor.
		inline ColoredTexturedVertex() :																		ColoredVertex(), u(0.0f), v(0.0f) { }
		/// @brief Constructor.
		/// @param[in] x X-coordinate.
		/// @param[in] y Y-coordinate.
		/// @param[in] z Z-coordinate.
		inline ColoredTexturedVertex(float x, float y, float z) :												ColoredVertex(x, y, z), u(0.0f), v(0.0f) { }
		/// @brief Constructor.
		/// @param[in] position Position of the vertex.
		inline ColoredTexturedVertex(gvec3 position) :															ColoredVertex(position), u(0.0f), v(0.0f) { }
		/// @brief Constructor.
		/// @param[in] color Color of the vertex.
		/// @note The 'color' value is in native format, not RGBA MSB!)
		inline ColoredTexturedVertex(unsigned int color) :														ColoredVertex(color), u(0.0f), v(0.0f) { }
		/// @brief Constructor.
		/// @param[in] u Texture U-coordinate.
		/// @param[in] v Texture V-coordinate.
		inline ColoredTexturedVertex(float u, float v) :														ColoredVertex(), u(u), v(v) { }
		/// @brief Constructor.
		/// @param[in] textureCoordinate Texture coordinate of the vertex.
		inline ColoredTexturedVertex(gvec2 textureCoordiate) :													ColoredVertex(), u(textureCoordiate.x), v(textureCoordiate.y) { }
		/// @brief Constructor.
		/// @param[in] x X-coordinate.
		/// @param[in] y Y-coordinate.
		/// @param[in] z Z-coordinate.
		/// @param[in] color Color of the vertex.
		/// @note The 'color' value is in native format, not RGBA MSB!)
		inline ColoredTexturedVertex(float x, float y, float z, unsigned int color) :							ColoredVertex(x, y, z, color), u(0.0f), v(0.0f) { }
		/// @brief Constructor.
		/// @param[in] position Position of the vertex.
		/// @param[in] color Color of the vertex.
		/// @note The 'color' value is in native format, not RGBA MSB!)
		inline ColoredTexturedVertex(gvec3 position, unsigned int color) :										ColoredVertex(position, color), u(0.0f), v(0.0f) { }
		/// @brief Constructor.
		/// @param[in] x X-coordinate.
		/// @param[in] y Y-coordinate.
		/// @param[in] z Z-coordinate.
		/// @param[in] u Texture U-coordinate.
		/// @param[in] v Texture V-coordinate.
		inline ColoredTexturedVertex(float x, float y, float z, float u, float v) :								ColoredVertex(x, y, z), u(u), v(v) { }
		/// @brief Constructor.
		/// @param[in] position Position of the vertex.
		/// @param[in] u Texture U-coordinate.
		/// @param[in] v Texture V-coordinate.
		inline ColoredTexturedVertex(gvec3 position, float u, float v) :										ColoredVertex(position), u(u), v(v) { }
		/// @brief Constructor.
		/// @param[in] x X-coordinate.
		/// @param[in] y Y-coordinate.
		/// @param[in] z Z-coordinate.
		/// @param[in] textureCoordinate Texture coordinate of the vertex.
		inline ColoredTexturedVertex(float x, float y, float z, gvec2 textureCoordiate) :						ColoredVertex(x, y, z), u(textureCoordiate.x), v(textureCoordiate.y) { }
		/// @brief Constructor.
		/// @param[in] position Position of the vertex.
		/// @param[in] textureCoordinate Texture coordinate of the vertex.
		inline ColoredTexturedVertex(gvec3 position, gvec2 textureCoordiate) :									ColoredVertex(position), u(textureCoordiate.x), v(textureCoordiate.y) { }
		/// @brief Constructor.
		/// @param[in] x X-coordinate.
		/// @param[in] y Y-coordinate.
		/// @param[in] z Z-coordinate.
		/// @param[in] color Color of the vertex.
		/// @param[in] u Texture U-coordinate.
		/// @param[in] v Texture V-coordinate.
		/// @note The 'color' value is in native format, not RGBA MSB!)
		inline ColoredTexturedVertex(float x, float y, float z, unsigned int color, float u, float v) :			ColoredVertex(x, y, z, color), u(u), v(v) { }
		/// @brief Constructor.
		/// @param[in] position Position of the vertex.
		/// @param[in] color Color of the vertex.
		/// @param[in] u Texture U-coordinate.
		/// @param[in] v Texture V-coordinate.
		/// @note The 'color' value is in native format, not RGBA MSB!)
		inline ColoredTexturedVertex(gvec3 position, unsigned int color, float u, float v) :					ColoredVertex(position, color), u(u), v(v) { }
		/// @brief Constructor.
		/// @param[in] x X-coordinate.
		/// @param[in] y Y-coordinate.
		/// @param[in] z Z-coordinate.
		/// @param[in] color Color of the vertex.
		/// @param[in] textureCoordinate Texture coordinate of the vertex.
		/// @note The 'color' value is in native format, not RGBA MSB!)
		inline ColoredTexturedVertex(float x, float y, float z, unsigned int color, gvec2 textureCoordiate) :	ColoredVertex(x, y, z, color), u(textureCoordiate.x), v(textureCoordiate.y) { }
		/// @brief Constructor.
		/// @param[in] position Position of the vertex.
		/// @param[in] color Color of the vertex.
		/// @param[in] textureCoordinate Texture coordinate of the vertex.
		/// @note The 'color' value is in native format, not RGBA MSB!)
		inline ColoredTexturedVertex(gvec3 position, unsigned int color, gvec2 textureCoordiate) :				ColoredVertex(position, color), u(textureCoordiate.x), v(textureCoordiate.y) { }

	};
	
	/// @brief Converts an RGB value to an HSL value.
	/// @param[in] r Red value of the color.
	/// @param[in] g Green value of the color.
	/// @param[in] b Blue value of the color.
	/// @param[out] h Hue value of the color.
	/// @param[out] s Saturation value of the color.
	/// @param[out] l Lightness value of the color.
	/// @note This conversion is lossy.
	aprilFnExport void rgbToHsl(unsigned char r, unsigned char g, unsigned char b, float* h, float* s, float* l);
	/// @brief Converts an HSL value to an RGB value.
	/// @param[in] h Hue value of the color.
	/// @param[in] s Saturation value of the color.
	/// @param[in] l Lightness value of the color.
	/// @param[out] r Red value of the color.
	/// @param[out] g Green value of the color.
	/// @param[out] b Blue value of the color.
	/// @note This conversion is lossy.
	aprilFnExport void hslToRgb(float h, float s, float l, unsigned char* r, unsigned char* g, unsigned char* b);
	/// @brief Generates a unique name with a given prefix.
	/// @param[in] prefix The prefix used for name generation.
	/// @return Unique name for the given prefix.
	aprilFnExport hstr generateName(chstr prefix);
	/// @brief Creates a string representation for a gtypes::Vector2.
	/// @param[in] vector The gtypes::Vector2 to convert.
	/// @brief A string.
	/// @note The proper format is: integers or floats separated by ',' (comma) characters
	aprilFnExport hstr gvec2ToHstr(gvec2 vector);
	/// @param[in] vector The gtypes::Vector3 to convert.
	/// @brief Creates a string representation for a gtypes::Vector3.
	/// @brief A string.
	/// @note The proper format is: integers or floats separated by ',' (comma) characters
	aprilFnExport hstr gvec3ToHstr(gvec3 vector);
	/// @param[in] vector The gtypes::Rectangle to convert.
	/// @brief Creates a string representation for a gtypes::Rectangle.
	/// @brief A string.
	/// @note The proper format is: integers or floats separated by ',' (comma) characters
	aprilFnExport hstr grectToHstr(grect rect);
	/// @brief Creates gtypes::Vector2 from a string representation.
	/// @param[in] string The string to use for conversion.
	/// @brief A gtypes::Vector2.
	/// @note The proper format is: integers or floats separated by ',' (comma) characters
	aprilFnExport gvec2 hstrToGvec2(chstr string);
	/// @brief Creates gtypes::Vector3 from a string representation.
	/// @param[in] string The string to use for conversion.
	/// @brief A gtypes::Vector3.
	/// @note The proper format is: integers or floats separated by ',' (comma) characters
	aprilFnExport gvec3 hstrToGvec3(chstr string);
	/// @brief Creates gtypes::Rectangle from a string representation.
	/// @param[in] string The string to use for conversion.
	/// @brief A gtypes::Rectangle.
	/// @note The proper format is: integers or floats separated by ',' (comma) characters
	aprilFnExport grect hstrToGrect(chstr string);

}

#endif
