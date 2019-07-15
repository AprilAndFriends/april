/// @file
/// @version 5.2
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#ifdef _OPENGL

#define __HL_INCLUDE_PLATFORM_HEADERS
#include <hltypes/hplatform.h>
#if __APPLE__
	#include <TargetConditionals.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#if defined(_WIN32) && defined(_OPENGLES)
	#include <EGL/egl.h>
#endif

#include <gtypes/Rectangle.h>
#include <hltypes/hlog.h>
#include <hltypes/hstring.h>

#include "april.h"
#include "Color.h"
#include "Image.h"
#include "OpenGL_RenderSystem.h"
#include "OpenGL_Texture.h"
#include "Platform.h"
#include "RenderState.h"
#include "UpdateDelegate.h"
#ifdef _WIN32_WINDOW
#include "Win32_Window.h"
#endif

#ifdef __ANDROID__
#define _SEGMENTED_RENDERING
#endif
#define MAX_VERTEX_COUNT 65535

namespace april
{
	// translation from abstract render ops to gl's render ops
	int OpenGL_RenderSystem::_glRenderOperations[] =
	{
		GL_TRIANGLES,
		GL_TRIANGLE_STRIP,
		GL_LINES,
		GL_LINE_STRIP,
		GL_POINTS,
		GL_TRIANGLE_FAN,
	};

	OpenGL_RenderSystem::OpenGL_RenderSystem() :
		RenderSystem(),
		blendSeparationSupported(false),
		deviceState_vertexStride(0),
		deviceState_vertexPointer(NULL),
		deviceState_textureStride(0),
		deviceState_texturePointer(NULL),
		deviceState_colorStride(0),
		deviceState_colorPointer(NULL)
	{
		// because GL has to defy screen logic and has (0,0) in the bottom left corner
		for_iter (i, 0, APRIL_INTERMEDIATE_TEXTURE_VERTICES_COUNT)
		{
			this->_intermediateRenderVertices[i].y = -this->_intermediateRenderVertices[i].y;
		}
#if defined(_WIN32) && !defined(_UWP)
		this->hWnd = 0;
		this->hDC = 0;
#endif
	}

	void OpenGL_RenderSystem::_deviceInit()
	{
		this->deviceState_vertexStride = 0;
		this->deviceState_vertexPointer = NULL;
		this->deviceState_textureStride = 0;
		this->deviceState_texturePointer = NULL;
		this->deviceState_colorStride = 0;
		this->deviceState_colorPointer = NULL;
#if defined(_WIN32) && !defined(_UWP)
		this->hWnd = 0;
		this->hDC = 0;
#endif
	}

	bool OpenGL_RenderSystem::_deviceCreate(RenderSystem::Options options)
	{
		return true;
	}

	bool OpenGL_RenderSystem::_deviceDestroy()
	{
#if defined(_WIN32) && !defined(_UWP)
		this->_releaseWindow();
#endif
		return true;
	}

	void OpenGL_RenderSystem::_deviceAssignWindow(Window* window)
	{
#if defined(_WIN32) && !defined(_UWP)
		this->_initWin32(window);
#endif
	}

	void OpenGL_RenderSystem::_deviceSetupCaps()
	{
		GL_SAFE_CALL(glGetIntegerv, (GL_MAX_TEXTURE_SIZE, &this->caps.maxTextureSize));
	}

#if defined(_WIN32) && !defined(_UWP)
	void OpenGL_RenderSystem::_releaseWindow()
	{
		if (this->hDC != 0)
		{
			ReleaseDC(this->hWnd, this->hDC);
			this->hDC = 0;
		}
	}

	bool OpenGL_RenderSystem::_initWin32(Window* window)
	{
		this->hWnd = (HWND)window->getBackendId();
		this->hDC = GetDC(this->hWnd);
		if (this->hDC == 0)
		{
			hlog::error(logTag, "Can't create a GL device context!");
			return false;
		}
		return true;
	}
#endif

	void OpenGL_RenderSystem::_deviceSetup()
	{
		GL_SAFE_CALL(glClearColor, (0.0f, 0.0f, 0.0f, 1.0f));
		// GL defaults
		GL_SAFE_CALL(glEnable, (GL_BLEND));
		// pixel data
		GL_SAFE_CALL(glPixelStorei, (GL_UNPACK_ALIGNMENT, 1));
		GL_SAFE_CALL(glPixelStorei,(GL_PACK_ALIGNMENT, 1));
		// other
		if (this->options.depthBuffer)
		{
			GL_SAFE_CALL(glDepthFunc, (GL_LEQUAL));
		}
		this->_setGlTextureEnabled(this->deviceState->useTexture);
		this->_setGlColorEnabled(this->deviceState->useColor);
		this->_setGlVertexPointer(this->deviceState_vertexStride, this->deviceState_vertexPointer);
		this->_setGlTexturePointer(this->deviceState_textureStride, this->deviceState_texturePointer);
		this->_setGlColorPointer(this->deviceState_colorStride, this->deviceState_colorPointer);
	}

