/// @file
/// @version 5.2
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
#include <hltypes/henum.h>
#include <hltypes/hstring.h>

#include "aprilExport.h"

/// @brief Used internally for vector/rect/etc string representations.
#define APRIL_HSTR_SEPARATOR ','

namespace april
{
	/// @class RenderOperation
	/// @brief Defines possible rendering methods.
	HL_ENUM_CLASS_PREFIX_DECLARE(aprilExport, RenderOperation,
	(
		/// @var static const RenderOperation RenderOperation::TriangleList
		/// @brief Triangle-list.
		HL_ENUM_DECLARE(RenderOperation, TriangleList);
		/// @var static const RenderOperation RenderOperation::TriangleStrip
		/// @brief Triangle-strip.
		HL_ENUM_DECLARE(RenderOperation, TriangleStrip);
		/// @var static const RenderOperation RenderOperation::LineList
		/// @brief Line-list.
		HL_ENUM_DECLARE(RenderOperation, LineList);
		/// @var static const RenderOperation RenderOperation::LineStrip
		/// @brief Line-strip.
		HL_ENUM_DECLARE(RenderOperation, LineStrip);
		/// @var static const RenderOperation RenderOperation::PointList
		/// @brief Point-list.
		HL_ENUM_DECLARE(RenderOperation, PointList);

		/// @brief Checks if this is a triangle-based RenderOperation.
		/// @return True if triangle-based RenderOperation.
		bool isTriangle() const;
		/// @brief Checks if this is a triangle-based RenderOperation.
		/// @return True if triangle-based RenderOperation.
		bool isLine() const;
		/// @brief Checks if this is a triangle-based RenderOperation.
		/// @return True if triangle-based RenderOperation.
		bool isPoint() const;

		// DEPRECATED!
		HL_ENUM_DECLARE(RenderOperation, TriangleFan);

	));

	/// @class BlendMode
	/// @brief Defines blending modes.
	HL_ENUM_CLASS_PREFIX_DECLARE(aprilExport, BlendMode,
	(
		/// @var static const BlendMode BlendMode::Alpha
		/// @brief Alpha blending.
		/// @note This is the default.
		HL_ENUM_DECLARE(BlendMode, Alpha);
		/// @var static const BlendMode BlendMode::Add
		/// @brief Additive blending.
		HL_ENUM_DECLARE(BlendMode, Add);
		/// @var static const BlendMode BlendMode::Subtract
		/// @brief Subtractive blending.
		/// @note This might not work on all platforms.
		HL_ENUM_DECLARE(BlendMode, Subtract);
		/// @var static const BlendMode BlendMode::Overwrite
		/// @brief Overwrite data blending.
		HL_ENUM_DECLARE(BlendMode, Overwrite);
	));

	/// @class ColorMode
	/// @brief Defines color modes.
	HL_ENUM_CLASS_PREFIX_DECLARE(aprilExport, ColorMode,
	(
		/// @var static const ColorMode ColorMode::Multiply
		/// @brief Multiply.
		/// @note This is the default.
		HL_ENUM_DECLARE(ColorMode, Multiply);
		/// @var static const ColorMode ColorMode::AlphaMap
		/// @brief Alpha Map.
		HL_ENUM_DECLARE(ColorMode, AlphaMap);
		/// @var static const ColorMode ColorMode::Lerp
		/// @brief Linear Interpolation.
		/// @note Requires usage of additional lerp-factor when setting the ColorMode.
		HL_ENUM_DECLARE(ColorMode, Lerp);
		/// @var static const ColorMode ColorMode::Desaturate
		/// @brief Desaturation filter.
		/// @note Requires usage of additional lerp-factor when setting the ColorMode.
		HL_ENUM_DECLARE(ColorMode, Desaturate);
		/// @var static const ColorMode ColorMode::Sepia
		/// @brief Sepia filter.
		/// @note Requires usage of additional lerp-factor when setting the ColorMode.
		HL_ENUM_DECLARE(ColorMode, Sepia);
	));

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
		explicit inline PlainVertex() :								x(0.0f), y(0.0f), z(0.0f) { }
		/// @brief Constructor.
		/// @param[in] x X-coordinate.
		/// @param[in] y Y-coordinate.
		/// @param[in] z Z-coordinate.
		explicit inline PlainVertex(float x, float y, float z) :	x(x), y(y), z(z) { }
		/// @brief Constructor.
		/// @param[in] position Position of the vertex.
		explicit inline PlainVertex(cgvec3f position) :				x(position.x), y(position.y), z(position.z) { }

