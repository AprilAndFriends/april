/// @file
/// @version 4.1
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Defines a generic image data source.

#ifndef APRIL_IMAGE_H
#define APRIL_IMAGE_H

#include <gtypes/Rectangle.h>
#include <gtypes/Vector2.h>
#include <hltypes/hmap.h>
#include <hltypes/hsbase.h>
#include <hltypes/hstring.h>

#include "aprilExport.h"
#include "Color.h"

namespace april
{
	/// @brief Defines a generic image data source.
	class aprilExport Image
	{
	public:
		/// @brief Defines the pixel format of image data.
		/// @note Some formats are intended to improve speed with the underlying engine if really needed. *X* formats are always 4 BPP even if that byte is not used.
		enum Format
		{
			/// @brief Invalid format definition.
			FORMAT_INVALID = 0,
			/// @brief Defines RGBA.
			FORMAT_RGBA,
			/// @brief Defines ARGB.
			FORMAT_ARGB,
			/// @brief Defines BGRA.
			FORMAT_BGRA,
			/// @brief Defines ABGR.
			FORMAT_ABGR,
			/// @brief Defines RGBX.
			FORMAT_RGBX,
			/// @brief Defines XRGB.
			FORMAT_XRGB,
			/// @brief Defines BGRX.
			FORMAT_BGRX,
			/// @brief Defines XBGR.
			FORMAT_XBGR,
			/// @brief Defines RGB.
			FORMAT_RGB,
			/// @brief Defines BGR.
			FORMAT_BGR,
			/// @brief Defines a single-color-channel image, the context being the alpha channel.
			/// @see FORMAT_GRAYSCALE
			FORMAT_ALPHA,
			/// @brief Defines a single-color-channel image, the context being a grayscale image.
			/// @see FORMAT_ALPHA
			FORMAT_GRAYSCALE,
			/// @brief Defines an image with palette colors.
			FORMAT_PALETTE // TODOa - this is msotly unsupported right now
		};

		/// @brief Defines image file formats.
		/// @note Usually only used for file writing.
		enum FileFormat
		{
			FILE_FORMAT_PNG = 0
		};

		/// @brief The raw image data.
		unsigned char* data;
		/// @brief Width of the image in pixels.
		int w;
		/// @brief Height of the image in pixels.
		int h;
		/// @brief Pixel format of the image data.
		Format format;
		/// @brief Internal format identifier.
		/// @note Needed for special platform dependent formats, usually used internally only.
		int internalFormat;
		/// @brief The byte size of the image data when handling compressed formats (e.g. FORMAT_PALETTE).
		int compressedSize;

		/// @brief Destructor.
		virtual ~Image();
		
		/// @brief Gets the byte-per-pixel value.
		/// @return The byte-per-pixel value.
		/// @note e.g. a value of 3 means that one pixels uses 3 bytes.
		int getBpp() const;
		/// @brief Gets the size of the image data in bytes.
		/// @return Size of the image data in bytes.
		/// @note The calculation is basically "w * h * getBpp()".
		int getByteSize() const;
		/// @brief Checks if the image is a valid image data construct with appropriate meta data.
		/// @return True if the image is a valid image data construct with appropriate meta data.
		bool isValid() const;

