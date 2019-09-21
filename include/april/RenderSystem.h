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
/// Defines a generic render system.

#ifndef APRIL_RENDER_SYSTEM_H
#define APRIL_RENDER_SYSTEM_H

#include <hltypes/harray.h>
#include <hltypes/henum.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hstring.h>
#include <gtypes/Matrix4.h>
#include <gtypes/Rectangle.h>
#include <gtypes/Vector2.h>
#include <gtypes/Vector3.h>

#include "aprilExport.h"
#include "aprilUtil.h"
#include "Color.h"
#include "Image.h"
#include "Texture.h"

#define APRIL_INTERMEDIATE_TEXTURE_VERTICES_COUNT 6

namespace april
{
	class Application;
	class AssignWindowCommand;
	class AsyncCommand;
	class AsyncCommandQueue;
	class ClearCommand;
	class ClearColorCommand;
	class ClearDepthCommand;
	class CreateCommand;
	class DestroyCommand;
	class Image;
	class PixelShader;
	class PresentFrameCommand;
	class RenderCommand;
	class RenderHelper;
	class RenderState;
	class ResetCommand;
	class StateUpdateCommand;
	class SuspendCommand;
	class TakeScreenshotCommand;
	class Texture;
	template <typename T> class VertexRenderCommand;
	class VertexShader;
	class Window;

	/// @brief Defines a generic render system.
	class aprilExport RenderSystem
	{
	public:
		friend class Application;
		friend class AssignWindowCommand;
		friend class ClearCommand;
		friend class ClearColorCommand;
		friend class ClearDepthCommand;
		friend class CreateCommand;
		friend class DestroyCommand;
		friend class PresentFrameCommand;
		friend class RenderCommand;
		friend class RenderHelper;
		friend class RenderHelperLayered2D;
		friend class ResetCommand;
		friend class StateUpdateCommand;
		friend class SuspendCommand;
		friend class TakeScreenshotCommand;
		friend class Texture;
		template <typename T> friend class VertexRenderCommand;
		friend class Window;

		/// @class RenderMode
		/// @brief Defines possible rendering methods.
		HL_ENUM_CLASS_PREFIX_DECLARE(aprilExport, RenderMode,
		(
			/// @var static const RenderMode RenderMode::Normal
			/// @brief Normal rendering.
			HL_ENUM_DECLARE(RenderMode, Normal);
			/// @var static const RenderMode RenderMode::Layered2D
			/// @brief Optimized layered rendering for 2D.
			/// @note The Layered 2D mode has a few limitations and rules.
			/// 1. Only RO_TRIANGLE_LIST and RO_LINE_LIST are supported. Using others will cause a flush.
			/// 2. Using depth buffer is not supported and will cause a flush.
			/// 3. Changing the viewport will cause a flush, but it is supported.
			/// 4. The additional option "max_layers" will cause to flush when that number of layers is exceeded. Omit option to disable this feature.
			/// 5. The additional option "layer_pull_up_merge" will cause layers below the intersected layer to be merged. It optimizes layers better, but heavier on the CPU. Enabled by default.
			HL_ENUM_DECLARE(RenderMode, Layered2D);
		));

		/// @brief Defines a possible display mode for the screen.
		struct aprilExport DisplayMode
		{
		public:
			/// @brief Width in pixels.
			int width;
			/// @brief Height in pixels.
			int height;
			/// @brief Screen refresh rate in Hz.
			int refreshRate;

			/// @brief Basic constructor.
			/// @param[in] width Width in pixels.
			/// @param[in] height Height in pixels.
			/// @param[in] refreshRate Screen refresh rate in Hz.
			DisplayMode(int width, int height, int refreshRate);

			/// @return True if both DisplayMode instances are the same.
			bool operator==(const DisplayMode& other) const;
			/// @return True if the DisplayMode differ.
			bool operator!=(const DisplayMode& other) const;

			/// @brief Gets the string representation of the DisplayMode.
			/// @return String representation of the DisplayMode.
			hstr toString();

		};
		
		/// @brief Defines options for render system creation.
		struct aprilExport Options
		{
		public:
			/// @brief Whether to use a depth buffer.
			bool depthBuffer;
			/// @brief Vertical Sync.
			bool vSync;
			/// @brief Triple back buffer.
			bool tripleBuffering;
			/// @brief Whether to clear backbuffer on suspend.
			bool clearOnSuspend;
			/// @brief Whether to use intermediate render texture.
			bool intermediateRenderTexture;
			/// @brief Special debug info.
			bool debugInfo;

			/// @brief Basic constructor.
			Options();

			/// @brief Gets the string representation of the Options object.
			/// @return String representation of the Options object.
			hstr toString();

		};

		/// @brief Defines the available capabilities of the render system.
		struct aprilExport Caps
		{
		public:
			/// @brief The maximum supported texture size.
			int maxTextureSize;
			/// @brief Whether the "limited" implementation of non-power-of-two textures is supported.
			bool npotTexturesLimited;
			/// @brief Whether the full implementation of non-power-of-two textures is supported.
			bool npotTextures;
			/// @brief Whether External texture type is supported.
			bool externalTextures;
			/// @brief Supported texture pixel formats.
			harray<Image::Format> textureFormats;
			/// @brief Whether render targets are supported properly. Also 
			/// @note This also controls internal rendertarget usage for basic rendering.
			bool renderTarget;

			/// @brief Basic constructor.
			Caps();

		};

		/// @brief Basic constructor.
		RenderSystem();
		/// @brief Destructor.
		virtual ~RenderSystem();

		/// @brief Initializes the RenderSystem.
		void init();
		/// @brief Creates the internal RenderSystem with the given options.
		/// @param[in] options Options for the RenderSystem creation
		/// @return True if successful.
		bool create(Options options);
		/// @brief Destroyes the RenderSystem
		/// @return True if successful.
		bool destroy();
		/// @brief Assigns a window to the RenderSystem where it should render.
		/// @param[in] window The Window instance.
		void assignWindow(Window* window);
		/// @brief Resets the RenderSystem.
		/// @note Some systems require this call in certain situations. April attempts to reset the RenderSystem internally when needed so external calls should be unneccessary.
		void reset();
		/// @brief Suspends the RenderSystem.
		/// @note Some systems require this call when the application loses focus. It ensures that textures and other resources can be recreated with the context.
		/// April attempts to reset the RenderSystem internally when needed so external calls should be unneccessary.
		void suspend();