		/// @brief Assigns a position to this vertex.
		/// @param[in] x X-coordinate.
		/// @param[in] y Y-coordinate.
		/// @param[in] z Z-coordinate.
		inline void set(float x, float y, float z)					{ this->x = x; this->y = y; this->z = z; }
		/// @brief Assigns a gtypes::Vector3 position to this vertex.
		/// @param[in] position Position of the vector.
		inline void set(cgvec3f position)							{ this->x = position.x; this->y = position.y; this->z = position.z; }

		/// @brief Returns the position as gtypes::Vector3.
		/// @return The position as gtypes::Vector3.
		inline gvec3f toGvec3f()										{ return gvec3f(this->x, this->y, this->z); }

	};

	/// @brief Represents a vertex with a color component.
	class aprilExport ColoredVertex : public PlainVertex
	{
	public:
		/// @brief The vertex color.
		/// @note This value is in native format, not RGBA MSB!
		unsigned int color;

		/// @brief Basic constructor.
		explicit inline ColoredVertex() :												PlainVertex(), color(0xFFFFFFFF) { }
		/// @brief Constructor.
		/// @param[in] x X-coordinate.
		/// @param[in] y Y-coordinate.
		/// @param[in] z Z-coordinate.
		explicit inline ColoredVertex(float x, float y, float z) :						PlainVertex(x, y, z), color(0xFFFFFFFF) { }
		/// @brief Constructor.
		/// @param[in] position Position of the vertex.
		explicit inline ColoredVertex(cgvec3f position) :								PlainVertex(position), color(0xFFFFFFFF) { }
		/// @brief Constructor.
		/// @param[in] color Color of the vertex.
		/// @note The 'color' value is in native format, not RGBA MSB!)
		explicit inline ColoredVertex(unsigned int color) :								PlainVertex(), color(color) { }
		/// @brief Constructor.
		/// @param[in] x X-coordinate.
		/// @param[in] y Y-coordinate.
		/// @param[in] z Z-coordinate.
		/// @param[in] color Color of the vertex.
		/// @note The 'color' value is in native format, not RGBA MSB!)
		explicit inline ColoredVertex(float x, float y, float z, unsigned int color) :	PlainVertex(x, y, z), color(color) { }
		/// @brief Constructor.
		/// @param[in] position Position of the vertex.
		/// @param[in] color Color of the vertex.
		/// @note The 'color' value is in native format, not RGBA MSB!)
		explicit inline ColoredVertex(cgvec3f position, unsigned int color) :			PlainVertex(position), color(color) { }

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
		explicit inline TexturedVertex() :														PlainVertex(), u(0.0f), v(0.0f) { }
		/// @brief Constructor.
		/// @param[in] x X-coordinate.
		/// @param[in] y Y-coordinate.
		/// @param[in] z Z-coordinate.
		explicit inline TexturedVertex(float x, float y, float z) :								PlainVertex(x, y, z), u(0.0f), v(0.0f) { }
		/// @brief Constructor.
		/// @param[in] position Position of the vertex.
		explicit inline TexturedVertex(cgvec3f position) :										PlainVertex(position), u(0.0f), v(0.0f) { }
		/// @brief Constructor.
		/// @param[in] u Texture U-coordinate.
		/// @param[in] v Texture V-coordinate.
		explicit inline TexturedVertex(float u, float v) :										PlainVertex(), u(u), v(v) { }
		/// @brief Constructor.
		/// @param[in] textureCoordinate Texture coordinate of the vertex.
		explicit inline TexturedVertex(cgvec2f textureCoordinate) :								PlainVertex(), u(textureCoordinate.x), v(textureCoordinate.y) { }
		/// @brief Constructor.
		/// @param[in] x X-coordinate.
		/// @param[in] y Y-coordinate.
		/// @param[in] z Z-coordinate.
		/// @param[in] u Texture U-coordinate.
		/// @param[in] v Texture V-coordinate.
		explicit inline TexturedVertex(float x, float y, float z, float u, float v) :			PlainVertex(x, y, z), u(u), v(v) { }
		/// @brief Constructor.
		/// @param[in] x X-coordinate.
		/// @param[in] y Y-coordinate.
		/// @param[in] z Z-coordinate.
		/// @param[in] textureCoordinate Texture coordinate of the vertex.
		explicit inline TexturedVertex(float x, float y, float z, cgvec2f textureCoordinate) :	PlainVertex(x, y, z), u(textureCoordinate.x), v(textureCoordinate.y) { }
		/// @brief Constructor.
		/// @param[in] position Position of the vertex.
		/// @param[in] u Texture U-coordinate.
		/// @param[in] v Texture V-coordinate.
		explicit inline TexturedVertex(cgvec3f position, float u, float v) :						PlainVertex(position), u(u), v(v) { }
		/// @brief Constructor.
		/// @param[in] position Position of the vertex.
		/// @param[in] textureCoordinate Texture coordinate of the vertex.
		explicit inline TexturedVertex(cgvec3f position, cgvec2f textureCoordinate) :				PlainVertex(position), u(textureCoordinate.x), v(textureCoordinate.y) { }

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
		explicit inline ColoredTexturedVertex() :																			ColoredVertex(), u(0.0f), v(0.0f) { }
		/// @brief Constructor.
		/// @param[in] x X-coordinate.
		/// @param[in] y Y-coordinate.
		/// @param[in] z Z-coordinate.
		explicit inline ColoredTexturedVertex(float x, float y, float z) :													ColoredVertex(x, y, z), u(0.0f), v(0.0f) { }
		/// @brief Constructor.
		/// @param[in] position Position of the vertex.
		explicit inline ColoredTexturedVertex(cgvec3f position) :															ColoredVertex(position), u(0.0f), v(0.0f) { }
		/// @brief Constructor.
		/// @param[in] color Color of the vertex.
		/// @note The 'color' value is in native format, not RGBA MSB!)
		explicit inline ColoredTexturedVertex(unsigned int color) :															ColoredVertex(color), u(0.0f), v(0.0f) { }
		/// @brief Constructor.
		/// @param[in] u Texture U-coordinate.
		/// @param[in] v Texture V-coordinate.
		explicit inline ColoredTexturedVertex(float u, float v) :															ColoredVertex(), u(u), v(v) { }
		/// @brief Constructor.
		/// @param[in] textureCoordinate Texture coordinate of the vertex.
		explicit inline ColoredTexturedVertex(cgvec2f textureCoordinate) :													ColoredVertex(), u(textureCoordinate.x), v(textureCoordinate.y) { }
		/// @brief Constructor.
		/// @param[in] x X-coordinate.
		/// @param[in] y Y-coordinate.
		/// @param[in] z Z-coordinate.
		/// @param[in] color Color of the vertex.
		/// @note The 'color' value is in native format, not RGBA MSB!)
		explicit inline ColoredTexturedVertex(float x, float y, float z, unsigned int color) :								ColoredVertex(x, y, z, color), u(0.0f), v(0.0f) { }
		/// @brief Constructor.
		/// @param[in] position Position of the vertex.
		/// @param[in] color Color of the vertex.
		/// @note The 'color' value is in native format, not RGBA MSB!)
		explicit inline ColoredTexturedVertex(cgvec3f position, unsigned int color) :										ColoredVertex(position, color), u(0.0f), v(0.0f) { }
		/// @brief Constructor.
		/// @param[in] x X-coordinate.
		/// @param[in] y Y-coordinate.
		/// @param[in] z Z-coordinate.
		/// @param[in] u Texture U-coordinate.
		/// @param[in] v Texture V-coordinate.
		explicit inline ColoredTexturedVertex(float x, float y, float z, float u, float v) :								ColoredVertex(x, y, z), u(u), v(v) { }
		/// @brief Constructor.
		/// @param[in] position Position of the vertex.
		/// @param[in] u Texture U-coordinate.
		/// @param[in] v Texture V-coordinate.
		explicit inline ColoredTexturedVertex(cgvec3f position, float u, float v) :											ColoredVertex(position), u(u), v(v) { }
		/// @brief Constructor.
		/// @param[in] x X-coordinate.
		/// @param[in] y Y-coordinate.
		/// @param[in] z Z-coordinate.
		/// @param[in] textureCoordinate Texture coordinate of the vertex.
		explicit inline ColoredTexturedVertex(float x, float y, float z, cgvec2f textureCoordinate) :						ColoredVertex(x, y, z), u(textureCoordinate.x), v(textureCoordinate.y) { }
		/// @brief Constructor.
		/// @param[in] position Position of the vertex.
		/// @param[in] textureCoordinate Texture coordinate of the vertex.
		explicit inline ColoredTexturedVertex(cgvec3f position, cgvec2f textureCoordinate) :									ColoredVertex(position), u(textureCoordinate.x), v(textureCoordinate.y) { }
		/// @brief Constructor.
		/// @param[in] x X-coordinate.
		/// @param[in] y Y-coordinate.
		/// @param[in] z Z-coordinate.
		/// @param[in] color Color of the vertex.
		/// @param[in] u Texture U-coordinate.
		/// @param[in] v Texture V-coordinate.
		/// @note The 'color' value is in native format, not RGBA MSB!)
		explicit inline ColoredTexturedVertex(float x, float y, float z, unsigned int color, float u, float v) :			ColoredVertex(x, y, z, color), u(u), v(v) { }
		/// @brief Constructor.
		/// @param[in] position Position of the vertex.
		/// @param[in] color Color of the vertex.
		/// @param[in] u Texture U-coordinate.
		/// @param[in] v Texture V-coordinate.
		/// @note The 'color' value is in native format, not RGBA MSB!)
		explicit inline ColoredTexturedVertex(cgvec3f position, unsigned int color, float u, float v) :						ColoredVertex(position, color), u(u), v(v) { }
		/// @brief Constructor.
		/// @param[in] x X-coordinate.
		/// @param[in] y Y-coordinate.
		/// @param[in] z Z-coordinate.
		/// @param[in] color Color of the vertex.
		/// @param[in] textureCoordinate Texture coordinate of the vertex.
		/// @note The 'color' value is in native format, not RGBA MSB!)
		explicit inline ColoredTexturedVertex(float x, float y, float z, unsigned int color, cgvec2f textureCoordinate) :	ColoredVertex(x, y, z, color), u(textureCoordinate.x), v(textureCoordinate.y) { }
		/// @brief Constructor.
		/// @param[in] position Position of the vertex.
		/// @param[in] color Color of the vertex.
		/// @param[in] textureCoordinate Texture coordinate of the vertex.
		/// @note The 'color' value is in native format, not RGBA MSB!)
		explicit inline ColoredTexturedVertex(cgvec3f position, unsigned int color, cgvec2f textureCoordinate) :				ColoredVertex(position, color), u(textureCoordinate.x), v(textureCoordinate.y) { }

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
	/// @return A string.
	/// @note The proper format is: integers or floats separated by ',' (comma) characters
	template <typename T>
	inline hstr gvec2ToHstr(const gvec2<T>& vector)
	{
		return hsprintf("%s%c%s", hstr(vector.x).cStr(), APRIL_HSTR_SEPARATOR, hstr(vector.y).cStr());
	}
	/// @brief Creates a string representation for a gtypes::Vector3.
	/// @param[in] vector The gtypes::Vector3 to convert.
	/// @return A string.
	/// @note The proper format is: integers or floats separated by ',' (comma) characters
	template <typename T>
	inline hstr gvec3ToHstr(const gvec3<T>& vector)
	{
		return hsprintf("%s%c%s%c%s", hstr(vector.x).cStr(), APRIL_HSTR_SEPARATOR, hstr(vector.y).cStr(), APRIL_HSTR_SEPARATOR, hstr(vector.z).cStr());
	}
	/// @brief Creates a string representation for a gtypes::Rectangle.
	/// @param[in] rect The gtypes::Rectangle to convert.
	/// @return A string.
	/// @note The proper format is: integers or floats separated by ',' (comma) characters
	template <typename T>
	inline hstr grectToHstr(const grect<T>& rect)
	{
		return hsprintf("%s%c%s%c%s%c%s", hstr(rect.x).cStr(), APRIL_HSTR_SEPARATOR, hstr(rect.y).cStr(), APRIL_HSTR_SEPARATOR, hstr(rect.w).cStr(), APRIL_HSTR_SEPARATOR, hstr(rect.h).cStr());
	}
	/// @brief Creates gtypes::Vector2 from a string representation.
	/// @param[in] string The string to use for conversion.
	/// @return A gtypes::Vector2.
	/// @note The proper format is: integers or floats separated by ',' (comma) characters
	template <typename T>
	inline gvec2<T> hstrToGvec2(chstr string)
	{
		harray<hstr> data = string.split(APRIL_HSTR_SEPARATOR);
		if (data.size() != 2)
		{
			throw Exception("Cannot convert string '" + string + "' to gtypes::Vector2.");
		}
		return gvec2<T>(data[0].trimmed(), data[1].trimmed());
	}
	/// @brief Creates gtypes::Vector3 from a string representation.
	/// @param[in] string The string to use for conversion.
	/// @return A gtypes::Vector3.
	/// @note The proper format is: integers or floats separated by ',' (comma) characters
	template <typename T>
	inline gvec3<T> hstrToGvec3(chstr string)
	{
		harray<hstr> data = string.split(APRIL_HSTR_SEPARATOR);
		if (data.size() != 3)
		{
			throw Exception("Cannot convert string '" + string + "' to gtypes::Vector3.");
		}
		return gvec3<T>(data[0].trimmed(), data[1].trimmed(), data[2].trimmed());
	}
	/// @brief Creates gtypes::Rectangle from a string representation.
	/// @param[in] string The string to use for conversion.
	/// @return A gtypes::Rectangle.
	/// @note The proper format is: integers or floats separated by ',' (comma) characters
	template <typename T>
	inline grect<T> hstrToGrect(chstr string)
	{
		harray<hstr> data = string.split(APRIL_HSTR_SEPARATOR);
		if (data.size() != 4)
		{
			throw Exception("Cannot convert string '" + string + "' to gtypes::Rectangle.");
		}
		return grect<T>(data[0].trimmed(), data[1].trimmed(), data[2].trimmed(), data[3].trimmed());
	}

#ifndef DOXYGEN_SHOULD_SKIP_THIS
	HL_DEPRECATED("Deprecated API. Use april::gvec2ToHstr() instead." )
	aprilFnExport inline hstr gvec2fToHstr(cgvec2f vector) { return gvec2ToHstr<float>(vector); }
	HL_DEPRECATED("Deprecated API. Use april::gvec3ToHstr() instead.")
	aprilFnExport inline hstr gvec3fToHstr(cgvec3f vector) { return gvec3ToHstr<float>(vector); }
	HL_DEPRECATED("Deprecated API. Use april::grectToHstr() instead.")
	aprilFnExport inline hstr grectfToHstr(cgrectf rect) { return grectToHstr<float>(rect); }
	HL_DEPRECATED("Deprecated API. Use april::hstrToGvec2() instead.")
	aprilFnExport inline gvec2f hstrToGvec2f(chstr string) { return hstrToGvec2<float>(string); }
	HL_DEPRECATED("Deprecated API. Use april::hstrToGvec3() instead.")
	aprilFnExport inline gvec3f hstrToGvec3f(chstr string) { return hstrToGvec3<float>(string); }
	HL_DEPRECATED("Deprecated API. Use april::hstrToGrect() instead.")
	aprilFnExport inline grectf hstrToGrectf(chstr string) { return hstrToGrect<float>(string); }
#endif

}
#endif
