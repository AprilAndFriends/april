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
/// Defines a generic texture.

#ifndef APRIL_TEXTURE_H
#define APRIL_TEXTURE_H

#include <gtypes/Rectangle.h>
#include <gtypes/Vector2.h>
#include <hltypes/harray.h>
#include <hltypes/henum.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hmutex.h>
#include <hltypes/hstring.h>
#include <hltypes/hstream.h>

#include "aprilExport.h"
#include "Color.h"
#include "Image.h"

namespace april
{
	class DestroyTextureCommand;
	class Image;
	class RenderSystem;
	class ResetCommand;
	class TextureAsync;
	class UnloadTextureCommand;
	
	/// @brief Defines a generic texture.
	class aprilExport Texture
	{
	public:
		friend class DestroyTextureCommand;
		friend class RenderSystem;
		friend class ResetCommand;
		friend class TextureAsync;
		friend class UnloadTextureCommand;

		/// @class Type
		/// @brief Defines texture types in order to control their behavior and certain features.
		HL_ENUM_CLASS_PREFIX_DECLARE(aprilExport, Type,
		(
			/// @var static const Type Type::Immutable
			/// @brief Cannot be modified or read. Texture with manual data will have a copy of the data in RAM, files will be reloaded from persistent memory.
			HL_ENUM_DECLARE(Type, Immutable);
			/// @var static const Type Type::Managed
			/// @brief Resides in RAM and on GPU, can be modified. Best used for manually created textures or loaded from files which will be modified.
			HL_ENUM_DECLARE(Type, Managed);
			/// @var static const Type Type::External
			/// @brief Special external texture, usually used for external feeds of data.
			/// @note Only supported on certain render systems. When support is not available, will default to Managed.
			HL_ENUM_DECLARE(Type, External);
			/// @var static const Type Type::RenderTarget
			/// @brief Used as render target.
			HL_ENUM_DECLARE(Type, RenderTarget);
		));

		/// @class Filter
		/// @brief Defines texture filtering modes.
		HL_ENUM_CLASS_PREFIX_DECLARE(aprilExport, Filter,
		(
			/// @var static const Filter Filter::Nearest
			/// @brief Nearest neighbor.
			HL_ENUM_DECLARE(Filter, Nearest);
			/// @var static const Filter Filter::Linear
			/// @brief Linear interpolation.
			HL_ENUM_DECLARE(Filter, Linear);
		));

		/// @class AddressMode
		/// @brief Defines UV address mode.
		HL_ENUM_CLASS_PREFIX_DECLARE(aprilExport, AddressMode,
		(
			/// @var static const AddressMode AddressMode::Clamp
			/// @brief Clamping UV coordinates to the edge pixel.
			HL_ENUM_DECLARE(AddressMode, Clamp);
			/// @var static const AddressMode AddressMode::Wrap
			/// @brief Wrapping UV coordinates.
			HL_ENUM_DECLARE(AddressMode, Wrap);
		));

		/// @class LoadMode
		/// @brief Defines how and when textures should be loaded into VRAM.
		HL_ENUM_CLASS_PREFIX_DECLARE(aprilExport, LoadMode,
		(
			/// @var static const LoadMode LoadMode::Async
			/// @brief Loads the texture asynchronously right away and uploads the data to the GPU as soon as it is available before the next frame.
			/// @note If the texture is used before it loaded asynchronously, it cannot be uploaded to the GPU and will not be rendered properly.
			HL_ENUM_DECLARE(LoadMode, Async);
			/// @var static const LoadMode LoadMode::AsyncDeferredUpload
			/// @brief Loads the texture asynchronously right away, but it will upload the data to the GPU on the first use.
			/// @note If the texture is used before it loaded asynchronously, it cannot be uploaded to the GPU and will not be rendered properly.
			HL_ENUM_DECLARE(LoadMode, AsyncDeferredUpload);
			/// @var static const LoadMode LoadMode::OnDemand
			/// @brief Doesn't load the texture yet at all. It will be loaded and uploaded to the GPU on the first use.
			HL_ENUM_DECLARE(LoadMode, OnDemand);
		));

