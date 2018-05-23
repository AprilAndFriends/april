/// @file
/// @version 5.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Defines API functions.

#ifndef APRIL_H
#define APRIL_H

#include <hltypes/hstring.h>
#include <hltypes/harray.h>

#include "aprilExport.h"
#include "RenderSystem.h"
#include "Window.h"

namespace april
{
	class RenderSystem;
	class Window;

	/// @brief Used for logging display.
	aprilExport extern hstr logTag;

	/// @class RenderSystemType
	/// @brief Defines render system types.
	HL_ENUM_CLASS_PREFIX_DECLARE(aprilExport, RenderSystemType,
	(
		/// @var static const RenderSystemType RenderSystemType::Default
		/// @brief Use platform default render system if available.
		HL_ENUM_DECLARE(RenderSystemType, Default);
		/// @var static const RenderSystemType RenderSystemType::DirectX9
		/// @brief Use DirectX 9 render system if available.
		HL_ENUM_DECLARE(RenderSystemType, DirectX9);
		/// @var static const RenderSystemType RenderSystemType::DirectX11
		/// @brief Use DirectX 11 render system if available.
		HL_ENUM_DECLARE(RenderSystemType, DirectX11);
		/// @var static const RenderSystemType RenderSystemType::DirectX12
		/// @brief Use DirectX 12 render system if available.
		HL_ENUM_DECLARE(RenderSystemType, DirectX12);
		/// @var static const RenderSystemType RenderSystemType::OpenGL1
		/// @brief Use OpenGL 1 render system if available.
		HL_ENUM_DECLARE(RenderSystemType, OpenGL1);
		/// @var static const RenderSystemType RenderSystemType::OpenGLES2
		/// @brief Use OpenGLES 2 render system if available.
		HL_ENUM_DECLARE(RenderSystemType, OpenGLES2);
	));

	/// @class WindowType
	/// @brief Defines window system types.
	HL_ENUM_CLASS_PREFIX_DECLARE(aprilExport, WindowType,
	(
		/// @var static const WindowType WindowType::Default
		/// @brief Use platform default window system if available.
		HL_ENUM_DECLARE(WindowType, Default);
		/// @var static const WindowType WindowType::Win32
		/// @brief Use Win32 window system if available.
		HL_ENUM_DECLARE(WindowType, Win32);
		/// @var static const WindowType WindowType::WinRT
		/// @brief Use WinRT window system if available.
		HL_ENUM_DECLARE(WindowType, WinRT);
		/// @var static const WindowType WindowType::WinUWP
		/// @brief Use WinUWP window system if available.
		HL_ENUM_DECLARE(WindowType, WinUWP);
		/// @var static const WindowType WindowType::SDL
		/// @brief Use SDL window system if available.
		HL_ENUM_DECLARE(WindowType, SDL);
		/// @var static const WindowType WindowType::Mac
		/// @brief Use Mac window system if available.
		HL_ENUM_DECLARE(WindowType, Mac);
		/// @var static const WindowType WindowType::iOS
		/// @brief Use iOS window system if available.
		HL_ENUM_DECLARE(WindowType, iOS);
		/// @var static const WindowType WindowType::AndroidJNI
		/// @brief Use Android window system if available.
		HL_ENUM_DECLARE(WindowType, AndroidJNI);
		/// @var static const WindowType WindowType::OpenKODE
		/// @brief Use OpenKODE window system if available.
		HL_ENUM_DECLARE(WindowType, OpenKODE);
	));

