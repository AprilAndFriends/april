/// @file
/// @version 3.5
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include <hltypes/hplatform.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <gl/GL.h>
#define GL_GLEXT_PROTOTYPES
#include <gl/glext.h>

#include <april/april.h>
#include <april/Color.h>
#include <april/Image.h>
#include <april/RenderSystem.h>
#include <april/Texture.h>
#include <gtypes/Rectangle.h>
#include <hltypes/hlog.h>
#include <hltypes/hstring.h>

#include "CustomRenderSystem.h"
#include "CustomTexture.h"
#include "CustomWindow.h"

#define UINT_RGBA_TO_ABGR(c) (((c & 0xFF000000) >> 24) | ((c & 0x00FF0000) >> 8) | ((c & 0x0000FF00) << 8) | ((c & 0x000000FF) << 24));

// translation from abstract render ops to gl's render ops
static int gl_render_ops[] =
{
	0,
	GL_TRIANGLES,		// RO_TRIANGLE_LIST
	GL_TRIANGLE_STRIP,	// RO_TRIANGLE_STRIP
	GL_TRIANGLE_FAN,	// RO_TRIANGLE_FAN
	GL_LINES,			// RO_LINE_LIST
	GL_LINE_STRIP,		// RO_LINE_STRIP
	GL_POINTS,			// RO_POINT_LIST
};

CustomRenderSystem::CustomRenderSystem() : april::RenderSystem(), activeTexture(NULL)
{
	this->hWnd = 0;
	this->hDC = 0;
	this->hRC = 0;
}

CustomRenderSystem::~CustomRenderSystem()
{
	this->destroy(); // has to be called here
}

bool CustomRenderSystem::create(RenderSystem::Options options)
{
	if (!RenderSystem::create(options))
	{
		return false;
	}
	this->activeTexture = NULL;
	return true;
}

bool CustomRenderSystem::destroy()
{
	if (!RenderSystem::destroy())
	{
		return false;
	}
	this->activeTexture = NULL;
	this->_releaseWindow();
	return true;
}

void CustomRenderSystem::_releaseWindow()
{
	if (this->hRC != 0)
	{
		wglMakeCurrent(NULL, NULL);
		wglDeleteContext(this->hRC);
		this->hRC = 0;
	}
	if (this->hDC != 0)
	{
		ReleaseDC(this->hWnd, this->hDC);
		this->hDC = 0;
	}
}

bool CustomRenderSystem::_initWin32(april::Window* window)
{
	this->hWnd = (HWND)window->getBackendId();
	PIXELFORMATDESCRIPTOR pfd;
	memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 24;
	pfd.cStencilBits = 16;
	pfd.dwLayerMask = PFD_MAIN_PLANE;
	this->hDC = GetDC(this->hWnd);
	if (this->hDC == 0)
	{
		hlog::error(LOG_TAG, "Can't create a GL device context!");
		return false;
	}
	GLuint pixelFormat = ChoosePixelFormat(this->hDC, &pfd);
	if (pixelFormat == 0)
	{
		hlog::error(LOG_TAG, "Can't find a suitable pixel format!");
		this->_releaseWindow();
		return false;
	}
	if (SetPixelFormat(this->hDC, pixelFormat, &pfd) == 0)
	{
		hlog::error(LOG_TAG, "Can't set the pixel format!");
		this->_releaseWindow();
		return false;
	}
	return true;
}

void CustomRenderSystem::assignWindow(april::Window* window)
{
	if (!this->_initWin32(window))
	{
		return;
	}
	this->hRC = wglCreateContext(this->hDC);
	if (this->hRC == 0)
	{
		hlog::error(LOG_TAG, "Can't create a GL rendering context!");
		this->_releaseWindow();
		return;
	}
	if (wglMakeCurrent(this->hDC, this->hRC) == 0)
	{
		hlog::error(LOG_TAG, "Can't activate the GL rendering context!");
		this->_releaseWindow();
		return;
	}
	this->_setupDefaultParameters();
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	this->orthoProjection.setSize(window->getSize());
}

void CustomRenderSystem::reset()
{
	RenderSystem::reset();
	this->_setupDefaultParameters();
}