		/// @brief The texture filename if available.
		HL_DEFINE_GET(hstr, filename, Filename);
		/// @brief The texture's custom name.
		/// @note Does not require to be unique.
		HL_DEFINE_GETSET(hstr, name, Name);
		/// @brief The texture type.
		HL_DEFINE_GET(Type, type, Type);
		/// @brief The texture load mode.
		HL_DEFINE_GET(LoadMode, loadMode, LoadMode);
		/// @brief The texture's image data pixel format.
		HL_DEFINE_GET(Image::Format, format, Format);
		/// @brief The texture's filtering mode.
		HL_DEFINE_GETSET(Filter, filter, Filter);
		/// @brief The texture's UV coordinate address mode.
		HL_DEFINE_GETSET(AddressMode, addressMode, AddressMode);
		/// @brief Whether the texture is locked for raw image data manipulation.
		HL_DEFINE_IS(locked, Locked);
		/// @brief Whether the texture is dirty and needs to be reuploaded to the GPU.
		HL_DEFINE_IS(dirty, Dirty);
		/// @brief Whether the texture was loaded from a resource file or a normal file.
		HL_DEFINE_IS(fromResource, FromResource);
		/// @brief Gets the width of the texture in pixels.
		/// @return Width of the texture in pixels.
		int getWidth() const;
		/// @brief Gets the height of the texture in pixels.
		/// @return Height of the texture in pixels.
		int getHeight() const;
		/// @brief Gets the byte-per-pixel value.
		/// @return The byte-per-pixel value.
		/// @note e.g. a value of 3 means that one pixels uses 3 bytes.
		int getBpp() const;
		/// @brief Gets the size of the image data in bytes.
		/// @return Size of the image data in bytes.
		/// @note The calculation is basically "width * height * bpp".
		int getByteSize() const;
		/// @brief Gets the current VRAM consumption, RAM consumption and RAM consumption of the asynchronously loaded data.
		/// @return The current VRAM consumption, RAM consumption and RAM consumption of the asynchronously loaded data.
		/// @note This code is useful to check the full RAM consumption without requiring multiple mutex locks.
		int getCurrentAllRamSize();
		/// @brief Gets the current VRAM consumption.
		/// @return The current VRAM consumption.
		int getCurrentVRamSize();
		/// @brief Gets the current RAM consumption.
		/// @return The current RAM consumption.
		int getCurrentRamSize();
		/// @brief Gets the current RAM consumption of the asynchronously loaded data.
		/// @return The current RAM consumption of the asynchronously loaded data.
		int getCurrentAsyncRamSize();
		/// @brief Checks whether the texture has been uploaded to the GPU.
		/// @return True if the texture has been uploaded to the GPU.
		bool isUploaded();
		/// @brief Checks whether the texture is ready to be uploaded to the GPU.
		/// @return True if the texture is ready to be uploaded to the GPU.
		bool isReadyForUpload();
		/// @brief Checks whether the texture is waiting to be loaded asynchronously.
		/// @return True if the texture is waiting to be loaded asynchronously.
		bool isAsyncLoadQueued();
		/// @brief Checks whether TextureAsync needs to upload this texture to the GPU.
		/// @return True if TextureAsync needs to upload this texture to the GPU.
		bool isAsyncUploadQueued();
		/// @brief Checks whether the texture is completely unloaded.
		/// @return True if the texture is completely unloaded.
		/// @see isUploaded()
		/// @see isReadyForUpload()
		/// @see isAsyncLoadQueued()
		bool isUnloaded();
		/// @brief Gets the Texture's internal backend ID.
		/// @return The Texture's internal backend ID.
		virtual void* getBackendId() const = 0;

		/// @brief Loads the texture asynchronously.
		/// @return True if queueing was successful.
		/// @note Using a texture before it is uploaded to the GPU will cause undefined behavior. Always call load() before using it, because this ensures that it's loaded.
		bool loadAsync();
		/// @brief Loads the texture's meta data only.
		/// @return True if successful or already loaded.
		bool loadMetaData();
		/// @brief Unloads the texture from the GPU.
		/// @note When type is Type::Managed, the raw image data remains in RAM.
		void unload();
		/// @brief Waits for the texture to load asynchronously.
		/// @param[in] timeout How long to wait maximally in seconds.
		/// @note A timeout value of 0.0 means indefinitely.
		/// @note This should usually not be used externally. load() uses this internally when the texture was already queued to be loaded asynchronously.
		/// @see loadAsync
		void waitForAsyncLoad(float timeout = 0.0f);