	int OpenGL_RenderSystem::getVRam() const
	{
		return 0; // this API is likely not available on OpenGL
	}

	void OpenGL_RenderSystem::_deviceChangeResolution(int width, int height, bool fullscreen)
	{
		grecti viewport(0, 0, width, height);
		this->setViewport(viewport);
		this->setOrthoProjection(viewport);
		this->_updateDeviceState(this->state, true);
	}

	void OpenGL_RenderSystem::_setDeviceViewport(cgrecti rect)
	{
		// because GL has to defy screen logic and has (0,0) in the bottom left corner
		GL_SAFE_CALL(glViewport, (rect.x, april::window->getHeight() - rect.h - rect.y, rect.w, rect.h));
	}

	void OpenGL_RenderSystem::_setDeviceDepthBuffer(bool enabled, bool writeEnabled)
	{
		if (enabled)
		{
			GL_SAFE_CALL(glEnable, (GL_DEPTH_TEST))
		}
		else
		{
			GL_SAFE_CALL(glDisable, (GL_DEPTH_TEST));
		}
		GL_SAFE_CALL(glDepthMask, (writeEnabled));
	}

	void OpenGL_RenderSystem::_setDeviceRenderMode(bool useTexture, bool useColor)
	{
		// since GL sets these separately, it makes sense to enable/disable them only when each one changes
		if (this->deviceState->useTexture != useTexture)
		{
			this->_setGlTextureEnabled(useTexture);
		}
		if (this->deviceState->useColor != useColor)
		{
			this->_setGlColorEnabled(useColor);
		}
	}

	void OpenGL_RenderSystem::_setDeviceTexture(Texture* texture)
	{
		if (texture != NULL)
		{
			GL_SAFE_CALL(glBindTexture, (GL_TEXTURE_2D, ((OpenGL_Texture*)texture)->textureId));
		}
		else
		{
			GL_SAFE_CALL(glBindTexture, (GL_TEXTURE_2D, 0));
		}
	}

