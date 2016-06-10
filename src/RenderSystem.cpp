/// @file
/// @version 4.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include <stdio.h>
#include <algorithm>
#ifdef __APPLE__
#include <TargetConditionals.h>
#endif

#include <hltypes/harray.h>
#include <hltypes/hfile.h>
#include <hltypes/hlog.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hresource.h>
#include <hltypes/hstring.h>
#include <hltypes/hthread.h>

#include "april.h"
#include "aprilUtil.h"
#include "Image.h"
#include "RenderHelperLayered2D.h"
#include "RenderSystem.h"
#include "RenderState.h"
#include "PixelShader.h"
#include "Platform.h"
#include "Texture.h"
#include "TextureAsync.h"
#include "VertexShader.h"
#include "Window.h"

namespace april
{
	// optimizations, but they are not thread-safe
	static PlainVertex pv[5];
	static TexturedVertex tv[5];
	
	RenderSystem* rendersys = NULL;

	RenderSystem::DisplayMode::DisplayMode(int width, int height, int refreshRate)
	{
		this->width = width;
		this->height = height;
		this->refreshRate = refreshRate;
	}

	RenderSystem::DisplayMode::~DisplayMode()
	{
	}
	
	bool RenderSystem::DisplayMode::operator==(const DisplayMode& other) const
	{
		return (this->width == other.width && this->height == other.height && this->refreshRate == other.refreshRate);
	}

	bool RenderSystem::DisplayMode::operator!=(const DisplayMode& other) const
	{
		return (this->width != other.width || this->height != other.height || this->refreshRate != other.refreshRate);
	}
	
	hstr RenderSystem::DisplayMode::toString()
	{
		return hsprintf("%dx%d@%dHz", this->width, this->height, this->refreshRate);
	}
	
	RenderSystem::Options::Options()
	{
		this->depthBuffer = false;
	}

	RenderSystem::Options::~Options()
	{
	}

	RenderSystem::Caps::Caps()
	{
		this->maxTextureSize = 0;
		this->npotTexturesLimited = false;
		this->npotTextures = false;
		this->textureFormats += Image::FORMAT_ABGR;
		this->textureFormats += Image::FORMAT_RGBA;
		this->textureFormats += Image::FORMAT_ARGB;
		this->textureFormats += Image::FORMAT_BGRA;
		this->textureFormats += Image::FORMAT_ABGR;
		this->textureFormats += Image::FORMAT_RGBX;
		this->textureFormats += Image::FORMAT_XRGB;
		this->textureFormats += Image::FORMAT_BGRX;
		this->textureFormats += Image::FORMAT_XBGR;
		this->textureFormats += Image::FORMAT_RGB;
		this->textureFormats += Image::FORMAT_BGR;
		this->textureFormats += Image::FORMAT_ALPHA;
		this->textureFormats += Image::FORMAT_GRAYSCALE;
		this->textureFormats += Image::FORMAT_PALETTE;
	}

	RenderSystem::Caps::~Caps()
	{
	}

	hstr RenderSystem::Options::toString()
	{
		harray<hstr> options;
		if (this->depthBuffer)
		{
			options += "depth-buffer";
		}
		if (options.size() == 0)
		{
			options += "none";
		}
		return options.joined(',');
	}
	
	RenderSystem::RenderSystem() : renderHelper(NULL)
	{
		this->name = "Generic";
		this->created = false;
		this->pixelOffset = 0.0f;
		this->state = new RenderState();
		this->deviceState = new RenderState();
		this->statCurrentFrameRenderCalls = 0;
		this->statLastFrameRenderCalls = 0;
		this->statCurrentFrameTextureSwitches = 0;
		this->statLastFrameTextureSwitches = 0;
		this->statCurrentFrameVertexCount = 0;
		this->statLastFrameVertexCount = 0;
		this->statCurrentFrameTriangleCount = 0;
		this->statLastFrameTriangleCount = 0;
		this->statCurrentFrameLineCount = 0;
		this->statLastFrameLineCount = 0;
	}
	
	RenderSystem::~RenderSystem()
	{
		if (this->created)
		{
			hlog::warn(logTag, "Deleting rendersystem before destroy() was called!");
		}
		delete this->state;
		delete this->deviceState;
		if (this->renderHelper != NULL)
		{
			delete this->renderHelper;
		}
	}

	void RenderSystem::init()
	{
		this->_deviceInit();
	}
	
	bool RenderSystem::create(RenderSystem::Options options)
	{
		if (!this->created)
		{
			hlog::writef(logTag, "Creating rendersystem: '%s' (options: %s)", this->name.cStr(), options.toString().cStr());
			this->options = options;
			this->state->reset();
			this->deviceState->reset();
			this->statCurrentFrameRenderCalls = 0;
			this->statLastFrameRenderCalls = 0;
			this->statCurrentFrameTextureSwitches = 0;
			this->statLastFrameTextureSwitches = 0;
			this->statCurrentFrameVertexCount = 0;
			this->statLastFrameVertexCount = 0;
			this->statCurrentFrameTriangleCount = 0;
			this->statLastFrameTriangleCount = 0;
			this->statCurrentFrameLineCount = 0;
			this->statLastFrameLineCount = 0;
			// create the actual device
			this->_deviceInit();
			this->created = this->_deviceCreate(options);
			if (!this->created)
			{
				this->destroy();
			}
		}
		return this->created;
	}