		/// @brief Clears the entire image and sets all image data to zeroes.
		/// @return True if successful.
		bool clear();
		/// @brief Gets the color of a specific pixel.
		/// @param[in] x X-coordinate.
		/// @param[in] y Y-coordinate.
		/// @return The Color of the pixel.
		Color getPixel(int x, int y);
		/// @brief Sets the color of a specific pixel.
		/// @param[in] x X-coordinate.
		/// @param[in] y Y-coordinate.
		/// @param[in] color The new Color of the pixel.
		/// @return True if successful.
		bool setPixel(int x, int y, const Color& color);
		/// @brief Gets the linearly interpolated color between two pixels.
		/// @param[in] x Decimal X-coordinate.
		/// @param[in] y Decimal Y-coordinate.
		/// @return The interpolated Color of the pixel.
		Color getInterpolatedPixel(float x, float y);
		/// @brief Fills a rectangle area with one color.
		/// @param[in] x X-coordinate.
		/// @param[in] y Y-coordinate.
		/// @param[in] w Width of the area.
		/// @param[in] h Height of the area.
		/// @param[in] color The Color used for filling.
		/// @return True if successful.
		bool fillRect(int x, int y, int w, int h, const Color& color);
		/// @brief Blits a rectangle area with one color.
		/// @param[in] x X-coordinate.
		/// @param[in] y Y-coordinate.
		/// @param[in] w Width of the area.
		/// @param[in] h Height of the area.
		/// @param[in] color The Color used for blitting.
		/// @return True if successful.
		bool blitRect(int x, int y, int w, int h, const Color& color);
		/// @brief Copies the image data into a buffer.
		/// @param[out] output The output buffer.
		/// @param[in] format In what format the output should be done.
		/// @return True if successful.
		/// @note If output has not been allocated yet, it will be allocated with the new operator.
		/// @note When using a different format that the image's actual format, conversion will be done automatically and can be slower.
		bool copyPixelData(unsigned char** output, Image::Format format);
		/// @brief Creates an Image object from the image data.
		/// @param[in] format In what format the output should be done.
		/// @return The created Image object or NULL if failed.
		Image* createImage(Image::Format format);
		/// @brief Writes image data directly onto the texture.
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
		/// @brief Writes image data directly onto the texture.
		/// @param[in] sx Source data X-coordinate.
		/// @param[in] sy Source data Y-coordinate.
		/// @param[in] sw Width of the area on the source to be copied.
		/// @param[in] sh Height of the area on the source to be copied.
		/// @param[in] dx Destination X-coordinate.
		/// @param[in] dy Destination Y-coordinate.
		/// @param[in] texture The source Texture object.
		/// @return True if successful.
		/// @note Pixels on the destination will be overwritten.
		bool write(int sx, int sy, int sw, int sh, int dx, int dy, Texture* texture);
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
		/// @brief Writes image data directly onto the image while trying to stretch the pixels. Stretched pixels will be linearly interpolated.
		/// @param[in] sx Source data X-coordinate.
		/// @param[in] sy Source data Y-coordinate.
		/// @param[in] sw Width of the area on the source to be copied.
		/// @param[in] sh Height of the area on the source to be copied.
		/// @param[in] dx Destination X-coordinate.
		/// @param[in] dy Destination Y-coordinate.
		/// @param[in] dw Width of the destination area.
		/// @param[in] dh Height of the destination area.
		/// @param[in] texture The source Texture object.
		/// @return True if successful.
		/// @note Pixels on the destination will be overwritten.
		bool writeStretch(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, Texture* texture);
		/// @brief Does an image data block transfer onto the texture.
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
		/// @brief Does an image data block transfer onto the texture.
		/// @param[in] sx Source data X-coordinate.
		/// @param[in] sy Source data Y-coordinate.
		/// @param[in] sw Width of the area on the source to be copied.
		/// @param[in] sh Height of the area on the source to be copied.
		/// @param[in] dx Destination X-coordinate.
		/// @param[in] dy Destination Y-coordinate.
		/// @param[in] texture The source Texture object.
		/// @param[in] alpha Alpha multiplier on the entire source image.
		/// @return True if successful.
		/// @note Pixels on the destination will be overwritten will be blended with alpha-blending using the source pixels.
		/// @note The parameter alpha is especially useful when blitting source images that don't have an alpha channel.
		bool blit(int sx, int sy, int sw, int sh, int dx, int dy, Texture* texture, unsigned char alpha = 255);
		/// @brief Does a stretched image data block transfer onto the texture.
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
		/// @brief Does a stretched image data block transfer onto the texture.
		/// @param[in] sx Source data X-coordinate.
		/// @param[in] sy Source data Y-coordinate.
		/// @param[in] sw Width of the area on the source to be copied.
		/// @param[in] sh Height of the area on the source to be copied.
		/// @param[in] dx Destination X-coordinate.
		/// @param[in] dy Destination Y-coordinate.
		/// @param[in] dw Width of the destination area.
		/// @param[in] dh Height of the destination area.
		/// @param[in] texture The source Texture object.
		/// @param[in] alpha Alpha multiplier on the entire source image.
		/// @return True if successful.
		/// @note Pixels on the destination will be overwritten will be blended with alpha-blending using the source pixels.
		/// @note The parameter alpha is especially useful when blitting source images that don't have an alpha channel.
		bool blitStretch(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, Texture* texture, unsigned char alpha = 255);
		/// @brief Rotates the pixel hue of a rectangle area on the texture.
		/// @param[in] x X-coordinate of the area to change.
		/// @param[in] y Y-coordinate of the area to change.
		/// @param[in] w Width of the area to change.
		/// @param[in] h Height of the area to change.
		/// @param[in] degrees By how many degrees the the should be rotated.
		/// @return True if successful.
		/// @note This is lossy operation.
		/// @note This is an expensive operation and should be used sparingly.
		bool rotateHue(int x, int y, int w, int h, float degrees);
		/// @brief Changes the saturation level of pixels of a rectangle area on the texture.
		/// @param[in] x X-coordinate of the area to change.
		/// @param[in] y Y-coordinate of the area to change.
		/// @param[in] w Width of the area to change.
		/// @param[in] h Height of the area to change.
		/// @param[in] factor The saturation multiplier factor.
		/// @return True if successful.
		/// @note This is lossy operation.
		/// @note This is an expensive operation and should be used sparingly.
		bool saturate(int x, int y, int w, int h, float factor);
		/// @brief Inverts the pixel colors of a rectangle area on the texture.
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
		bool insertAlphaMap(unsigned char* srcData, Image::Format srcFormat, unsigned char median, int ambiguity);
		/// @brief Inserts image data as alpha channel into this image.
		/// @param[in] texture The source Texture object.
		/// @param[in] median The median value for insertion.
		/// @param[in] ambiguity How "hard" the alpha channel transition should be.
		/// @return True if successful.
		/// @note The image data in srcData must be the same width and height as the image.
		/// @note This is an expensive operation and should be used sparingly.
		bool insertAlphaMap(Texture* texture, unsigned char median, int ambiguity);