		/// @brief Gets the RenderSystem name.
		HL_DEFINE_GET(hstr, name, Name);
		/// @brief Gets the Options with which the RenderSystem was created.
		HL_DEFINE_GET(Options, options, Options);
		/// @brief Gets the capabilities of the RenderSystem.
		HL_DEFINE_GET(Caps, caps, Caps);
		/// @brief Gets the current RenderMode.
		HL_DEFINE_GET(RenderMode, renderMode, RenderMode);
		/// @brief Gets how frames in advance can be updated.
		HL_DEFINE_GET(int, frameAdvanceUpdates, FrameAdvanceUpdates);
		/// @brief Gets how many times a frame should be duplicated during rendering.
		/// @return How many times a frame should be duplicated during rendering.
		int getFrameDuplicates();
		/// @brief Sets how many times a frame should be duplicated during rendering.
		/// @parma[in] value The new value.
		void setFrameDuplicates(int const& value);
		/// @brief Gets how many times a render call was called during this frame.
		HL_DEFINE_GET(int, statCurrentFrameRenderCalls, StatCurrentFrameRenderCalls);
		/// @brief Gets how many times a render call was called during the last frame.
		HL_DEFINE_GET(int, statLastFrameRenderCalls, StatLastFrameRenderCalls);
		/// @brief Gets how many times the texture was switched during this frame.
		HL_DEFINE_GET(int, statCurrentFrameTextureSwitches, StatCurrentFrameTextureSwitches);
		/// @brief Gets how many times the texture was switched during the last frame.
		HL_DEFINE_GET(int, statLastFrameTextureSwitches, StatLastFrameTextureSwitches);
		/// @brief Gets how many vertices were rendered during this frame.
		HL_DEFINE_GET(int, statCurrentFrameVertexCount, StatCurrentFrameVertexCount);
		/// @brief Gets how many vertices were rendered during the last frame.
		HL_DEFINE_GET(int, statLastFrameVertexCount, StatLastFrameVertexCount);
		/// @brief Gets how many triangles were rendered during this frame.
		HL_DEFINE_GET(int, statCurrentFrameTriangleCount, StatCurrentFrameTriangleCount);
		/// @brief Gets how many triangles were rendered during the last frame.
		HL_DEFINE_GET(int, statLastFrameTriangleCount, StatLastFrameTriangleCount);
		/// @brief Gets how many lines were rendered during this frame.
		HL_DEFINE_GET(int, statCurrentFrameLineCount, StatCurrentFrameLineCount);
		/// @brief Gets how many lines were rendered during the last frame.
		HL_DEFINE_GET(int, statLastFrameLineCount, StatLastFrameLineCount);
		/// @brief Gets how frames are queued for rendering.
		int getAsyncQueuesCount();
		/// @brief Gets all currently existing textures in the RenderSystem.
		/// @return All currently existing textures in the RenderSystem.
		harray<Texture*> getTextures();
		/// @brief Gets a list of all supported display modes.
		/// @return A list of all supported display modes.
		harray<DisplayMode> getDisplayModes();
		/// @brief Whether low-level graphics API calls are allowed.
		/// @return True if low-level graphics API calls are allowed.
		/// @note This is used on some OS's that don't allow API calls to be made after the app is suspended (e.g. Android).
		virtual bool canUseLowLevelCalls() const;
		/// @return The current video RAM consumption.
		/// @note This is calculated based on loaded textures and may not be 100% accurate. The actual consumption could be higher.
		int64_t getVRamConsumption();
		/// @brief Gets the current RAM consumption of textures.
		/// @return The current RAM consumption of textures.
		/// @note This is the RAM consumed by only by the textures, not the entire process.
		int64_t getRamConsumption();
		/// @brief Gets the the current RAM consumption for all textures that are still in the process of being loaded asynchronously.
		/// @return The current RAM consumption for all textures that are still in the process of being loaded asynchronously.
		int64_t getAsyncRamConsumption();
		/// @brief Checks if there are any textures queued to be loaded asynchronously.
		/// @return True if there are any textures queued to be loaded asynchronously.
		bool hasAsyncTexturesQueued() const;
		/// @brief Checks if there are any textures ready to be uploaded.
		/// @return True if there are any textures ready to be uploaded.
		bool hasTexturesReadyForUpload() const;
		/// @brief Gets the render viewport.
		/// @return The render viewport.
		grecti getViewport() const;
		/// @brief Sets the render viewport.
		/// @param[in] value The viewport rectangle.
		void setViewport(cgrecti value);
		/// @brief Gets the current modelview matrix.
		/// @return The current modelview matrix.
		gmat4 getModelviewMatrix() const;
		/// @brief Sets the current modelview matrix.
		/// @param[in] value The new modelview matrix.
		void setModelviewMatrix(const gmat4& value);
		/// @brief Gets the projection matrix.
		/// @return The current projection matrix.
		gmat4 getProjectionMatrix() const;
		/// @brief Sets the current projection matrix.
		/// @param[in] value The new projection matrix.
		void setProjectionMatrix(const gmat4& value);

		/// @brief Gets the amount of video RAM available.
		/// @return The amount of video RAM available.
		/// @note This value is in MB (1 MB = 1024 kB, 1 kB = 1024 B).
		virtual int getVRam() const = 0;

		/// @brief Updates the RenderSystem.
		/// @param[in] timeDelta Time since last frame.
		/// @return Whether a frame has been rendered.
		bool update(float timeDelta);
		/// @brief Waits for the currently async commands to be executed.
		/// @param[in] forced Whether to force executing all commands so far.
		/// @note This is primarily used to help with synchronization. Future refactoring might introduce completely thread-safe access and render this functionality obsolete.
		void waitForAsyncCommands(bool forced = false);