	void OpenGL_RenderSystem::_setDeviceTextureFilter(const Texture::Filter& textureFilter)
	{
		if (textureFilter == Texture::Filter::Linear)
		{
			GL_SAFE_CALL(glTexParameteri, (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
			GL_SAFE_CALL(glTexParameteri, (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
		}
		else if (textureFilter == Texture::Filter::Nearest)
		{
			GL_SAFE_CALL(glTexParameteri, (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
			GL_SAFE_CALL(glTexParameteri, (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
		}
		else
		{
			hlog::warn(logTag, "Trying to set unsupported texture filter!");
		}
	}

	void OpenGL_RenderSystem::_setDeviceTextureAddressMode(const Texture::AddressMode& textureAddressMode)
	{
		if (textureAddressMode == Texture::AddressMode::Wrap)
		{
			GL_SAFE_CALL(glTexParameteri, (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
			GL_SAFE_CALL(glTexParameteri, (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
		}
		else if (textureAddressMode == Texture::AddressMode::Clamp)
		{
			GL_SAFE_CALL(glTexParameteri, (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
			GL_SAFE_CALL(glTexParameteri, (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
		}
		else
		{
			hlog::warn(logTag, "Trying to set unsupported texture address mode!");
		}
	}

	void OpenGL_RenderSystem::_setDeviceBlendMode(const BlendMode& blendMode)
	{
		if (blendMode == BlendMode::Alpha)
		{
			GL_SAFE_CALL(glBlendFunc, (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
		}
		else if (blendMode == BlendMode::Add)
		{
			GL_SAFE_CALL(glBlendFunc, (GL_SRC_ALPHA, GL_ONE));
		}
		else if (blendMode == BlendMode::Overwrite)
		{
			GL_SAFE_CALL(glBlendFunc, (GL_ONE, GL_ZERO));
		}
		else
		{
			GL_SAFE_CALL(glBlendFunc, (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
			hlog::warn(logTag, "Trying to set unsupported blend mode!");
		}
	}

	void OpenGL_RenderSystem::_deviceClear(bool depth)
	{
		GLbitfield mask = GL_COLOR_BUFFER_BIT;
		if (depth)
		{
			mask |= GL_DEPTH_BUFFER_BIT;
		}
		GL_SAFE_CALL(glClearColor, (0.0f, 0.0f, 0.0f, 1.0f));
		GL_SAFE_CALL(glClear, (mask));
	}

	void OpenGL_RenderSystem::_deviceClear(const Color& color, bool depth)
	{
		GLbitfield mask = GL_COLOR_BUFFER_BIT;
		if (depth)
		{
			mask |= GL_DEPTH_BUFFER_BIT;
		}
		GL_SAFE_CALL(glClearColor, (color.r_f(), color.g_f(), color.b_f(), color.a_f()));
		GL_SAFE_CALL(glClear, (mask));
	}

	void OpenGL_RenderSystem::_deviceClearDepth()
	{
		GL_SAFE_CALL(glClear, (GL_DEPTH_BUFFER_BIT));
	}

	void OpenGL_RenderSystem::_deviceRender(const RenderOperation& renderOperation, const PlainVertex* vertices, int count)
	{
		// This kind of approach to render chunks of vertices is caused by problems on OpenGLES
		// hardware that may allow only a certain amount of vertices to be rendered at the time.
		// Apparently that number is 65535 on HTC Evo 3D so this is used for MAX_VERTEX_COUNT by default.
		static int size = 0;
		size = count;
#ifdef _SEGMENTED_RENDERING
		for_iter_step (i, 0, count, size)
		{
			size = this->_limitVertices(renderOperation, hmin(count - i, MAX_VERTEX_COUNT));
#endif
			this->_setDeviceVertexPointer(sizeof(PlainVertex), vertices);
			GL_SAFE_CALL(glDrawArrays, (_glRenderOperations[renderOperation.value], 0, size));
#ifdef _SEGMENTED_RENDERING
			vertices += size;
		}
#endif
	}

	void OpenGL_RenderSystem::_deviceRender(const RenderOperation& renderOperation, const TexturedVertex* vertices, int count)
	{
		// This kind of approach to render chunks of vertices is caused by problems on OpenGLES
		// hardware that may allow only a certain amount of vertices to be rendered at the time.
		// Apparently that number is 65535 on HTC Evo 3D so this is used for MAX_VERTEX_COUNT by default.
		static int size = 0;
		size = count;
#ifdef _SEGMENTED_RENDERING
		for_iter_step (i, 0, count, size)
		{
			size = this->_limitVertices(renderOperation, hmin(count - i, MAX_VERTEX_COUNT));
#endif
			this->_setDeviceVertexPointer(sizeof(TexturedVertex), vertices);
			this->_setDeviceTexturePointer(sizeof(TexturedVertex), &vertices->u);
			GL_SAFE_CALL(glDrawArrays, (_glRenderOperations[renderOperation.value], 0, size));
#ifdef _SEGMENTED_RENDERING
			vertices += size;
		}
#endif
	}

	void OpenGL_RenderSystem::_deviceRender(const RenderOperation& renderOperation, const ColoredVertex* vertices, int count)
	{
		// This kind of approach to render chunks of vertices is caused by problems on OpenGLES
		// hardware that may allow only a certain amount of vertices to be rendered at the time.
		// Apparently that number is 65535 on HTC Evo 3D so this is used for MAX_VERTEX_COUNT by default.
		static int size = 0;
		size = count;
#ifdef _SEGMENTED_RENDERING
		for_iter_step (i, 0, count, size)
		{
			size = this->_limitVertices(renderOperation, hmin(count - i, MAX_VERTEX_COUNT));
#endif
			this->_setDeviceVertexPointer(sizeof(ColoredVertex), vertices);
			this->_setDeviceColorPointer(sizeof(ColoredVertex), &vertices->color);
			GL_SAFE_CALL(glDrawArrays, (_glRenderOperations[renderOperation.value], 0, size));
#ifdef _SEGMENTED_RENDERING
			vertices += size;
		}
#endif
	}

	void OpenGL_RenderSystem::_deviceRender(const RenderOperation& renderOperation, const ColoredTexturedVertex* vertices, int count)
	{
		// This kind of approach to render chunks of vertices is caused by problems on OpenGLES
		// hardware that may allow only a certain amount of vertices to be rendered at the time.
		// Apparently that number is 65535 on HTC Evo 3D so this is used for MAX_VERTEX_COUNT by default.
		static int size = 0;
		size = count;
#ifdef _SEGMENTED_RENDERING
		for_iter_step (i, 0, count, size)
		{
			size = this->_limitVertices(renderOperation, hmin(count - i, MAX_VERTEX_COUNT));
#endif
			this->_setDeviceVertexPointer(sizeof(ColoredTexturedVertex), vertices);
			this->_setDeviceColorPointer(sizeof(ColoredTexturedVertex), &vertices->color);
			this->_setDeviceTexturePointer(sizeof(ColoredTexturedVertex), &vertices->u);
			GL_SAFE_CALL(glDrawArrays, (_glRenderOperations[renderOperation.value], 0, size));
#ifdef _SEGMENTED_RENDERING
			vertices += size;
		}
#endif
	}

	void OpenGL_RenderSystem::_setDeviceVertexPointer(int stride, const void* pointer, bool forceUpdate)
	{
		if (forceUpdate || this->deviceState_vertexStride != stride || this->deviceState_vertexPointer != pointer)
		{
			this->_setGlVertexPointer(stride, pointer);
			this->deviceState_vertexStride = stride;
			this->deviceState_vertexPointer = pointer;
		}
	}

	void OpenGL_RenderSystem::_setDeviceTexturePointer(int stride, const void* pointer, bool forceUpdate)
	{
		if (forceUpdate || this->deviceState_textureStride != stride || this->deviceState_texturePointer != pointer)
		{
			this->_setGlTexturePointer(stride, pointer);
			this->deviceState_textureStride = stride;
			this->deviceState_texturePointer = pointer;
		}
	}

	void OpenGL_RenderSystem::_setDeviceColorPointer(int stride, const void* pointer, bool forceUpdate)
	{
		if (forceUpdate || this->deviceState_colorStride != stride || this->deviceState_colorPointer != pointer)
		{
			this->_setGlColorPointer(stride, pointer);
			this->deviceState_colorStride = stride;
			this->deviceState_colorPointer = pointer;
		}
	}

	void OpenGL_RenderSystem::_deviceTakeScreenshot(Image::Format format)
	{
		int w = april::window->getWidth();
		int h = april::window->getHeight();
		unsigned char* temp = new unsigned char[w * (h + 1) * 4]; // 4 BPP and one extra row just in case some OpenGL implementations don't blit properly and cause a memory leak
		GL_SAFE_CALL(glReadPixels, (0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, temp));
		// GL returns all pixels flipped vertically so this needs to be corrected first
		int stride = w * 4;
		unsigned char* data = new unsigned char[stride * h];
		for_iter (i, 0, h)
		{
			memcpy(&data[i * stride], &temp[(h - i - 1) * stride], stride);
		}
		delete[] temp;
		temp = data;
		data = NULL;
		if (Image::convertToFormat(w, h, temp, Image::Format::RGBX, &data, format, false))
		{
			april::window->queueScreenshot(Image::create(w, h, data, format));
			delete[] data;
		}
		delete[] temp;
	}

	Image::Format OpenGL_RenderSystem::getNativeTextureFormat(Image::Format format) const
	{
		if (format == Image::Format::ARGB || format == Image::Format::ABGR || format == Image::Format::RGBA)
		{
			return Image::Format::RGBA;
		}
		if (format == Image::Format::XRGB || format == Image::Format::RGBX || format == Image::Format::XBGR || format == Image::Format::BGRX)
		{
			return Image::Format::RGBX;
		}
		if (format == Image::Format::BGRA)
		{
#if !defined(__ANDROID__) && !defined(_WIN32)
#ifndef __APPLE__
			return Image::Format::BGRA; // for optimizations
#else
			return Image::Format::BGRA; // for optimizations
#endif
#else
			return Image::Format::RGBA;
#endif
		}
		if (format == Image::Format::RGB)
		{
			return Image::Format::RGB;
		}
		if (format == Image::Format::BGR)
		{
#if !defined(__ANDROID__) && !defined(_WIN32)
#ifndef __APPLE__
			return Image::Format::BGR; // for optimizations
#else
			return Image::Format::BGRA; // for optimizations
#endif
#else
			return Image::Format::RGB;
#endif
		}
		if (format == Image::Format::Alpha || format == Image::Format::Greyscale || format == Image::Format::Compressed || format == Image::Format::Palette)
		{
			return format;
		}
		return Image::Format::Invalid;
	}

	unsigned int OpenGL_RenderSystem::getNativeColorUInt(const april::Color& color) const
	{
		return ((color.a << 24) | (color.b << 16) | (color.g << 8) | color.r);
	}

}
#endif
