/// @file
/// @version 3.7
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
#ifdef _WIN32_WINDOW
#include "Win32_Window.h"
#endif

namespace april
{
	// translation from abstract render ops to gl's render ops
	int OpenGL_RenderSystem::glRenderOperations[] =
	{
		0,
		GL_TRIANGLES,		// RO_TRIANGLE_LIST
		GL_TRIANGLE_STRIP,	// RO_TRIANGLE_STRIP
		GL_TRIANGLE_FAN,	// RO_TRIANGLE_FAN
		GL_LINES,			// RO_LINE_LIST
		GL_LINE_STRIP,		// RO_LINE_STRIP
		GL_POINTS,			// RO_POINT_LIST
	};

	// TODOa - put in state class
	static Color lastColor = Color::Black;

	OpenGL_RenderSystem::OpenGL_RenderSystem() : RenderSystem(), activeTexture(NULL)
	{
		this->state = new RenderState(); // TODOa
#if defined(_WIN32) && !defined(_WINRT)
		this->hWnd = 0;
		this->hDC = 0;
#endif
	}

	OpenGL_RenderSystem::~OpenGL_RenderSystem()
	{
	}

	bool OpenGL_RenderSystem::create(RenderSystem::Options options)
	{
		if (!RenderSystem::create(options))
		{
			return false;
		}
		this->activeTexture = NULL;
		this->deviceState.reset();
		this->currentState.reset();
		this->state->reset();
		return true;
	}

	bool OpenGL_RenderSystem::destroy()
	{
		if (!RenderSystem::destroy())
		{
			return false;
		}
		this->activeTexture = NULL;
		this->deviceState.reset();
		this->currentState.reset();
		this->state->reset();
#if defined(_WIN32) && !defined(_WINRT)
		this->_releaseWindow();
#endif
		return true;
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

	void OpenGL_RenderSystem::assignWindow(Window* window)
	{
#if defined(_WIN32) && !defined(_WINRT)
		if (!this->_initWin32(window))
		{
			return;
		}
#endif
		this->currentState.modelviewMatrix.setIdentity();
		this->currentState.projectionMatrix.setIdentity();
		this->currentState.modelviewMatrixChanged = true;
		this->currentState.projectionMatrixChanged = true;
		this->_setupDefaultParameters();
		this->orthoProjection.setSize(window->getSize());
	}

	void OpenGL_RenderSystem::reset()
	{
		RenderSystem::reset();
		this->currentState.reset();
		this->deviceState.reset();
		this->_setupDefaultParameters();
		this->currentState.modelviewMatrixChanged = true;
		this->currentState.projectionMatrixChanged = true;
		this->_applyStateChanges();
	}

	void OpenGL_RenderSystem::_setupDefaultParameters()
	{
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		lastColor.set(0, 0, 0, 255);
		this->setViewport(grect(0.0f, 0.0f, april::window->getSize()));
		// GL defaults
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_TEXTURE_2D);
		// pixel data
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glPixelStorei(GL_PACK_ALIGNMENT, 1);
		// other
		if (this->options.depthBuffer)
		{
			glDepthFunc(GL_LEQUAL);
		}
		glBindTexture(GL_TEXTURE_2D, this->deviceState.textureId);
		this->currentState.textureFilter = april::Texture::FILTER_NEAREST;
		this->currentState.textureAddressMode = april::Texture::ADDRESS_WRAP;
		this->currentState.blendMode = april::BM_UNDEFINED;
		this->currentState.colorMode = april::CM_UNDEFINED;
		this->_setDepthBuffer(this->deviceState.depthBuffer, this->deviceState.depthBufferWrite);
	}

	void OpenGL_RenderSystem::setViewport(grect rect)
	{
		RenderSystem::setViewport(rect);
		// because GL has to defy screen logic and has (0,0) in the bottom left corner
		glViewport((int)rect.x, (int)(april::window->getHeight() - rect.h - rect.y), (int)rect.w, (int)rect.h);
	}
	
	void OpenGL_RenderSystem::setDepthBuffer(bool enabled, bool writeEnabled)
	{
		RenderSystem::setDepthBuffer(enabled, writeEnabled);
		if (this->options.depthBuffer)
		{
			this->currentState.depthBuffer = enabled;
			this->currentState.depthBufferWrite = writeEnabled;
		}
	}