		/// @brief Sets the current RenderMode.
		void setRenderMode(RenderMode renderMode, const hmap<hstr, hstr>& options = hmap<hstr, hstr>());

		/// @brief Creates a Texture object from a resource file.
		/// @param[in] filename The filename of the resource.
		/// @param[in] type The Texture type that should be created.
		/// @param[in] loadMode How and when the Texture should be loaded.
		/// @return The created Texture object or NULL if failed.
		Texture* createTextureFromResource(chstr filename, Texture::Type type = Texture::Type::Immutable, Texture::LoadMode loadMode = Texture::LoadMode::Async);
		/// @brief Creates a Texture object from a resource file.
		/// @param[in] filename The filename of the resource.
		/// @param[in] format To which pixel format the loaded data should be converted.
		/// @param[in] type The Texture type that should be created.
		/// @param[in] loadMode How and when the Texture should be loaded.
		/// @return The created Texture object or NULL if failed.
		/// @note When a format is forced, it's best to use managed (but not necessary).
		Texture* createTextureFromResource(chstr filename, Image::Format format, Texture::Type type = Texture::Type::Managed, Texture::LoadMode loadMode = Texture::LoadMode::Async);
		/// @brief Creates a Texture object from a file.
		/// @param[in] filename The filename of the file.
		/// @param[in] type The Texture type that should be created.
		/// @param[in] loadMode How and when the Texture should be loaded.
		/// @return The created Texture object or NULL if failed.
		Texture* createTextureFromFile(chstr filename, Texture::Type type = Texture::Type::Immutable, Texture::LoadMode loadMode = Texture::LoadMode::Async);
		/// @brief Creates a Texture object from a file.
		/// @param[in] filename The filename of the file.
		/// @param[in] format To which pixel format the loaded data should be converted.
		/// @param[in] type The Texture type that should be created.
		/// @param[in] loadMode How and when the Texture should be loaded.
		/// @return The created Texture object or NULL if failed.
		/// @note When a format is forced, it's best to use managed (but not necessary).
		Texture* createTextureFromFile(chstr filename, Image::Format format, Texture::Type type = Texture::Type::Managed, Texture::LoadMode loadMode = Texture::LoadMode::Async);
		/// @brief Creates a Texture object from raw data.
		/// @param[in] w Width of the image data.
		/// @param[in] h Height of the image data.
		/// @param[in] data The raw image data.
		/// @param[in] format The format of the provided image data.
		/// @param[in] type The Texture type that should be created.
		/// @return The created Texture object or NULL if failed.
		/// @note Only Managed and External are supported. Will be forced to Managed if RenderSystem does not support external textures.
		Texture* createTexture(int w, int h, unsigned char* data, Image::Format format, Texture::Type type = Texture::Type::Managed);
		/// @brief Creates a Texture object completely filled with a specific color.
		/// @param[in] w Width of the image data.
		/// @param[in] h Height of the image data.
		/// @param[in] color The color used for filling the texture.
		/// @param[in] format The format of the provided image data.
		/// @param[in] type The Texture type that should be created.
		/// @return The created Texture object or NULL if failed.
		/// @note Only Managed and External are supported. Will be forced to Managed if RenderSystem does not support external textures.
		Texture* createTexture(int w, int h, Color color, Image::Format format, Texture::Type type = Texture::Type::Managed);
		/// @brief Destroys a Texture object.
		/// @param[in] texture The Texture that should be destroyed.
		/// @note After this call the Texture pointer becomes invalid.
		void destroyTexture(Texture* texture);

		/// @brief Creates a pixel shader from a resource file.
		/// @param[in] filename The filename of the resource file.
		/// @return The PixelShader object or NULL if failed.
		PixelShader* createPixelShaderFromResource(chstr filename);
		/// @brief Creates a pixel shader from a file.
		/// @param[in] filename The filename of the file.
		/// @return The PixelShader object or NULL if failed.
		PixelShader* createPixelShaderFromFile(chstr filename);
		/// @brief Creates an empty pixel shader.
		/// @return The PixelShader object or NULL if failed.
		PixelShader* createPixelShader();
		/// @brief Creates a vertex shader from a resource file.
		/// @param[in] filename The filename of the resource file.
		/// @return The VertexShader object or NULL if failed.
		VertexShader* createVertexShaderFromResource(chstr filename);
		/// @brief Creates a vertex shader from a file.
		/// @param[in] filename The filename of the file.
		/// @return The VertexShader object or NULL if failed.
		VertexShader* createVertexShaderFromFile(chstr filename);
		/// @brief Creates a vertex shader.
		/// @return The VertexShader object or NULL if failed.
		VertexShader* createVertexShader();
		/// @brief Destroy a pixel shader.
		/// @param[in] shader The pixel shader that should be destroyed.
		/// @note After this call the PixelShader pointer becomes invalid.
		void destroyPixelShader(PixelShader* shader);
		/// @brief Destroy a vertex shader.
		/// @param[in] shader The vertex shader that should be destroyed.
		/// @note After this call the VertexShader pointer becomes invalid.
		void destroyVertexShader(VertexShader* shader);