	bool RenderSystem::destroy()
	{
		if (this->created)
		{
			hlog::writef(logTag, "Destroying rendersystem '%s'.", this->name.cStr());
			this->created = false;
			if (this->renderHelper != NULL)
			{
				delete this->renderHelper;
				this->renderHelper = NULL;
			}
			// first wait for queud textures to cancel
			harray<Texture*> textures = this->getTextures();
			if (this->hasAsyncTexturesQueued())
			{
				foreach (Texture*, it, textures)
				{
					if ((*it)->isAsyncLoadQueued())
					{
						(*it)->unload(); // to cancel all async loads
					}
				}
				this->waitForAsyncTextures();
			}
			// creating a copy (again), because deleting a texture modifies this->textures
			textures = this->getTextures();
			foreach (Texture*, it, textures)
			{
				delete (*it);
			}
			this->state->reset();
			this->deviceState->reset();
			this->statCurrentFrameRenderCalls = 0;
			this->statLastFrameRenderCalls = 0;
			this->statCurrentFrameTextureSwitches = 0;
			this->statLastFrameTextureSwitches = 0;
			this->statCurrentFrameVertexCount = 0;
			this->statLastFrameVertexCount = 0;
			this->statCurrentFrameTriangleCount = 0;
			this->statLastFrameTriangleCount = 0;
			this->statCurrentFrameLineCount = 0;
			this->statLastFrameLineCount = 0;
			if (!this->_deviceDestroy())
			{
				return false;
			}
			this->_deviceInit();
			return true;
		}
		return false;
	}
	
	void RenderSystem::assignWindow(Window* window)
	{
		this->_deviceAssignWindow(window);
		this->_deviceSetupCaps();
		this->_deviceSetup();
		grect viewport(0.0f, 0.0f, april::window->getSize());
		this->setViewport(viewport);
		this->setOrthoProjection(viewport);
		this->_updateDeviceState(true);
		this->clear();
	}

	void RenderSystem::reset()
	{
		hlog::write(logTag, "Resetting rendersystem.");
		this->statCurrentFrameRenderCalls = 0;
		this->statLastFrameRenderCalls = 0;
		this->statCurrentFrameTextureSwitches = 0;
		this->statLastFrameTextureSwitches = 0;
		this->statCurrentFrameVertexCount = 0;
		this->statLastFrameVertexCount = 0;
		this->statCurrentFrameTriangleCount = 0;
		this->statLastFrameTriangleCount = 0;
		this->statCurrentFrameLineCount = 0;
		this->statLastFrameLineCount = 0;
		this->_deviceReset();
		this->_deviceSetup();
		if (this->deviceState->texture != NULL)
		{
			this->deviceState->texture->load();
		}
		this->setViewport(grect(0.0f, 0.0f, april::window->getSize()));
		this->_updateDeviceState(true);
	}

	void RenderSystem::_deviceReset()
	{
	}

	void RenderSystem::suspend()
	{
		hlog::write(logTag, "Suspending rendersystem.");
		this->_deviceSuspend();
	}

	void RenderSystem::_deviceSuspend()
	{
	}

	void RenderSystem::_deviceSetupDisplayModes()
	{
		gvec2 resolution = april::getSystemInfo().displayResolution;
		this->displayModes += RenderSystem::DisplayMode((int)resolution.x, (int)resolution.y, 60);
	}

	harray<Texture*> RenderSystem::getTextures()
	{
		hmutex::ScopeLock lock(&this->texturesMutex);
		return this->textures;
	}

	harray<RenderSystem::DisplayMode> RenderSystem::getDisplayModes()
	{
		if (this->displayModes.size() == 0)
		{
			this->_deviceSetupDisplayModes();
		}
		return this->displayModes;
	}

	int64_t RenderSystem::getVRamConsumption()
	{
		int64_t result = 0LL;
		harray<Texture*> textures = this->getTextures();
		foreach (Texture*, it, textures)
		{
			result += (int64_t)(*it)->getCurrentVRamSize();
		}
		return result;
	}

	int64_t RenderSystem::getRamConsumption()
	{
		int64_t result = 0LL;
		harray<Texture*> textures = this->getTextures();
		foreach (Texture*, it, textures)
		{
			result += (int64_t)(*it)->getCurrentRamSize();
		}
		return result;
	}

	int64_t RenderSystem::getAsyncRamConsumption()
	{
		int64_t result = 0LL;
		harray<Texture*> textures = this->getTextures();
		foreach (Texture*, it, textures)
		{
			result += (int64_t)(*it)->getCurrentAsyncRamSize();
		}
		return result;
	}

	bool RenderSystem::hasAsyncTexturesQueued()
	{
		return TextureAsync::isRunning();
	}

