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
/// Defines a color object.

#ifndef APRIL_COLOR_H
#define APRIL_COLOR_H

#include <hltypes/hexception.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hstring.h>

#include "aprilExport.h"

namespace april
{
	/// @brief Defines a color object.
	class aprilExport Color
	{
	public:
		/// @brief The red value.
		unsigned char r;
		/// @brief The green value.
		unsigned char g;
		/// @brief The blue value.
		unsigned char b;
		/// @brief The alpha value.
		unsigned char a;
		
		/// @brief Basic constructor.
		inline Color() : r(255), g(255), b(255), a(255)
		{
		}
		/// @brief Constructor.
		/// @param[in] r The red value.
		/// @param[in] g The green value.
		/// @param[in] b The blue value.
		/// @param[in] a The alpha value.
		inline Color(int r, int g, int b, int a = 255) : r(r), g(g), b(b), a(a)
		{
		}
		/// @brief Constructor.
		/// @param[in] r The red value.
		/// @param[in] g The green value.
		/// @param[in] b The blue value.
		/// @param[in] a The alpha value.
		inline Color(unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255) : r(r), g(g), b(b), a(a)
		{
		}
		/// @brief Constructor.
		/// @param[in] color The color value.
		/// @note The unsigned int is in RGBA MSB order.
		inline Color(unsigned int color)
		{
			this->set(color);
		}
		/// @brief Constructor.
		/// @param[in] hex The hex values of the color.
		/// @note Can be RRGGBB or RRGGBBAA. The 0x prefix is optional.
		inline Color(chstr hex)
		{
			this->set(hex);
		}
		/// @brief Constructor.
		/// @param[in] hex The hex values of the color.
		/// @note Can be RRGGBB or RRGGBBAA. The 0x prefix is optional.
		inline Color(const char* hex)
		{
			this->set(hex);
		}
		/// @brief Constructor.
		/// @param[in] color The Color to copy.
		/// @param[in] a The replacement alpha value.
		inline Color(const Color& color, unsigned char a) : r(color.r), g(color.g), b(color.b), a(a)
		{
		}

		/// @brief Sets the Color's values.
		/// @param[in] r The red value.
		/// @param[in] g The green value.
		/// @param[in] b The blue value.
		/// @param[in] a The alpha value.
		inline void set(int r, int g, int b, int a = 255)
		{
			this->r = (unsigned char)r;
			this->g = (unsigned char)g;
			this->b = (unsigned char)b;
			this->a = (unsigned char)a;
		}
		/// @brief Sets the Color's values.
		/// @param[in] r The red value.
		/// @param[in] g The green value.
		/// @param[in] b The blue value.
		/// @param[in] a The alpha value.
		inline void set(unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255)
		{
			this->r = r;
			this->g = g;
			this->b = b;
			this->a = a;
		}
		/// @brief Sets the Color's values.
		/// @param[in] color The color value.
		/// @note The unsigned int is in RGBA MSB order.
		inline void set(unsigned int color)
		{
			this->r = (unsigned char)((color >> 24) & 0xFF);
			this->g = (unsigned char)((color >> 16) & 0xFF);
			this->b = (unsigned char)((color >> 8) & 0xFF);
			this->a = (unsigned char)(color & 0xFF);
		}
		/// @brief Sets the Color's values.
		/// @param[in] hex The hex values of the color.
		/// @note Can be RRGGBB or RRGGBBAA. The 0x prefix is optional.
		inline void set(chstr hex)
		{
			// not using Color::isColor(), because of performance reasons
			hstr value = (hex.startsWith("0x") ? hex(2, -1) : hex);
			int size = value.size();
			if ((size != 6 && size != 8) || !value.isHex())
			{
				throw Exception(hsprintf("Incorrect hex color format '%s'! It must be either 0xRRGGBBAA or 0xRRGGBB (with or without 0x prefix)", hex.cStr()));
			}
			this->r = (unsigned char)value(0, 2).unhex();
			this->g = (unsigned char)value(2, 2).unhex();
			this->b = (unsigned char)value(4, 2).unhex();
			this->a = (size == 8 ? (unsigned char)value(6, 2).unhex() : 255);
		}
		/// @brief Sets the Color's values.
		/// @param[in] hex The hex values of the color.
		/// @note Can be RRGGBB or RRGGBBAA. The 0x prefix is optional.
		inline void set(const char* hex)
		{
			this->set(hstr(hex));
		}
		/// @brief Sets the Color's values.
		/// @param[in] color The Color to copy.
		/// @param[in] a The replacement alpha value.
		inline void set(const Color& color, unsigned char a)
		{
			this->r = color.r;
			this->g = color.g;
			this->b = color.b;
			this->a = a;
		}