	/// @brief Initializes APRIL.
	/// @param[in] renderSystemType Which render system should be used.
	/// @param[in] windowType Which window system should be used.
	/// @note After this, you still have to call createRenderSystem() and createWindow().
	/// @see createRenderSystem
	/// @see createWindow
	aprilFnExport void init(RenderSystemType renderSystemType, WindowType windowType);
	/// @brief Initializes APRIL.
	/// @param[in] customRenderSystem Custom implementation for RenderSystem.
	/// @param[in] windowType Which window system should be used.
	/// @note After this, you still have to call createRenderSystem() and createWindow().
	/// @see createRenderSystem
	/// @see createWindow
	aprilFnExport void init(RenderSystem* customRenderSystem, WindowType windowType);
	/// @brief Initializes APRIL.
	/// @param[in] renderSystemType Which render system should be used.
	/// @param[in] customWindow Custom implementation for Window.
	/// @note After this, you still have to call createRenderSystem() and createWindow().
	/// @see createRenderSystem
	/// @see createWindow
	aprilFnExport void init(RenderSystemType renderSystemType, Window* customWindow);
	/// @brief Initializes APRIL.
	/// @param[in] customRenderSystem Custom implementation for RenderSystem.
	/// @param[in] customWindow Custom implementation for Window.
	/// @note After this, you still have to call createRenderSystem() and createWindow().
	/// @see createRenderSystem
	/// @see createWindow
	aprilFnExport void init(RenderSystem* customRenderSystem, Window* customWindow);
	/// @brief Initializes APRIL.
	/// @param[in] renderSystemType Which render system should be used.
	/// @param[in] windowType Which window system should be used.
	/// @param[in] renderSystemOptions Define any special options you need for the render system.
	/// @param[in] w Window width.
	/// @param[in] h Winnow height.
	/// @param[in] fullscreen Whether the window should be displayed in fullscreen.
	/// @param[in] title Window title (usually displayed in title bar if platform supports).
	/// @param[in] windowOptions Define any special options you need for the window.
	/// @note After this, you mustn't call createRenderSystem() or createWindow() anymore.
	/// @see createRenderSystem
	/// @see createWindow
	aprilFnExport void init(RenderSystemType renderSystemType, WindowType windowType,
		RenderSystem::Options renderSystemOptions, int w, int h, bool fullscreen, chstr title, Window::Options windowOptions);
	/// @brief Initializes APRIL.
	/// @param[in] customRenderSystem Custom implementation for RenderSystem.
	/// @param[in] windowType Which window system should be used.
	/// @param[in] renderSystemOptions Define any special options you need for the render system.
	/// @param[in] w Window width.
	/// @param[in] h Winnow height.
	/// @param[in] fullscreen Whether the window should be displayed in fullscreen.
	/// @param[in] title Window title (usually displayed in title bar if platform supports).
	/// @param[in] windowOptions Define any special options you need for the window.
	/// @note After this, you still have to call createRenderSystem() and createWindow().
	/// @see createRenderSystem
	/// @see createWindow
	aprilFnExport void init(RenderSystem* customRenderSystem, WindowType windowType,
		RenderSystem::Options renderSystemOptions, int w, int h, bool fullscreen, chstr title, Window::Options windowOptions);
	/// @brief Initializes APRIL.
	/// @param[in] renderSystemType Which render system should be used.
	/// @param[in] customWindow Custom implementation for Window.
	/// @param[in] renderSystemOptions Define any special options you need for the render system.
	/// @param[in] w Window width.
	/// @param[in] h Winnow height.
	/// @param[in] fullscreen Whether the window should be displayed in fullscreen.
	/// @param[in] title Window title (usually displayed in title bar if platform supports).
	/// @param[in] windowOptions Define any special options you need for the window.
	/// @note After this, you still have to call createRenderSystem() and createWindow().
	/// @see createRenderSystem
	/// @see createWindow
	aprilFnExport void init(RenderSystemType renderSystemType, Window* customWindow,
		RenderSystem::Options renderSystemOptions, int w, int h, bool fullscreen, chstr title, Window::Options windowOptions);
	/// @brief Initializes APRIL.
	/// @param[in] customRenderSystem Custom implementation for RenderSystem.
	/// @param[in] customWindow Custom implementation for Window.
	/// @param[in] renderSystemOptions Define any special options you need for the render system.
	/// @param[in] w Window width.
	/// @param[in] h Winnow height.
	/// @param[in] fullscreen Whether the window should be displayed in fullscreen.
	/// @param[in] title Window title (usually displayed in title bar if platform supports).
	/// @param[in] windowOptions Define any special options you need for the window.
	/// @note After this, you still have to call createRenderSystem() and createWindow().
	/// @see createRenderSystem
	/// @see createWindow
	aprilFnExport void init(RenderSystem* customRenderSystem, Window* customWindow,
		RenderSystem::Options renderSystemOptions, int w, int h, bool fullscreen, chstr title, Window::Options windowOptions);
	/// @brief Creates the actual render system.
	/// @param[in] options Define any special options you need for the render system.
	/// @note This should be called after init().
	/// @see init
	aprilFnExport void createRenderSystem(RenderSystem::Options options = RenderSystem::Options());
	/// @brief Creates the actual window.
	/// @param[in] w Window width.
	/// @param[in] h Winnow height.
	/// @param[in] fullscreen Whether the window should be displayed in fullscreen.
	/// @param[in] title Window title (usually displayed in title bar if platform supports).
	/// @param[in] options Define any special options you need for the window.
	/// @note "fullscreen" works only on certain platforms.
	/// @note "title" works only on platforms where this is supported and usually displays the name in the title bar.
	/// @note This should be called after createRenderSystem().
	/// @see createRenderSystem
	aprilFnExport void createWindow(int w, int h, bool fullscreen, chstr title, Window::Options options = Window::Options());
	/// @brief Destroys APRIL.
	aprilFnExport void destroy();
	/// @brief Adds a file extension for extension-insensitive texture filenames.
	/// @param[in] extension The new extension.
	/// @note Extensions must always include the "." (dot) character.
	aprilFnExport void addTextureExtension(chstr extension);
	/// @brief Gets the list of all file extensions for extension-insensitive texture filenames.
	/// @return The list of all file extensions for extension-insensitive texture filenames.
	/// @note Extensions must always include the "." (dot) character.
	aprilFnExport harray<hstr> getTextureExtensions();
	/// @brief Sets a list of all file extensions for extension-insensitive texture filenames.
	/// @param[in] extensions The new extensions.
	/// @note Extensions must always include the "." (dot) character.
	aprilFnExport void setTextureExtensions(const harray<hstr>& extensions);
	/// @brief Gets the max number of async textures uploaded to the GPU per frame.
	/// @return The max number of async textures uploaded tio the GPU per frame.
	aprilFnExport int getMaxAsyncTextureUploadsPerFrame();
	/// @brief Sets the max number of async textures uploaded to the GPU per frame.
	/// @param[in] value The max number of async textures uploaded to the GPU per frame.
	/// @note A value of 0 or less indicates all currently loaded textures.
	aprilFnExport void setMaxAsyncTextureUploadsPerFrame(int value);
	/// @brief Gets the max number of async textures concurrently loaded in RAM and waiting for upload.
	/// @return The max number of async textures concurrently loaded in RAM and waiting for upload.
	aprilFnExport int getMaxWaitingAsyncTextures();
	/// @brief Sets the max number of async textures concurrently loaded in RAM and waiting for upload.
	/// @param[in] value The max number of async textures concurrently loaded in RAM and waiting for upload.
	/// @note A value of 0 or less indicates no limit.
	aprilFnExport void setMaxWaitingAsyncTextures(int value);
	/// @brief Adds a new symbolic Color.
	/// @param[in] symbolicName The name of the Color.
	/// @param[in] color The Color object.
	/// @note If a symbolic Color with the same name already exists, it will be overwritten.
	aprilFnExport void addSymbolicColor(chstr symbolicName, const april::Color& color);
	/// @brief Loads a list of symbolic Color instances from a formatted file.
	/// @param[in] filename The filename.
	/// @note If a symbolic Color with the same name already exists, it will be overwritten.
	/// @note The format is "color:name", one entry per line. Lines starting with # are regarded as comments and will be ignored.
	aprilFnExport void loadSymbolicColorsFromFile(chstr filename);
	/// @brief Loads a list of symbolic Color instances from a formatted resource file.
	/// @param[in] filename The filename.
	/// @note If a symbolic Color with the same name already exists, it will be overwritten.
	/// @note The format is "color:name", one entry per line. Lines starting with # are regarded as comments and will be ignored.
	aprilFnExport void loadSymbolicColorsFromResource(chstr filename);
	/// @brief Finds the Color with the given symbolic name.
	/// @param[in] symbolicName The name of the Color.
	/// @param[out] color The output Color object.
	/// @return True if Color exists and return succeeded. False if Color under symbolicName does not exist.
	aprilFnExport bool findSymbolicColor(chstr symbolicName, april::Color& color);
	/// @brief Finds the Color with the given symbolic name.
	/// @param[in] symbolicName The name of the Color.
	/// @param[out] hexColor The output hex string for the color.
	/// @return True if Color exists and return succeeded. False if Color under symbolicName does not exist.
	aprilFnExport bool findSymbolicColor(chstr symbolicName, hstr& hexColor);
	/// @brief Resets symbolic colors to default.
	aprilFnExport void resetSymbolicColors();

}
#endif