	grect RenderSystem::getViewport()
	{
		return this->state->viewport;
	}

	void RenderSystem::setViewport(const grect& value)
	{
		this->state->viewport = value;
		this->state->viewportChanged = true;
	}

	gmat4 RenderSystem::getModelviewMatrix()
	{
		return this->state->modelviewMatrix;
	}

	void RenderSystem::setModelviewMatrix(const gmat4& value)
	{
		this->state->modelviewMatrix = value;
		this->state->modelviewMatrixChanged = true;
	}

	gmat4 RenderSystem::getProjectionMatrix()
	{
		return this->state->projectionMatrix;
	}

	void RenderSystem::setProjectionMatrix(const gmat4& value)
	{
		this->state->projectionMatrix = value;
		this->state->projectionMatrixChanged = true;
	}
	
	bool RenderSystem::isLayeredRenderer2dEnabled()
	{
		return (dynamic_cast<RenderHelperLayered2D*>(this->renderHelper) != NULL);
	}
	
	void RenderSystem::setLayeredRenderer2dEnabled(bool value)
	{
		bool isLayeredRenderer = (dynamic_cast<RenderHelperLayered2D*>(this->renderHelper) != NULL);
		if (value)
		{
			if (this->renderHelper != NULL && !isLayeredRenderer)
			{
				this->renderHelper->destroy();
				delete this->renderHelper;
			}
			this->renderHelper = new RenderHelperLayered2D();
			this->renderHelper->create();
		}
		else if (isLayeredRenderer)
		{
			this->renderHelper->destroy();
			delete this->renderHelper;
			this->renderHelper = NULL;
		}
	}

	Texture* RenderSystem::createTextureFromResource(chstr filename, Texture::Type type, Texture::LoadMode loadMode)
	{
		return this->_createTextureFromSource(true, filename, type, loadMode);
	}

	Texture* RenderSystem::createTextureFromResource(chstr filename, Image::Format format, Texture::Type type, Texture::LoadMode loadMode)
	{
		return this->_createTextureFromSource(true, filename, type, loadMode, format);
	}

	Texture* RenderSystem::createTextureFromFile(chstr filename, Texture::Type type, Texture::LoadMode loadMode)
	{
		return this->_createTextureFromSource(false, filename, type, loadMode);
	}

	Texture* RenderSystem::createTextureFromFile(chstr filename, Image::Format format, Texture::Type type, Texture::LoadMode loadMode)
	{
		return this->_createTextureFromSource(false, filename, type, loadMode, format);
	}

	Texture* RenderSystem::_createTextureFromSource(bool fromResource, chstr filename, Texture::Type type, Texture::LoadMode loadMode, Image::Format format)
	{
		if (format != Image::FORMAT_INVALID && !this->getCaps().textureFormats.has(format))
		{
			hlog::errorf(logTag, "Cannot create texture '%s', the texture format '%d' is not supported!", filename.cStr(), format);
			return NULL;
		}
		hstr name = (fromResource ? this->findTextureResource(filename) : this->findTextureFile(filename));
		if (name == "")
		{
			return NULL;
		}
		Texture* texture = this->_deviceCreateTexture(fromResource);
		bool result = (format == Image::FORMAT_INVALID ? texture->_create(name, type, loadMode) : texture->_create(name, format, type, loadMode));
		if (result)
		{
			if (loadMode == Texture::LOAD_IMMEDIATE)
			{
				result = texture->load();
			}
			else if (loadMode == Texture::LOAD_ASYNC || loadMode == Texture::LOAD_ASYNC_ON_DEMAND)
			{
				result = texture->loadAsync();
			}
		}
		if (!result)
		{
			delete texture;
			return NULL;
		}
		hmutex::ScopeLock lock(&this->texturesMutex);
		this->textures += texture;
		return texture;
	}

	Texture* RenderSystem::createTexture(int w, int h, unsigned char* data, Image::Format format, Texture::Type type)
	{
		Texture* texture = this->_deviceCreateTexture(true);
		if (!texture->_create(w, h, data, format, type))
		{
			delete texture;
			return NULL;
		}
		hmutex::ScopeLock lock(&this->texturesMutex);
		this->textures += texture;
		return texture;
	}

	Texture* RenderSystem::createTexture(int w, int h, Color color, Image::Format format, Texture::Type type)
	{
		Texture* texture = this->_deviceCreateTexture(true);
		if (!texture->_create(w, h, color, format, type))
		{
			delete texture;
			return NULL;
		}
		hmutex::ScopeLock lock(&this->texturesMutex);
		this->textures += texture;
		return texture;
	}

	void RenderSystem::destroyTexture(Texture* texture)
	{
		if (this->renderHelper != NULL)
		{
			this->renderHelper->flush();
		}
		texture->unload();
		texture->waitForAsyncLoad(); // waiting for all async stuff to finish
		hmutex::ScopeLock lock(&this->texturesMutex);
		this->textures -= texture;
		lock.release();
		if (this->state->texture == texture)
		{
			this->state->texture = NULL;
		}
		if (this->deviceState->texture == texture)
		{
			this->deviceState->texture = NULL;
			this->_setDeviceTexture(NULL);
		}
		delete texture;
	}