		/// @brief Clears the entire image and sets all image data to zeroes.
		/// @return True if successful.
		bool clear();
		/// @brief Gets the color of a specific pixel.
		/// @param[in] x X-coordinate.
		/// @param[in] y Y-coordinate.
		/// @return The Color of the pixel.
		Color getPixel(int x, int y) const;
		/// @brief Sets the color of a specific pixel.
		/// @param[in] x X-coordinate.
		/// @param[in] y Y-coordinate.
		/// @param[in] color The new Color of the pixel.
		/// @return True if successful.
		bool setPixel(int x, int y, Color color);
		/// @brief Gets the linearly interpolated color between two pixels.
		/// @param[in] x Decimal X-coordinate.
		/// @param[in] y Decimal Y-coordinate.
		/// @return The interpolated Color of the pixel.
		Color getInterpolatedPixel(float x, float y) const;
		/// @brief Fills a rectangle area with one color.
		/// @param[in] x X-coordinate.
		/// @param[in] y Y-coordinate.
		/// @param[in] w Width of the area.
		/// @param[in] h Height of the area.
		/// @param[in] color The Color used for filling.
		/// @return True if successful.
		bool fillRect(int x, int y, int w, int h, Color color);
		/// @brief Copies the image data into a buffer.
		/// @param[out] output The output buffer.
		/// @param[in] format In what format the output should be done.
		/// @return True if successful.
		/// @note If output has not been allocated yet, it will be allocated with the new operator.
		bool copyPixelData(unsigned char** output, Format format) const;
		/// @brief Writes image data directly onto the image.
		/// @param[in] sx Source data X-coordinate.
		/// @param[in] sy Source data Y-coordinate.
		/// @param[in] sw Width of the area on the source to be copied.
		/// @param[in] sh Height of the area on the source to be copied.
		/// @param[in] dx Destination X-coordinate.
		/// @param[in] dy Destination Y-coordinate.
		/// @param[in] srcData The source raw image data.
		/// @param[in] srcWidth The width of source raw image data.
		/// @param[in] srcHeight The height of source raw image data.
		/// @param[in] srcFormat The pixel format of source raw image data.
		/// @return True if successful.
		/// @note Pixels on the destination will be overwritten.
		bool write(int sx, int sy, int sw, int sh, int dx, int dy, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat);
		/// @brief Writes image data directly onto the image while trying to stretch the pixels. Stretched pixels will be linearly interpolated.
		/// @param[in] sx Source data X-coordinate.
		/// @param[in] sy Source data Y-coordinate.
		/// @param[in] sw Width of the area on the source to be copied.
		/// @param[in] sh Height of the area on the source to be copied.
		/// @param[in] dx Destination X-coordinate.
		/// @param[in] dy Destination Y-coordinate.
		/// @param[in] dw Width of the destination area.
		/// @param[in] dh Height of the destination area.
		/// @param[in] srcData The source raw image data.
		/// @param[in] srcWidth The width of source raw image data.
		/// @param[in] srcHeight The height of source raw image data.
		/// @param[in] srcFormat The pixel format of source raw image data.
		/// @return True if successful.
		/// @note Pixels on the destination will be overwritten.
		bool writeStretch(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat);
		/// @brief Does an image data block transfer onto the image.
		/// @param[in] sx Source data X-coordinate.
		/// @param[in] sy Source data Y-coordinate.
		/// @param[in] sw Width of the area on the source to be copied.
		/// @param[in] sh Height of the area on the source to be copied.
		/// @param[in] dx Destination X-coordinate.
		/// @param[in] dy Destination Y-coordinate.
		/// @param[in] srcData The source raw image data.
		/// @param[in] srcWidth The width of source raw image data.
		/// @param[in] srcHeight The height of source raw image data.
		/// @param[in] srcFormat The pixel format of source raw image data.
		/// @param[in] alpha Alpha multiplier on the entire source image.
		/// @return True if successful.
		/// @note Pixels on the destination will be overwritten will be blended with alpha-blending using the source pixels.
		/// @note The parameter alpha is especially useful when blitting source images that don't have an alpha channel.
		bool blit(int sx, int sy, int sw, int sh, int dx, int dy, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat, unsigned char alpha = 255);
		/// @brief Does a stretched image data block transfer onto the image.
		/// @param[in] sx Source data X-coordinate.
		/// @param[in] sy Source data Y-coordinate.
		/// @param[in] sw Width of the area on the source to be copied.
		/// @param[in] sh Height of the area on the source to be copied.
		/// @param[in] dx Destination X-coordinate.
		/// @param[in] dy Destination Y-coordinate.
		/// @param[in] dw Width of the destination area.
		/// @param[in] dh Height of the destination area.
		/// @param[in] srcData The source raw image data.
		/// @param[in] srcWidth The width of source raw image data.
		/// @param[in] srcHeight The height of source raw image data.
		/// @param[in] srcFormat The pixel format of source raw image data.
		/// @param[in] alpha Alpha multiplier on the entire source image.
		/// @return True if successful.
		/// @note Pixels on the destination will be overwritten will be blended with alpha-blending using the source pixels.
		/// @note The parameter alpha is especially useful when blitting source images that don't have an alpha channel.
		bool blitStretch(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat, unsigned char alpha = 255);
		/// @brief Rotates the pixel hue of a rectangle area on the image.
		/// @param[in] x X-coordinate of the area to change.
		/// @param[in] y Y-coordinate of the area to change.
		/// @param[in] w Width of the area to change.
		/// @param[in] h Height of the area to change.
		/// @param[in] degrees By how many degrees the the should be rotated.
		/// @return True if successful.
		/// @note This is lossy operation.
		/// @note This is an expensive operation and should be used sparingly.
		bool rotateHue(int x, int y, int w, int h, float degrees);
		/// @brief Changes the saturation level of pixels of a rectangle area on the image.
		/// @param[in] x X-coordinate of the area to change.
		/// @param[in] y Y-coordinate of the area to change.
		/// @param[in] w Width of the area to change.
		/// @param[in] h Height of the area to change.
		/// @param[in] factor The saturation multiplier factor.
		/// @return True if successful.
		/// @note This is lossy operation.
		/// @note This is an expensive operation and should be used sparingly.
		bool saturate(int x, int y, int w, int h, float factor);
		/// @brief Inverts the pixel colors of a rectangle area on the image.
		/// @param[in] x X-coordinate of the area to change.
		/// @param[in] y Y-coordinate of the area to change.
		/// @param[in] w Width of the area to change.
		/// @param[in] h Height of the area to change.
		/// @return True if successful.
		bool invert(int x, int y, int w, int h);
		/// @brief Inserts image data as alpha channel into this image.
		/// @param[in] srcData The source raw image data.
		/// @param[in] srcFormat The pixel format of source raw image data.
		/// @param[in] median The median value for insertion.
		/// @param[in] ambiguity How "hard" the alpha channel transition should be.
		/// @return True if successful.
		/// @note The image data in srcData must be the same width and height as the image.
		/// @note This is an expensive operation and should be used sparingly.
		bool insertAlphaMap(unsigned char* srcData, Format srcFormat, unsigned char median, int ambiguity); // TODOa - this functionality might be removed since shaders are much faster
		/// @brief Dilates the image.
		/// @param[in] srcData The constructing image's raw image data.
		/// @param[in] srcWidth The width of the constructing image's raw image data.
		/// @param[in] srcHeight The height of the constructing image's raw image data.
		/// @param[in] srcFormat The pixel format of the constructing image's raw image data.
		/// @return True if successful.
		/// @note This is an expensive operation and should be used sparingly.
		/// @note Currently this operation is only supported for single-channel 8-bit images.
		bool dilate(unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat);

		/// @brief Extracts the red color channel of the image.
		/// @return Extracted image as FORMAT_ALPHA or NULL if channel cannot be extracted.
		Image* extractRed() const;
		/// @brief Extracts the greeb color channel of the image.
		/// @return Extracted image as FORMAT_ALPHA or NULL if channel cannot be extracted.
		Image* extractGreen() const;
		/// @brief Extracts the blue color channel of the image.
		/// @return Extracted image as FORMAT_ALPHA or NULL if channel cannot be extracted.
		Image* extractBlue() const;
		/// @brief Extracts the alpha channel of the image.
		/// @return Extracted image as FORMAT_ALPHA or NULL if channel cannot be extracted.
		Image* extractAlpha() const;
		/// @brief Extracts the color channel at the given index of the image.
		/// @param[in] index Index of the color channel.
		/// @return Extracted image as FORMAT_ALPHA or NULL if channel cannot be extracted.
		/// @note The index is different in different pixel formats so this methods isn't usually used directly, but is still public for completeness.
		/// @see extractRed
		/// @see extractGreen
		/// @see extractBlue
		/// @see extractAlpha
		Image* extractColor(int index) const;

