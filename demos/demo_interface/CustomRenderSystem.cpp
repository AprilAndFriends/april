/// @file
/// @version 4.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#define __HL_INCLUDE_PLATFORM_HEADERS
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

// translation from abstract render ops to gl's render ops
int CustomRenderSystem::CustomRenderSystem::_glRenderOperations[] =
{
	0,
	GL_TRIANGLES,		// RO_TRIANGLE_LIST
	GL_TRIANGLE_STRIP,	// RO_TRIANGLE_STRIP
	GL_TRIANGLE_FAN,	// RO_TRIANGLE_FAN
	GL_LINES,			// RO_LINE_LIST
	GL_LINE_STRIP,		// RO_LINE_STRIP
	GL_POINTS,			// RO_POINT_LIST
};

CustomRenderSystem::CustomRenderSystem() : april::RenderSystem()
{
	this->name = "Custom";
	this->hWnd = 0;
	this->hDC = 0;
	this->hRC = 0;
}

CustomRenderSystem::~CustomRenderSystem()
{
	this->destroy(); // has to be called here
}

void CustomRenderSystem::_deviceInit()
{
	this->hWnd = 0;
	this->hDC = 0;
	this->hRC = 0;
}

bool CustomRenderSystem::_deviceCreate(RenderSystem::Options options)
{
	return true;
}

bool CustomRenderSystem::_deviceDestroy()
{
	this->_releaseWindow();
	return true;
}

void CustomRenderSystem::_deviceAssignWindow(april::Window* window)
{
	this->_initWin32(window);
}

void CustomRenderSystem::_deviceSetupCaps()
{
	if (this->hRC == 0)
	{
		return;
	}
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &this->caps.maxTextureSize);
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
	this->hDC = GetDC(this->hWnd);
	if (this->hDC == 0)
	{
		hlog::error(LOG_TAG, "Can't create a GL device context!");
		return false;
	}
	PIXELFORMATDESCRIPTOR pfd;
	memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 24;
	pfd.cDepthBits = 16;
	pfd.dwLayerMask = PFD_MAIN_PLANE;
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
	this->hRC = wglCreateContext(this->hDC);
	if (this->hRC == 0)
	{
		hlog::error(LOG_TAG, "Can't create a GL rendering context!");
		this->_releaseWindow();
		return false;
	}
	if (wglMakeCurrent(this->hDC, this->hRC) == 0)
	{
		hlog::error(LOG_TAG, "Can't activate the GL rendering context!");
		this->_releaseWindow();
		return false;
	}
	return true;
}

void CustomRenderSystem::_deviceSetup()
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	// GL defaults
	glEnable(GL_BLEND);
	glEnable(GL_TEXTURE_2D);
	// pixel data
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	// other system
	glEnableClientState(GL_VERTEX_ARRAY);
	glAlphaFunc(GL_GREATER, 0.0f);
	// other
	if (this->options.depthBuffer)
	{
		glDepthFunc(GL_LEQUAL);
	}
	glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
	glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	glPixelStorei(GL_UNPACK_SWAP_BYTES, GL_FALSE);
}

april::Texture* CustomRenderSystem::_deviceCreateTexture(bool fromResource)
{
	return new CustomTexture(fromResource);
}

void CustomRenderSystem::_setDeviceViewport(cgrect rect)
{
	// because GL has to defy screen logic and has (0,0) in the bottom left corner
	glViewport((int)rect.x, (int)(april::window->getHeight() - rect.h - rect.y), (int)rect.w, (int)rect.h);
}

void CustomRenderSystem::_setDeviceModelviewMatrix(const gmat4& matrix)
{
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(matrix.data);
}

void CustomRenderSystem::_setDeviceProjectionMatrix(const gmat4& matrix)
{
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(matrix.data);
}

void CustomRenderSystem::_setDeviceDepthBuffer(bool enabled, bool writeEnabled)
{
	if (enabled)
	{
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_ALPHA_TEST);
	}
	else
	{
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_ALPHA_TEST);
	}
	glDepthMask(writeEnabled);
}

void CustomRenderSystem::_setDeviceRenderMode(bool useTexture, bool useColor)
{
	useTexture ? glEnableClientState(GL_TEXTURE_COORD_ARRAY) : glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	useColor ? glEnableClientState(GL_COLOR_ARRAY) : glDisableClientState(GL_COLOR_ARRAY);
}

void CustomRenderSystem::_setDeviceTexture(april::Texture* texture)
{
	if (texture != NULL)
	{
		glBindTexture(GL_TEXTURE_2D, ((CustomTexture*)texture)->textureId);
	}
	else
	{
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}

void CustomRenderSystem::_setDeviceTextureFilter(const april::Texture::Filter& textureFilter)
{
	if (textureFilter == april::Texture::FILTER_LINEAR)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	}
	else if (textureFilter == april::Texture::FILTER_NEAREST)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}
	else
	{
		hlog::warn(LOG_TAG, "Trying to set unsupported texture filter!");
	}
}

void CustomRenderSystem::_setDeviceTextureAddressMode(const april::Texture::AddressMode& textureAddressMode)
{
	if (textureAddressMode == april::Texture::ADDRESS_WRAP)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}
	else if (textureAddressMode == april::Texture::ADDRESS_CLAMP)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}
	else
	{
		hlog::warn(LOG_TAG, "Trying to set unsupported texture address mode!");
	}
}