	PixelShader* RenderSystem::createPixelShaderFromResource(chstr filename)
	{
		return this->_createPixelShaderFromSource(true, filename);
	}

	PixelShader* RenderSystem::createPixelShaderFromFile(chstr filename)
	{
		return this->_createPixelShaderFromSource(false, filename);
	}

	PixelShader* RenderSystem::createPixelShader()
	{
		return this->_deviceCreatePixelShader();
	}

	VertexShader* RenderSystem::createVertexShaderFromResource(chstr filename)
	{
		return this->_createVertexShaderFromSource(true, filename);
	}

	VertexShader* RenderSystem::createVertexShaderFromFile(chstr filename)
	{
		return this->_createVertexShaderFromSource(false, filename);
	}

	VertexShader* RenderSystem::createVertexShader()
	{
		return this->_deviceCreateVertexShader();
	}

	PixelShader* RenderSystem::_createPixelShaderFromSource(bool fromResource, chstr filename)
	{
		PixelShader* shader = this->_deviceCreatePixelShader();
		if (shader != NULL)
		{
			bool loaded = (fromResource ? shader->loadResource(filename) : shader->loadFile(filename));
			if (!loaded)
			{
				delete shader;
				shader = NULL;
			}
		}
		return shader;
	}

	VertexShader* RenderSystem::_createVertexShaderFromSource(bool fromResource, chstr filename)
	{
		VertexShader* shader = this->_deviceCreateVertexShader();
		if (shader != NULL)
		{
			bool loaded = (fromResource ? shader->loadResource(filename) : shader->loadFile(filename));
			if (!loaded)
			{
				delete shader;
				shader = NULL;
			}
		}
		return shader;
	}

	PixelShader* RenderSystem::_deviceCreatePixelShader()
	{
		hlog::warnf(logTag, "Pixel shaders are not implemented in render system '%s'!", this->name.cStr());
		return NULL;
	}

	VertexShader* RenderSystem::_deviceCreateVertexShader()
	{
		hlog::warnf(logTag, "Vertex shaders are not implemented in render system '%s'!", this->name.cStr());
		return NULL;
	}

	void RenderSystem::destroyPixelShader(PixelShader* shader)
	{
		delete shader;
	}

	void RenderSystem::destroyVertexShader(VertexShader* shader)
	{
		delete shader;
	}

	grect RenderSystem::getOrthoProjection()
	{
		grect result;
		if (this->state->projectionMatrix.data[0] != 0.0f && this->state->projectionMatrix.data[5] != 0.0f)
		{
			result.w = 2.0f / this->state->projectionMatrix.data[0];
			result.h = -2.0f / this->state->projectionMatrix.data[5];
			result.x = (1.0f + this->state->projectionMatrix.data[12]) * result.w * 0.5f;
			result.y = (1.0f - this->state->projectionMatrix.data[13]) * result.h * 0.5f;
			result += result.getSize() * this->pixelOffset / april::window->getSize();
		}
		return result;
	}

	void RenderSystem::setOrthoProjection(grect rect)
	{
		rect -= rect.getSize() * this->pixelOffset / april::window->getSize();
		this->state->projectionMatrix.setOrthoProjection(rect);
		this->state->projectionMatrixChanged = true;
	}

	void RenderSystem::setOrthoProjection(grect rect, float nearZ, float farZ)
	{
		rect -= rect.getSize() * this->pixelOffset / april::window->getSize();
		this->state->projectionMatrix.setOrthoProjection(rect, nearZ, farZ);
		this->state->projectionMatrixChanged = true;
	}

	void RenderSystem::setOrthoProjection(gvec2 size)
	{
		this->setOrthoProjection(grect(0.0f, 0.0f, size));
	}

	void RenderSystem::setOrthoProjection(gvec2 size, float nearZ, float farZ)
	{
		this->setOrthoProjection(grect(0.0f, 0.0f, size), nearZ, farZ);
	}

	void RenderSystem::setDepthBuffer(bool enabled, bool writeEnabled)
	{
		if (this->options.depthBuffer)
		{
			this->state->depthBuffer = enabled;
			this->state->depthBufferWrite = writeEnabled;
		}
		else
		{
			hlog::error(logTag, "Cannot change depth-buffer state, RenderSystem was not created with this option!");
		}
	}

	void RenderSystem::setTexture(Texture* texture)
	{
		this->state->texture = texture;
	}

	void RenderSystem::setBlendMode(BlendMode blendMode)
	{
		this->state->blendMode = blendMode;
	}

	void RenderSystem::setColorMode(ColorMode colorMode, float colorModeFactor)
	{
		this->state->colorMode = colorMode;
		this->state->colorModeFactor = colorModeFactor;
	}

	void RenderSystem::_deviceChangeResolution(int w, int h, bool fullscreen)
	{
		hlog::warnf(logTag, "Changing resolutions is not implemented in render system '%s'!", this->name.cStr());
	}

