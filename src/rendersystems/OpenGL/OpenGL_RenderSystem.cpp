/// @file
/// @version 5.0
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

#define MAX_VERTEX_COUNT 65535
#ifdef _ANDROID
#define _SEGMENTED_RENDERING
#endif

namespace april
{
	// translation from abstract render ops to gl's render ops
	int OpenGL_RenderSystem::_glRenderOperations[] =
	{
		GL_TRIANGLES,		// RO_TRIANGLE_LIST
		GL_TRIANGLE_STRIP,	// RO_TRIANGLE_STRIP
		GL_LINES,			// RO_LINE_LIST
		GL_LINE_STRIP,		// RO_LINE_STRIP
		GL_POINTS,			// RO_POINT_LIST
		GL_TRIANGLE_FAN,	// RO_TRIANGLE_FAN
	};

	OpenGL_RenderSystem::OpenGL_RenderSystem() : RenderSystem(), blendSeparationSupported(false), deviceState_vertexStride(0),
		deviceState_vertexPointer(NULL), deviceState_textureStride(0), deviceState_texturePointer(NULL), deviceState_colorStride(0),
		deviceState_colorPointer(NULL), renderTarget(NULL)
	{
		april::TexturedVertex* v = this->_intermediateRenderVertices;
		// because GL has to defy screen logic and has (0,0) in the bottom left corner
		for_iter (i, 0, 6)
		{
			this->_intermediateRenderVertices[i].y = -this->_intermediateRenderVertices[i].y;
		}
#if defined(_WIN32) && !defined(_WINRT)
		this->hWnd = 0;
		this->hDC = 0;
#endif
	}

	OpenGL_RenderSystem::~OpenGL_RenderSystem()
	{
	}

	void OpenGL_RenderSystem::_deviceInit()
	{
		this->deviceState_vertexStride = 0;
		this->deviceState_vertexPointer = NULL;
		this->deviceState_textureStride = 0;
		this->deviceState_texturePointer = NULL;
		this->deviceState_colorStride = 0;
		this->deviceState_colorPointer = NULL;
		this->renderTarget = NULL;
#if defined(_WIN32) && !defined(_WINRT)
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
#if defined(_WIN32) && !defined(_WINRT)
		this->_releaseWindow();
#endif
		return true;
	}

	void OpenGL_RenderSystem::_deviceAssignWindow(Window* window)
	{
#if defined(_WIN32) && !defined(_WINRT)
		this->_initWin32(window);
#endif
		this->renderTarget = NULL;
		this->_updateIntermediateRenderTexture();
		glBindFramebuffer(GL_FRAMEBUFFER, ((OpenGL_Texture*)this->_intermediateRenderTexture)->frameBufferId);
	}

	void OpenGL_RenderSystem::_deviceReset()
	{
		OpenGL_Texture* intermediateRenderTexture = (OpenGL_Texture*)this->_intermediateRenderTexture;
		if (intermediateRenderTexture != NULL)
		{
			intermediateRenderTexture->_ensureAsyncCompleted();
			intermediateRenderTexture->_deviceUnloadTexture();
		}
		RenderSystem::_deviceReset();
		this->_updateIntermediateRenderTexture();
		this->setRenderTarget(this->renderTarget);
	}

	void OpenGL_RenderSystem::_deviceSetupCaps()
	{
		glGetIntegerv(GL_MAX_TEXTURE_SIZE, &this->caps.maxTextureSize);
	}

#if defined(_WIN32) && !defined(_WINRT)
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
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		// GL defaults
		glEnable(GL_BLEND);
		glEnable(GL_TEXTURE_2D);
		// pixel data
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glPixelStorei(GL_PACK_ALIGNMENT, 1);
		// other
		if (this->options.depthBuffer)
		{
			glDepthFunc(GL_LEQUAL);
		}
		this->_setGlTextureEnabled(this->deviceState->useTexture);
		this->_setGlColorEnabled(this->deviceState->useColor);
		this->_setGlVertexPointer(this->deviceState_vertexStride, this->deviceState_vertexPointer);
		this->_setGlTexturePointer(this->deviceState_textureStride, this->deviceState_texturePointer);
		this->_setGlColorPointer(this->deviceState_colorStride, this->deviceState_colorPointer);
	}

	int OpenGL_RenderSystem::getVRam() const
	{
		return 0;
	}

	void OpenGL_RenderSystem::_deviceChangeResolution(int w, int h, bool fullscreen)
	{
		grect viewport(0.0f, 0.0f, (float)w, (float)h);
		this->setViewport(viewport);
		this->setOrthoProjection(viewport);
		this->_updateDeviceState(this->state, true);
	}

	void OpenGL_RenderSystem::_setDeviceViewport(cgrect rect)
	{
		// because GL has to defy screen logic and has (0,0) in the bottom left corner
		glViewport((int)rect.x, (int)(april::window->getHeight() - rect.h - rect.y), (int)rect.w, (int)rect.h);
	}

	void OpenGL_RenderSystem::_setDeviceDepthBuffer(bool enabled, bool writeEnabled)
	{
		enabled ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST);
		glDepthMask(writeEnabled);
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
			glBindTexture(GL_TEXTURE_2D, ((OpenGL_Texture*)texture)->textureId);
		}
		else
		{
			glBindTexture(GL_TEXTURE_2D, 0);
		}
	}

	void OpenGL_RenderSystem::_setDeviceTextureFilter(const Texture::Filter& textureFilter)
	{
		if (textureFilter == Texture::Filter::Linear)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		}
		else if (textureFilter == Texture::Filter::Nearest)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
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
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		}
		else if (textureAddressMode == Texture::AddressMode::Clamp)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
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
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}
		else if (blendMode == BlendMode::Add)
		{
			glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		}
		else
		{
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
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
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(mask);
	}

	void OpenGL_RenderSystem::_deviceClear(const Color& color, bool depth)
	{
		GLbitfield mask = GL_COLOR_BUFFER_BIT;
		if (depth)
		{
			mask |= GL_DEPTH_BUFFER_BIT;
		}
		glClearColor(color.r_f(), color.g_f(), color.b_f(), color.a_f());
		glClear(mask);
	}

	void OpenGL_RenderSystem::_deviceClearDepth()
	{
		glClear(GL_DEPTH_BUFFER_BIT);
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
			glDrawArrays(_glRenderOperations[renderOperation.value], 0, size);
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
			glDrawArrays(_glRenderOperations[renderOperation.value], 0, size);
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
			glDrawArrays(_glRenderOperations[renderOperation.value], 0, size);
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
			glDrawArrays(_glRenderOperations[renderOperation.value], 0, size);
#ifdef _SEGMENTED_RENDERING
			vertices += size;
		}