		/// @brief Gets the ortho-projection rectangle.
		/// @return The ortho-projection rectangle.
		grectf getOrthoProjection() const;
		/// @brief Activate ortho-projection using a rectangle.
		/// @param[in] rect The ortho-projection rectangle.
		void setOrthoProjection(cgrectf rect);
		/// @brief Activate ortho-projection using a rectangle.
		/// @param[in] rect The ortho-projection rectangle.
		/// @param[in] nearZ The near clipping plane Z coordinate.
		/// @param[in] farZ The far clipping plane Z coordinate.
		void setOrthoProjection(cgrectf rect, float nearZ, float farZ);
		/// @brief Activate ortho-projection using just the size.
		/// @param[in] size The ortho-projection size.
		/// @note This assumes the coordinates of the ortho-projection is (0,0).
		void setOrthoProjection(cgvec2f size);
		/// @brief Activate ortho-projection using just the size.
		/// @param[in] size The ortho-projection size.
		/// @param[in] nearZ The near clipping plane Z coordinate.
		/// @param[in] farZ The far clipping plane Z coordinate.
		/// @note This assumes the coordinates of the ortho-projection is (0,0).
		void setOrthoProjection(cgvec2f size, float nearZ, float farZ);
		/// @brief Sets the current depth-buffer state.
		/// @param[in] enabled Whether to enable or disable the depth-buffer itself.
		/// @param[in] writeEnabled Whether writing to the depth-buffer is enabled.
		void setDepthBuffer(bool enabled, bool writeEnabled = true);
		/// @brief Sets the current Texture for rendering.
		/// @param[in] texture The Texture to be set for rendering.
		void setTexture(Texture* texture);
		/// @brief Sets the current blending mode for rendering.
		/// @param[in] blendMode The blending mode to be set for rendering.
		void setBlendMode(const BlendMode& blendMode);
		/// @brief Sets the current color mode for rendering.
		/// @param[in] colorMode The color mode to be set for rendering.
		/// @param[in] colorModeFactor The additioanl color mode factor.
		/// @note The parameter colorModeFactor is only used when the color mode is LERP.
		void setColorMode(const ColorMode& colorMode, float colorModeFactor = 1.0f);

		/// @brief Gets the Texture render target.
		/// @return The Texture render target.
		virtual Texture* getRenderTarget();
		/// @brief Sets the Texture render target.
		/// @param[in] texture The new Texture render target.
		virtual void setRenderTarget(Texture* texture);
		/// @brief Sets the current pixel shader Texture render target.
		/// @param[in] pixelShader The new pixel shader.
		virtual void setPixelShader(PixelShader* pixelShader);
		/// @brief Sets the current vertex shader Texture render target.
		/// @param[in] vertexShader The new vertex shader.
		virtual void setVertexShader(VertexShader* vertexShader);

		/// @brief Resets the modelview matrix to an identity matrix.
		void setIdentityTransform();
		/// @brief Translates the modelview matrix.
		/// @param[in] x The X-coordinate.
		/// @param[in] y The Y-coordinate.
		/// @param[in] z The Z-coordinate.
		void translate(float x, float y, float z = 0.0f);
		/// @brief Translates the modelview matrix.
		/// @param[in] vector The translation vector.
		void translate(cgvec3f vector);
		/// @brief Translates the modelview matrix.
		/// @param[in] vector The 2D translation vector.
		/// @note This ignores the Z-coordinate.
		void translate(cgvec2f vector);
		/// @brief Rotates the modelview matrix around the -Z axis.
		/// @param[in] angle Angle to rotate.
		void rotate(float angle);
		/// @brief Rotates the modelview matrix around an axis.
		/// @param[in] ax Rotation axis X-coordinate.
		/// @param[in] ay Rotation axis Y-coordinate.
		/// @param[in] az Rotation axis Z-coordinate.
		/// @param[in] angle Angle to rotate.
		/// @note This ignores the Z-coordinate.
		void rotate(float ax, float ay, float az, float angle);
		/// @brief Rotates the modelview matrix around an axis.
		/// @param[in] axis The rotation axis.
		/// @param[in] angle Angle to rotate.
		void rotate(cgvec3f axis, float angle);
		/// @brief Scales the modelview matrix by a certain factor.
		/// @param[in] factor The scaling factor.
		void scale(float factor);
		/// @brief Scales the modelview matrix by a certain factor.
		/// @param[in] factorX The scaling factor along the X axis.
		/// @param[in] factorY The scaling factor along the Y axis.
		/// @param[in] factorZ The scaling factor along the Z axis.
		void scale(float factorX, float factorY, float factorZ);
		/// @brief Scales the modelview matrix by a certain factor.
		/// @param[in] vector The scaling vector.
		void scale(cgvec3f vector);
		/// @brief Scales the modelview matrix by a certain factor.
		/// @param[in] vector The 2D scaling vector.
		/// @note This ignores the Z-coordinate.
		void scale(cgvec2f vector);
		/// @brief Sets the modelview matrix up to look toward a certain point.
		/// @param[in] eye The origin position of the camera.
		/// @param[in] target The target position where to look.
		/// @param[in] up The up-vector to decide the roll angle of the camera.
		void lookAt(cgvec3f eye, cgvec3f target, cgvec3f up);
		/// @brief Sets the perspective of the modelview matrix.
		/// @param[in] fov The angle of the field of view.
		/// @param[in] aspect The aspect ratio of the viewport.
		/// @param[in] nearZ The near clipping plane Z coordinate.
		/// @param[in] farZ The far clipping plane Z coordinate.
		void setPerspective(float fov, float aspect, float nearZ, float farZ);

		/// @brief Clears the backbuffer.
		/// @param[in] depth If true, clears the depth-buffer as well.
		void clear(bool depth = false);
		/// @brief Clears the backbuffer with a certain color.
		/// @param[in] color Color that is used to fill the backbuffer.
		/// @param[in] depth If true, clears the depth buffer as well.
		/// @note The parameter color is not applied to the depth-buffer.
		void clear(Color color, bool depth = false);
		/// @brief Clears the depth-buffer.
		void clearDepth();
		/// @brief Renders an array of vertices to the backbuffer.
		/// @param[in] renderOperation The RenderOperation that should be used to render the vertices.
		/// @param[in] vertices An array of vertices.
		/// @param[in] count How many vertices from the array should be rendered.
		/// @note Calling this will effectively set the current texture to NULL.
		void render(const RenderOperation& renderOperation, const PlainVertex* vertices, int count);
		/// @brief Renders an array of vertices to the backbuffer.
		/// @param[in] renderOperation The RenderOperation that should be used to render the vertices.
		/// @param[in] vertices An array of vertices.
		/// @param[in] count How many vertices from the array should be rendered.
		/// @param[in] color Color to apply globally on all vertices.
		/// @note Calling this will effectively set the current texture to NULL.
		void render(const RenderOperation& renderOperation, const PlainVertex* vertices, int count, const Color& color);
		/// @brief Renders an array of vertices to the backbuffer.
		/// @param[in] renderOperation The RenderOperation that should be used to render the vertices.
		/// @param[in] vertices An array of vertices.
		/// @param[in] count How many vertices from the array should be rendered.
		void render(const RenderOperation& renderOperation, const TexturedVertex* vertices, int count);
		/// @brief Renders an array of vertices to the backbuffer.
		/// @param[in] renderOperation The RenderOperation that should be used to render the vertices.
		/// @param[in] vertices An array of vertices.
		/// @param[in] count How many vertices from the array should be rendered.
		/// @param[in] color Color to apply globally on all vertices.
		void render(const RenderOperation& renderOperation, const TexturedVertex* vertices, int count, const Color& color);
		/// @brief Renders an array of vertices to the backbuffer.
		/// @param[in] renderOperation The RenderOperation that should be used to render the vertices.
		/// @param[in] vertices An array of vertices.
		/// @param[in] count How many vertices from the array should be rendered.
		/// @note Calling this will effectively set the current texture to NULL.
		void render(const RenderOperation& renderOperation, const ColoredVertex* vertices, int count);
		/// @brief Renders an array of vertices to the backbuffer.
		/// @param[in] renderOperation The RenderOperation that should be used to render the vertices.
		/// @param[in] vertices An array of vertices.
		/// @param[in] count How many vertices from the array should be rendered.
		void render(const RenderOperation& renderOperation, const ColoredTexturedVertex* vertices, int count);
		