		/// @brief Gets the color of a specific pixel.
		/// @param[in] position Pixel coordinate.
		/// @return The Color of the pixel.
		/// @see getPixel(int x, int y)
		Color getPixel(cgvec2i position);
		/// @brief Sets the color of a specific pixel.
		/// @param[in] position Pixel coordinate.
		/// @param[in] color The new Color of the pixel.
		/// @return True if successful.
		/// @see setPixel(int x, int y, const Color& color)
		bool setPixel(cgvec2i position, const Color& color);
		/// @brief Gets the linearly interpolated color between two pixels.
		/// @param[in] position Pixel coordinate.
		/// @return The interpolated Color of the pixel.
		/// @see getInterpolatedPixel(float x, float y)
		Color getInterpolatedPixel(cgvec2f position);
		/// @brief Fills a rectangle area with one color.
		/// @param[in] rect The rectangle area.
		/// @param[in] color The Color used for filling.
		/// @return True if successful.
		/// @see fillRect(int x, int y, int w, int h, const Color& color)
		bool fillRect(cgrecti rect, const Color& color);
		/// @brief Blits a rectangle area with one color.
		/// @param[in] rect The rectangle area.
		/// @param[in] color The Color used for blitting.
		/// @return True if successful.
		/// @see blitRect(int x, int y, int w, int h, const Color& color)
		bool blitRect(cgrecti rect, const Color& color);
		/// @brief Copies the image data into a buffer.
		/// @param[out] output The output buffer.
		/// @return True if successful.
		/// @note If output has not been allocated yet, it will be allocated with the new operator.
		/// @note The data will be copied in the same pixel format as this Image.
		bool copyPixelData(unsigned char** output);
		/// @brief Creates an Image object from the image data.
		/// @return The created Image object or NULL if failed.
		Image* createImage();
		/// @brief Writes image data directly onto the image.
		/// @param[in] sx Source data X-coordinate.
		/// @param[in] sy Source data Y-coordinate.
		/// @param[in] sw Width of the area on the source to be copied.
		/// @param[in] sh Height of the area on the source to be copied.
		/// @param[in] dx Destination X-coordinate.
		/// @param[in] dy Destination Y-coordinate.
		/// @param[in] image The source Image.
		/// @return True if successful.
		/// @note Pixels on the destination will be overwritten.
		/// @see write(int sx, int sy, int sw, int sh, int dx, int dy, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat)
		bool write(int sx, int sy, int sw, int sh, int dx, int dy, Image* image);
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
		bool write(cgrecti srcRect, cgvec2i destPosition, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat);
		/// @brief Writes image data directly onto the image.
		/// @param[in] srcRect Source data rectangle.
		/// @param[in] destPosition Destination coordinates.
		/// @param[in] texture The source Texture.
		/// @return True if successful.
		/// @note Pixels on the destination will be overwritten.
		/// @see write(int sx, int sy, int sw, int sh, int dx, int dy, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat)
		bool write(cgrecti srcRect, cgvec2i destPosition, Texture* texture);
		/// @brief Writes image data directly onto the image.
		/// @param[in] srcRect Source data rectangle.
		/// @param[in] destPosition Destination coordinates.
		/// @param[in] image The source Image.
		/// @return True if successful.
		/// @note Pixels on the destination will be overwritten.
		/// @see write(int sx, int sy, int sw, int sh, int dx, int dy, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat)
		bool write(cgrecti srcRect, cgvec2i destPosition, Image* image);