		/// @brief Gets the red value as float in the range of [0,1].
		inline float r_f() const { return this->r * 0.003921569f; } // equals r / 255, multiplication is faster than division
		/// @brief Gets the green value as float in the range of [0,1].
		inline float g_f() const { return this->g * 0.003921569f; } // equals g / 255, multiplication is faster than division
		/// @brief Gets the blue value as float in the range of [0,1].
		inline float b_f() const { return this->b * 0.003921569f; } // equals b / 255, multiplication is faster than division
		/// @brief Gets the alpha value as float in the range of [0,1].
		inline float a_f() const { return this->a * 0.003921569f; } // equals a / 255, multiplication is faster than division
		
		/// @brief Gets the hex string representation of the Color.
		/// @param[in] rgbOnly Whether alpha should not be included.
		/// @return The hex string representation of the Color.
		/// @note Careful when using rgbOnly!
		inline hstr hex(bool rgbOnly = false) const
		{
			return (!rgbOnly ? hsprintf("%02X%02X%02X%02X", this->r, this->g, this->b, this->a) : hsprintf("%02X%02X%02X", this->r, this->g, this->b));
		}
		/// @brief Creates a linearly interpolated Color between this and a given Color.
		/// @param[in] other Color to interpolate with.
		/// @param[in] factor Interpolation factor.
		/// @note A factor of 0 or less will result in this Color while a factor of 1 or more will result in other.
		inline april::Color lerp(const april::Color& other, float factor) const
		{
			if (factor <= 0.0f)
			{
				return (*this);
			}
			if (factor >= 1.0f)
			{
				return other;
			}
			april::Color result;
			result.r = (unsigned char)hclamp((int)(this->r + ((int)other.r - this->r) * factor), 0, 255);
			result.g = (unsigned char)hclamp((int)(this->g + ((int)other.g - this->g) * factor), 0, 255);
			result.b = (unsigned char)hclamp((int)(this->b + ((int)other.b - this->b) * factor), 0, 255);
			result.a = (unsigned char)hclamp((int)(this->a + ((int)other.a - this->a) * factor), 0, 255);
			return result;
		}

		/// @brief Gets the unsigned int representation of the Color.
		/// @return The unsigned int representation of the Color.
		/// @note The return value is in RGBA MSB order.
		inline operator unsigned int() const
		{
			return (((int)this->r << 24) | ((int)this->g << 16) | ((int)this->b << 8) | (int)this->a);
		}

		/// @brief Compares whether two colors are equal.
		/// @return True if the colors are equal.
		inline bool operator==(const Color& other) const
		{
			return (this->r == other.r && this->g == other.g && this->b == other.b && this->a == other.a);
		}
		/// @brief Compares whether two colors are not equal.
		/// @return True if the colors are not equal.
		inline bool operator!=(const Color& other) const
		{
			return (this->r != other.r || this->g != other.g || this->b != other.b || this->a != other.a);
		}