		/// @brief Renders a rectangle.
		/// @param[in] rect Position and size of the rectangle.
		/// @param[in] color Color of the rectangle.
		/// @note Calling this will effectively set the current texture to NULL.
		void drawRect(cgrectf rect, const Color& color);
		/// @brief Renders a rectangle filled with a color.
		/// @param[in] rect Position and size of the rectangle.
		/// @param[in] color Color of the rectangle.
		/// @note Calling this will effectively set the current texture to NULL.
		void drawFilledRect(cgrectf rect, const Color& color);
		/// @brief Renders a textured rectangle.
		/// @param[in] rect Position and size of the rectangle.
		/// @param[in] src UV rectangle on the currently set Texture.
		/// @note Remember to call setTexture() before calling this.
		/// @see setTexture
		void drawTexturedRect(cgrectf rect, cgrectf src);
		/// @brief Renders a textured rectangle.
		/// @param[in] rect Position and size of the rectangle.
		/// @param[in] src UV rectangle on the currently set Texture.
		/// @param[in] color Color that should be applied to the texture.
		/// @note Remember to call setTexture() before calling this.
		/// @see setTexture
		void drawTexturedRect(cgrectf rect, cgrectf src, const Color& color);

		/// @brief Executes a custom command into the render-queue.
		/// @param[in] function The function to call.
		/// @param[in] args Arguments for the function.
		void executeCustomCommand(void (*function)(const harray<void*>& args), const harray<void*>& args = harray<void*>());
		/// @brief Finds the actual filename of a texture resource file.
		/// @param[in] filename Resource filename without the extension.
		/// @return The detected resource filename or an empty string if no resource file could be found.
		hstr findTextureResource(chstr filename) const;
		/// @brief Finds the actual filename of a texture file.
		/// @param[in] filename Filename without the extension.
		/// @return The detected filename or an empty string if no file could be found.
		hstr findTextureFile(chstr filename) const;
		/// @brief Unloads all textures.
		/// @note Useful for clearing all memory or if something invalidates textures and cannot guarantee that they are loaded anymore.
		void unloadTextures();
		/// @brief Waits for all currently queued textures to load asynchronously.
		/// @param[in] timeout How long to wait maximally in seconds.
		/// @note A timeout value of 0.0 means indefinitely.
		void waitForAsyncTextures(float timeout = 0.0f) const;

		/// @brief Converts a pixel format into the system native pixel format while keeping the number of colors intact.
		/// @param[in] format The format to convert.
		/// @return The system native pixel format.
		/// @note This is usually used for optimizations when deciding on texture formats to avoid additional conversions.
		///       e.g. Volatile textures can be sped up significantly if they use the system native format.
		virtual Image::Format getNativeTextureFormat(Image::Format format) const = 0;
		/// @brief Converts a Color into a system native unsigned int value used for colors internally.
		/// @param[in] color The Color to convert.
		/// @return A system native unsigned int value used for colors internally.
		virtual unsigned int getNativeColorUInt(const Color& color) const = 0;
		/// @brief Flushes the currently rendered data.
		/// @note Usually this doesn't need to be called manually. This is needed for some implemenations that don't call presentFrame() within C++ at all.
		/// @see presentFrame
		virtual void flushFrame(bool updateStats = false);
		/// @brief Takes a screenshot a.k.a. captures the image data of the backbuffer.
		/// @param[in] format The format to which the screenshot should be converted.
		void takeScreenshot(Image::Format format);
		/// @brief Flushes the currently rendered data to the backbuffer for display.
		/// @note Usually this doesn't need to be called manually. Calls flushFrame().
		/// @see flushFrame
		void presentFrame();