		/// @brief Writes image data directly onto the image while trying to stretch the pixels. Stretched pixels will be linearly interpolated.
		/// @param[in] sx Source data X-coordinate.
		/// @param[in] sy Source data Y-coordinate.
		/// @param[in] sw Width of the area on the source to be copied.
		/// @param[in] sh Height of the area on the source to be copied.
		/// @param[in] dx Destination X-coordinate.
		/// @param[in] dy Destination Y-coordinate.
		/// @param[in] dw Width of the destination area.
		/// @param[in] dh Height of the destination area.
		/// @param[in] image The source Image.
		/// @return True if successful.
		/// @note Pixels on the destination will be overwritten.
		/// @see writeStretch(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat)
		bool writeStretch(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, Image* image);
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
		bool writeStretch(cgrecti srcRect, cgrecti destRect, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat);
		/// @brief Writes image data directly onto the image while trying to stretch the pixels. Stretched pixels will be linearly interpolated.
		/// @param[in] srcRect Source data rectangle.
		/// @param[in] destRect Destination rectangle.
		/// @param[in] texture The source Texture.
		/// @return True if successful.
		/// @note Pixels on the destination will be overwritten.
		/// @see writeStretch(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat)
		bool writeStretch(cgrecti srcRect, cgrecti destRect, Texture* texture);
		/// @brief Writes image data directly onto the image while trying to stretch the pixels. Stretched pixels will be linearly interpolated.
		/// @param[in] srcRect Source data rectangle.
		/// @param[in] destRect Destination rectangle.
		/// @param[in] image The source Image.
		/// @return True if successful.
		/// @note Pixels on the destination will be overwritten.
		/// @see writeStretch(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat)
		bool writeStretch(cgrecti srcRect, cgrecti destRect, Image* image);
		/// @brief Does an image data block transfer onto the image.
		/// @param[in] sx Source data X-coordinate.
		/// @param[in] sy Source data Y-coordinate.
		/// @param[in] sw Width of the area on the source to be copied.
		/// @param[in] sh Height of the area on the source to be copied.
		/// @param[in] dx Destination X-coordinate.
		/// @param[in] dy Destination Y-coordinate.
		/// @param[in] image The source Image.
		/// @param[in] alpha Alpha multiplier on the entire source image.
		/// @return True if successful.
		/// @note Pixels on the destination will be overwritten will be blended with alpha-blending using the source pixels.
		/// @note The parameter alpha is especially useful when blitting source images that don't have an alpha channel.
		/// @see blit(int sx, int sy, int sw, int sh, int dx, int dy, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat, unsigned char alpha)
		bool blit(int sx, int sy, int sw, int sh, int dx, int dy, Image* image, unsigned char alpha = 255);
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
		bool blit(cgrecti srcRect, cgvec2i destPosition, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat, unsigned char alpha = 255);
		/// @brief Does an image data block transfer onto the image.
		/// @param[in] srcRect Source data rectangle.
		/// @param[in] destPosition Destination coordinates.
		/// @param[in] texture The source Texture.
		/// @param[in] alpha Alpha multiplier on the entire source image.
		/// @return True if successful.
		/// @note Pixels on the destination will be overwritten will be blended with alpha-blending using the source pixels.
		/// @note The parameter alpha is especially useful when blitting source images that don't have an alpha channel.
		/// @see blit(int sx, int sy, int sw, int sh, int dx, int dy, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat, unsigned char alpha)
		bool blit(cgrecti srcRect, cgvec2i destPosition, Texture* texture, unsigned char alpha = 255);
		/// @brief Does an image data block transfer onto the image.
		/// @param[in] srcRect Source data rectangle.
		/// @param[in] destPosition Destination coordinates.
		/// @param[in] image The source Image.
		/// @param[in] alpha Alpha multiplier on the entire source image.
		/// @return True if successful.
		/// @note Pixels on the destination will be overwritten will be blended with alpha-blending using the source pixels.
		/// @note The parameter alpha is especially useful when blitting source images that don't have an alpha channel.
		/// @see blit(int sx, int sy, int sw, int sh, int dx, int dy, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat, unsigned char alpha)
		bool blit(cgrecti srcRect, cgvec2i destPosition, Image* image, unsigned char alpha = 255);
		/// @brief Does a stretched image data block transfer onto the image.
		/// @param[in] sx Source data X-coordinate.
		/// @param[in] sy Source data Y-coordinate.
		/// @param[in] sw Width of the area on the source to be copied.
		/// @param[in] sh Height of the area on the source to be copied.
		/// @param[in] dx Destination X-coordinate.
		/// @param[in] dy Destination Y-coordinate.
		/// @param[in] dw Width of the destination area.
		/// @param[in] dh Height of the destination area.
		/// @param[in] image The source Image.
		/// @param[in] alpha Alpha multiplier on the entire source image.
		/// @return True if successful.
		/// @note Pixels on the destination will be overwritten will be blended with alpha-blending using the source pixels.
		/// @note The parameter alpha is especially useful when blitting source images that don't have an alpha channel.
		/// @see blitStretch(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat, unsigned char alpha)
		bool blitStretch(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, Image* image, unsigned char alpha = 255);
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
		bool blitStretch(cgrecti srcRect, cgrecti destRect, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat, unsigned char alpha = 255);
		/// @brief Does a stretched image data block transfer onto the image.
		/// @param[in] srcRect Source data rectangle.
		/// @param[in] destRect Destination rectangle.
		/// @param[in] texture The source Texture.
		/// @param[in] alpha Alpha multiplier on the entire source image.
		/// @return True if successful.
		/// @note Pixels on the destination will be overwritten will be blended with alpha-blending using the source pixels.
		/// @note The parameter alpha is especially useful when blitting source images that don't have an alpha channel.
		/// @see blitStretch(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat, unsigned char alpha)
		bool blitStretch(cgrecti srcRect, cgrecti destRect, Texture* texture, unsigned char alpha = 255);
		/// @brief Does a stretched image data block transfer onto the image.
		/// @param[in] srcRect Source data rectangle.
		/// @param[in] destRect Destination rectangle.
		/// @param[in] image The source Image.
		/// @param[in] alpha Alpha multiplier on the entire source image.
		/// @return True if successful.
		/// @note Pixels on the destination will be overwritten will be blended with alpha-blending using the source pixels.
		/// @note The parameter alpha is especially useful when blitting source images that don't have an alpha channel.
		/// @see blitStretch(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat, unsigned char alpha)
		bool blitStretch(cgrecti srcRect, cgrecti destRect, Image* image, unsigned char alpha = 255);
		/// @brief Rotates the pixel hue of a rectangle area on the image.
		/// @param[in] rect Rectangle area.
		/// @param[in] degrees By how many degrees the the should be rotated.
		/// @return True if successful.
		/// @note This is lossy operation.
		/// @note This is an expensive operation and should be used sparingly.
		bool rotateHue(cgrecti rect, float degrees);
		/// @brief Changes the saturation level of pixels of a rectangle area on the image.
		/// @param[in] rect Rectangle area.
		/// @param[in] factor The saturation multiplier factor.
		/// @return True if successful.
		/// @note This is lossy operation.
		/// @note This is an expensive operation and should be used sparingly.
		bool saturate(cgrecti rect, float factor);
		/// @brief Inverts the pixel colors of a rectangle area on the image.
		/// @param[in] rect Rectangle area.
		/// @return True if successful.
		bool invert(cgrecti rect);
		/// @brief Inserts image data as alpha channel into this image.
		/// @param[in] image The source Image.
		/// @param[in] median The median value for insertion.
		/// @param[in] ambiguity How "hard" the alpha channel transition should be.
		/// @return True if successful.
		/// @note The pixel format of image must be the same this image's.
		/// @note This is an expensive operation and should be used sparingly.
		bool insertAlphaMap(Image* image, unsigned char median, int ambiguity); // TODOa - this functionality might be removed since shaders are much faster