	void OpenGL_RenderSystem::_setDepthBuffer(bool enabled, bool writeEnabled)
	{
		enabled ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST);
		glDepthMask(writeEnabled);
	}

	void OpenGL_RenderSystem::bindTexture(unsigned int textureId)
	{
		this->currentState.textureId = textureId;
	}

	void OpenGL_RenderSystem::setTexture(Texture* texture)
	{
		this->activeTexture = (OpenGL_Texture*)texture;
		if (this->activeTexture == NULL)
		{
			this->bindTexture(0);
		}
		else
		{
			this->setTextureFilter(this->activeTexture->getFilter());
			this->setTextureAddressMode(this->activeTexture->getAddressMode());
			// filtering and wrapping applied before loading texture data, iOS OpenGL guidelines suggest it as an optimization
			this->activeTexture->load();
			this->activeTexture->unlock();
			this->bindTexture(this->activeTexture->textureId);
		}
	}

	void OpenGL_RenderSystem::setMatrixMode(unsigned int mode)
	{
		// performance call, minimize redundant calls to setMatrixMode
		if (this->deviceState.modeMatrix != mode)
		{
			this->deviceState.modeMatrix = mode;
			this->_setMatrixMode(mode);
		}
	}

	void OpenGL_RenderSystem::setTextureBlendMode(BlendMode mode)
	{
		this->currentState.blendMode = mode;
	}
	
	void OpenGL_RenderSystem::_setTextureBlendMode(BlendMode textureBlendMode)
	{
		if (textureBlendMode == BM_ALPHA || textureBlendMode == BM_DEFAULT)
		{
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}
		else if (textureBlendMode == BM_ADD)
		{
			glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		}
		else
		{
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			hlog::warn(logTag, "Trying to set unsupported blend mode!");
		}
	}
	
	void OpenGL_RenderSystem::setTextureColorMode(ColorMode textureColorMode, float factor)
	{
		this->currentState.colorMode = textureColorMode;
		this->currentState.colorModeFactor = factor;
	}

	void OpenGL_RenderSystem::setTextureFilter(Texture::Filter textureFilter)
	{
		this->currentState.textureFilter = textureFilter;
	}

	void OpenGL_RenderSystem::_setTextureFilter(Texture::Filter textureFilter)
	{
		this->textureFilter = textureFilter;
		switch (textureFilter)
		{
		case Texture::FILTER_LINEAR:
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			break;
		case Texture::FILTER_NEAREST:
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			break;
		default:
			hlog::warn(logTag, "Trying to set unsupported texture filter!");
			break;
		}
	}

	void OpenGL_RenderSystem::setTextureAddressMode(Texture::AddressMode textureAddressMode)
	{
		this->currentState.textureAddressMode = textureAddressMode;
	}

	void OpenGL_RenderSystem::_setTextureAddressMode(Texture::AddressMode textureAddressMode)
	{
		this->textureAddressMode = textureAddressMode;
		switch (textureAddressMode)
		{
		case Texture::ADDRESS_WRAP:
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			break;
		case Texture::ADDRESS_CLAMP:
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			break;
		default:
			hlog::warn(logTag, "Trying to set unsupported texture address mode!");
			break;
		}
	}

	void OpenGL_RenderSystem::clear(bool useColor, bool depth)
	{
		GLbitfield mask = 0;
		if (useColor)
		{
			mask |= GL_COLOR_BUFFER_BIT;
		}
		if (depth)
		{
			mask |= GL_DEPTH_BUFFER_BIT;
		}
		glClear(mask);
	}

	void OpenGL_RenderSystem::clear(bool depth, grect rect, Color color)
	{
		if (color != lastColor) // used to minimize redundant calls to OpenGL
		{
			glClearColor(color.r_f(), color.g_f(), color.b_f(), color.a_f());
			lastColor = color;
		}
		this->clear(true, depth);
	}

	void OpenGL_RenderSystem::_applyStateChanges()
	{
		// texture has to be bound first or else filter and address mode won't be applied afterwards
		if (this->currentState.textureFilter != this->deviceState.textureFilter || this->deviceState.textureFilter == Texture::FILTER_UNDEFINED)
		{
			this->_setTextureFilter(this->currentState.textureFilter);
			this->deviceState.textureFilter = this->currentState.textureFilter;
		}
		if (this->currentState.textureAddressMode != this->deviceState.textureAddressMode || this->deviceState.textureAddressMode == Texture::ADDRESS_UNDEFINED)
		{
			this->_setTextureAddressMode(this->currentState.textureAddressMode);
			this->deviceState.textureAddressMode = this->currentState.textureAddressMode;
		}
		if (this->currentState.blendMode != this->deviceState.blendMode)
		{
			this->_setTextureBlendMode(this->currentState.blendMode);
			this->deviceState.blendMode = this->currentState.blendMode;
		}
		if (this->currentState.colorMode != this->deviceState.colorMode || this->currentState.colorModeFactor != this->deviceState.colorModeFactor)
		{
			this->_setTextureColorMode(this->currentState.colorMode, this->currentState.colorModeFactor);
			this->deviceState.colorMode = this->currentState.colorMode;
			this->deviceState.colorModeFactor = this->currentState.colorModeFactor;
		}
		if (this->currentState.depthBuffer != this->deviceState.depthBuffer || this->currentState.depthBufferWrite != this->deviceState.depthBufferWrite)
		{
			this->_setDepthBuffer(this->currentState.depthBuffer, this->currentState.depthBufferWrite);
			this->deviceState.depthBuffer = this->currentState.depthBuffer;
			this->deviceState.depthBufferWrite = this->currentState.depthBufferWrite;
		}
	}

	void OpenGL_RenderSystem::_setModelviewMatrix(const gmat4& matrix)
	{
		this->currentState.modelviewMatrix = matrix;
		this->currentState.modelviewMatrixChanged = true;
	}

	void OpenGL_RenderSystem::_setProjectionMatrix(const gmat4& matrix)
	{
		this->currentState.projectionMatrix = matrix;
		this->currentState.projectionMatrixChanged = true;
	}

	void OpenGL_RenderSystem::_setupCaps()
	{
		glGetIntegerv(GL_MAX_TEXTURE_SIZE, &this->caps.maxTextureSize);
	}

	void OpenGL_RenderSystem::_setResolution(int w, int h, bool fullscreen)
	{
		glViewport(0, 0, w, h);
		this->orthoProjection.setSize((float)w, (float)h);
		this->setOrthoProjection(this->orthoProjection);
		this->_applyStateChanges();
	}

	Image::Format OpenGL_RenderSystem::getNativeTextureFormat(Image::Format format)
	{
		switch (format)
		{
		case Image::FORMAT_ARGB:
		case Image::FORMAT_ABGR:
		case Image::FORMAT_RGBA:
			return Image::FORMAT_RGBA;
		case Image::FORMAT_XRGB:
		case Image::FORMAT_RGBX:
		case Image::FORMAT_XBGR:
		case Image::FORMAT_BGRX:
			return Image::FORMAT_RGBX;
		// for optimizations
		case Image::FORMAT_BGRA:
#if !defined(_ANDROID) && !defined(_WIN32)
#ifndef __APPLE__
			return Image::FORMAT_BGRA;
#else
			return Image::FORMAT_BGRA;
#endif
#else
			return Image::FORMAT_RGBA;
#endif
		case Image::FORMAT_RGB:
			return Image::FORMAT_RGB;
			break;
		// for optimizations
		case Image::FORMAT_BGR:
#if !defined(_ANDROID) && !defined(_WIN32)
#ifndef __APPLE__
			return Image::FORMAT_BGR;
#else
			return Image::FORMAT_BGRA;
#endif
#else
			return Image::FORMAT_RGB;
#endif
		case Image::FORMAT_ALPHA:
			return Image::FORMAT_ALPHA;
		case Image::FORMAT_GRAYSCALE:
			return Image::FORMAT_GRAYSCALE;
			break;
		case Image::FORMAT_PALETTE: // TODOaa - does palette use RGBA?
			return Image::FORMAT_PALETTE;
			break;
		default:
			break;
		}
		return Image::FORMAT_INVALID;
	}

	unsigned int OpenGL_RenderSystem::getNativeColorUInt(const april::Color& color)
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
		if (Image::convertToFormat(w, h, temp, Image::FORMAT_RGBA, &data, format, false))
		{
			image = Image::create(w, h, data, format);
			delete[] data;
		}
		delete[] temp;
		return image;
	}

}
#endif