	protected:
		/// @brief The RenderSystem's name.
		hstr name;
		/// @brief Whether it's been created or not.
		bool created;
		/// @brief The Options object containing the currently used options.
		Options options;
		/// @brief The capabilities of the RenderSystem.
		Caps caps;
		/// @brief The current RenderMode
		RenderMode renderMode;
		/// @brief The pixel offset when rendering.
		/// @note Some implementations don't take the upper left corner of a pixel as pivot rendering coordinate and need this special fix.
		float pixelOffset;
		/// @brief All currently existing textures.
		harray<Texture*> textures;
		/// @brief All supported display modes.
		harray<DisplayMode> displayModes;
		/// @brief The current state of the RenderSystem.
		RenderState* state;
		/// @brief The actual device state.
		RenderState* deviceState;
		/// @brief Mutex required for registering and unregistering of textures that allows for multi-threaded access.
		hmutex texturesMutex;
		/// @brief Mutex required for async update/rendering.
		hmutex asyncMutex;
		/// @brief Special helper object that can handle rendering in a different way.
		RenderHelper* renderHelper;
		/// @brief Async command queue.
		harray<AsyncCommandQueue*> asyncCommandQueues;
		/// @brief Last special async command queue.
		AsyncCommandQueue* lastAsyncCommandQueue;
		/// @brief Whether async commands are being processed right now.
		bool processingAsync;
		/// @brief How many times a render call was called during this frame.
		int frameAdvanceUpdates;
		/// @brief How many times a frame should be duplicated during rendering.
		int frameDuplicates;
		/// @brief Current special texture used as utility for rendering.
		Texture* _currentIntermediateRenderTexture;
		/// @brief Last special texture used as utility for rendering.
		Texture* _lastIntermediateRenderTexture;
		/// @brief Special textures used as utility for rendering.
		harray<Texture*> _intermediateRenderTextures;
		/// @brief Number of current special textures used as utility for rendering.
		int _intermediateRenderTextureCount;
		/// @brief Index of current special textures used as utility for rendering.
		int _intermediateRenderTextureIndex;
		/// @brief Whether an update of the last intermediate render textures should occur after rendering.
		bool _updateLastIntermediateRenderTexture;
		/// @brief Fixed RenderState for rendering the intermediate render texture.
		RenderState* _intermediateState;
		/// @brief Fixed vertices for rendering the intermediate render texture.
		april::TexturedVertex _intermediateRenderVertices[APRIL_INTERMEDIATE_TEXTURE_VERTICES_COUNT];

		/// @brief How many times a render call was called during this frame.
		int statCurrentFrameRenderCalls;
		/// @brief How many times a render call was called during the last frame.
		int statLastFrameRenderCalls;
		/// @brief How many times the texture was switched during this frame.
		int statCurrentFrameTextureSwitches;
		/// @brief How many times the texture was switched during the last frame.
		int statLastFrameTextureSwitches;
		/// @brief How many vertices were rendered during this frame.
		int statCurrentFrameVertexCount;
		/// @brief How many vertices were rendered during the last frame.
		int statLastFrameVertexCount;
		/// @brief How many triangles were rendered during this frame.
		int statCurrentFrameTriangleCount;
		/// @brief How many triangles were rendered during the last frame.
		int statLastFrameTriangleCount;
		/// @brief How many lines were rendered during this frame.
		int statCurrentFrameLineCount;
		/// @brief How many lines were rendered during the last frame.
		int statLastFrameLineCount;

		/// @brief Registers a created Texture in the system.
		/// @param[in] texture The Texture to be registered.
		void _registerTexture(Texture* texture);
		/// @note Unregisters a Texture from the system.
		/// @param[in] texture The Texture to be unregistered.
		void _unregisterTexture(Texture* texture);

		/// @brief Internally safe method for creating a Texture object.
		/// @param[in] fromResource Whether the Texture should be created from a resource file or a normal file.
		/// @param[in] filename The filename of the texture.
		/// @param[in] type The Texture type that should be created.
		/// @param[in] loadMode How and when the Texture should be loaded.
		/// @param[in] format To which pixel format the loaded data should be converted.
		/// @return The created Texture object or NULL if failed.
		Texture* _createTextureFromSource(bool fromResource, chstr filename, Texture::Type type, Texture::LoadMode loadMode, Image::Format format = Image::Format::Invalid);
		/// @brief Internally safe method for creating a PixelShader object.
		/// @param[in] fromResource Whether the PixelShader should be created from a resource file or a normal file.
		/// @param[in] filename The filename of the pixel shader.
		/// @return The created PixelShader object or NULL if failed.
		PixelShader* _createPixelShaderFromSource(bool fromResource, chstr filename);
		/// @brief Internally safe method for creating a VertexShader object.
		/// @param[in] fromResource Whether the VertexShader should be created from a resource file or a normal file.
		/// @param[in] filename The filename of the vertex shader.
		/// @return The created VertexShader object or NULL if failed.
		VertexShader* _createVertexShaderFromSource(bool fromResource, chstr filename);

		/// @brief Updates the device state based on the current render system state. This method only updates things tha have changed to improve performance.
		/// @param[in] forceUpdate If true, will force an update of the entire device state, regardless of the current state.
		/// @note The parameter forceUpdate is useful when the device is in an unknown or inconsistent state, but should be used with care as it invalidates all optimizations.
		virtual void _updateDeviceState(RenderState* state, bool forceUpdate = false);
		/// @brief Adds a render command to the queue.
		/// @param[in] command The command to add.
		void _addAsyncCommand(AsyncCommand* command);
		/// @brief Adds a special texture-unload command to the queue.
		/// @param[in] command The command to add.
		void _addUnloadTextureCommand(UnloadTextureCommand* command);
		/// @brief Flushes all remaining async commands.
		void _flushAsyncCommands();

		/// @brief Routing method for async creation.
		/// @param[in] options The options to use.
		void _systemCreate(Options options);
		/// @brief Routing method for async destruction.
		void _systemDestroy();
		/// @brief Routing method for async assignment of window.
		/// @param[in] window The window instance.
		void _systemAssignWindow(Window* window);

		/// @brief Initializes everything internal for the RenderSystem
		virtual void _deviceInit() = 0;
		/// @brief Creates the internal device.
		/// @param[in] options The options to use.
		/// @return True if successful.
		virtual bool _deviceCreate(Options options) = 0;
		/// @brief Destroys the internal device.
		/// @return True if successful.
		virtual bool _deviceDestroy() = 0;
		/// @brief Calls the internal Window assignment code.
		virtual void _deviceAssignWindow(Window* window) = 0;
		/// @brief Resets the internal device.
		virtual void _deviceReset();
		/// @brief Suspends the internal device.
		virtual void _deviceSuspend();
		/// @brief Sets up the capabilities listing.
		virtual void _deviceSetupCaps() = 0;
		/// @brief Sets up the internal device.
		virtual void _deviceSetup() = 0;
		/// @brief Sets up the supported display modes listing.
		virtual void _deviceSetupDisplayModes();