		/// @brief Gets the color of a specific pixel.
		/// @param[in] position Pixel coordinate.
		/// @return The Color of the pixel.
		/// @see getPixel(int x, int y)
		Color getPixel(gvec2 position) const;
		/// @brief Sets the color of a specific pixel.
		/// @param[in] position Pixel coordinate.
		/// @param[in] color The new Color of the pixel.
		/// @return True if successful.
		/// @see setPixel(int x, int y, Color color)
		bool setPixel(gvec2 position, Color color);
		/// @brief Gets the linearly interpolated color between two pixels.
		/// @param[in] position Pixel coordinate.
		/// @return The interpolated Color of the pixel.
		/// @see getInterpolatedPixel(float x, float y)
		Color getInterpolatedPixel(gvec2 position) const;
		/// @brief Fills a rectangle area with one color.
		/// @param[in] rect The rectangle area.
		/// @param[in] color The Color used for filling.
		/// @return True if successful.
		/// @see fillRect(int x, int y, int w, int h, Color color)
		bool fillRect(grect rect, Color color);
		/// @brief Copies the image data into a buffer.
		/// @param[out] output The output buffer.
		/// @return True if successful.
		/// @note If output has not been allocated yet, it will be allocated with the new operator.
		/// @note The data will be copied in the same pixel format as this Image.
		bool copyPixelData(unsigned char** output) const;
		/// @brief Writes image data directly onto the image.
		/// @param[in] sx Source data X-coordinate.
		/// @param[in] sy Source data Y-coordinate.
		/// @param[in] sw Width of the area on the source to be copied.
		/// @param[in] sh Height of the area on the source to be copied.
		/// @param[in] dx Destination X-coordinate.
		/// @param[in] dy Destination Y-coordinate.
		/// @param[in] other The source Image.
		/// @return True if successful.
		/// @note Pixels on the destination will be overwritten.
		/// @see write(int sx, int sy, int sw, int sh, int dx, int dy, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat)
		bool write(int sx, int sy, int sw, int sh, int dx, int dy, Image* other);
		/// @brief Writes image data directly onto the image.
		/// @param[in] srcRect Source data rectangle.
		/// @param[in] destPosition Destination coordinates.
		/// @param[in] srcData The source raw image data.
		/// @param[in] srcWidth The width of source raw image data.
		/// @param[in] srcHeight The height of source raw image data.
		/// @param[in] srcFormat The pixel format of source raw image data.
		/// @return True if successful.
		/// @note Pixels on the destination will be overwritten.
		/// @see write(int sx, int sy, int sw, int sh, int dx, int dy, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat)
		bool write(grect srcRect, gvec2 destPosition, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat);
		/// @brief Writes image data directly onto the image.
		/// @param[in] srcRect Source data rectangle.
		/// @param[in] destPosition Destination coordinates.
		/// @param[in] other The source Image.
		/// @return True if successful.
		/// @note Pixels on the destination will be overwritten.
		/// @see write(int sx, int sy, int sw, int sh, int dx, int dy, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat)
		bool write(grect srcRect, gvec2 destPosition, Image* other);
		/// @brief Writes image data directly onto the image while trying to stretch the pixels. Stretched pixels will be linearly interpolated.
		/// @param[in] sx Source data X-coordinate.
		/// @param[in] sy Source data Y-coordinate.
		/// @param[in] sw Width of the area on the source to be copied.
		/// @param[in] sh Height of the area on the source to be copied.
		/// @param[in] dx Destination X-coordinate.
		/// @param[in] dy Destination Y-coordinate.
		/// @param[in] dw Width of the destination area.
		/// @param[in] dh Height of the destination area.
		/// @param[in] other The source Image.
		/// @return True if successful.
		/// @note Pixels on the destination will be overwritten.
		/// @see writeStretch(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat)
		bool writeStretch(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, Image* other);
		/// @brief Writes image data directly onto the image while trying to stretch the pixels. Stretched pixels will be linearly interpolated.
		/// @param[in] srcRect Source data rectangle.
		/// @param[in] destRect Destination rectangle.
		/// @param[in] srcData The source raw image data.
		/// @param[in] srcWidth The width of source raw image data.
		/// @param[in] srcHeight The height of source raw image data.
		/// @param[in] srcFormat The pixel format of source raw image data.
		/// @return True if successful.
		/// @note Pixels on the destination will be overwritten.
		/// @see writeStretch(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat)
		bool writeStretch(grect srcRect, grect destRect, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat);
		/// @brief Writes image data directly onto the image while trying to stretch the pixels. Stretched pixels will be linearly interpolated.
		/// @param[in] srcRect Source data rectangle.
		/// @param[in] destRect Destination rectangle.
		/// @param[in] other The source Image.
		/// @return True if successful.
		/// @note Pixels on the destination will be overwritten.
		/// @see writeStretch(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat)
		bool writeStretch(grect srcRect, grect destRect, Image* other);
		/// @brief Does an image data block transfer onto the image.
		/// @param[in] sx Source data X-coordinate.
		/// @param[in] sy Source data Y-coordinate.
		/// @param[in] sw Width of the area on the source to be copied.
		/// @param[in] sh Height of the area on the source to be copied.
		/// @param[in] dx Destination X-coordinate.
		/// @param[in] dy Destination Y-coordinate.
		/// @param[in] other The source Image.
		/// @param[in] alpha Alpha multiplier on the entire source image.
		/// @return True if successful.
		/// @note Pixels on the destination will be overwritten will be blended with alpha-blending using the source pixels.
		/// @note The parameter alpha is especially useful when blitting source images that don't have an alpha channel.
		/// @see blit(int sx, int sy, int sw, int sh, int dx, int dy, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat, unsigned char alpha)
		bool blit(int sx, int sy, int sw, int sh, int dx, int dy, Image* other, unsigned char alpha = 255);
		/// @brief Does an image data block transfer onto the image.
		/// @param[in] srcRect Source data rectangle.
		/// @param[in] destPosition Destination coordinates.
		/// @param[in] srcData The source raw image data.
		/// @param[in] srcWidth The width of source raw image data.
		/// @param[in] srcHeight The height of source raw image data.
		/// @param[in] srcFormat The pixel format of source raw image data.
		/// @param[in] alpha Alpha multiplier on the entire source image.
		/// @return True if successful.
		/// @note Pixels on the destination will be overwritten will be blended with alpha-blending using the source pixels.
		/// @note The parameter alpha is especially useful when blitting source images that don't have an alpha channel.
		/// @see blit(int sx, int sy, int sw, int sh, int dx, int dy, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat, unsigned char alpha)
		bool blit(grect srcRect, gvec2 destPosition, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat, unsigned char alpha = 255);
		/// @brief Does an image data block transfer onto the image.
		/// @param[in] srcRect Source data rectangle.
		/// @param[in] destPosition Destination coordinates.
		/// @param[in] other The source Image.
		/// @param[in] alpha Alpha multiplier on the entire source image.
		/// @return True if successful.
		/// @note Pixels on the destination will be overwritten will be blended with alpha-blending using the source pixels.
		/// @note The parameter alpha is especially useful when blitting source images that don't have an alpha channel.
		/// @see blit(int sx, int sy, int sw, int sh, int dx, int dy, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat, unsigned char alpha)
		bool blit(grect srcRect, gvec2 destPosition, Image* other, unsigned char alpha = 255);
		/// @brief Does a stretched image data block transfer onto the image.
		/// @param[in] sx Source data X-coordinate.
		/// @param[in] sy Source data Y-coordinate.
		/// @param[in] sw Width of the area on the source to be copied.
		/// @param[in] sh Height of the area on the source to be copied.
		/// @param[in] dx Destination X-coordinate.
		/// @param[in] dy Destination Y-coordinate.
		/// @param[in] dw Width of the destination area.
		/// @param[in] dh Height of the destination area.
		/// @param[in] other The source Image.
		/// @param[in] alpha Alpha multiplier on the entire source image.
		/// @return True if successful.
		/// @note Pixels on the destination will be overwritten will be blended with alpha-blending using the source pixels.
		/// @note The parameter alpha is especially useful when blitting source images that don't have an alpha channel.
		/// @see blitStretch(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat, unsigned char alpha)
		bool blitStretch(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, Image* other, unsigned char alpha = 255);
		/// @brief Does a stretched image data block transfer onto the image.
		/// @param[in] srcRect Source data rectangle.
		/// @param[in] destRect Destination rectangle.
		/// @param[in] srcData The source raw image data.
		/// @param[in] srcWidth The width of source raw image data.
		/// @param[in] srcHeight The height of source raw image data.
		/// @param[in] srcFormat The pixel format of source raw image data.
		/// @param[in] alpha Alpha multiplier on the entire source image.
		/// @return True if successful.
		/// @note Pixels on the destination will be overwritten will be blended with alpha-blending using the source pixels.
		/// @note The parameter alpha is especially useful when blitting source images that don't have an alpha channel.
		/// @see blitStretch(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat, unsigned char alpha)
		bool blitStretch(grect srcRect, grect destRect, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat, unsigned char alpha = 255);
		/// @brief Does a stretched image data block transfer onto the image.
		/// @param[in] srcRect Source data rectangle.
		/// @param[in] destRect Destination rectangle.
		/// @param[in] other The source Image.
		/// @param[in] alpha Alpha multiplier on the entire source image.
		/// @return True if successful.
		/// @note Pixels on the destination will be overwritten will be blended with alpha-blending using the source pixels.
		/// @note The parameter alpha is especially useful when blitting source images that don't have an alpha channel.
		/// @see blitStretch(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat, unsigned char alpha)
		bool blitStretch(grect srcRect, grect destRect, Image* other, unsigned char alpha = 255);
		/// @brief Rotates the pixel hue of a rectangle area on the image.
		/// @param[in] rect Rectangle area.
		/// @param[in] degrees By how many degrees the the should be rotated.
		/// @return True if successful.
		/// @note This is lossy operation.
		/// @note This is an expensive operation and should be used sparingly.
		bool rotateHue(grect rect, float degrees);
		/// @brief Changes the saturation level of pixels of a rectangle area on the image.
		/// @param[in] rect Rectangle area.
		/// @param[in] factor The saturation multiplier factor.
		/// @return True if successful.
		/// @note This is lossy operation.
		/// @note This is an expensive operation and should be used sparingly.
		bool saturate(grect rect, float factor);
		/// @brief Inverts the pixel colors of a rectangle area on the image.
		/// @param[in] rect Rectangle area.
		/// @return True if successful.
		bool invert(grect rect);
		/// @brief Inserts image data as alpha channel into this image.
		/// @param[in] srcData The source raw image data.
		/// @param[in] srcFormat The pixel format of source raw image data.
		/// @return True if successful.
		/// @note The image data in srcData must be the same width and height as the image.
		/// @note This is an expensive operation and should be used sparingly.
		bool insertAlphaMap(unsigned char* srcData, Format srcFormat); // TODOa - this functionality might be removed since shaders are much faster
		/// @brief Inserts image data as alpha channel into this image.
		/// @param[in] image The source Image.
		/// @param[in] median The median value for insertion.
		/// @param[in] ambiguity How "hard" the alpha channel transition should be.
		/// @return True if successful.
		/// @note The pixel format of image must be the same this image's.
		/// @note This is an expensive operation and should be used sparingly.
		bool insertAlphaMap(Image* image, unsigned char median, int ambiguity); // TODOa - this functionality might be removed since shaders are much faster
		/// @brief Inserts image data as alpha channel into this image.
		/// @param[in] image The source Image.
		/// @return True if successful.
		/// @note The pixel format of image must be the same this image's.
		/// @note This is an expensive operation and should be used sparingly.
		bool insertAlphaMap(Image* image); // TODOa - this functionality might be removed since shaders are much faster
		/// @brief Dilates the image.
		/// @param[in] image The source Image.
		/// @return True if successful.
		/// @note This is an expensive operation and should be used sparingly.
		/// @note Currently this operation is only supported for single-channel 8-bit images.
		bool dilate(Image* image);

