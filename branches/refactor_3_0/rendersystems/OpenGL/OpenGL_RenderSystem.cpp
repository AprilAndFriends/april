/// @file
/// @author  Kresimir Spes
/// @author  Ivan Vucica
/// @author  Boris Mikic
/// @version 3.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef _OPENGL

#include <hltypes/hplatform.h>
#if __APPLE__
#include <TargetConditionals.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifndef __APPLE__
#include <gl/GL.h>
#define GL_GLEXT_PROTOTYPES
#include <gl/glext.h>
#else
#include <OpenGL/gl.h>
#endif

#include <gtypes/Rectangle.h>
#include <hltypes/hlog.h>
#include <hltypes/hstring.h>

#include "april.h"
#include "Color.h"
#include "OpenGL_RenderSystem.h"
#include "OpenGL_Texture.h"

namespace april
{
	static Color lastColor = Color::Black;

	OpenGL_RenderSystem::OpenGL_RenderSystem() : RenderSystem(), activeTexture(NULL)
	{
	}

	OpenGL_RenderSystem::~OpenGL_RenderSystem()
	{
	}

	bool OpenGL_RenderSystem::create(chstr options)
	{
		if (!RenderSystem::create(options))
		{
			return false;
		}
		this->activeTexture = NULL;
		this->options = options;
		this->deviceState.reset();
		this->state.reset();
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
		this->state.reset();
		return true;
	}

	void OpenGL_RenderSystem::reset()
	{
		RenderSystem::reset();
		this->state.reset();
		this->deviceState.reset();
		this->_setupDefaultParameters();
		this->state.modelviewMatrixChanged = true;
		this->state.projectionMatrixChanged = true;
		this->_applyStateChanges();
	}

	void OpenGL_RenderSystem::_setupDefaultParameters()
	{
		glClearColor(0, 0, 0, 1);
		lastColor.set(0, 0, 0, 255);
		glViewport(0, 0, april::window->getWidth(), april::window->getHeight());
		// GL defaults
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnable(GL_TEXTURE_2D);
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
		if (this->state.textureCoordinatesEnabled != this->deviceState.textureCoordinatesEnabled)
		{
			this->_setClientState(GL_TEXTURE_COORD_ARRAY, this->state.textureCoordinatesEnabled);
			this->deviceState.textureCoordinatesEnabled = this->state.textureCoordinatesEnabled;
		}
		if (this->state.colorEnabled != this->deviceState.colorEnabled)
		{
			this->_setClientState(GL_COLOR_ARRAY, this->state.colorEnabled);
			this->deviceState.colorEnabled = this->state.colorEnabled;
		}
		if (this->state.systemColor != this->deviceState.systemColor)
		{
			glColor4f(this->state.systemColor.r_f(), this->state.systemColor.g_f(), this->state.systemColor.b_f(), this->state.systemColor.a_f());
			this->deviceState.systemColor = this->state.systemColor;
		}
		if (this->state.textureId != this->deviceState.textureId)
		{
			glBindTexture(GL_TEXTURE_2D, this->state.textureId);
			this->deviceState.textureId = this->state.textureId;
			// TODO - you should memorize address and filter modes per texture in opengl to avoid unnecesarry calls
			this->deviceState.textureAddressMode = Texture::ADDRESS_UNDEFINED;
			this->deviceState.textureFilter = Texture::FILTER_UNDEFINED;
		}
		// texture has to be bound first or else filter and address mode won't be applied afterwards
		if (this->state.textureFilter != this->deviceState.textureFilter || this->deviceState.textureFilter == Texture::FILTER_UNDEFINED)
		{
			this->_setTextureFilter(this->state.textureFilter);
			this->deviceState.textureFilter = this->state.textureFilter;
		}
		if (this->state.textureAddressMode != this->deviceState.textureAddressMode || this->deviceState.textureAddressMode == Texture::ADDRESS_UNDEFINED)
		{
			this->_setTextureAddressMode(this->state.textureAddressMode);
			this->deviceState.textureAddressMode = this->state.textureAddressMode;
		}
		if (this->state.blendMode != this->deviceState.blendMode)
		{
			this->_setTextureBlendMode(this->state.blendMode);
			this->deviceState.blendMode = this->state.blendMode;
		}
		if (this->state.colorMode != this->deviceState.colorMode || this->state.colorModeAlpha != this->deviceState.colorModeAlpha)
		{
			this->_setTextureColorMode(this->state.colorMode, this->state.colorModeAlpha);
			this->deviceState.colorMode = this->state.colorMode;
			this->deviceState.colorModeAlpha = this->state.colorModeAlpha;
		}
		if (this->state.modelviewMatrixChanged && this->modelviewMatrix != this->deviceState.modelviewMatrix)
		{
			this->setMatrixMode(GL_MODELVIEW);
			glLoadMatrixf(this->modelviewMatrix.data);
			this->deviceState.modelviewMatrix = this->modelviewMatrix;
			this->state.modelviewMatrixChanged = false;
		}
		if (this->state.projectionMatrixChanged && this->projectionMatrix != this->deviceState.projectionMatrix)
		{
			this->setMatrixMode(GL_PROJECTION);
			glLoadMatrixf(this->projectionMatrix.data);
			this->deviceState.projectionMatrix = this->projectionMatrix;
			this->state.projectionMatrixChanged = false;
		}
	}

