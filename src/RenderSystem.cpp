/// @file
/// @version 5.0
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
#include "AsyncCommands.h"
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
	HL_ENUM_CLASS_DEFINE(RenderSystem::RenderMode,
	(
		HL_ENUM_DEFINE(RenderSystem::RenderMode, Normal);
		HL_ENUM_DEFINE(RenderSystem::RenderMode, Layered2D);
	));

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
		this->vSync = true;
		this->tripleBuffering = false;
		this->debugInfo = false;
	}

	RenderSystem::Options::~Options()
	{
	}

	RenderSystem::Caps::Caps()
	{
		this->maxTextureSize = 0;
		this->npotTexturesLimited = false;
		this->npotTextures = false;
		this->textureFormats = Image::Format::getValues();
	}

	RenderSystem::Caps::~Caps()
	{
	}

	hstr RenderSystem::Options::toString()
	{
		harray<hstr> options;
		if (this->depthBuffer)
		{
			options += "Depth-Buffer";
		}
		if (this->vSync)
		{
			options += "V-Sync";
		}
		if (this->debugInfo)
		{
			options += "Debug Info";
		}
		if (options.size() == 0)
		{
			options += "none";
		}
		return options.joined(',');
	}
	
	RenderSystem::RenderSystem() : renderMode(RenderMode::Normal), renderHelper(NULL)
	{
		this->name = "Generic";
		this->created = false;
		this->pixelOffset = 0.0f;
		this->state = new RenderState();
		this->deviceState = new RenderState();
		this->processingAsync = false;
		this->frameAdvanceUpdates = 1;
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
			this->created = true;
			this->_addAsyncCommand(new CreateCommand(options));
		}
		return this->created;
	}

	void RenderSystem::_systemCreate(Options options)
	{
		hlog::writef(logTag, "Creating rendersystem: '%s' (options: %s)", this->name.cStr(), options.toString().cStr());
		hmutex::ScopeLock lock(&this->renderMutex);
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
		if (!this->_deviceCreate(options))
		{
			this->created = false;
			this->_systemDestroy();
		}
	}

	bool RenderSystem::destroy()
	{
		if (this->created)
		{
			this->created = false;
			this->_addAsyncCommand(new DestroyCommand());
			return true;
		}
		return false;
	}

	void RenderSystem::_systemDestroy()
	{
		hlog::writef(logTag, "Destroying rendersystem '%s'.", this->name.cStr());
		hmutex::ScopeLock lock(&this->renderMutex);
		this->renderMode = RenderMode::Normal;
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
		foreach (AsyncCommandQueue*, it, this->asyncCommandQueues)
		{
			delete (*it);
		}
		this->asyncCommandQueues.clear();
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
		this->_deviceDestroy();
		this->_deviceInit();
	}
	
	void RenderSystem::assignWindow(Window* window)
	{
		this->_addAsyncCommand(new AssignWindowCommand(window));
	}

	void RenderSystem::_systemAssignWindow(Window* window)
	{
		hmutex::ScopeLock lock(&this->renderMutex);
		this->_deviceAssignWindow(window);
		this->_deviceSetupCaps();
		this->_deviceSetup();
		grect viewport(0.0f, 0.0f, april::window->getSize());
		this->setViewport(viewport);
		this->setOrthoProjection(viewport);
		this->_updateDeviceState(this->state, true);
		this->_deviceClear(true);
	}

	void RenderSystem::reset()
	{
		this->_addAsyncCommand(new ResetCommand(*this->state, april::window->getSize()));
	}

	void RenderSystem::_deviceReset()
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
	}

	void RenderSystem::suspend()
	{
		this->_addAsyncCommand(new SuspendCommand(*this->state));
	}

	void RenderSystem::_deviceSuspend()
	{
		hlog::write(logTag, "Suspending rendersystem.");
	}

	void RenderSystem::_deviceSetupDisplayModes()
	{
		gvec2 resolution = april::getSystemInfo().displayResolution;
		this->displayModes += RenderSystem::DisplayMode((int)resolution.x, (int)resolution.y, 60);
	}

	void RenderSystem::setRenderMode(RenderMode value, const hmap<hstr, hstr>& options)
	{
		if (this->renderMode != value)
		{
			this->renderMode = value;
			if (this->renderHelper != NULL)
			{
				this->renderHelper->destroy();
				delete this->renderHelper;
				this->renderHelper = NULL;
			}
			if (this->renderMode == RenderMode::Layered2D)
			{
				this->renderHelper = new RenderHelperLayered2D(options);
				this->renderHelper->create();
			}
		}
	}

	int RenderSystem::getAsyncQueuesCount()
	{
		hmutex::ScopeLock lock(&this->renderMutex);
		return hmax(this->asyncCommandQueues.size() - 1, 0);
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

	bool RenderSystem::hasAsyncTexturesQueued() const
	{
		return TextureAsync::isRunning();
	}

	grect RenderSystem::getViewport() const
	{
		return this->state->viewport;
	}

	void RenderSystem::setViewport(cgrect value)
	{
		this->state->viewport = value;
		this->state->viewportChanged = true;
	}

	gmat4 RenderSystem::getModelviewMatrix() const
	{
		return this->state->modelviewMatrix;
	}

	void RenderSystem::setModelviewMatrix(cgmat4 value)
	{
		this->state->modelviewMatrix = value;
		this->state->modelviewMatrixChanged = true;
	}

	gmat4 RenderSystem::getProjectionMatrix() const
	{
		return this->state->projectionMatrix;
	}

	void RenderSystem::setProjectionMatrix(cgmat4 value)
	{
		this->state->projectionMatrix = value;
		this->state->projectionMatrixChanged = true;
	}

	bool RenderSystem::update(float timeDelta)
	{
		bool result = false;
		hmutex::ScopeLock lock(&this->renderMutex);
		if (this->asyncCommandQueues.size() >= 2)
		{
			// TODOx - maybe the other block can take care of everything
			//if (timeDelta > 0.0f)
			{
				AsyncCommandQueue* queue = this->asyncCommandQueues.removeFirst();
				this->processingAsync = true;
				lock.release();
				foreach (AsyncCommand*, it, queue->commands)
				{
					(*it)->execute();
				}
				delete queue;
				lock.acquire(&this->renderMutex);
				this->processingAsync = false;
				lock.release();
				result = true;
			}
			/*
			else
			{
				harray<AsyncCommandQueue*> queues = this->asyncCommandQueues.removeFirst(this->asyncCommandQueues.size() - 1);
				this->processingAsync = true;
				lock.release();
				foreach (AsyncCommandQueue*, it, queues)
				{
					foreach (AsyncCommand*, it2, (*it)->commands)
					{
						(*it2)->execute();
					}
					delete (*it);
				}
				lock.acquire(&this->renderMutex);
				this->processingAsync = false;
				lock.release();
				result = true;
			}
			*/
		}
		return result;
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
		if (format != Image::Format::Invalid && !this->getCaps().textureFormats.has(format))
		{
			hlog::errorf(logTag, "Cannot create texture '%s', the texture format '%s' is not supported!", filename.cStr(), format.getName().cStr());
			return NULL;
		}
		hstr name = (fromResource ? this->findTextureResource(filename) : this->findTextureFile(filename));
		if (name == "")
		{
			return NULL;
		}
		Texture* texture = this->_deviceCreateTexture(fromResource);
		bool result = (format == Image::Format::Invalid ? texture->_create(name, type, loadMode) : texture->_create(name, format, type, loadMode));
		if (result)
		{
			if (loadMode == Texture::LoadMode::Immediate)
			{
				result = texture->load();
			}
			else if (loadMode == Texture::LoadMode::Async || loadMode == Texture::LoadMode::AsyncDeferredUpload)
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
		if (format != Image::Format::Invalid && !this->getCaps().textureFormats.has(format))
		{
#ifdef _WIN32
			hstr address = hsprintf("<0x%p>", data);
#else
			hstr address = hsprintf("<%p>", data); // on Unix %p adds the 0x
#endif
			hlog::errorf(logTag, "Cannot create texture with data %s, the texture format '%s' is not supported!", address.cStr(), format.getName().cStr());
			return NULL;
		}
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
		if (format != Image::Format::Invalid && !this->getCaps().textureFormats.has(format))
		{
			hlog::errorf(logTag, "Cannot create texture with color '%s', the texture format '%s' is not supported!", color.hex().cStr(), format.getName().cStr());
			return NULL;
		}
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
		if (texture == NULL)
		{
			throw Exception("Cannot call destroyTexture(), texture is NULL!");
		}
		if (this->renderHelper != NULL)
		{
			this->renderHelper->flush();
		}
		hmutex::ScopeLock lock(&this->texturesMutex);
		this->textures -= texture;
		lock.release();
		this->_addAsyncCommand(new DestroyTextureCommand(texture));
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

	grect RenderSystem::getOrthoProjection() const
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

	void RenderSystem::setOrthoProjection(cgrect rect)
	{
		this->state->projectionMatrix.setOrthoProjection(rect - rect.getSize() * this->pixelOffset / april::window->getSize());
		this->state->projectionMatrixChanged = true;
	}

	void RenderSystem::setOrthoProjection(cgrect rect, float nearZ, float farZ)
	{
		this->state->projectionMatrix.setOrthoProjection(rect - rect.getSize() * this->pixelOffset / april::window->getSize(), nearZ, farZ);
		this->state->projectionMatrixChanged = true;
	}

	void RenderSystem::setOrthoProjection(cgvec2 size)
	{
		this->setOrthoProjection(grect(0.0f, 0.0f, size));
	}

	void RenderSystem::setOrthoProjection(cgvec2 size, float nearZ, float farZ)
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

	void RenderSystem::setBlendMode(const BlendMode& blendMode)
	{
		this->state->blendMode = blendMode;
	}

	void RenderSystem::setColorMode(const ColorMode& colorMode, float colorModeFactor)
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
	
	void RenderSystem::translate(cgvec3 vector)
	{
		this->state->modelviewMatrix.translate(vector);
		this->state->modelviewMatrixChanged = true;
	}

	void RenderSystem::translate(cgvec2 vector)
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
	
	void RenderSystem::rotate(cgvec3 axis, float angle)
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
	
	void RenderSystem::scale(cgvec3 vector)
	{
		this->state->modelviewMatrix.scale(vector);
		this->state->modelviewMatrixChanged = true;
	}

	void RenderSystem::scale(cgvec2 vector)
	{
		this->state->modelviewMatrix.scale(vector.x, vector.y, 1.0f);
		this->state->modelviewMatrixChanged = true;
	}

	void RenderSystem::lookAt(cgvec3 eye, cgvec3 target, cgvec3 up)
	{
		this->state->modelviewMatrix.lookAt(eye, target, up);
		this->state->modelviewMatrixChanged = true;
	}
		
	void RenderSystem::setPerspective(float fov, float aspect, float nearZ, float farZ)
	{
		this->state->projectionMatrix.setPerspective(fov, aspect, nearZ, farZ);
		this->state->projectionMatrixChanged = true;
	}

	void RenderSystem::_updateDeviceState(RenderState* state, bool forceUpdate)
	{
		// viewport
		if (forceUpdate || state->viewportChanged)
		{
			if (forceUpdate || this->deviceState->viewport != state->viewport)
			{
				this->_setDeviceViewport(state->viewport);
				this->deviceState->viewport = state->viewport;
			}
			state->viewportChanged = false;
		}
		// modelview matrix
		if (forceUpdate || state->modelviewMatrixChanged)
		{
			if (forceUpdate || this->deviceState->modelviewMatrix != state->modelviewMatrix)
			{
				this->_setDeviceModelviewMatrix(state->modelviewMatrix);
				this->deviceState->modelviewMatrix = state->modelviewMatrix;
			}
			state->modelviewMatrixChanged = false;
		}
		// projection matrix
		if (forceUpdate || state->projectionMatrixChanged)
		{
			if (forceUpdate || this->deviceState->projectionMatrix != state->projectionMatrix)
			{
				this->_setDeviceProjectionMatrix(state->projectionMatrix);
				this->deviceState->projectionMatrix = state->projectionMatrix;
			}
			state->projectionMatrixChanged = false;
		}
		// depth buffer
		if (forceUpdate || this->deviceState->depthBuffer != state->depthBuffer || this->deviceState->depthBufferWrite != state->depthBufferWrite)
		{
			this->_setDeviceDepthBuffer(state->depthBuffer, state->depthBufferWrite);
			this->deviceState->depthBuffer = state->depthBuffer;
			this->deviceState->depthBufferWrite = state->depthBufferWrite;
		}
		// device render mode
		if (forceUpdate || this->deviceState->useTexture != state->useTexture || this->deviceState->useColor != state->useColor)
		{
			this->_setDeviceRenderMode(state->useTexture, state->useColor);
		}
		// texture
		if (forceUpdate || this->deviceState->texture != state->texture || this->deviceState->useTexture != state->useTexture)
		{
			// filtering and wrapping applied before loading texture data, some systems are optimized to work like this (e.g. iOS OpenGLES guidelines suggest it)
			if (state->texture != NULL && state->useTexture)
			{
				++this->statCurrentFrameTextureSwitches;
				state->texture->loadAsync();
				state->texture->_ensureInternalLoaded();
				state->texture->upload();
				this->_setDeviceTexture(state->texture);
				this->_setDeviceTextureFilter(state->texture->getFilter());
				this->_setDeviceTextureAddressMode(state->texture->getAddressMode());
			}
			else
			{
				this->_setDeviceTexture(NULL);
			}
			this->deviceState->texture = state->texture;
		}
		// blend mode
		if (forceUpdate || this->deviceState->blendMode != state->blendMode)
		{
			this->_setDeviceBlendMode(state->blendMode);
			this->deviceState->blendMode = state->blendMode;
		}
		// color mode
		if (forceUpdate || this->deviceState->colorMode != state->colorMode || this->deviceState->colorModeFactor != state->colorModeFactor ||
			this->deviceState->useTexture != state->useTexture || this->deviceState->useColor != state->useColor ||
			this->deviceState->systemColor != state->systemColor)
		{
			this->_setDeviceColorMode(state->colorMode, state->colorModeFactor, state->useTexture, state->useColor, state->systemColor);
			this->deviceState->colorMode = state->colorMode;
			this->deviceState->colorModeFactor = state->colorModeFactor;
			this->deviceState->useColor = state->useColor;
			this->deviceState->systemColor = state->systemColor;
		}
		// shared variables
		this->deviceState->useTexture = state->useTexture;
		this->deviceState->useColor = state->useColor;
	}

	void RenderSystem::_addAsyncCommand(AsyncCommand* command)
	{
		if (command->isUseState())
		{
			this->state->viewportChanged = false;
			this->state->modelviewMatrixChanged = false;
			this->state->projectionMatrixChanged = false;
		}
		hmutex::ScopeLock lock(&this->renderMutex);
		if (this->asyncCommandQueues.size() == 0)
		{
			this->asyncCommandQueues += new AsyncCommandQueue();
		}
		this->asyncCommandQueues.last()->commands += command;
		if (command->isFinalizer())
		{
			this->asyncCommandQueues += new AsyncCommandQueue();
		}
	}

	void RenderSystem::_flushAsyncCommands()
	{
		hmutex::ScopeLock lock(&this->renderMutex);
		harray<AsyncCommandQueue*> queues = this->asyncCommandQueues;
		this->asyncCommandQueues.clear();
		lock.release();
		foreach (AsyncCommandQueue*, it, queues)
		{
			foreach (AsyncCommand*, it2, (*it)->commands)
			{
				if ((*it2)->isSystemCommand())
				{
					(*it2)->execute();
				}
			}
			delete (*it);
		}
	}

	void RenderSystem::waitForAsyncCommands(bool forced)
	{
		hmutex::ScopeLock lock(&this->renderMutex);
		if (forced && this->asyncCommandQueues.size() == 1 && this->asyncCommandQueues.first()->commands.size() > 0)
		{
			this->asyncCommandQueues += new AsyncCommandQueue();
		}
		while (this->asyncCommandQueues.size() > 1 || this->processingAsync)
		{
			lock.release();
			hthread::sleep(0.001f);
			lock.acquire(&this->renderMutex);
		}
	}

	void RenderSystem::clear(bool depth)
	{
		if (this->renderHelper != NULL)
		{
			this->renderHelper->clear();
			return;
		}
		if (!this->options.depthBuffer)
		{
			depth = false;
		}
		this->_addAsyncCommand(new ClearCommand(*this->state, depth));
	}

	void RenderSystem::clear(Color color, bool depth)
	{
		if (!this->options.depthBuffer)
		{
			depth = false;
		}
		this->_addAsyncCommand(new ClearColorCommand(*this->state, color, depth));
	}

	void RenderSystem::clearDepth()
	{
		if (this->options.depthBuffer)
		{
			this->_addAsyncCommand(new ClearDepthCommand(*this->state));
		}
	}

	void RenderSystem::render(const RenderOperation& renderOperation, const PlainVertex* vertices, int count)
	{
		if (this->renderHelper == NULL || !this->renderHelper->render(renderOperation, vertices, count))
		{
			this->_renderInternal(renderOperation, vertices, count);
		}
	}

	void RenderSystem::render(const RenderOperation& renderOperation, const PlainVertex* vertices, int count, const Color& color)
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

	void RenderSystem::render(const RenderOperation& renderOperation, const TexturedVertex* vertices, int count)
	{
		if (this->renderHelper == NULL || !this->renderHelper->render(renderOperation, vertices, count))
		{
			this->_renderInternal(renderOperation, vertices, count);
		}
	}

	void RenderSystem::render(const RenderOperation& renderOperation, const TexturedVertex* vertices, int count, const Color& color)
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

	void RenderSystem::render(const RenderOperation& renderOperation, const ColoredVertex* vertices, int count)
	{
		if (this->renderHelper == NULL || !this->renderHelper->render(renderOperation, vertices, count))
		{
			this->_renderInternal(renderOperation, vertices, count);
		}
	}

	void RenderSystem::render(const RenderOperation& renderOperation, const ColoredTexturedVertex* vertices, int count)
	{
		if (this->renderHelper == NULL || !this->renderHelper->render(renderOperation, vertices, count))
		{
			this->_renderInternal(renderOperation, vertices, count);
		}
	}

	void RenderSystem::drawRect(cgrect rect, const Color& color)
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

	void RenderSystem::drawFilledRect(cgrect rect, const Color& color)
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

	void RenderSystem::drawTexturedRect(cgrect rect, cgrect src)
	{
		if (this->renderHelper == NULL || !this->renderHelper->drawTexturedRect(rect, src))
		{
			this->_drawTexturedRectInternal(rect, src);
		}
	}

	void RenderSystem::drawTexturedRect(cgrect rect, cgrect src, const Color& color)
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

	void RenderSystem::_renderInternal(const RenderOperation& renderOperation, const PlainVertex* vertices, int count)
	{
		this->_increaseStats(renderOperation, count);
		this->state->useTexture = false;
		this->state->useColor = false;
		this->state->systemColor = Color::White;
		this->_addAsyncCommand(new VertexRenderCommand<PlainVertex>(*this->state, renderOperation, vertices, count));
	}

	void RenderSystem::_renderInternal(const RenderOperation& renderOperation, const PlainVertex* vertices, int count, const Color& color)
	{
		if (color.a == 0)
		{
			return;
		}
		this->_increaseStats(renderOperation, count);
		this->state->useTexture = false;
		this->state->useColor = false;
		this->state->systemColor = color;
		this->_addAsyncCommand(new VertexRenderCommand<PlainVertex>(*this->state, renderOperation, vertices, count));
	}

	void RenderSystem::_renderInternal(const RenderOperation& renderOperation, const TexturedVertex* vertices, int count)
	{
		this->_increaseStats(renderOperation, count);
		this->state->useTexture = true;
		this->state->useColor = false;
		this->state->systemColor = Color::White;
		this->_addAsyncCommand(new VertexRenderCommand<TexturedVertex>(*this->state, renderOperation, vertices, count));
	}

	void RenderSystem::_renderInternal(const RenderOperation& renderOperation, const TexturedVertex* vertices, int count, const Color& color)
	{
		if (color.a == 0)
		{
			return;
		}
		this->_increaseStats(renderOperation, count);
		this->state->useTexture = true;
		this->state->useColor = false;
		this->state->systemColor = color;
		this->_addAsyncCommand(new VertexRenderCommand<TexturedVertex>(*this->state, renderOperation, vertices, count));
	}

	void RenderSystem::_renderInternal(const RenderOperation& renderOperation, const ColoredVertex* vertices, int count)
	{
		this->_increaseStats(renderOperation, count);
		this->state->useTexture = false;
		this->state->useColor = true;
		this->state->systemColor = Color::White;
		this->_addAsyncCommand(new VertexRenderCommand<ColoredVertex>(*this->state, renderOperation, vertices, count));
	}

	void RenderSystem::_renderInternal(const RenderOperation& renderOperation, const ColoredTexturedVertex* vertices, int count)
	{
		this->_increaseStats(renderOperation, count);
		this->state->useTexture = true;
		this->state->useColor = true;
		this->state->systemColor = Color::White;
		this->_addAsyncCommand(new VertexRenderCommand<ColoredTexturedVertex>(*this->state, renderOperation, vertices, count));
	}

	void RenderSystem::_drawRectInternal(cgrect rect, const Color& color)
	{
		if (color.a == 0)
		{
			return;
		}
		pv[0].x = pv[3].x = pv[4].x = rect.x;
		pv[0].y = pv[1].y = pv[4].y = rect.y;
		pv[1].x = pv[2].x = rect.x + rect.w;
		pv[2].y = pv[3].y = rect.y + rect.h;
		this->_renderInternal(RenderOperation::LineStrip, pv, 5, color);
	}

	void RenderSystem::_drawFilledRectInternal(cgrect rect, const Color& color)
	{
		if (color.a == 0)
		{
			return;
		}
		pv[0].x = pv[2].x = rect.x;
		pv[0].y = pv[1].y = rect.y;
		pv[1].x = pv[3].x = rect.x + rect.w;
		pv[2].y = pv[3].y = rect.y + rect.h;
		this->_renderInternal(RenderOperation::TriangleStrip, pv, 4, color);
	}
	
	void RenderSystem::_drawTexturedRectInternal(cgrect rect, cgrect src)
	{
		tv[0].x = tv[2].x = rect.x;
		tv[0].y = tv[1].y = rect.y;
		tv[0].u = tv[2].u = src.x;
		tv[0].v = tv[1].v = src.y;
		tv[1].x = tv[3].x = rect.x + rect.w;
		tv[1].u = tv[3].u = src.x + src.w;
		tv[2].y = tv[3].y = rect.y + rect.h;
		tv[2].v = tv[3].v = src.y + src.h;
		this->_renderInternal(RenderOperation::TriangleStrip, tv, 4);
	}
	
	void RenderSystem::_drawTexturedRectInternal(cgrect rect, cgrect src, const Color& color)
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
		this->_renderInternal(RenderOperation::TriangleStrip, tv, 4, color);
	}

	void RenderSystem::_increaseStats(const RenderOperation& renderOperation, int count)
	{
		++this->statCurrentFrameRenderCalls;
		this->statCurrentFrameVertexCount += count;
		if (renderOperation.isTriangle())
		{
			this->statCurrentFrameTriangleCount += this->_numPrimitives(renderOperation, count);
		}
		else if (renderOperation.isLine())
		{
			this->statCurrentFrameLineCount += this->_numPrimitives(renderOperation, count);
		}
	}

	hstr RenderSystem::findTextureResource(chstr filename) const
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
	
	hstr RenderSystem::findTextureFile(chstr filename) const
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

	void RenderSystem::waitForAsyncTextures(float timeout) const
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

	void RenderSystem::flushFrame(bool updateStats)
	{
		if (this->renderHelper != NULL)
		{
			this->renderHelper->flush();
		}
		if (updateStats)
		{
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
	}

	void RenderSystem::presentFrame()
	{
		this->_addAsyncCommand(new PresentFrameCommand(*this->state));
	}

	void RenderSystem::_devicePresentFrame()
	{
		this->flushFrame(true);
		april::window->presentFrame();
	}

	unsigned int RenderSystem::_numPrimitives(const RenderOperation& renderOperation, int count) const
	{
		if (renderOperation == RenderOperation::TriangleList)	return count / 3;
		if (renderOperation == RenderOperation::TriangleStrip)	return count - 2;
		if (renderOperation == RenderOperation::LineList)		return count / 2;
		if (renderOperation == RenderOperation::LineStrip)		return count - 1;
		if (renderOperation == RenderOperation::PointList)		return count;
		return 0;
	}
	
	unsigned int RenderSystem::_limitVertices(const RenderOperation& renderOperation, int count) const
	{
		if (renderOperation == RenderOperation::TriangleList)	return count / 3 * 3;
		if (renderOperation == RenderOperation::TriangleStrip)	return count;
		if (renderOperation == RenderOperation::LineList)		return count / 2 * 2;
		if (renderOperation == RenderOperation::LineStrip)		return count;
		if (renderOperation == RenderOperation::PointList)		return count;
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