		/// @brief Creates an Image object from a resource file.
		/// @param[in] filename Filename of the resource file.
		/// @return The loaded Image object or NULL if failed.
		static Image* createFromResource(chstr filename);
		/// @brief Creates an Image object from a resource file.
		/// @param[in] filename Filename of the resource file.
		/// @param[in] format Convert to a certain pixel format during load.
		/// @return The loaded Image object or NULL if failed.
		static Image* createFromResource(chstr filename, Format format);
		/// @brief Creates an Image object from a file.
		/// @param[in] filename Filename of the file.
		/// @return The loaded Image object or NULL if failed.
		static Image* createFromFile(chstr filename);
		/// @brief Creates an Image object from a file.
		/// @param[in] filename Filename of the file.
		/// @param[in] format Convert to a certain pixel format during load.
		/// @return The loaded Image object or NULL if failed.
		static Image* createFromFile(chstr filename, Format format);
		/// @brief Creates an Image object from a data stream.
		/// @param[in] stream Data stream containing the compressed image data.
		/// @param[in] logicalExtension The logical extension of the loaded stream so the method knows what data is contained in the stream.
		/// @return The loaded Image object or NULL if failed.
		/// @note This is usually called internally only.
		static Image* createFromStream(hsbase& stream, chstr logicalExtension);
		/// @brief Creates an Image object from a data stream.
		/// @param[in] stream Data stream containing the compressed image data.
		/// @param[in] logicalExtension The logical extension of the loaded stream so the method knows what data is contained in the stream.
		/// @param[in] format Convert to a certain pixel format during load.
		/// @return The loaded Image object or NULL if failed.
		static Image* createFromStream(hsbase& stream, chstr logicalExtension, Format format);
		/// @brief Creates an Image object from a raw image data.
		/// @param[in] w Width of the image data.
		/// @param[in] h Height of the image data.
		/// @param[in] data The raw image data.
		/// @param[in] format Pixel format of the image data.
		/// @return The loaded Image object or NULL if failed.
		static Image* create(int w, int h, unsigned char* data, Format format);
		/// @brief Creates an Image object filled with a single color.
		/// @param[in] w Width of the image data.
		/// @param[in] h Height of the image data.
		/// @param[in] color Color to fill the image.
		/// @param[in] format Pixel format for the new Image.
		/// @return The loaded Image object or NULL if failed.
		static Image* create(int w, int h, Color color, Format format);
		/// @brief Creates a deep copy of the Image object.
		/// @param[in] other The other Image.
		/// @return The loaded Image object or NULL if failed.
		static Image* create(Image* other);
		/// @brief Saves the image data in a certain file format.
		/// @param[in] image The Image to be saved.
		/// @param[in] filename The filename.
		/// @param[in] format The file format of the file.
		/// @return True if successful.
		static bool save(Image* image, chstr filename, FileFormat format);
		/// @brief Creates an Image without image data, but with meta-data from a resource file.
		/// @param[in] filename The filename of the resource file.
		/// @return The loaded Image object or NULL if failed.
		static Image* readMetaDataFromResource(chstr filename);
		/// @brief Creates an Image without image data, but with meta-data from a file.
		/// @param[in] filename The filename of the resource file.
		/// @return The loaded Image object or NULL if failed.
		static Image* readMetaDataFromFile(chstr filename);
		/// @brief Creates an Image without image data, but with meta-data from a data stream.
		/// @param[in] stream Data stream containing the compressed image data.
		/// @param[in] logicalExtension The logical extension of the loaded stream so the method knows what data is contained in the stream.
		/// @return The loaded Image object or NULL if failed.
		/// @note This is usually called internally only.
		static Image* readMetaDataFromStream(hsbase& stream, chstr logicalExtension);

		/// @brief Gets the byte-per-pixel value for a given pixel format.
		/// @param[in] format The pixel format.
		/// @return The byte-per-pixel value.
		static int getFormatBpp(Format format);