	void OpenGL_RenderSystem::_setClientState(unsigned int type, bool enabled)
	{
		enabled ? glEnableClientState(type) : glDisableClientState(type);
	}

	void OpenGL_RenderSystem::bindTexture(unsigned int textureId)
	{
		this->state.textureId = textureId;
	}

	void OpenGL_RenderSystem::setMatrixMode(unsigned int mode)
	{
		// performance call, minimize redundant calls to setMatrixMode
		if (this->deviceState.modeMatrix != mode)
		{
			this->deviceState.modeMatrix = mode;
			glMatrixMode(mode);
		}
	}

	void OpenGL_RenderSystem::setTextureBlendMode(BlendMode mode)
	{
		this->state.blendMode = mode;
	}
	
	void OpenGL_RenderSystem::_setTextureBlendMode(BlendMode textureBlendMode)
	{
		// old-school blending mode for your mom
		if (textureBlendMode == ALPHA_BLEND || textureBlendMode == DEFAULT)
		{
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}
		else if (textureBlendMode == ADD)
		{
			glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		}
		else
		{
			hlog::warn(april::logTag, "Trying to set unsupported blend mode!");
		}
	}
	
	void OpenGL_RenderSystem::setTextureColorMode(ColorMode textureColorMode, unsigned char alpha)
	{
		this->state.colorMode = textureColorMode;
		this->state.colorModeAlpha = alpha;
	}

	void OpenGL_RenderSystem::_setTextureColorMode(ColorMode textureColorMode, unsigned char alpha)
	{
		static float constColor[4];
		for_iter (i, 0, 4)
		{
			constColor[i] = alpha / 255.0f;
		}
		switch (textureColorMode)
		{
		case NORMAL:
		case MULTIPLY:
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_TEXTURE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_ALPHA, GL_PRIMARY_COLOR);
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_TEXTURE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_RGB, GL_PRIMARY_COLOR);
			break;
		case LERP:
			glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, constColor);
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_TEXTURE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_ALPHA, GL_CONSTANT);
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_PRIMARY_COLOR);
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_RGB, GL_TEXTURE);
			break;
		case ALPHA_MAP:
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_TEXTURE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_ALPHA, GL_PRIMARY_COLOR);
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_REPLACE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_PRIMARY_COLOR);
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_RGB, GL_PRIMARY_COLOR);
			break;
		default:
			hlog::warn(april::logTag, "Trying to set unsupported color mode!");
			break;
		}
	}

	void OpenGL_RenderSystem::setTextureFilter(Texture::Filter textureFilter)
	{
		this->state.textureFilter = textureFilter;
	}

	void OpenGL_RenderSystem::_setTextureFilter(Texture::Filter textureFilter)
	{
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
			hlog::warn(april::logTag, "Trying to set unsupported texture filter!");
			break;
		}
		this->textureFilter = textureFilter;
	}

	void OpenGL_RenderSystem::setTextureAddressMode(Texture::AddressMode textureAddressMode)
	{
		this->state.textureAddressMode = textureAddressMode;
	}

	void OpenGL_RenderSystem::_setTextureAddressMode(Texture::AddressMode textureAddressMode)
	{
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
			hlog::warn(april::logTag, "Trying to set unsupported texture address mode!");
			break;
		}
		this->textureAddressMode = textureAddressMode;
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
			this->bindTexture(this->activeTexture->textureId);
		}
	}

	Texture* OpenGL_RenderSystem::getRenderTarget()
	{
		return NULL;
	}
	
	void OpenGL_RenderSystem::setRenderTarget(Texture* texture)
	{
		// TODO
	}
	
	void OpenGL_RenderSystem::setPixelShader(PixelShader* pixelShader)
	{
		hlog::warn(april::logTag, "Pixel shaders are not implemented!");
	}

	void OpenGL_RenderSystem::setVertexShader(VertexShader* vertexShader)
	{
		hlog::warn(april::logTag, "Vertex shaders are not implemented!");
	}

	void OpenGL_RenderSystem::setResolution(int w, int h)
	{
		hlog::warn(april::logTag, "setResolution() is not implemented!");
	}

	PixelShader* OpenGL_RenderSystem::createPixelShader()
	{
		hlog::warn(april::logTag, "Pixel shaders are not implemented!");
		return NULL;
	}

	PixelShader* OpenGL_RenderSystem::createPixelShader(chstr filename)
	{
		hlog::warn(april::logTag, "Pixel shaders are not implemented!");
		return NULL;
	}

	VertexShader* OpenGL_RenderSystem::createVertexShader()
	{
		hlog::warn(april::logTag, "Vertex shaders are not implemented!");
		return NULL;
	}

	VertexShader* OpenGL_RenderSystem::createVertexShader(chstr filename)
	{
		hlog::warn(april::logTag, "Vertex shaders are not implemented!");
		return NULL;
	}

}
#endif