	protected:
		/// @brief Defines a texture read lock for reading and writing to improve performance.
		struct Lock
		{
		public:
			/// @brief Contains a pointer to the system buffer.
			/// @note The buffer can differ depending on the implementation, the texture type, the data etc.
			void* systemBuffer;
			/// @brief X-coordinate of the lock rectangle.
			int x;
			/// @brief Y-coordinate of the lock rectangle.
			int y;
			/// @brief Width of the lock rectangle.
			int w;
			/// @brief Height of the lock rectangle.
			int h;
			/// @brief Destination X-coordinate.
			int dx;
			/// @brief Destination Y-coordinate.
			int dy;
			/// @brief Data that is used for locking.
			unsigned char* data;
			/// @brief Width of the locked image data.
			int dataWidth;
			/// @brief Height of the locked image data.
			int dataHeight;
			/// @brief Format of the locked image data.
			Image::Format format;
			/// @brief Whether the data is currently locked.
			bool locked;
			/// @brief Whether the data could not be locked.
			bool failed;
			/// @brief Whether the data is from a render target.
			bool renderTarget;

			/// @brief Basic constructor.
			Lock();
			/// @brief Destructor.
			~Lock();

			/// @brief Marks the lock as failed.
			void activateFail();
			/// @brief Marks the lock as successful and stores the lock data. This has to be called on non-render targets.
			void activateLock(int x, int y, int w, int h, int dx, int dy, unsigned char* data, int dataWidth, int dataHeight, Image::Format format);

		};

		/// @brief The filename if available.
		hstr filename;
		/// @brief The texture's custom name.
		/// @note Does not require to be unique.
		hstr name;
		/// @brief The texture type.
		Type type;
		/// @brief Whether the texture was uploaded to the GPU.
		bool uploaded;
		/// @brief The texture load mode.
		LoadMode loadMode;
		/// @brief The texture's image data pixel format.
		Image::Format format;
		/// @brief Internal format identifier.
		/// @note Needed for special platform dependent formats, usually used internally only.
		unsigned int dataFormat;
		/// @brief Width of the texture in pixels.
		int width;
		/// @brief Height of the texture in pixels.
		int height;
		/// @brief Effective width of the texture in pixels.
		/// @note Used only with software NPOT textures internally.
		float effectiveWidth;
		/// @brief Effective width of the texture in pixels.
		/// @note Used only with software NPOT textures internally.
		float effectiveHeight;
		/// @brief The byte size of the image data when handling compressed formats (e.g. Format::Compressed or Format::Palette).
		int compressedSize;
		/// @brief The texture's filtering mode.
		Filter filter;
		/// @brief The texture's UV coordinate address mode.
		AddressMode addressMode;
		/// @brief Whether the texture is locked for raw image data manipulation.
		bool locked;
		/// @brief Whether the texture is dirty and needs to be reuploaded to the GPU.
		bool dirty;
		/// @brief The raw image data.
		unsigned char* data;
		/// @brief Mutex used for thread synchronization when using asynchronous loading.
		hmutex asyncDataMutex;
		/// @brief The raw image data that was loaded asynchronously and is waiting to be uploaded to the GPU.
		unsigned char* dataAsync;
		/// @brief Whether asynchronous loading was queued.
		bool asyncLoadQueued;
		/// @brief Whether asynchronously loaded data was discared.
		/// @note This is used to discard loaded data when unload() is called before asynchronous loading finishes.
		/// @see unload
		bool asyncLoadDiscarded;
		/// @brief Mutex used for thread synchronization when using asynchronous loading.
		hmutex asyncLoadMutex;
		/// @brief Whether the texture was loaded from a resource file or a normal file.
		/// @note This has no meaning if the data was created from memory and not loaded from a file.
		bool fromResource;
		/// @brief Whether a first upload to the GPU already happened.
		/// @note Required because of how some RenderSystem implementations work (e.g. OpenGL and OpenGLES).
		bool firstUpload;

		/// @brief Constructor.
		/// @param[in] fromResource Whether the texture was loaded from a resource file or a normal file.
		Texture(bool fromResource);
		/// @brief Destructor.
		virtual ~Texture();