		/// @brief Gets the color of a specific pixel in raw image data.
		/// @param[in] x X-coordinate.
		/// @param[in] y Y-coordinate.
		/// @param[in] srcData The source raw image data.
		/// @param[in] srcWidth The width of source raw image data.
		/// @param[in] srcHeight The height of source raw image data.
		/// @param[in] srcFormat The pixel format of source raw image data.
		/// @return The Color of the pixel.
		/// @note This is usually called internally only.
		static Color getPixel(int x, int y, unsigned char* srcData, int srcWidth, int srcHeight, Format srcFormat);
		/// @brief Sets the color of a specific pixel in raw image data.
		/// @param[in] x X-coordinate.
		/// @param[in] y Y-coordinate.
		/// @param[in] color The new Color of the pixel.
		/// @param[in,out] destData The destination raw image data.
		/// @param[in] destWidth The width of destination raw image data.
		/// @param[in] destHeight The height of destination raw image data.
		/// @param[in] destFormat The pixel format of destination raw image data.
		/// @return True if successful.
		/// @note This is usually called internally only.
		static bool setPixel(int x, int y, Color color, unsigned char* destData, int destWidth, int destHeight, Format destFormat);
		/// @brief Gets the linearly interpolated color between two pixels in raw image data.
		/// @param[in] x Decimal X-coordinate.
		/// @param[in] y Decimal Y-coordinate.
		/// @param[in] srcData The source raw image data.
		/// @param[in] srcWidth The width of source raw image data.
		/// @param[in] srcHeight The height of source raw image data.
		/// @param[in] srcFormat The pixel format of source raw image data.
		/// @return The interpolated Color of the pixel.
		/// @note This is usually called internally only.
		static Color getInterpolatedPixel(float x, float y, unsigned char* srcData, int srcWidth, int srcHeight, Format srcFormat);
		/// @brief Fills a rectangle area with one color in raw image data.
		/// @param[in] x X-coordinate.
		/// @param[in] y Y-coordinate.
		/// @param[in] w Width of the area.
		/// @param[in] h Height of the area.
		/// @param[in] color The Color used for filling.
		/// @param[in,out] destData The destination raw image data.
		/// @param[in] destWidth The width of destination raw image data.
		/// @param[in] destHeight The height of destination raw image data.
		/// @param[in] destFormat The pixel format of destination raw image data.
		/// @return True if successful.
		/// @note This is usually called internally only.
		static bool fillRect(int x, int y, int w, int h, Color color, unsigned char* destData, int destWidth, int destHeight, Format destFormat);
		/// @brief Writes image data directly onto raw image data.
		/// @param[in] sx Source data X-coordinate.
		/// @param[in] sy Source data Y-coordinate.
		/// @param[in] sw Width of the area on the source to be copied.
		/// @param[in] sh Height of the area on the source to be copied.
		/// @param[in] dx Destination X-coordinate.
		/// @param[in] dy Destination Y-coordinate.
		/// @param[in] srcData The source raw image data.
		/// @param[in] srcWidth The width of source raw image data.
		/// @param[in] srcHeight The height of source raw image data.
		/// @param[in] srcFormat The pixel format of source raw image data.
		/// @param[in,out] destData The destination raw image data.
		/// @param[in] destWidth The width of destination raw image data.
		/// @param[in] destHeight The height of destination raw image data.
		/// @param[in] destFormat The pixel format of destination raw image data.
		/// @return True if successful.
		/// @note Pixels on the destination will be overwritten.
		/// @note This is usually called internally only.
		static bool write(int sx, int sy, int sw, int sh, int dx, int dy, unsigned char* srcData, int srcWidth, int srcHeight, Format srcFormat, unsigned char* destData, int destWidth, int destHeight, Format destFormat);
		/// @brief Writes image data directly onto the raw image data while trying to stretch the pixels. Stretched pixels will be linearly interpolated.
		/// @param[in] sx Source data X-coordinate.
		/// @param[in] sy Source data Y-coordinate.
		/// @param[in] sw Width of the area on the source to be copied.
		/// @param[in] sh Height of the area on the source to be copied.
		/// @param[in] dx Destination X-coordinate.
		/// @param[in] dy Destination Y-coordinate.
		/// @param[in] dw Width of the destination area.
		/// @param[in] dh Height of the destination area.
		/// @param[in] srcData The source raw image data.
		/// @param[in] srcWidth The width of source raw image data.
		/// @param[in] srcHeight The height of source raw image data.
		/// @param[in] srcFormat The pixel format of source raw image data.
		/// @param[in,out] destData The destination raw image data.
		/// @param[in] destWidth The width of destination raw image data.
		/// @param[in] destHeight The height of destination raw image data.
		/// @param[in] destFormat The pixel format of destination raw image data.
		/// @return True if successful.
		/// @note Pixels on the destination will be overwritten.
		/// @note This is usually called internally only.
		static bool writeStretch(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat, unsigned char* destData, int destWidth, int destHeight, Format destFormat);
		/// @brief Does an image data block transfer onto the raw image data.
		/// @param[in] sx Source data X-coordinate.
		/// @param[in] sy Source data Y-coordinate.
		/// @param[in] sw Width of the area on the source to be copied.
		/// @param[in] sh Height of the area on the source to be copied.
		/// @param[in] dx Destination X-coordinate.
		/// @param[in] dy Destination Y-coordinate.
		/// @param[in] srcData The source raw image data.
		/// @param[in] srcWidth The width of source raw image data.
		/// @param[in] srcHeight The height of source raw image data.
		/// @param[in] srcFormat The pixel format of source raw image data.
		/// @param[in,out] destData The destination raw image data.
		/// @param[in] destWidth The width of destination raw image data.
		/// @param[in] destHeight The height of destination raw image data.
		/// @param[in] destFormat The pixel format of destination raw image data.
		/// @param[in] alpha Alpha multiplier on the entire source image.
		/// @return True if successful.
		/// @note Pixels on the destination will be overwritten will be blended with alpha-blending using the source pixels.
		/// @note The parameter alpha is especially useful when blitting source images that don't have an alpha channel.
		/// @note This is usually called internally only.
		static bool blit(int sx, int sy, int sw, int sh, int dx, int dy, unsigned char* srcData, int srcWidth, int srcHeight, Format srcFormat, unsigned char* destData, int destWidth, int destHeight, Format destFormat, unsigned char alpha = 255);
		/// @brief Does a stretched image data block transfer onto the raw image data.
		/// @param[in] sx Source data X-coordinate.
		/// @param[in] sy Source data Y-coordinate.
		/// @param[in] sw Width of the area on the source to be copied.
		/// @param[in] sh Height of the area on the source to be copied.
		/// @param[in] dx Destination X-coordinate.
		/// @param[in] dy Destination Y-coordinate.
		/// @param[in] dw Width of the destination area.
		/// @param[in] dh Height of the destination area.
		/// @param[in] srcData The source raw image data.
		/// @param[in] srcWidth The width of source raw image data.
		/// @param[in] srcHeight The height of source raw image data.
		/// @param[in] srcFormat The pixel format of source raw image data.
		/// @param[in,out] destData The destination raw image data.
		/// @param[in] destWidth The width of destination raw image data.
		/// @param[in] destHeight The height of destination raw image data.
		/// @param[in] destFormat The pixel format of destination raw image data.
		/// @param[in] alpha Alpha multiplier on the entire source image.
		/// @return True if successful.
		/// @note Pixels on the destination will be overwritten will be blended with alpha-blending using the source pixels.
		/// @note The parameter alpha is especially useful when blitting source images that don't have an alpha channel.
		/// @note This is usually called internally only.
		static bool blitStretch(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat, unsigned char* destData, int destWidth, int destHeight, Format destFormat, unsigned char alpha = 255);
		/// @brief Rotates the pixel hue of a rectangle area on the raw image data.
		/// @param[in] x X-coordinate of the area to change.
		/// @param[in] y Y-coordinate of the area to change.
		/// @param[in] w Width of the area to change.
		/// @param[in] h Height of the area to change.
		/// @param[in] srcData The source raw image data.
		/// @param[in] srcWidth The width of source raw image data.
		/// @param[in] srcHeight The height of source raw image data.
		/// @param[in] srcFormat The pixel format of source raw image data.
		/// @param[in] degrees By how many degrees the the should be rotated.
		/// @return True if successful.
		/// @note This is lossy operation.
		/// @note This is an expensive operation and should be used sparingly.
		/// @note This is usually called internally only.
		static bool rotateHue(int x, int y, int w, int h, float degrees, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat);
		/// @brief Changes the saturation level of pixels of a rectangle area on the raw image data.
		/// @param[in] x X-coordinate of the area to change.
		/// @param[in] y Y-coordinate of the area to change.
		/// @param[in] w Width of the area to change.
		/// @param[in] h Height of the area to change.
		/// @param[in] srcData The source raw image data.
		/// @param[in] srcWidth The width of source raw image data.
		/// @param[in] srcHeight The height of source raw image data.
		/// @param[in] srcFormat The pixel format of source raw image data.
		/// @param[in] factor The saturation multiplier factor.
		/// @return True if successful.
		/// @note This is lossy operation.
		/// @note This is an expensive operation and should be used sparingly.
		/// @note This is usually called internally only.
		static bool saturate(int x, int y, int w, int h, float factor, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat);
		/// @brief Inverts the pixel colors of a rectangle area on the raw image data.
		/// @param[in] x X-coordinate of the area to change.
		/// @param[in] y Y-coordinate of the area to change.
		/// @param[in] w Width of the area to change.
		/// @param[in] h Height of the area to change.
		/// @param[in] srcData The source raw image data.
		/// @param[in] srcWidth The width of source raw image data.
		/// @param[in] srcHeight The height of source raw image data.
		/// @param[in] srcFormat The pixel format of source raw image data.
		/// @return True if successful.
		/// @note This is usually called internally only.
		static bool invert(int x, int y, int w, int h, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat);
		/// @brief Inserts raw image data as alpha channel into other raw image data.
		/// @param[in] w Width of the raw image data.
		/// @param[in] h Height of the raw image data.
		/// @param[in] srcData The source raw image data.
		/// @param[in] srcFormat The pixel format of source raw image data.
		/// @param[in,out] destData The destination raw image data.
		/// @param[in] destFormat The pixel format of destination raw image data.
		/// @param[in] median The median value for insertion.
		/// @param[in] ambiguity How "hard" the alpha channel transition should be.
		/// @return True if successful.
		/// @note The image data in srcData must be the same width and height as the image.
		/// @note This is an expensive operation and should be used sparingly.
		/// @note This is usually called internally only.
		static bool insertAlphaMap(int w, int h, unsigned char* srcData, Format srcFormat, unsigned char* destData, Format destFormat, unsigned char median, int ambiguity); // TODOa - this functionality might be removed since shaders are much faster
		/// @brief Dilates the raw image data.
		/// @param[in] srcData The constructing image's raw image data.
		/// @param[in] srcWidth The width of the constructing image's raw image data.
		/// @param[in] srcHeight The height of the constructing image's raw image data.
		/// @param[in] srcFormat The pixel format of the constructing image's raw image data.
		/// @param[in,out] destData The destination raw image data.
		/// @param[in] destWidth The width of destination raw image data.
		/// @param[in] destHeight The height of destination raw image data.
		/// @param[in] destFormat The pixel format of destination raw image data.
		/// @return True if successful.
		/// @note This is an expensive operation and should be used sparingly.
		/// @note Currently this operation is only supported for single-channel 8-bit images.
		/// @note This is usually called internally only.
		static bool dilate(unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat, unsigned char* destData, int destWidth, int destHeight, Image::Format destFormat);