void CustomRenderSystem::_setDeviceBlendMode(april::BlendMode blendMode)
{
	if (blendMode == april::BM_ALPHA || blendMode == april::BM_DEFAULT)
	{
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
	else if (blendMode == april::BM_ADD)
	{
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	}
	else
	{
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		hlog::warn(LOG_TAG, "Trying to set unsupported blend mode!");
	}
}

void CustomRenderSystem::_setDeviceColorMode(april::ColorMode colorMode, float colorModeFactor, bool useTexture, bool useColor, const april::Color& systemColor)
{
	static float constColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	constColor[0] = colorModeFactor;
	constColor[1] = colorModeFactor;
	constColor[2] = colorModeFactor;
	constColor[3] = colorModeFactor;
	if (colorMode == april::CM_DEFAULT || colorMode == april::CM_MULTIPLY)
	{
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
		if (useTexture)
		{
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_TEXTURE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_ALPHA, GL_PRIMARY_COLOR);
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_TEXTURE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_RGB, GL_PRIMARY_COLOR);
		}
		else
		{
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_PRIMARY_COLOR);
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_REPLACE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_PRIMARY_COLOR);
		}
	}
	else if (colorMode == april::CM_ALPHA_MAP)
	{
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
		if (useTexture)
		{
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_TEXTURE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_ALPHA, GL_PRIMARY_COLOR);
		}
		else
		{
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_PRIMARY_COLOR);
		}
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_REPLACE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_PRIMARY_COLOR);
	}
	else if (colorMode == april::CM_LERP)
	{
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
		if (useTexture)
		{
			glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, constColor);
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_TEXTURE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_ALPHA, GL_PRIMARY_COLOR);
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_INTERPOLATE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_PRIMARY_COLOR);
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_RGB, GL_TEXTURE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC2_RGB, GL_CONSTANT);
		}
		else
		{
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_PRIMARY_COLOR);
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_REPLACE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_PRIMARY_COLOR);
		}
	}
	else
	{
		hlog::warn(LOG_TAG, "Trying to set unsupported color mode!");
	}
	if (!useColor)
	{
		glColor4f(systemColor.r_f(), systemColor.g_f(), systemColor.b_f(), systemColor.a_f());
	}
}

void CustomRenderSystem::_deviceClear(bool depth)
{
	GLbitfield mask = GL_COLOR_BUFFER_BIT;
	if (depth)
	{
		mask |= GL_DEPTH_BUFFER_BIT;
	}
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(mask);
}

void CustomRenderSystem::_deviceClear(const april::Color& color, bool depth)
{
	GLbitfield mask = GL_COLOR_BUFFER_BIT;
	if (depth)
	{
		mask |= GL_DEPTH_BUFFER_BIT;
	}
	glClearColor(color.r_f(), color.g_f(), color.b_f(), color.a_f());
	glClear(mask);
}

void CustomRenderSystem::_deviceClearDepth()
{
	glClear(GL_DEPTH_BUFFER_BIT);
}

void CustomRenderSystem::_deviceRender(const april::RenderOperation& renderOperation, const april::PlainVertex* v, int nVertices)
{
	glVertexPointer(3, GL_FLOAT, sizeof(april::PlainVertex), v);
	glDrawArrays(_glRenderOperations[renderOperation], 0, nVertices);
}

void CustomRenderSystem::_deviceRender(const april::RenderOperation& renderOperation, const april::TexturedVertex* v, int nVertices)
{
	glVertexPointer(3, GL_FLOAT, sizeof(april::TexturedVertex), v);
	glTexCoordPointer(2, GL_FLOAT, sizeof(april::TexturedVertex), &v->u);
	glDrawArrays(_glRenderOperations[renderOperation], 0, nVertices);
}

void CustomRenderSystem::_deviceRender(const april::RenderOperation& renderOperation, const april::ColoredVertex* v, int nVertices)
{
	glVertexPointer(3, GL_FLOAT, sizeof(april::ColoredVertex), v);
	glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(april::ColoredVertex), &v->color);
	glDrawArrays(_glRenderOperations[renderOperation], 0, nVertices);
}

void CustomRenderSystem::_deviceRender(const april::RenderOperation& renderOperation, const april::ColoredTexturedVertex* v, int nVertices)
{
	glVertexPointer(3, GL_FLOAT, sizeof(april::ColoredTexturedVertex), v);
	glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(april::ColoredTexturedVertex), &v->color);
	glTexCoordPointer(2, GL_FLOAT, sizeof(april::ColoredTexturedVertex), &v->u);
	glDrawArrays(_glRenderOperations[renderOperation], 0, nVertices);
}

april::Image::Format CustomRenderSystem::getNativeTextureFormat(april::Image::Format format)
{
	if (format == april::Image::FORMAT_ARGB || format == april::Image::FORMAT_ABGR || format == april::Image::Format::RGBA || format == april::Image::FORMAT_BGRA)
	{
		return april::Image::Format::RGBA;
	}
	if (format == april::Image::FORMAT_XRGB || format == april::Image::FORMAT_RGBX || format == april::Image::FORMAT_XBGR || format == april::Image::FORMAT_BGRX)
	{
		return april::Image::FORMAT_RGBX;
	}
	if (format == april::Image::FORMAT_RGB || format == april::Image::FORMAT_BGR)
	{
		return april::Image::FORMAT_RGB;
	}
	if (format == april::Image::FORMAT_ALPHA)
	{
		return april::Image::FORMAT_ALPHA;
	}
	if (format == april::Image::FORMAT_GRAYSCALE)
	{
		return april::Image::FORMAT_GRAYSCALE;
	}
	if (format == april::Image::FORMAT_COMPRESSED)
	{
		return april::Image::FORMAT_COMPRESSED;
	}
	if (format == april::Image::FORMAT_PALETTE)
	{
		return april::Image::FORMAT_PALETTE;
	}
	return april::Image::FORMAT_INVALID;
}

unsigned int CustomRenderSystem::getNativeColorUInt(const april::Color& color)
{
	return ((color.a << 24) | (color.b << 16) | (color.g << 8) | color.r);
}