	void RenderSystem::setIdentityTransform()
	{
		this->state->modelviewMatrix.setIdentity();
		this->state->modelviewMatrixChanged = true;
	}
	
	void RenderSystem::translate(float x, float y, float z)
	{
		this->state->modelviewMatrix.translate(x, y, z);
		this->state->modelviewMatrixChanged = true;
	}
	
	void RenderSystem::translate(const gvec3& vector)
	{
		this->state->modelviewMatrix.translate(vector);
		this->state->modelviewMatrixChanged = true;
	}

	void RenderSystem::translate(const gvec2& vector)
	{
		this->state->modelviewMatrix.translate(vector.x, vector.y, 0.0f);
		this->state->modelviewMatrixChanged = true;
	}

	void RenderSystem::rotate(float angle)
	{
		this->state->modelviewMatrix.rotate(0.0f, 0.0f, -1.0f, angle);
		this->state->modelviewMatrixChanged = true;
	}

	void RenderSystem::rotate(float ax, float ay, float az, float angle)
	{
		this->state->modelviewMatrix.rotate(ax, ay, az, angle);
		this->state->modelviewMatrixChanged = true;
	}	
	
	void RenderSystem::rotate(const gvec3& axis, float angle)
	{
		this->state->modelviewMatrix.rotate(axis, angle);
		this->state->modelviewMatrixChanged = true;
	}

	void RenderSystem::scale(float factor)
	{
		this->state->modelviewMatrix.scale(factor);
		this->state->modelviewMatrixChanged = true;
	}
	
	void RenderSystem::scale(float factorX, float factorY, float factorZ)
	{
		this->state->modelviewMatrix.scale(factorX, factorY, factorZ);
		this->state->modelviewMatrixChanged = true;
	}
	
	void RenderSystem::scale(const gvec3& vector)
	{
		this->state->modelviewMatrix.scale(vector);
		this->state->modelviewMatrixChanged = true;
	}

	void RenderSystem::scale(const gvec2& vector)
	{
		this->state->modelviewMatrix.scale(vector.x, vector.y, 1.0f);
		this->state->modelviewMatrixChanged = true;
	}

	void RenderSystem::lookAt(const gvec3& eye, const gvec3& target, const gvec3& up)
	{
		this->state->modelviewMatrix.lookAt(eye, target, up);
		this->state->modelviewMatrixChanged = true;
	}
		
	void RenderSystem::setPerspective(float fov, float aspect, float nearZ, float farZ)
	{
		this->state->projectionMatrix.setPerspective(fov, aspect, nearZ, farZ);
		this->state->projectionMatrixChanged = true;
	}

	void RenderSystem::_updateDeviceState(bool forceUpdate)
	{
		// viewport
		if (forceUpdate || this->state->viewportChanged)
		{
			if (forceUpdate || this->deviceState->viewport != this->state->viewport)
			{
				this->_setDeviceViewport(this->state->viewport);
				this->deviceState->viewport = this->state->viewport;
			}
			this->state->viewportChanged = false;
		}
		// modelview matrix
		if (forceUpdate || this->state->modelviewMatrixChanged)
		{
			if (forceUpdate || this->deviceState->modelviewMatrix != this->state->modelviewMatrix)
			{
				this->_setDeviceModelviewMatrix(this->state->modelviewMatrix);
				this->deviceState->modelviewMatrix = this->state->modelviewMatrix;
			}
			this->state->modelviewMatrixChanged = false;
		}
		// projection matrix
		if (forceUpdate || this->state->projectionMatrixChanged)
		{
			if (forceUpdate || this->deviceState->projectionMatrix != this->state->projectionMatrix)
			{
				this->_setDeviceProjectionMatrix(this->state->projectionMatrix);
				this->deviceState->projectionMatrix = this->state->projectionMatrix;
			}
			this->state->projectionMatrixChanged = false;
		}
		// depth buffer
		if (forceUpdate || this->deviceState->depthBuffer != this->state->depthBuffer || this->deviceState->depthBufferWrite != this->state->depthBufferWrite)
		{
			this->_setDeviceDepthBuffer(this->state->depthBuffer, this->state->depthBufferWrite);
			this->deviceState->depthBuffer = this->state->depthBuffer;
			this->deviceState->depthBufferWrite = this->state->depthBufferWrite;
		}
		// device render mode
		if (forceUpdate || this->deviceState->useTexture != this->state->useTexture || this->deviceState->useColor != this->state->useColor)
		{
			this->_setDeviceRenderMode(this->state->useTexture, this->state->useColor);
		}
		// texture
		if (forceUpdate || this->deviceState->texture != this->state->texture || this->deviceState->useTexture != this->state->useTexture)
		{
			// filtering and wrapping applied before loading texture data, some systems are optimized to work like this (e.g. iOS OpenGLES guidelines suggest it)
			if (this->state->texture != NULL && this->state->useTexture)
			{
				++this->statCurrentFrameTextureSwitches;
				this->state->texture->load();
				this->state->texture->unlock();
				this->_setDeviceTexture(this->state->texture);
				this->_setDeviceTextureFilter(this->state->texture->getFilter());
				this->_setDeviceTextureAddressMode(this->state->texture->getAddressMode());
			}
			else
			{
				this->_setDeviceTexture(NULL);
			}
			this->deviceState->texture = this->state->texture;
		}
		// blend mode
		if (forceUpdate || this->deviceState->blendMode != this->state->blendMode)
		{
			this->_setDeviceBlendMode(this->state->blendMode);
			this->deviceState->blendMode = this->state->blendMode;
		}
		// color mode
		if (forceUpdate || this->deviceState->colorMode != this->state->colorMode || this->deviceState->colorModeFactor != this->state->colorModeFactor ||
			this->deviceState->useTexture != this->state->useTexture || this->deviceState->useColor != this->state->useColor ||
			this->deviceState->systemColor != this->state->systemColor)
		{
			this->_setDeviceColorMode(this->state->colorMode, this->state->colorModeFactor, this->state->useTexture, this->state->useColor, this->state->systemColor);
			this->deviceState->colorMode = this->state->colorMode;
			this->deviceState->colorModeFactor = this->state->colorModeFactor;
			this->deviceState->useColor = this->state->useColor;
			this->deviceState->systemColor = this->state->systemColor;
		}
		// shared variables
		this->deviceState->useTexture = this->state->useTexture;
		this->deviceState->useColor = this->state->useColor;
	}