		/// @brief Converts raw image data from one format into another.
		/// @param[in] w Width of the raw image data.
		/// @param[in] h Height of the raw image data.
		/// @param[in] srcData The source raw image data.
		/// @param[in] srcFormat The pixel format of source raw image data.
		/// @param[in,out] destData The destination raw image data.
		/// @param[in] destFormat The pixel format of destination raw image data.
		/// @param[in] preventCopy If true, it will make a copy even if source and destination formats are the same.
		/// @return True if successful.
		/// @note If destData has not been allocated yet, it will be allocated with the new operator.
		/// @note Data loss can occur when converting from a format that contains more pixel information into a format with less pixel information.
		static bool convertToFormat(int w, int h, unsigned char* srcData, Format srcFormat, unsigned char** destData, Format destFormat, bool preventCopy = true);
		/// @brief Checks if an image format conversion is needed.
		/// @param[in] srcFormat The pixel format of source raw image data.
		/// @param[in] destFormat The pixel format of destination raw image data.
		/// @param[in] preventCopy If true, it will return false if source and destination formats are the same.
		/// @return True if successful.
		/// @note Helps to determine whether there is a need to convert an image format into another. It can be helpful to avoid conversions from e.g. RGBA to RGBX if the GPU ignores the X anyway.
		static bool needsConversion(Format srcFormat, Format destFormat, bool preventCopy = true);
		
