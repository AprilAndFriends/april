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
/// Defines a generic render system.

#ifndef APRIL_H
#define APRIL_H

#include <hltypes/hstring.h>
#include <hltypes/harray.h>

#include "aprilExport.h"
#include "RenderSystem.h"
#include "Window.h"

/// @brief Defines the name for the OpenGL 1 render system.
#define APRIL_RS_OPENGL1 "OpenGL1"
/// @brief Defines the name for the OpenGLES 1 render system.
#define APRIL_RS_OPENGLES1 "OpenGLES1"
/// @brief Defines the name for the OpenGLES 2 render system.
#define APRIL_RS_OPENGLES2 "OpenGLES2"
/// @brief Defines the name for the DirectX 9 render system.
#define APRIL_RS_DIRECTX9 "DirectX9"
/// @brief Defines the name for the DirectX 11 render system.
#define APRIL_RS_DIRECTX11 "DirectX11"

/// @brief Defines the name for Win32 window system.
#define APRIL_WS_WIN32 "Win32"
/// @brief Defines the name for WinRT window system.
#define APRIL_WS_WINRT "WinRT"
/// @brief Defines the name for SDL window system.
#define APRIL_WS_SDL "SDL"
/// @brief Defines the name for Mac window system.
#define APRIL_WS_MAC "Mac"
/// @brief Defines the name for iOS window system.
#define APRIL_WS_IOS "iOS"
/// @brief Defines the name for Android window system.
#define APRIL_WS_ANDROIDJNI "AndroidJNI"
/// @brief Defines the name for OpenKODE window system.
#define APRIL_WS_OPENKODE "OpenKODE"

namespace april
{
	class RenderSystem;
	class Window;

	extern hstr logTag;

	/// @brief Defines render system types.
	enum RenderSystemType
	{
		/// @brief Use platform default render system if available.
		RS_DEFAULT = 0,
		/// @brief Use DirectX 9 render system if available.
		RS_DIRECTX9 = 1,
		/// @brief Use DirectX 11 render system if available.
		RS_DIRECTX11 = 2,
		/// @brief Use OpenGL 1 render system if available.
		RS_OPENGL1 = 3,
		/// @brief Use OpenGLES 1 render system if available.
		RS_OPENGLES1 = 4,
		/// @brief Use OpenGLES 2 render system if available.
		RS_OPENGLES2 = 5
	};

	/// @brief Defines window system types.
	enum WindowType
	{
		/// @brief Use platform default window system if available.
		WS_DEFAULT = 0,
		/// @brief Use Win32 window system if available.
		WS_WIN32 = 1,
		/// @brief Use WinRT window system if available.
		WS_WINRT = 2,
		/// @brief Use SDL window system if available.
		WS_SDL = 3,
		/// @brief Use Mac window system if available.
		WS_MAC = 4,
		/// @brief Use iOS window system if available.
		WS_IOS = 5,
		/// @brief Use Android window system if available.
		WS_ANDROIDJNI = 6,
		/// @brief Use OpenKODE window system if available.
		WS_OPENKODE = 7
	};

	/// @brief Initializes APRIL.
	/// @param[in] renderSystemType Which render system should be used.
	/// @param[in] windowSystemType Which window system should be used.
	/// @note After this, you still have to call createRenderSystem() and createWindow().
	/// @see createRenderSystem()
	/// @see createWindow()
	aprilFnExport void init(RenderSystemType renderSystemType, WindowType windowType);
	/// @brief Initializes APRIL.
	/// @param[in] customRenderSystem Custom implementation for RenderSystem.
	/// @param[in] windowSystemType Which window system should be used.
	/// @note After this, you still have to call createRenderSystem() and createWindow().
	/// @see createRenderSystem()
	/// @see createWindow()
	aprilFnExport void init(RenderSystem* customRenderSystem, WindowType windowType);
	/// @brief Initializes APRIL.
	/// @param[in] renderSystemType Which render system should be used.
	/// @param[in] customWindow Custom implementation for Window.
	/// @note After this, you still have to call createRenderSystem() and createWindow().
	/// @see createRenderSystem()
	/// @see createWindow()
	aprilFnExport void init(RenderSystemType renderSystemType, Window* customWindow);
	/// @brief Initializes APRIL.
	/// @param[in] customRenderSystem Custom implementation for RenderSystem.
	/// @param[in] customWindow Custom implementation for Window.
	/// @note After this, you still have to call createRenderSystem() and createWindow().
	/// @see createRenderSystem()
	/// @see createWindow()
	aprilFnExport void init(RenderSystem* customRenderSystem, Window* customWindow);
	/// @brief Initializes APRIL.
	/// @param[in] renderSystemType Which render system should be used.
	/// @param[in] windowSystemType Which window system should be used.
	/// @param[in] renderSystemOptions Define any special options you need for the render system.
	/// @param[in] w Window width.
	/// @param[in] h Winnow height.
	/// @param[in] fullscreen Whether the window should be displayed in fullscreen.
	/// @param[in] title Window title (usually displayed in title bar if platform supports).
	/// @param[in] windowOptions Define any special options you need for the window.
	/// @note After this, you mustn't call createRenderSystem() or createWindow() anymore.
	/// @see createRenderSystem()
	/// @see createWindow()
	aprilFnExport void init(RenderSystemType renderSystemType, WindowType windowType,
		RenderSystem::Options renderSystemOptions, int w, int h, bool fullscreen, chstr title, Window::Options windowOptions);
	/// @brief Initializes APRIL.
	/// @param[in] customRenderSystem Custom implementation for RenderSystem.
	/// @param[in] windowSystemType Which window system should be used.
	/// @param[in] renderSystemOptions Define any special options you need for the render system.
	/// @param[in] w Window width.
	/// @param[in] h Winnow height.
	/// @param[in] fullscreen Whether the window should be displayed in fullscreen.
	/// @param[in] title Window title (usually displayed in title bar if platform supports).
	/// @param[in] windowOptions Define any special options you need for the window.
	/// @note After this, you still have to call createRenderSystem() and createWindow().
	/// @see createRenderSystem()
	/// @see createWindow()
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
	/// @see createRenderSystem()
	/// @see createWindow()
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
	/// @see createRenderSystem()
	/// @see createWindow()
	aprilFnExport void init(RenderSystem* customRenderSystem, Window* customWindow,
		RenderSystem::Options renderSystemOptions, int w, int h, bool fullscreen, chstr title, Window::Options windowOptions);
	/// @brief Creates the actual render system.
	/// @param[in] options Define any special options you need for the render system.
	/// @note This should be called after init().
	/// @see init()
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
	/// @see createRenderSystem()
	aprilFnExport void createWindow(int w, int h, bool fullscreen, chstr title, Window::Options options = Window::Options());
	/// @brief Destroys APRIL.
	aprilFnExport void destroy();
	/// @brief Adds a file extension for extension-insensitive texture filenames.
	/// @param[in] extension The new extension.
	/// @note Extensions must always include the "." (dot) character.
	aprilFnExport void addTextureExtension(chstr extension);
	/// @brief Gets the list of all file extensions for extension-insensitive texture filenames.
	/// @param[in] extension The new extension.
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

}

#endif