#endif
	}

	void OpenGL_RenderSystem::_devicePresentFrame(bool systemEnabled)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		this->_presentIntermediateRenderTexture();
		RenderSystem::_devicePresentFrame(systemEnabled);
		this->_updateIntermediateRenderTexture();
		glBindFramebuffer(GL_FRAMEBUFFER, ((OpenGL_Texture*)this->_intermediateRenderTexture)->frameBufferId);
	}

	void OpenGL_RenderSystem::_deviceRepeatLastFrame()
	{
		if (this->_intermediateRenderTexture != NULL)
		{
			this->_devicePresentFrame(true);
		}
	}

	void OpenGL_RenderSystem::_deviceCopyRenderTargetData(Texture* source, Texture* destination)
	{
		if (source->getType() != Texture::Type::RenderTarget)
		{
			hlog::error(logTag, "Cannot copy render target data, source texture is not a render target!");
			return;
		}
		if (destination->getType() != Texture::Type::RenderTarget)
		{
			hlog::error(logTag, "Cannot copy render target data, destination texture is not a render target!");
			return;
		}
		glBindFramebuffer(GL_FRAMEBUFFER, ((OpenGL_Texture*)destination)->frameBufferId);
		this->_intermediateState->viewport.setSize((float)source->getWidth(), (float)source->getHeight());
		this->_intermediateState->projectionMatrix.setOrthoProjection(
			grect(1.0f - 2.0f * this->pixelOffset / source->getWidth(), 1.0f - 2.0f * this->pixelOffset / source->getHeight(), 2.0f, 2.0f));
		this->_intermediateState->texture = source;
		this->_updateDeviceState(this->_intermediateState, true);
		this->_deviceRender(RenderOperation::TriangleList, this->_intermediateRenderVertices, 6);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		this->_updateDeviceState(this->state, true);
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
#if !defined(_ANDROID) && !defined(_WIN32)
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
#if !defined(_ANDROID) && !defined(_WIN32)
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

	Image* OpenGL_RenderSystem::takeScreenshot(Image::Format format)
	{
#ifdef _DEBUG
		hlog::write(logTag, "Taking screenshot...");
#endif
		int w = april::window->getWidth();
		int h = april::window->getHeight();
		unsigned char* temp = new unsigned char[w * (h + 1) * 4]; // 4 BPP and one extra row just in case some OpenGL implementations don't blit properly and cause a memory leak
		glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, temp);
		unsigned char* data = NULL;
		Image* image = NULL;
		if (Image::convertToFormat(w, h, temp, Image::Format::RGBA, &data, format, false))
		{
			image = Image::create(w, h, data, format);
			delete[] data;
		}
		delete[] temp;
		return image;
	}

	Texture* OpenGL_RenderSystem::getRenderTarget()
	{
		return this->renderTarget;
	}

	void OpenGL_RenderSystem::setRenderTarget(Texture* source)
	{
		OpenGL_Texture* texture = (OpenGL_Texture*)source;
		if (texture == NULL)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, ((OpenGL_Texture*)this->_intermediateRenderTexture)->frameBufferId);
		}
		else
		{
			glBindFramebuffer(GL_FRAMEBUFFER, texture->frameBufferId);
		}
		this->renderTarget = texture;
	}

}
#endif