		/// @brief Creates the actual device Texture.
		/// @param[in] fromResource Whether the Texture is created from a resource file or a normal file.
		/// @return The created Texture object or NULL if not supported on this RenderSystem.
		virtual Texture* _deviceCreateTexture(bool fromResource) = 0;
		/// @brief Creates the actual device PixelShader.
		/// @return The created PixelShader object or NULL if not supported on this RenderSystem.
		virtual PixelShader* _deviceCreatePixelShader();
		/// @brief Creates the actual device VertexShader.
		/// @return The created VertexShader object or NULL if not supported on this RenderSystem.
		virtual VertexShader* _deviceCreateVertexShader();

		/// @brief Changes the device backbuffer resolution and fullscreen mode.
		/// @param[in] width Width of the new backbuffer in pixels.
		/// @param[in] height Height of the new backbuffer in pixels.
		/// @param[in] fullscreen Whether the device is in fullscreen or not.
		virtual void _deviceChangeResolution(int width, int height, bool fullscreen);

		/// @brief Sets the device viewport.
		/// @param[in] rect The viewport rectangle.
		virtual void _setDeviceViewport(cgrecti rect) = 0;
		/// @brief Sets the device modelview matrix.
		/// @param[in] matrix The current modelview matrix.
		virtual void _setDeviceModelviewMatrix(const gmat4& matrix) = 0;
		/// @brief Sets the device projection matrix.
		/// @param[in] matrix The current projection matrix.
		virtual void _setDeviceProjectionMatrix(const gmat4& matrix) = 0;
		/// @brief Sets the device depth-buffer state.
		/// @param[in] enabled Whether to enable or disable the device depth-buffer itself.
		/// @param[in] writeEnabled Whether writing to the device depth-buffer is enabled.
		virtual void _setDeviceDepthBuffer(bool enabled, bool writeEnabled) = 0;
		/// @brief Sets the device's state regarding vertex format.
		/// @param[in] useTexture Whether the vertices support UV coordinates.
		/// @param[in] useColor Whether the vertices support separate color values.
		virtual void _setDeviceRenderMode(bool useTexture, bool useColor) = 0;
		/// @brief Sets the device Texture.
		/// @param[in] texture The current texture.
		virtual void _setDeviceTexture(Texture* texture) = 0;
		/// @brief Sets the current Texture's filter.
		/// @param[in] textureFilter The current texture filter.
		virtual void _setDeviceTextureFilter(const Texture::Filter& textureFilter) = 0;
		/// @brief Sets the current Texture's address mode.
		/// @param[in] textureAddressMode The current texture address mode.
		virtual void _setDeviceTextureAddressMode(const Texture::AddressMode& textureAddressMode) = 0;
		/// @brief Sets the device blending mode for rendering.
		/// @param[in] blendMode The blending mode to be set for rendering.
		virtual void _setDeviceBlendMode(const BlendMode& blendMode) = 0;
		/// @brief Sets the device color mode for rendering.
		/// @param[in] colorMode The color mode to be set for rendering.
		/// @param[in] colorModeFactor The additioanl color mode factor.
		/// @param[in] useTexture Whether the vertices support UV coordinates.
		/// @param[in] useColor Whether the vertices support separate color values.
		/// @param[in] systemColor The global color for all vertices if useColor is set to false.
		virtual void _setDeviceColorMode(const ColorMode& colorMode, float colorModeFactor, bool useTexture, bool useColor, const Color& systemColor) = 0;

		/// @brief Renders an array of vertices to the backbuffer.
		/// @param[in] renderOperation The RenderOperation that should be used to render the vertices.
		/// @param[in] vertices An array of vertices.
		/// @param[in] count How many vertices from the array should be rendered.
		/// @note Calling this will effectively set the current texture to NULL.
		/// @see render
		void _renderInternal(const RenderOperation& renderOperation, const PlainVertex* vertices, int count);
		/// @brief Renders an array of vertices to the backbuffer.
		/// @param[in] renderOperation The RenderOperation that should be used to render the vertices.
		/// @param[in] vertices An array of vertices.
		/// @param[in] count How many vertices from the array should be rendered.
		/// @param[in] color Color to apply globally on all vertices.
		/// @note Calling this will effectively set the current texture to NULL.
		/// @see render
		void _renderInternal(const RenderOperation& renderOperation, const PlainVertex* vertices, int count, const Color& color);
		/// @brief Renders an array of vertices to the backbuffer.
		/// @param[in] renderOperation The RenderOperation that should be used to render the vertices.
		/// @param[in] vertices An array of vertices.
		/// @param[in] count How many vertices from the array should be rendered.
		/// @see render
		void _renderInternal(const RenderOperation& renderOperation, const TexturedVertex* vertices, int count);
		/// @brief Renders an array of vertices to the backbuffer.
		/// @param[in] renderOperation The RenderOperation that should be used to render the vertices.
		/// @param[in] vertices An array of vertices.
		/// @param[in] count How many vertices from the array should be rendered.
		/// @param[in] color Color to apply globally on all vertices.
		/// @see render
		void _renderInternal(const RenderOperation& renderOperation, const TexturedVertex* vertices, int count, const Color& color);
		/// @brief Renders an array of vertices to the backbuffer.
		/// @param[in] renderOperation The RenderOperation that should be used to render the vertices.
		/// @param[in] vertices An array of vertices.
		/// @param[in] count How many vertices from the array should be rendered.
		/// @note Calling this will effectively set the current texture to NULL.
		/// @see render
		void _renderInternal(const RenderOperation& renderOperation, const ColoredVertex* vertices, int count);
		/// @brief Renders an array of vertices to the backbuffer.
		/// @param[in] renderOperation The RenderOperation that should be used to render the vertices.
		/// @param[in] vertices An array of vertices.
		/// @param[in] count How many vertices from the array should be rendered.
		/// @see render
		void _renderInternal(const RenderOperation& renderOperation, const ColoredTexturedVertex* vertices, int count);
		/// @brief Renders a rectangle.
		/// @param[in] rect Position and size of the rectangle.
		/// @param[in] color Color of the rectangle.
		/// @note Calling this will effectively set the current texture to NULL.
		/// @see drawRect
		void _drawRectInternal(cgrectf rect, const Color& color);
		/// @brief Renders a rectangle filled with a color.
		/// @param[in] rect Position and size of the rectangle.
		/// @param[in] color Color of the rectangle.
		/// @note Calling this will effectively set the current texture to NULL.
		/// @see drawFilledRect
		void _drawFilledRectInternal(cgrectf rect, const Color& color);
		/// @brief Renders a textured rectangle.
		/// @param[in] rect Position and size of the rectangle.
		/// @param[in] src UV rectangle on the currently set Texture.
		/// @note Remember to call setTexture() before calling this.
		/// @see setTexture
		/// @see drawTexturedRect
		void _drawTexturedRectInternal(cgrectf rect, cgrectf src);
		/// @brief Renders a textured rectangle.
		/// @param[in] rect Position and size of the rectangle.
		/// @param[in] src UV rectangle on the currently set Texture.
		/// @param[in] color Color that should be applied to the texture.
		/// @note Remember to call setTexture() before calling this.
		/// @see setTexture
		/// @see drawTexturedRect
		void _drawTexturedRectInternal(cgrectf rect, cgrectf src, const Color& color);
		/// @brief Increases all relevant rendering stats.
		/// @param[in] renderOperation The RenderOperation that should be used to render the vertices.
		/// @param[in] count How many vertices will be rendered.
		/// @see render
		void _increaseStats(const RenderOperation& renderOperation, int count);