void CustomRenderSystem::_setupDefaultParameters()
{
	glClearColor(0, 0, 0, 1);
	this->setViewport(grect(0.0f, 0.0f, april::window->getSize()));
	// GL defaults
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnable(GL_TEXTURE_2D);
	// pixel data
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	// other
	if (this->options.depthBuffer)
	{
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
	}
	this->_setClientState(GL_TEXTURE_COORD_ARRAY, false);
	this->_setClientState(GL_COLOR_ARRAY, false);
	glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void CustomRenderSystem::setViewport(grect rect)
{
	RenderSystem::setViewport(rect);
	// GL uses mathematical logic rather than screen logic and has (0,0) in the bottom left corner
	glViewport((int)rect.x, (int)(april::window->getHeight() - rect.h - rect.y), (int)rect.w, (int)rect.h);
}

void CustomRenderSystem::clear(bool useColor, bool depth)
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

void CustomRenderSystem::clear(bool depth, grect rect, april::Color color)
{
	glClearColor(color.r_f(), color.g_f(), color.b_f(), color.a_f());
	this->clear(true, depth);
}

void CustomRenderSystem::_setClientState(unsigned int type, bool enabled)
{
	enabled ? glEnableClientState(type) : glDisableClientState(type);
}

void CustomRenderSystem::setTextureBlendMode(april::BlendMode textureBlendMode)
{
	if (textureBlendMode == april::BM_ALPHA || textureBlendMode == april::BM_DEFAULT)
	{
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
	else if (textureBlendMode == april::BM_ADD)
	{
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	}
	else
	{
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		hlog::warn(LOG_TAG, "Trying to set unsupported blend mode!");
	}
}

void CustomRenderSystem::setTextureColorMode(april::ColorMode textureColorMode, float factor)
{
	static float constColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	constColor[3] = factor;
	switch (textureColorMode)
	{
	case april::CM_DEFAULT:
	case april::CM_MULTIPLY:
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_TEXTURE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_ALPHA, GL_PRIMARY_COLOR);
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_TEXTURE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_RGB, GL_PRIMARY_COLOR);
		break;
	case april::CM_LERP:
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
		glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, constColor);
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_TEXTURE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_ALPHA, GL_PRIMARY_COLOR);
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_INTERPOLATE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_PRIMARY_COLOR);
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_RGB, GL_TEXTURE);
		break;
	case april::CM_ALPHA_MAP:
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_TEXTURE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_ALPHA, GL_PRIMARY_COLOR);
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_REPLACE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_PRIMARY_COLOR);
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_RGB, GL_PRIMARY_COLOR);
		break;
	default:
		hlog::warn(LOG_TAG, "Trying to set unsupported color mode!");
		break;
	}
}

void CustomRenderSystem::setTextureFilter(april::Texture::Filter textureFilter)
{
	this->textureFilter = textureFilter;
	switch (textureFilter)
	{
	case april::Texture::FILTER_LINEAR:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		break;
	case april::Texture::FILTER_NEAREST:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		break;
	default:
		hlog::warn(LOG_TAG, "Trying to set unsupported texture filter!");
		break;
	}
}

void CustomRenderSystem::setTextureAddressMode(april::Texture::AddressMode textureAddressMode)
{
	this->textureAddressMode = textureAddressMode;
	switch (textureAddressMode)
	{
	case april::Texture::ADDRESS_WRAP:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		break;
	case april::Texture::ADDRESS_CLAMP:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		break;
	default:
		hlog::warn(LOG_TAG, "Trying to set unsupported texture address mode!");
		break;
	}
}

void CustomRenderSystem::setTexture(april::Texture* texture)
{
	this->activeTexture = (CustomTexture*)texture;
	if (this->activeTexture != NULL)
	{
		this->activeTexture->load();
	}
}

april::Texture* CustomRenderSystem::_createTexture(bool fromResource)
{
	return new CustomTexture(fromResource);
}

