/// @file
/// @version 3.5
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
		Color();
		/// @brief Constructor.
		/// @param[in] r The red value.
		/// @param[in] g The green value.
		/// @param[in] b The blue value.
		/// @param[in] a The alpha value.
		Color(int r, int g, int b, int a = 255);
		/// @brief Constructor.
		/// @param[in] r The red value.
		/// @param[in] g The green value.
		/// @param[in] b The blue value.
		/// @param[in] a The alpha value.
		Color(unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255);
		/// @brief Constructor.
		/// @param[in] color The color value.
		/// @note The unsigned int is in RGBA MSB order.
		Color(unsigned int color);
		/// @brief Constructor.
		/// @param[in] hex The hex values of the color.
		/// @note Can be RRGGBB or RRGGBBAA. The 0x prefix is optional.
		Color(chstr hex);
		/// @brief Constructor.
		/// @param[in] hex The hex values of the color.
		/// @note Can be RRGGBB or RRGGBBAA. The 0x prefix is optional.
		Color(const char* hex);
		/// @brief Constructor.
		/// @param[in] color The Color to copy.
		/// @param[in] a The replacement alpha value.
		Color(const Color& color, unsigned char a);
		/// @brief Destructor.
		~Color();

		/// @brief Sets the Color's values.
		/// @param[in] r The red value.
		/// @param[in] g The green value.
		/// @param[in] b The blue value.
		/// @param[in] a The alpha value.
		void set(int r, int g, int b, int a = 255);
		/// @brief Sets the Color's values.
		/// @param[in] r The red value.
		/// @param[in] g The green value.
		/// @param[in] b The blue value.
		/// @param[in] a The alpha value.
		void set(unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255);
		/// @brief Sets the Color's values.
		/// @param[in] color The color value.
		/// @note The unsigned int is in RGBA MSB order.
		void set(unsigned int color);
		/// @brief Sets the Color's values.
		/// @param[in] hex The hex values of the color.
		/// @note Can be RRGGBB or RRGGBBAA. The 0x prefix is optional.
		void set(chstr hex);
		/// @brief Sets the Color's values.
		/// @param[in] hex The hex values of the color.
		/// @note Can be RRGGBB or RRGGBBAA. The 0x prefix is optional.
		void set(const char* hex);
		/// @brief Sets the Color's values.
		/// @param[in] color The Color to copy.
		/// @param[in] a The replacement alpha value.
		void set(const Color& color, unsigned char a);

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
		/// @brief The hex string representation of the Color.
		/// @note Careful when using rgbOnly!
		hstr hex(bool rgbOnly = false) const;
		
		/// @brief Gets the unsigned int representation of the Color.
		/// @brief The unsigned int representation of the Color.
		/// @note The return value is in RGBA MSB order.
		operator unsigned int() const;
		
		/// @brief Compares whether two colors are equal.
		/// @return True if the colors are equal.
		bool operator==(const Color& other) const;
		/// @brief Compares whether two colors are not equal.
		/// @return True if the colors are not equal.
		bool operator!=(const Color& other) const;
		
		/// @brief Adds two Colors.
		/// @return The resulting Color.
		Color operator+(const Color& other) const;
		/// @brief Subtracts two Colors.
		/// @return The resulting Color.
		Color operator-(const Color& other) const;
		/// @brief Multiplies two Colors.
		/// @return The resulting Color.
		/// @note Multiplication is done in a [0,1] range logic.
		Color operator*(const Color& other) const;
		/// @brief Divides two Colors.
		/// @return The resulting Color.
		/// @note Division is done in a [0,1] range logic (beware of division with zero!).
		Color operator/(const Color& other) const;
		/// @brief Multiplies a Color with a factor.
		/// @return The resulting Color.
		Color operator*(float value) const;
		/// @brief Divides a Color with a factor.
		/// @return The resulting Color.
		Color operator/(float value) const;
		/// @brief Adds another Color to this one.
		/// @return This modified Color.
		Color operator+=(const Color& other);
		/// @brief Subtracts another Color from this one.
		/// @return This modified Color.
		Color operator-=(const Color& other);
		/// @brief Multiplies this Color with another one.
		/// @return This modified Color.
		/// @note Multiplication is done in a [0,1] range logic.
		Color operator*=(const Color& other);
		/// @brief Divides this Color with another one.
		/// @return This modified Color.
		/// @note Division is done in a [0,1] range logic (beware of division with zero!).
		Color operator/=(const Color& other);
		/// @brief Multiplies this Color with a factor.
		/// @return This modified Color.
		Color operator*=(float value);
		/// @brief Divides this Color with a factor.
		/// @return This modified Color.
		Color operator/=(float value);

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