		/// @brief Checks if the coordinates are out of bounds.
		/// @param[in] x Position X-coordinate.
		/// @param[in] y Position Y-coordinate.
		/// @param[in] destWidth Destination image data width.
		/// @param[in] destHeight Destination image data height.
		/// @return True if it is in bounds.
		static bool checkRect(int x, int y, int destWidth, int destHeight);
		/// @brief Checks if the rect is out of bounds.
		/// @param[in] x Rectangle X-coordinate.
		/// @param[in] y Rectangle Y-coordinate.
		/// @param[in] w Rectangle width.
		/// @param[in] h Rectangle height.
		/// @param[in] destWidth Destination image data width.
		/// @param[in] destHeight Destination image data height.
		/// @return True if it is in bounds.
		static bool checkRect(int x, int y, int w, int h, int destWidth, int destHeight);
		/// @brief Corrects rectangle to stay in bounds of the image data.
		/// @param[in,out] x Rectangle X-coordinate.
		/// @param[in,out] y Rectangle Y-coordinate.
		/// @param[in,out] w Rectangle width.
		/// @param[in,out] h Rectangle height.
		/// @param[in] dataWidth Image data width.
		/// @param[in] dataHeight Image data height.
		/// @return True if correction could be made successfully or no correction was required. If false is returned, the values are not guaranteed to be unmodified.
		/// @note This is usually called internally only.
		static bool correctRect(int& x, int& y, int& w, int& h, int dataWidth, int dataHeight);
		/// @brief Corrects the source rectangle and destination coordinates to stay in bounds of the source and destination image data.
		/// @param[in,out] sx Source rectangle X-coordinate.
		/// @param[in,out] sy Source rectangle Y-coordinate.
		/// @param[in,out] sw Source rectangle width.
		/// @param[in,out] sh Source rectangle height.
		/// @param[in] srcWidth Source image data width.
		/// @param[in] srcHeight Source image data height.
		/// @param[in,out] dx Destination position X-coordinate.
		/// @param[in,out] dy Destination position Y-coordinate.
		/// @param[in] destWidth Destination image data width.
		/// @param[in] destHeight Destination image data height.
		/// @return True if correction could be made successfully or no correction was required. If false is returned, the values are not guaranteed to be unmodified.
		/// @note This is usually called internally only.
		static bool correctRect(int& sx, int& sy, int& sw, int& sh, int srcWidth, int srcHeight, int& dx, int& dy, int destWidth, int destHeight);
		/// @brief Corrects source and destination rectangles to stay in bounds of the source and destination image data.
		/// @param[in,out] sx Source rectangle X-coordinate.
		/// @param[in,out] sy Source rectangle Y-coordinate.
		/// @param[in,out] sw Source rectangle width.
		/// @param[in,out] sh Source rectangle height.
		/// @param[in] srcWidth Source image data width.
		/// @param[in] srcHeight Source image data height.
		/// @param[in,out] dx Destination rectangle X-coordinate.
		/// @param[in,out] dy Destination rectangle Y-coordinate.
		/// @param[in,out] dw Destination rectangle width.
		/// @param[in,out] dh Destination rectangle height.
		/// @param[in] destWidth Destination image data width.
		/// @param[in] destHeight Destination image data height.
		/// @return True if correction could be made successfully or no correction was required. If false is returned, the values are not guaranteed to be unmodified.
		/// @note This is usually called internally only.
		static bool correctRect(int& sx, int& sy, int& sw, int& sh, int srcWidth, int srcHeight, int& dx, int& dy, int& dw, int& dh, int destWidth, int destHeight);

		/// @brief Registers a custom image loader for custom image formats.
		/// @param[in] extension Filename extension.
		/// @param[in] loadFunction The function pointer to use for loading the Image.
		/// @param[in] metaDataLoadfunction The function pointer to use for loading the Image meta-data.
		/// @note The loading function will only be triggered if the extension is added with april::setTextureExtensions as well.
		/// @see setTextureExtensions
		static void registerCustomLoader(chstr extension, Image* (*loadFunction)(hsbase&), Image* (*metaDataLoadfunction)(hsbase&));

	protected:
		/// @brief Basic constructor.
		Image();
		/// @brief Copy constructor.
		/// @param[in] other Other Image object.
		Image(const Image& other);

		/// @brief Custom image format loaders.
		static hmap<hstr, Image* (*)(hsbase&)> customLoaders;
		/// @brief Custom image format meta data loaders.
		static hmap<hstr, Image* (*)(hsbase&)> customMetaDataLoaders;

		/// @brief Loads and decodes PNG file data.
		/// @param[in] stream The encoded image data stream.
		/// @param[in] size The size within the data stream that actually belongs to this encoded file.
		/// @return The created Image object or NULL if failed.
		static Image* _loadPng(hsbase& stream, int size);
		/// @brief Loads and decodes PNG file data.
		/// @param[in] stream The encoded image data stream.
		/// @return The created Image object or NULL if failed.
		static Image* _loadPng(hsbase& stream);
		/// @brief Loads and decodes JPG file data.
		/// @param[in] stream The encoded image data stream.
		/// @param[in] size The size within the data stream that actually belongs to this encoded file.
		/// @return The created Image object or NULL if failed.
		static Image* _loadJpg(hsbase& stream, int size);
		/// @brief Loads and decodes JPG file data.
		/// @param[in] stream The encoded image data stream.
		/// @return The created Image object or NULL if failed.
		static Image* _loadJpg(hsbase& stream);
		/// @brief Loads and decodes JPT file data.
		/// @param[in] stream The encoded image data stream.
		/// @return The created Image object or NULL if failed.
		static Image* _loadJpt(hsbase& stream);
		/// @brief Loads and decodes PVR file data.
		/// @param[in] stream The encoded image data stream.
		/// @return The created Image object or NULL if failed.
		static Image* _loadPvr(hsbase& stream);
		/// @brief Saves image data into a stream encoded as PNG file.
		/// @param[in,out] stream The destination image data stream.
		/// @param[in] image The Image object to save.
		/// @return True if successful.
		static bool _savePng(hsbase& stream, Image* image);
		/// @brief Loads and decodes meta data from PNG file data.
		/// @param[in] stream The encoded image data stream.
		/// @param[in] size The size within the data stream that actually belongs to this encoded file.
		/// @return The created Image object or NULL if failed.
		static Image* _readMetaDataPng(hsbase& stream, int size);
		/// @brief Loads and decodes meta data from PNG file data.
		/// @param[in] stream The encoded image data stream.
		/// @return The created Image object or NULL if failed.
		static Image* _readMetaDataPng(hsbase& stream);
		/// @brief Loads and decodes meta data from JPG file data.
		/// @param[in] stream The encoded image data stream.
		/// @param[in] size The size within the data stream that actually belongs to this encoded file.
		/// @return The created Image object or NULL if failed.
		static Image* _readMetaDataJpg(hsbase& stream, int size);
		/// @brief Loads and decodes meta data from JPG file data.
		/// @param[in] stream The encoded image data stream.
		/// @return The created Image object or NULL if failed.
		static Image* _readMetaDataJpg(hsbase& stream);
		/// @brief Loads and decodes meta data from JPT file data.
		/// @param[in] stream The encoded image data stream.
		/// @return The created Image object or NULL if failed.
		static Image* _readMetaDataJpt(hsbase& stream);
		/// @brief Loads and decodes meta data from PVR file data.
		/// @param[in] stream The encoded image data stream.
		/// @return The created Image object or NULL if failed.
		static Image* _readMetaDataPvr(hsbase& stream);

		/// @brief Gets the color channel byte incides for pixel format data.
		/// @param[in] format The format to check.
		/// @param[out] red The index of the red color channel.
		/// @param[out] green The index of the green color channel.
		/// @param[out] blue The index of the blue color channel.
		/// @param[out] alpha The index of the alpha channel.
		static void _getFormatIndices(Format format, int* red, int* green, int* blue, int* alpha);