		/// @brief Adds two Colors.
		/// @return The resulting Color.
		inline Color operator+(const Color& other) const
		{
			Color result(*this);
			result += other;
			return result;
		}
		/// @brief Subtracts two Colors.
		/// @return The resulting Color.
		inline Color operator-(const Color& other) const
		{
			Color result(*this);
			result -= other;
			return result;
		}
		/// @brief Multiplies two Colors.
		/// @return The resulting Color.
		/// @note Multiplication is done in a [0,1] range logic.
		inline Color operator*(const Color& other) const
		{
			Color result(*this);
			result *= other;
			return result;
		}
		/// @brief Divides two Colors.
		/// @return The resulting Color.
		/// @note Division is done in a [0,1] range logic (beware of division with zero!).
		inline Color operator/(const Color& other) const
		{
			Color result(*this);
			result /= other;
			return result;
		}
		/// @brief Multiplies a Color with a factor.
		/// @return The resulting Color.
		inline Color operator*(float value) const
		{
			Color result(*this);
			result *= value;
			return result;
		}
		/// @brief Divides a Color with a factor.
		/// @return The resulting Color.
		inline Color operator/(float value) const
		{
			Color result(*this);
			result /= value;
			return result;
		}
		/// @brief Adds another Color to this one.
		/// @return This modified Color.
		inline Color operator+=(const Color& other)
		{
			this->r = (unsigned char)hclamp((int)this->r + other.r, 0, 255);
			this->g = (unsigned char)hclamp((int)this->g + other.g, 0, 255);
			this->b = (unsigned char)hclamp((int)this->b + other.b, 0, 255);
			this->a = (unsigned char)hclamp((int)this->a + other.a, 0, 255);
			return (*this);
		}
		/// @brief Subtracts another Color from this one.
		/// @return This modified Color.
		inline Color operator-=(const Color& other)
		{
			this->r = (unsigned char)hclamp((int)this->r - other.r, 0, 255);
			this->g = (unsigned char)hclamp((int)this->g - other.g, 0, 255);
			this->b = (unsigned char)hclamp((int)this->b - other.b, 0, 255);
			this->a = (unsigned char)hclamp((int)this->a - other.a, 0, 255);
			return (*this);
		}
		/// @brief Multiplies this Color with another one.
		/// @return This modified Color.
		/// @note Multiplication is done in a [0,1] range logic.
		inline Color operator*=(const Color& other)
		{
			this->r = (unsigned char)hclamp((int)(this->r_f() * other.r), 0, 255);
			this->g = (unsigned char)hclamp((int)(this->g_f() * other.g), 0, 255);
			this->b = (unsigned char)hclamp((int)(this->b_f() * other.b), 0, 255);
			this->a = (unsigned char)hclamp((int)(this->a_f() * other.a), 0, 255);
			return (*this);
		}
		/// @brief Divides this Color with another one.
		/// @return This modified Color.
		/// @note Division is done in a [0,1] range logic (beware of division with zero!).
		inline Color operator/=(const Color& other)
		{
			this->r = (unsigned char)hclamp((int)(this->r_f() / other.r), 0, 255);
			this->g = (unsigned char)hclamp((int)(this->g_f() / other.g), 0, 255);
			this->b = (unsigned char)hclamp((int)(this->b_f() / other.b), 0, 255);
			this->a = (unsigned char)hclamp((int)(this->a_f() / other.a), 0, 255);
			return (*this);
		}
		/// @brief Multiplies this Color with a factor.
		/// @return This modified Color.
		inline Color operator*=(float value)
		{
			this->r = (unsigned char)hclamp((int)(this->r * value), 0, 255);
			this->g = (unsigned char)hclamp((int)(this->g * value), 0, 255);
			this->b = (unsigned char)hclamp((int)(this->b * value), 0, 255);
			this->a = (unsigned char)hclamp((int)(this->a * value), 0, 255);
			return (*this);
		}
		/// @brief Divides this Color with a factor.
		/// @return This modified Color.
		inline Color operator/=(float value)
		{
			float val = 1.0f / value;
			this->r = (unsigned char)hclamp((int)(this->r * val), 0, 255);
			this->g = (unsigned char)hclamp((int)(this->g * val), 0, 255);
			this->b = (unsigned char)hclamp((int)(this->b * val), 0, 255);
			this->a = (unsigned char)hclamp((int)(this->a * val), 0, 255);
			return (*this);
		}