	void RenderSystem::clear(bool depth)
	{
		if (this->renderHelper != NULL)
		{
			this->renderHelper->clear();
		}
		if (!this->options.depthBuffer)
		{
			depth = false;
		}
		this->_deviceClear(depth);
	}

	void RenderSystem::clear(Color color, bool depth)
	{
		if (!this->options.depthBuffer)
		{
			depth = false;
		}
		this->_deviceClear(color, depth);
	}

	void RenderSystem::clearDepth()
	{
		if (this->options.depthBuffer)
		{
			this->_deviceClearDepth();
		}
	}

	void RenderSystem::render(RenderOperation renderOperation, PlainVertex* vertices, int count)
	{
		if (this->renderHelper == NULL || !this->renderHelper->render(renderOperation, vertices, count))
		{
			this->_renderInternal(renderOperation, vertices, count);
		}
	}

	void RenderSystem::render(RenderOperation renderOperation, PlainVertex* vertices, int count, Color color)
	{
		if (color.a == 0)
		{
			return;
		}
		if (this->renderHelper == NULL || !this->renderHelper->render(renderOperation, vertices, count, color))
		{
			this->_renderInternal(renderOperation, vertices, count, color);
		}
	}

	void RenderSystem::render(RenderOperation renderOperation, TexturedVertex* vertices, int count)
	{
		if (this->renderHelper == NULL || !this->renderHelper->render(renderOperation, vertices, count))
		{
			this->_renderInternal(renderOperation, vertices, count);
		}
	}

	void RenderSystem::render(RenderOperation renderOperation, TexturedVertex* vertices, int count, Color color)
	{
		if (color.a == 0)
		{
			return;
		}
		if (this->renderHelper == NULL || !this->renderHelper->render(renderOperation, vertices, count, color))
		{
			this->_renderInternal(renderOperation, vertices, count, color);
		}
	}

	void RenderSystem::render(RenderOperation renderOperation, ColoredVertex* vertices, int count)
	{
		if (this->renderHelper == NULL || !this->renderHelper->render(renderOperation, vertices, count))
		{
			this->_renderInternal(renderOperation, vertices, count);
		}
	}

	void RenderSystem::render(RenderOperation renderOperation, ColoredTexturedVertex* vertices, int count)
	{
		if (this->renderHelper == NULL || !this->renderHelper->render(renderOperation, vertices, count))
		{
			this->_renderInternal(renderOperation, vertices, count);
		}
	}

	void RenderSystem::drawRect(grect rect, Color color)
	{
		if (color.a == 0)
		{
			return;
		}
		if (this->renderHelper == NULL || !this->renderHelper->drawRect(rect, color))
		{
			this->_drawRectInternal(rect, color);
		}
	}

	void RenderSystem::drawFilledRect(grect rect, Color color)
	{
		if (color.a == 0)
		{
			return;
		}
		if (this->renderHelper == NULL || !this->renderHelper->drawFilledRect(rect, color))
		{
			this->_drawFilledRectInternal(rect, color);
		}
	}

	void RenderSystem::drawTexturedRect(grect rect, grect src)
	{
		if (this->renderHelper == NULL || !this->renderHelper->drawTexturedRect(rect, src))
		{
			this->_drawTexturedRectInternal(rect, src);
		}
	}

	void RenderSystem::drawTexturedRect(grect rect, grect src, Color color)
	{
		if (color.a == 0)
		{
			return;
		}
		if (this->renderHelper == NULL || !this->renderHelper->drawTexturedRect(rect, src, color))
		{
			this->_drawTexturedRectInternal(rect, src, color);
		}
	}