void CustomRenderSystem::_applyTexture()
{
	if (this->activeTexture != NULL)
	{
		glBindTexture(GL_TEXTURE_2D, this->activeTexture->textureId);
		this->setTextureFilter(this->activeTexture->getFilter());
		this->setTextureAddressMode(this->activeTexture->getAddressMode());
	}
	else
	{
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}

void CustomRenderSystem::render(april::RenderOperation renderOperation, april::PlainVertex* v, int nVertices)
{
	glBindTexture(GL_TEXTURE_2D, 0);
	this->_setClientState(GL_TEXTURE_COORD_ARRAY, false);
	this->_setClientState(GL_COLOR_ARRAY, false);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glTexCoordPointer(2, GL_FLOAT, 0, NULL);
	glColorPointer(4, GL_UNSIGNED_BYTE, 0, NULL);
	glVertexPointer(3, GL_FLOAT, sizeof(april::PlainVertex), v);
	glDrawArrays(gl_render_ops[renderOperation], 0, nVertices);
}

void CustomRenderSystem::render(april::RenderOperation renderOperation, april::PlainVertex* v, int nVertices, april::Color color)
{
	glBindTexture(GL_TEXTURE_2D, 0);
	this->_setClientState(GL_TEXTURE_COORD_ARRAY, false);
	this->_setClientState(GL_COLOR_ARRAY, false);
	glColor4f(color.r_f(), color.g_f(), color.b_f(), color.a_f());
	glTexCoordPointer(2, GL_FLOAT, 0, NULL);
	glColorPointer(4, GL_UNSIGNED_BYTE, 0, NULL);
	glVertexPointer(3, GL_FLOAT, sizeof(april::PlainVertex), v);
	glDrawArrays(gl_render_ops[renderOperation], 0, nVertices);
}

void CustomRenderSystem::render(april::RenderOperation renderOperation, april::TexturedVertex* v, int nVertices)
{
	this->_applyTexture();
	this->_setClientState(GL_TEXTURE_COORD_ARRAY, true);
	this->_setClientState(GL_COLOR_ARRAY, false);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glTexCoordPointer(2, GL_FLOAT, sizeof(april::TexturedVertex), &v->u);
	glColorPointer(4, GL_UNSIGNED_BYTE, 0, NULL);
	glVertexPointer(3, GL_FLOAT, sizeof(april::TexturedVertex), v);
	glDrawArrays(gl_render_ops[renderOperation], 0, nVertices);
}

void CustomRenderSystem::render(april::RenderOperation renderOperation, april::TexturedVertex* v, int nVertices, april::Color color)
{
	this->_applyTexture();
	this->_setClientState(GL_TEXTURE_COORD_ARRAY, true);
	this->_setClientState(GL_COLOR_ARRAY, false);
	glColor4f(color.r_f(), color.g_f(), color.b_f(), color.a_f());
	glTexCoordPointer(2, GL_FLOAT, sizeof(april::TexturedVertex), &v->u);
	glColorPointer(4, GL_UNSIGNED_BYTE, 0, NULL);
	glVertexPointer(3, GL_FLOAT, sizeof(april::TexturedVertex), v);
	glDrawArrays(gl_render_ops[renderOperation], 0, nVertices);
}

void CustomRenderSystem::render(april::RenderOperation renderOperation, april::ColoredVertex* v, int nVertices)
{
	for_iter (i, 0, nVertices)
	{
		// making sure this is in AGBR order
		v[i].color = UINT_RGBA_TO_ABGR(v[i].color);
	}
	glBindTexture(GL_TEXTURE_2D, 0);
	this->_setClientState(GL_TEXTURE_COORD_ARRAY, false);
	this->_setClientState(GL_COLOR_ARRAY, true);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glTexCoordPointer(2, GL_FLOAT, 0, NULL);
	glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(april::ColoredVertex), &v->color);
	glVertexPointer(3, GL_FLOAT, sizeof(april::ColoredVertex), v);
	glDrawArrays(gl_render_ops[renderOperation], 0, nVertices);
}

void CustomRenderSystem::render(april::RenderOperation renderOperation, april::ColoredTexturedVertex* v, int nVertices)
{
	for_iter (i, 0, nVertices)
	{
		// making sure this is in AGBR order
		v[i].color = UINT_RGBA_TO_ABGR(v[i].color);
	}
	this->_applyTexture();
	this->_setClientState(GL_TEXTURE_COORD_ARRAY, true);
	this->_setClientState(GL_COLOR_ARRAY, true);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glTexCoordPointer(2, GL_FLOAT, sizeof(april::ColoredTexturedVertex), &v->u);
	glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(april::ColoredTexturedVertex), &v->color);
	glVertexPointer(3, GL_FLOAT, sizeof(april::ColoredTexturedVertex), v);
	glDrawArrays(gl_render_ops[renderOperation], 0, nVertices);
}

void CustomRenderSystem::_setModelviewMatrix(const gmat4& matrix)
{
	this->modelviewMatrix = matrix;
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(this->modelviewMatrix.data);
}

void CustomRenderSystem::_setProjectionMatrix(const gmat4& matrix)
{
	this->projectionMatrix = matrix;
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(this->projectionMatrix.data);
}

void CustomRenderSystem::_setupCaps()
{
	if (this->hRC == 0)
	{
		return;
	}
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &this->caps.maxTextureSize);
}

april::Image::Format CustomRenderSystem::getNativeTextureFormat(april::Image::Format format)
{
	switch (format)
	{
	case april::Image::FORMAT_ARGB:
	case april::Image::FORMAT_ABGR:
	case april::Image::FORMAT_RGBA:
	case april::Image::FORMAT_BGRA:
		return april::Image::FORMAT_RGBA;
	case april::Image::FORMAT_XRGB:
	case april::Image::FORMAT_RGBX:
	case april::Image::FORMAT_XBGR:
	case april::Image::FORMAT_BGRX:
		return april::Image::FORMAT_RGBX;
	case april::Image::FORMAT_RGB:
	case april::Image::FORMAT_BGR:
		return april::Image::FORMAT_RGB;
	case april::Image::FORMAT_ALPHA:
		return april::Image::FORMAT_ALPHA;
	case april::Image::FORMAT_GRAYSCALE:
		return april::Image::FORMAT_GRAYSCALE;
	case april::Image::FORMAT_PALETTE:
		return april::Image::FORMAT_PALETTE;
	}
	return april::Image::FORMAT_INVALID;
}

unsigned int CustomRenderSystem::getNativeColorUInt(const april::Color& color)
{
	return ((color.a << 24) | (color.b << 16) | (color.g << 8) | color.r);
}