		/// @brief Checks if string is valid color value.
		/// @param[in] hex The string to check.
		/// @return True if string is valid color value.
		inline static bool isColor(chstr hex)
		{
			hstr value = (hex.startsWith("0x") ? hex(2, -1) : hex);
			int size = value.size();
			return ((size == 6 || size == 8) && value.isHex());
		}

		/// @brief Provides a commonly used white Color.
		static Color White;
		/// @brief Provides a commonly used black Color.
		static Color Black;
		/// @brief Provides a commonly used grey Color.
		static Color Grey;
		/// @brief Provides a commonly used red Color.
		static Color Red;
		/// @brief Provides a commonly used green Color.
		static Color Green;
		/// @brief Provides a commonly used blue Color.
		static Color Blue;
		/// @brief Provides a commonly used yellow Color.
		static Color Yellow;
		/// @brief Provides a commonly used magenta Color.
		static Color Magenta;
		/// @brief Provides a commonly used cyan Color.
		static Color Cyan;
		/// @brief Provides a commonly used orange Color.
		static Color Orange;
		/// @brief Provides a commonly used pink Color.
		static Color Pink;
		/// @brief Provides a commonly used teal Color.
		static Color Teal;
		/// @brief Provides a commonly used neon Color.
		static Color Neon;
		/// @brief Provides a commonly used purple Color.
		static Color Purple;
		/// @brief Provides a commonly used aqua Color.
		static Color Aqua;
		/// @brief Provides a commonly used light-grey Color.
		static Color LightGrey;
		/// @brief Provides a commonly used light-red Color.
		static Color LightRed;
		/// @brief Provides a commonly used light-green Color.
		static Color LightGreen;
		/// @brief Provides a commonly used light-blue Color.
		static Color LightBlue;
		/// @brief Provides a commonly used light-yellow Color.
		static Color LightYellow;
		/// @brief Provides a commonly used light-magenta Color.
		static Color LightMagenta;
		/// @brief Provides a commonly used light-cyan Color.
		static Color LightCyan;
		/// @brief Provides a commonly used light-orange Color.
		static Color LightOrange;
		/// @brief Provides a commonly used light-pink Color.
		static Color LightPink;
		/// @brief Provides a commonly used light-teal Color.
		static Color LightTeal;
		/// @brief Provides a commonly used light-neon Color.
		static Color LightNeon;
		/// @brief Provides a commonly used light-purple Color.
		static Color LightPurple;
		/// @brief Provides a commonly used light-aqua Color.
		static Color LightAqua;
		/// @brief Provides a commonly used dark-grey Color.
		static Color DarkGrey;
		/// @brief Provides a commonly used dark-red Color.
		static Color DarkRed;
		/// @brief Provides a commonly used dark-green Color.
		static Color DarkGreen;
		/// @brief Provides a commonly used dark-blue Color.
		static Color DarkBlue;
		/// @brief Provides a commonly used dark-yellow Color.
		static Color DarkYellow;
		/// @brief Provides a commonly used dark-magenta Color.
		static Color DarkMagenta;
		/// @brief Provides a commonly used dark-cyan Color.
		static Color DarkCyan;
		/// @brief Provides a commonly used dark-orange Color.
		static Color DarkOrange;
		/// @brief Provides a commonly used dark-pink Color.
		static Color DarkPink;
		/// @brief Provides a commonly used dark-teal Color.
		static Color DarkTeal;
		/// @brief Provides a commonly used dark-neon Color.
		static Color DarkNeon;
		/// @brief Provides a commonly used dark-purple Color.
		static Color DarkPurple;
		/// @brief Provides a commonly used dark-aqua Color.
		static Color DarkAqua;
		/// @brief Provides a commonly used "clear" Color. This is black with an alpha of 0.
		static Color Clear;
		/// @brief Provides a commonly used "blank" Color. This is white with an alpha of 0.
		static Color Blank;
		
	};

}

#endif