		/// @brief Clears the device backbuffer.
		/// @param[in] depth If true, clears the depth-buffer as well.
		virtual void _deviceClear(bool depth) = 0;
		/// @brief Clears the device backbuffer with a certain color.
		/// @param[in] color Color that is used to fill the backbuffer.
		/// @param[in] depth If true, clears the depth buffer as well.
		/// @note The parameter color is not applied to the depth-buffer.
		virtual void _deviceClear(const Color& color, bool depth) = 0;
		/// @brief Clears the device depth-buffer.
		virtual void _deviceClearDepth() = 0;
		/// @brief Executes the final renders call for a vertex array.
		/// @param[in] renderOperation The RenderOperation that should be used to render the vertices.
		/// @param[in] vertices An array of vertices.
		/// @param[in] count How many vertices from the array should be rendered.
		virtual void _deviceRender(const RenderOperation& renderOperation, const PlainVertex* vertices, int count) = 0;
		/// @brief Executes the final renders call for a vertex array.
		/// @param[in] renderOperation The RenderOperation that should be used to render the vertices.
		/// @param[in] vertices An array of vertices.
		/// @param[in] count How many vertices from the array should be rendered.
		virtual void _deviceRender(const RenderOperation& renderOperation, const TexturedVertex* vertices, int count) = 0;
		/// @brief Executes the final renders call for a vertex array.
		/// @param[in] renderOperation The RenderOperation that should be used to render the vertices.
		/// @param[in] vertices An array of vertices.
		/// @param[in] count How many vertices from the array should be rendered.
		virtual void _deviceRender(const RenderOperation& renderOperation, const ColoredVertex* vertices, int count) = 0;
		/// @brief Executes the final renders call for a vertex array.
		/// @param[in] renderOperation The RenderOperation that should be used to render the vertices.
		/// @param[in] vertices An array of vertices.
		/// @param[in] count How many vertices from the array should be rendered.
		virtual void _deviceRender(const RenderOperation& renderOperation, const ColoredTexturedVertex* vertices, int count) = 0;
		/// @brief Flushes the currently rendered data to the backbuffer for display.
		/// @param[in] systemEnabled Whether the system present is actually enabled.
		virtual void _devicePresentFrame(bool systemEnabled);
		/// @brief Renders previous frame again.
		/// @param[in] systemEnabled Whether the system present is actually enabled.
		/// @see _devicePresentFrame
		virtual void _deviceRepeatLastFrame(bool systemEnabled);
		/// @brief Copies RenderTarget data from one texture to another.
		/// @note Both textures must be render targets.
		virtual void _deviceCopyRenderTargetData(Texture* source, Texture* destination);
		/// @brief Takes a screenshot a.k.a. captures the image data of the backbuffer.
		/// @param[in] format The format to which the screenshot should be converted.
		virtual void _deviceTakeScreenshot(Image::Format format);
		/// @brief Updates the intermediate render textures.
		void _updateIntermediateRenderTextures();
		/// @brief Creates the intermediate render textures.
		/// @param[in] width The texture width.
		/// @param[in] height The texture height.
		/// @return True if successful.
		bool _tryCreateIntermediateRenderTextures(int width, int height);
		/// @brief Destroys the intermediate render texture.
		/// @return True if successful.
		bool _tryDestroyIntermediateRenderTextures();
		/// @brief Renders the actual intermediate render texture.
		void _presentIntermediateRenderTexture();
		/// @brief Unloads all textures. Used internally only.
		/// @note Useful for clearing all memory or if something invalidates textures and cannot guarantee that they are loaded anymore.
		void _deviceUnloadTextures();

		/// @brief Calculates the number of primitives based on the number of vertices.
		/// @param[in] renderOperation The RenderOperation that is used for rendering
		/// @param[in] count How many vertices are expected to be rendered.
		/// @return How many primitives the given vertices represent.
		unsigned int _numPrimitives(const RenderOperation& renderOperation, int count) const;
		/// @brief Corrects the number of vertices allowed to be used in the given RenderOperation to avoid glitches and crashes.
		/// @param[in] renderOperation The RenderOperation that is used for rendering
		/// @param[in] count How many vertices are expected to be rendered.
		/// @return How many vertices are allowed to be used in the given RenderOperation.
		unsigned int _limitVertices(const RenderOperation& renderOperation, int count) const;

	private:
		/// @brief Queued value for frameDuplicates.
		int _queuedFrameDuplicates;
		/// @brief Number of remaining render target duplicates.
		int _renderTargetDuplicatesCount;
		/// @brief Mutex required for handling frameDuplicates change.
		hmutex _frameDuplicatesMutex;

	};

	/// @brief The global RenderSystem instance.
	aprilExport extern april::RenderSystem* rendersys;
	
}
#endif