		/// @brief Creates and sets up all of the texture's internal data.
		/// @param[in] filename Filename of the source file.
		/// @param[in] type The texture type.
		/// @param[in] loadMode The texture load mode.
		/// @return True if successful.
		virtual bool _create(chstr filename, Type type, LoadMode loadMode);
		/// @brief Creates and sets up all of the texture's internal data.
		/// @param[in] filename Filename of the source file.
		/// @param[in] format Pixel format of the image data.
		/// @param[in] type The texture type.
		/// @param[in] loadMode The texture load mode.
		/// @return True if successful.
		virtual bool _create(chstr filename, Image::Format format, Type type, LoadMode loadMode);
		/// @brief Creates and sets up all of the texture's internal data.
		/// @param[in] width Width of the raw image data.
		/// @param[in] height Height of the raw image data.
		/// @param[in] data The raw image data.
		/// @param[in] format The pixel format of the raw image data.
		/// @return True if successful.
		virtual bool _create(int width, int height, unsigned char* data, Image::Format format, Type type);
		/// @brief Creates and sets up all of the texture's internal data and fills it with the given Color.
		/// @param[in] width Width of the raw image data.
		/// @param[in] height Height of the raw image data.
		/// @param[in] color The color to be filled.
		/// @param[in] format The pixel format of the raw image data.
		/// @return True if successful.
		virtual bool _create(int width, int height, const Color& color, Image::Format format, Type type);
		/// @brief Creates and sets up all of the texture's internal data for render target usage.
		/// @param[in] width Width of the raw image data.
		/// @param[in] height Height of the raw image data.
		/// @param[in] format The pixel format of the raw image data.
		/// @return True if successful.
		/// @note The format parameter will automatically be converted into the native format of the system backend.
		virtual bool _createRenderTarget(int width, int height, Image::Format format);

		/// @brief Creates the device texture.
		/// @param[in] data The raw image data.
		/// @param[in] size The raw image data size in bytes.
		/// @return True if successful.
		virtual bool _deviceCreateTexture(unsigned char* data, int size) = 0;
		/// @brief Unloads the device texture.
		void _deviceUnloadTexture();
		/// @brief Destroy the device texture.
		/// @return True if successful.
		virtual bool _deviceDestroyTexture() = 0;
		/// @brief Assigns the internal device format.
		/// @note This is called internally.
		virtual void _assignFormat() = 0;

		/// @brief Prepares a stream object for async loading.
		/// @return A stream object for async loading.
		hstream* _prepareAsyncStream();
		/// @brief Decodes loaded image data.
		/// @param[in] stream The stream object where the loaded data is.
		void _decodeFromAsyncStream(hstream* stream);
		/// @brief If necessary, converts to image the a format supported by the RenderSystem.
		/// @param[in] image The loaded Image.
		/// @return The final Image. This may be the same as the same image as the parameter image or can be a new image.
		/// @note The parameter image may be invalidated and shouldn't be used anymore. Instead, use the returned Image.
		Image* _processImageFormatSupport(Image* image);

		/// @brief Gets the size of the image data in bytes.
		/// @return Size of the image data in bytes.
		/// @note The calculation is basically "width * height * bpp".
		/// @note This is called internally.
		int _getByteSize() const;
		/// @brief Gets the readable flag.
		/// @return True if the image data from the texture can be read.
		virtual bool _isReadable() const;
		/// @brief Gets the writable flag.
		/// @return True if the image data from the texture can be written.
		virtual bool _isWritable() const;
		/// @brief Gets the alterable flag.
		/// @return True if the image data from the texture can be both read and written.
		virtual bool _isAlterable() const;
		/// @brief Gets the internal texture name.
		/// @return The internal texture name.
		/// @note This is usually used for debugging and logging purposes.
		virtual hstr _getInternalName() const;
		/// @brief Checks whether TextureAsync needs to upload this texture to the GPU.
		/// @return True if TextureAsync needs to upload this texture to the GPU.
		bool _isAsyncUploadQueued();

		/// @brief Loads the texture asynchronously. Used internally only.
		/// @return True if queueing was successful.
		/// @note Using a texture before it is uploaded to the GPU will cause undefined behavior. Always call load() before using it, because this ensures that it's loaded.
		bool _loadAsync();
		/// @brief Loads the texture's meta data only. Used internally only.
		/// @return True if successful or already loaded.
		bool _loadMetaData();
		/// @brief Uploads the texture data to the GPU. Used internally only.
		/// @return True if can be uploaded and successful.
		bool _tryAsyncFinalUpload();
		/// @brief Makes sure that the texture has been uploaded to the GPU. Used internally only.
		/// @return True if successful or already loaded.
		bool _upload(hmutex::ScopeLock& lock);
		/// @brief Waits for the texture to be ready for upload. Used internally only. Called from main thread only.
		/// @see loadAsync
		/// @see _upload
		void _ensureAsyncCompleted();
		/// @brief Uploads the texture data to the GPU. Used internally only. Called from main thread only.
		/// @return True if successful or already loaded.
		bool _ensureUploaded();