	void RenderSystem::_renderInternal(RenderOperation renderOperation, PlainVertex* vertices, int count)
	{
		this->_increaseStats(renderOperation, count);
		this->state->useTexture = false;
		this->state->useColor = false;
		this->state->systemColor = Color::White;
		this->_updateDeviceState();
		this->_deviceRender(renderOperation, vertices, count);
	}

	void RenderSystem::_renderInternal(RenderOperation renderOperation, PlainVertex* vertices, int count, Color color)
	{
		if (color.a == 0)
		{
			return;
		}
		this->_increaseStats(renderOperation, count);
		this->state->useTexture = false;
		this->state->useColor = false;
		this->state->systemColor = color;
		this->_updateDeviceState();
		this->_deviceRender(renderOperation, vertices, count);
	}

	void RenderSystem::_renderInternal(RenderOperation renderOperation, TexturedVertex* vertices, int count)
	{
		this->_increaseStats(renderOperation, count);
		this->state->useTexture = true;
		this->state->useColor = false;
		this->state->systemColor = Color::White;
		this->_updateDeviceState();
		this->_deviceRender(renderOperation, vertices, count);
	}

	void RenderSystem::_renderInternal(RenderOperation renderOperation, TexturedVertex* vertices, int count, Color color)
	{
		if (color.a == 0)
		{
			return;
		}
		this->_increaseStats(renderOperation, count);
		this->state->useTexture = true;
		this->state->useColor = false;
		this->state->systemColor = color;
		this->_updateDeviceState();
		this->_deviceRender(renderOperation, vertices, count);
	}

	void RenderSystem::_renderInternal(RenderOperation renderOperation, ColoredVertex* vertices, int count)
	{
		this->_increaseStats(renderOperation, count);
		this->state->useTexture = false;
		this->state->useColor = true;
		this->state->systemColor = Color::White;
		this->_updateDeviceState();
		this->_deviceRender(renderOperation, vertices, count);
	}

	void RenderSystem::_renderInternal(RenderOperation renderOperation, ColoredTexturedVertex* vertices, int count)
	{
		this->_increaseStats(renderOperation, count);
		this->state->useTexture = true;
		this->state->useColor = true;
		this->state->systemColor = Color::White;
		this->_updateDeviceState();
		this->_deviceRender(renderOperation, vertices, count);
	}

	void RenderSystem::_drawRectInternal(grect rect, Color color)
	{
		if (color.a == 0)
		{
			return;
		}
		pv[0].x = pv[3].x = pv[4].x = rect.x;
		pv[0].y = pv[1].y = pv[4].y = rect.y;
		pv[1].x = pv[2].x = rect.x + rect.w;
		pv[2].y = pv[3].y = rect.y + rect.h;
		this->_renderInternal(RO_LINE_STRIP, pv, 5, color);
	}

	void RenderSystem::_drawFilledRectInternal(grect rect, Color color)
	{
		if (color.a == 0)
		{
			return;
		}
		pv[0].x = pv[2].x = rect.x;
		pv[0].y = pv[1].y = rect.y;
		pv[1].x = pv[3].x = rect.x + rect.w;
		pv[2].y = pv[3].y = rect.y + rect.h;
		this->_renderInternal(RO_TRIANGLE_STRIP, pv, 4, color);
	}
	
	void RenderSystem::_drawTexturedRectInternal(grect rect, grect src)
	{
		tv[0].x = tv[2].x = rect.x;
		tv[0].y = tv[1].y = rect.y;
		tv[0].u = tv[2].u = src.x;
		tv[0].v = tv[1].v = src.y;
		tv[1].x = tv[3].x = rect.x + rect.w;
		tv[1].u = tv[3].u = src.x + src.w;
		tv[2].y = tv[3].y = rect.y + rect.h;
		tv[2].v = tv[3].v = src.y + src.h;
		this->_renderInternal(RO_TRIANGLE_STRIP, tv, 4);
	}
	
	void RenderSystem::_drawTexturedRectInternal(grect rect, grect src, Color color)
	{
		if (color.a == 0)
		{
			return;
		}
		tv[0].x = tv[2].x = rect.x;
		tv[0].y = tv[1].y = rect.y;
		tv[0].u = tv[2].u = src.x;
		tv[0].v = tv[1].v = src.y;
		tv[1].x = tv[3].x = rect.x + rect.w;
		tv[1].u = tv[3].u = src.x + src.w;
		tv[2].y = tv[3].y = rect.y + rect.h;
		tv[2].v = tv[3].v = src.y + src.h;
		this->_renderInternal(RO_TRIANGLE_STRIP, tv, 4, color);
	}

	void RenderSystem::_increaseStats(RenderOperation renderOperation, int count)
	{
		++this->statCurrentFrameRenderCalls;
		this->statCurrentFrameVertexCount += count;
		if (renderOperation == RO_TRIANGLE_LIST || renderOperation == RO_TRIANGLE_STRIP || renderOperation == RO_TRIANGLE_FAN)
		{
			this->statCurrentFrameTriangleCount += this->_numPrimitives(renderOperation, count);
		}
		else if (renderOperation == RO_LINE_LIST || renderOperation == RO_LINE_STRIP)
		{
			this->statCurrentFrameLineCount += this->_numPrimitives(renderOperation, count);
		}
	}