		/// @brief Converts raw image data from a source pixel format with 1 byte-per-pixel to a raw image data destination.
		/// @param[in] w Width of the data to convert.
		/// @param[in] h Height of the data to convert.
		/// @param[in] srcData The source raw image data.
		/// @param[in] srcFormat The pixel format of source raw image data.
		/// @param[in,out] destData The destination raw image data.
		/// @param[in] destFormat The pixel format of destination raw image data.
		/// @return True if successful.
		/// @note This is a utility method called by convertToFormat to split a large method into a few smaller ones.
		/// @see convertToFormat
		static bool _convertFrom1Bpp(int w, int h, unsigned char* srcData, Format srcFormat, unsigned char** destData, Format destFormat);
		/// @brief Converts raw image data from a source pixel format with 3 byte-per-pixel to a raw image data destination.
		/// @param[in] w Width of the data to convert.
		/// @param[in] h Height of the data to convert.
		/// @param[in] srcData The source raw image data.
		/// @param[in] srcFormat The pixel format of source raw image data.
		/// @param[in,out] destData The destination raw image data.
		/// @param[in] destFormat The pixel format of destination raw image data.
		/// @return True if successful.
		/// @note This is a utility method called by convertToFormat to split a large method into a few smaller ones.
		/// @see convertToFormat
		static bool _convertFrom3Bpp(int w, int h, unsigned char* srcData, Format srcFormat, unsigned char** destData, Format destFormat);
		/// @brief Converts raw image data from a source pixel format with 4 byte-per-pixel to a raw image data destination.
		/// @param[in] w Width of the data to convert.
		/// @param[in] h Height of the data to convert.
		/// @param[in] srcData The source raw image data.
		/// @param[in] srcFormat The pixel format of source raw image data.
		/// @param[in,out] destData The destination raw image data.
		/// @param[in] destFormat The pixel format of destination raw image data.
		/// @return True if successful.
		/// @note This is a utility method called by convertToFormat to split a large method into a few smaller ones.
		/// @see convertToFormat
		static bool _convertFrom4Bpp(int w, int h, unsigned char* srcData, Format srcFormat, unsigned char** destData, Format destFormat);

		/// @brief Executes a raw image data block transfer from a source pixel format with 1 byte-per-pixel to a raw image data destination.
		/// @param[in] sx Source data X-coordinate.
		/// @param[in] sy Source data Y-coordinate.
		/// @param[in] sw Width of the area on the source to be copied.
		/// @param[in] sh Height of the area on the source to be copied.
		/// @param[in] dx Destination X-coordinate.
		/// @param[in] dy Destination Y-coordinate.
		/// @param[in] srcData The source raw image data.
		/// @param[in] srcWidth The width of source raw image data.
		/// @param[in] srcHeight The height of source raw image data.
		/// @param[in] srcFormat The pixel format of source raw image data.
		/// @param[in,out] destData The destination raw image data.
		/// @param[in] destWidth The width of destination raw image data.
		/// @param[in] destHeight The height of destination raw image data.
		/// @param[in] destFormat The pixel format of destination raw image data.
		/// @param[in] alpha Alpha multiplier on the entire source image.
		/// @return True if successful.
		/// @note Pixels on the destination will be overwritten will be blended with alpha-blending using the source pixels.
		/// @note The parameter alpha is especially useful when blitting source images that don't have an alpha channel.
		/// @note This is a utility method called by convertToFormat to split a large method into a few smaller ones.
		/// @see blit
		static bool _blitFrom1Bpp(int sx, int sy, int sw, int sh, int dx, int dy, unsigned char* srcData, int srcWidth, int srcHeight, Format srcFormat, unsigned char* destData, int destWidth, int destHeight, Format destFormat, unsigned char alpha);
		/// @brief Executes a raw image data block transfer from a source pixel format with 3 byte-per-pixel to a raw image data destination.
		/// @param[in] sx Source data X-coordinate.
		/// @param[in] sy Source data Y-coordinate.
		/// @param[in] sw Width of the area on the source to be copied.
		/// @param[in] sh Height of the area on the source to be copied.
		/// @param[in] dx Destination X-coordinate.
		/// @param[in] dy Destination Y-coordinate.
		/// @param[in] srcData The source raw image data.
		/// @param[in] srcWidth The width of source raw image data.
		/// @param[in] srcHeight The height of source raw image data.
		/// @param[in] srcFormat The pixel format of source raw image data.
		/// @param[in,out] destData The destination raw image data.
		/// @param[in] destWidth The width of destination raw image data.
		/// @param[in] destHeight The height of destination raw image data.
		/// @param[in] destFormat The pixel format of destination raw image data.
		/// @param[in] alpha Alpha multiplier on the entire source image.
		/// @return True if successful.
		/// @note Pixels on the destination will be overwritten will be blended with alpha-blending using the source pixels.
		/// @note The parameter alpha is especially useful when blitting source images that don't have an alpha channel.
		/// @note This is a utility method called by convertToFormat to split a large method into a few smaller ones.
		/// @see blit
		static bool _blitFrom3Bpp(int sx, int sy, int sw, int sh, int dx, int dy, unsigned char* srcData, int srcWidth, int srcHeight, Format srcFormat, unsigned char* destData, int destWidth, int destHeight, Format destFormat, unsigned char alpha);
		/// @brief Executes a raw image data block transfer from a source pixel format with 4 byte-per-pixel to a raw image data destination.
		/// @param[in] sx Source data X-coordinate.
		/// @param[in] sy Source data Y-coordinate.
		/// @param[in] sw Width of the area on the source to be copied.
		/// @param[in] sh Height of the area on the source to be copied.
		/// @param[in] dx Destination X-coordinate.
		/// @param[in] dy Destination Y-coordinate.
		/// @param[in] srcData The source raw image data.
		/// @param[in] srcWidth The width of source raw image data.
		/// @param[in] srcHeight The height of source raw image data.
		/// @param[in] srcFormat The pixel format of source raw image data.
		/// @param[in,out] destData The destination raw image data.
		/// @param[in] destWidth The width of destination raw image data.
		/// @param[in] destHeight The height of destination raw image data.
		/// @param[in] destFormat The pixel format of destination raw image data.
		/// @param[in] alpha Alpha multiplier on the entire source image.
		/// @return True if successful.
		/// @note Pixels on the destination will be overwritten will be blended with alpha-blending using the source pixels.
		/// @note The parameter alpha is especially useful when blitting source images that don't have an alpha channel.
		/// @note This is a utility method called by convertToFormat to split a large method into a few smaller ones.
		/// @see blit
		static bool _blitFrom4Bpp(int sx, int sy, int sw, int sh, int dx, int dy, unsigned char* srcData, int srcWidth, int srcHeight, Format srcFormat, unsigned char* destData, int destWidth, int destHeight, Format destFormat, unsigned char alpha);

	};
	
}

#endif