		/// @brief Clears the entire image and sets all image data to zeroes without safety checks. Used internally only.
		/// @return True if successful.
		bool _rawClear();
		/// @brief Sets the color of a specific pixel without safety checks. Used internally only.
		/// @param[in] x X-coordinate.
		/// @param[in] y Y-coordinate.
		/// @param[in] color The new Color of the pixel.
		/// @return True if successful.
		bool _rawSetPixel(int x, int y, const Color& color);
		/// @brief Fills a rectangle area with one color without safety checks. Used internally only.
		/// @param[in] x X-coordinate.
		/// @param[in] y Y-coordinate.
		/// @param[in] w Width of the area.
		/// @param[in] h Height of the area.
		/// @param[in] color The Color used for filling.
		/// @return True if successful.
		bool _rawFillRect(int x, int y, int w, int h, const Color& color);
		/// @brief Blits a rectangle area with one color without safety checks. Used internally only.
		/// @param[in] x X-coordinate.
		/// @param[in] y Y-coordinate.
		/// @param[in] w Width of the area.
		/// @param[in] h Height of the area.
		/// @param[in] color The Color used for blitting.
		/// @return True if successful.
		bool _rawBlitRect(int x, int y, int w, int h, const Color& color);
		/// @brief Writes image data directly onto the texture without safety checks. Used internally only.
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
		bool _rawWrite(int sx, int sy, int sw, int sh, int dx, int dy, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat);
		/// @brief Writes image data directly onto the image while trying to stretch the pixels. Stretched pixels will be linearly interpolated. Used internally only.
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
		bool _rawWriteStretch(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat);
		/// @brief Does an image data block transfer onto the texture. Used internally only.
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
		bool _rawBlit(int sx, int sy, int sw, int sh, int dx, int dy, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat, unsigned char alpha = 255);
		/// @brief Does a stretched image data block transfer onto the texture. Used internally only.
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
		bool _rawBlitStretch(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat, unsigned char alpha = 255);
		/// @brief Rotates the pixel hue of a rectangle area on the texture. Used internally only.
		/// @param[in] x X-coordinate of the area to change.
		/// @param[in] y Y-coordinate of the area to change.
		/// @param[in] w Width of the area to change.
		/// @param[in] h Height of the area to change.
		/// @param[in] degrees By how many degrees the the should be rotated.
		/// @return True if successful.
		/// @note This is lossy operation.
		/// @note This is an expensive operation and should be used sparingly.
		bool _rawRotateHue(int x, int y, int w, int h, float degrees);
		/// @brief Changes the saturation level of pixels of a rectangle area on the texture. Used internally only.
		/// @param[in] x X-coordinate of the area to change.
		/// @param[in] y Y-coordinate of the area to change.
		/// @param[in] w Width of the area to change.
		/// @param[in] h Height of the area to change.
		/// @param[in] factor The saturation multiplier factor.
		/// @return True if successful.
		/// @note This is lossy operation.
		/// @note This is an expensive operation and should be used sparingly.
		bool _rawSaturate(int x, int y, int w, int h, float factor);
		/// @brief Inverts the pixel colors of a rectangle area on the texture. Used internally only.
		/// @param[in] x X-coordinate of the area to change.
		/// @param[in] y Y-coordinate of the area to change.
		/// @param[in] w Width of the area to change.
		/// @param[in] h Height of the area to change.
		/// @return True if successful.
		bool _rawInvert(int x, int y, int w, int h);
		/// @brief Inserts image data as alpha channel into this image. Used internally only.
		/// @param[in] srcData The source raw image data.
		/// @param[in] srcFormat The pixel format of source raw image data.
		/// @param[in] median The median value for insertion.
		/// @param[in] ambiguity How "hard" the alpha channel transition should be.
		/// @return True if successful.
		/// @note The image data in srcData must be the same width and height as the image.
		/// @note This is an expensive operation and should be used sparingly.
		bool _rawInsertAlphaMap(unsigned char* srcData, Image::Format srcFormat, unsigned char median, int ambiguity);

		/// @brief Sets up dimensions and internal state of the texture to be power-of-two compliant.
		/// @param[out] outWidth Power-of-two width.
		/// @param[out] outHeight Power-of-two height.
		void _setupPot(int& outWidth, int& outHeight);
		/// @brief Creates image data in power-of-two dimensions and copies other image data to it.
		/// @param[in,out] outWidth Power-of-two width.
		/// @param[in,out] outHeight Power-of-two height.
		/// @param[in] data Original image data.
		unsigned char* _createPotData(int& outWidth, int& outHeight, unsigned char* data);
		/// @brief Creates image data in power-of-two dimensions with zeroes.
		/// @param[in,out] outWidth Power-of-two width.
		/// @param[in,out] outHeight Power-of-two height.
		unsigned char* _createPotClearData(int& outWidth, int& outHeight);

		/// @brief Try to lock a rectangle on the texture in the system implementation.
		/// @param[in] x X-coordinate of the rectangle.
		/// @param[in] y Y-coordinate of the rectangle.
		/// @param[in] w Width of the rectangle.
		/// @param[in] h Height of the rectangle.
		/// @return True if successful.
		virtual Lock _tryLockSystem(int x, int y, int w, int h) = 0;
		/// @brief Remove a lock in the system implementation.
		/// @param[in,out] lock The Lock object.
		/// @param[in] update True if the image data should be reuploaded to the GPU.
		/// @return True if successful.
		virtual bool _unlockSystem(Lock& lock, bool update) = 0;
		/// @brief Attempts to upload changed texture data to the GPU.
		/// @return True if successful.
		/// @note No upload will happen if no changes have been made or the texture is currently not loaded.
		bool _tryUploadDataToGpu();
		/// @brief Uploads local image data directly to the GPU.
		/// @param[in] x X-coordinate of the rectangle.
		/// @param[in] y Y-coordinate of the rectangle.
		/// @param[in] w Width of the rectangle.
		/// @param[in] h Height of the rectangle.
		/// @return True if successful.
		bool _uploadDataToGpu(int x, int y, int w, int h);
		/// @brief Uploads image data directly to the GPU.
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
		virtual bool _uploadToGpu(int sx, int sy, int sw, int sh, int dx, int dy, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat) = 0;

	private:
		/// @brief Checks max texture size from the system backend.
		void _checkMaxTextureSize();

	};
	
}
#endif