	hstr RenderSystem::findTextureResource(chstr filename)
	{
		if (hresource::exists(filename))
		{
			return filename;
		}
		hstr name;
		harray<hstr> extensions = april::getTextureExtensions();
		foreach (hstr, it, extensions)
		{
			name = filename + (*it);
			if (hresource::exists(name))
			{
				return name;
			}
		}
		hstr noExtensionName = hfile::withoutExtension(filename);
		if (noExtensionName != filename)
		{
			foreach (hstr, it, extensions)
			{
				name = noExtensionName + (*it);
				if (hresource::exists(name))
				{
					return name;
				}
			}
		}
		return "";
	}
	
	hstr RenderSystem::findTextureFile(chstr filename)
	{
		if (hfile::exists(filename))
		{
			return filename;
		}
		hstr name;
		harray<hstr> extensions = april::getTextureExtensions();
		foreach (hstr, it, extensions)
		{
			name = filename + (*it);
			if (hfile::exists(name))
			{
				return name;
			}
		}
		hstr noExtensionName = hfile::withoutExtension(filename);
		if (noExtensionName != filename)
		{
			foreach (hstr, it, extensions)
			{
				name = noExtensionName + (*it);
				if (hfile::exists(name))
				{
					return name;
				}
			}
		}
		return "";
	}
	
	void RenderSystem::unloadTextures()
	{
		harray<Texture*> textures = this->getTextures();
		foreach (Texture*, it, textures)
		{
			(*it)->unload();
		}
	}

	void RenderSystem::waitForAsyncTextures(float timeout)
	{
		float time = timeout;
		while ((time > 0.0f || timeout <= 0.0f) && this->hasAsyncTexturesQueued())
		{
			hthread::sleep(0.1f);
			time -= 0.0001f;
			TextureAsync::update();
		}
	}

	april::Image* RenderSystem::takeScreenshot(Image::Format format)
	{
		hlog::warnf(logTag, "Screenshots are not implemented in render system '%s'!", this->name.cStr());
		return NULL;
	}

	void RenderSystem::flushFrame()
	{
		if (this->renderHelper != NULL)
		{
			this->renderHelper->flush();
		}
		this->statLastFrameRenderCalls = this->statCurrentFrameRenderCalls;
		this->statCurrentFrameRenderCalls = 0;
		this->statLastFrameTextureSwitches = this->statCurrentFrameTextureSwitches;
		this->statCurrentFrameTextureSwitches = 0;
		this->statLastFrameVertexCount = this->statCurrentFrameVertexCount;
		this->statCurrentFrameVertexCount = 0;
		this->statLastFrameTriangleCount = this->statCurrentFrameTriangleCount;
		this->statCurrentFrameTriangleCount = 0;
		this->statLastFrameLineCount = this->statCurrentFrameLineCount;
		this->statCurrentFrameLineCount = 0;
	}

	void RenderSystem::presentFrame()
	{
		this->flushFrame();
		april::window->presentFrame();
	}

	unsigned int RenderSystem::_numPrimitives(RenderOperation renderOperation, int count)
	{
		switch (renderOperation)
		{
		case RO_TRIANGLE_LIST:	return count / 3;
		case RO_TRIANGLE_STRIP:	return count - 2;
		case RO_TRIANGLE_FAN:	return count - 2;
		case RO_LINE_LIST:		return count / 2;
		case RO_LINE_STRIP:		return count - 1;
		case RO_POINT_LIST:		return count;
		default:				break;
		}
		return 0;
	}
	
	unsigned int RenderSystem::_limitVertices(RenderOperation renderOperation, int count)
	{
		switch (renderOperation)
		{
		case RO_TRIANGLE_LIST:	return count / 3 * 3;
		case RO_TRIANGLE_STRIP:	return count;
		case RO_TRIANGLE_FAN:	return count;
		case RO_LINE_LIST:		return count / 2 * 2;
		case RO_LINE_STRIP:		return count;
		case RO_POINT_LIST:		return count;
		default:				break;
		}
		return count;
	}
	
	Texture* RenderSystem::getRenderTarget()
	{
		hlog::warnf(logTag, "Render targets are not implemented in render system '%s'!", this->name.cStr());
		return NULL;
	}

	void RenderSystem::setRenderTarget(Texture* texture)
	{
		hlog::warnf(logTag, "Render targets are not implemented in render system '%s'!", this->name.cStr());
	}

	void RenderSystem::setPixelShader(april::PixelShader* pixelShader)
	{
		hlog::warnf(logTag, "Pixel shaders are not implemented in render system '%s'!", this->name.cStr());
	}

	void RenderSystem::setVertexShader(april::VertexShader* vertexShader)
	{
		hlog::warnf(logTag, "Vertex shaders are not implemented in render system '%s'!", this->name.cStr());
	}

}
