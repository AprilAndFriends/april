/// @file
/// @version 5.2
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include <hltypes/harray.h>
#include <hltypes/hfile.h>
#include <hltypes/hlog.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hresource.h>
#include <hltypes/hstring.h>

#include "april.h"
#include "Color.h"
#include "Image.h"
#include "RenderSystem.h"
#include "Texture.h"
#include "TextureAsync.h"
#include "UnloadTextureCommand.h"

namespace april
{
	HL_ENUM_CLASS_DEFINE(Texture::Type,
	(
		HL_ENUM_DEFINE(Texture::Type, Immutable);
		HL_ENUM_DEFINE(Texture::Type, Managed);
		HL_ENUM_DEFINE(Texture::Type, External);
		HL_ENUM_DEFINE(Texture::Type, RenderTarget);
	));

	HL_ENUM_CLASS_DEFINE(Texture::Filter,
	(
		HL_ENUM_DEFINE(Texture::Filter, Nearest);
		HL_ENUM_DEFINE(Texture::Filter, Linear);
	));

	HL_ENUM_CLASS_DEFINE(Texture::AddressMode,
	(
		HL_ENUM_DEFINE(Texture::AddressMode, Clamp);
		HL_ENUM_DEFINE(Texture::AddressMode, Wrap);
	));

	HL_ENUM_CLASS_DEFINE(Texture::LoadMode,
	(
		HL_ENUM_DEFINE(Texture::LoadMode, Async);
		HL_ENUM_DEFINE(Texture::LoadMode, AsyncDeferredUpload);
		HL_ENUM_DEFINE(Texture::LoadMode, OnDemand);
	));

	Texture::Lock::Lock()
	{
		this->systemBuffer = NULL;
		this->x = 0;
		this->y = 0;
		this->w = 0;
		this->h = 0;
		this->dx = 0;
		this->dy = 0;
		this->data = NULL;
		this->dataWidth = 0;
		this->dataHeight = 0;
		this->format = Image::Format::Invalid;
		this->locked = false;
		this->failed = true;
		this->renderTarget = false;
	}

	Texture::Lock::~Lock()
	{
	}

	void Texture::Lock::activateFail()
	{
		this->locked = false;
		this->failed = true;
		this->renderTarget = false;
	}

	void Texture::Lock::activateLock(int x, int y, int w, int h, int dx, int dy, unsigned char* data, int dataWidth, int dataHeight, Image::Format format)
	{
		this->x = x;
		this->y = y;
		this->w = w;
		this->h = h;
		this->dx = dx;
		this->dy = dy;
		this->data = data;
		this->dataWidth = dataWidth;
		this->dataHeight = dataHeight;
		this->format = format;
		this->locked = true;
		this->failed = false;
		this->renderTarget = false;
	}

	Texture::Texture(bool fromResource)
	{
		this->filename = "";
		this->type = Type::Immutable;
		this->uploaded = false;
		this->loadMode = LoadMode::Async;
		this->format = Image::Format::Invalid;
		this->dataFormat = 0;
		this->width = 0;
		this->height = 0;
		this->effectiveWidth = 1.0f; // used only with software NPOT textures
		this->effectiveHeight = 1.0f; // used only with software NPOT textures
		this->compressedSize = 0; // used in compressed textures only
		this->filter = Filter::Linear;
		this->addressMode = AddressMode::Clamp;
		this->locked = false;
		this->dirty = false;
		this->data = NULL;
		this->dataAsync = NULL;
		this->asyncLoadQueued = false;
		this->asyncLoadDiscarded = false;
		this->fromResource = fromResource;
		this->firstUpload = true;
	}

	bool Texture::_create(chstr filename, Texture::Type type, Texture::LoadMode loadMode)
	{
		this->filename = filename;
		this->type = type;
		this->loadMode = loadMode;
		hlog::write(logTag, "Registering texture: " + this->_getInternalName());
		return true;
	}

	bool Texture::_create(chstr filename, Image::Format format, Texture::Type type, Texture::LoadMode loadMode)
	{
		this->filename = filename;
		this->type = type;
		this->loadMode = loadMode;
		this->format = format;
		hlog::write(logTag, "Registering texture: " + this->_getInternalName());
		return true;
	}

	bool Texture::_create(int width, int height, unsigned char* data, Image::Format format, Type type)
	{
		if (width <= 0 || height <= 0)
		{
			hlog::errorf(logTag, "Cannot create texture with dimensions %d,%d!", width, height);
			return false;
		}
		this->width = width;
		this->height = height;
		this->type = type;
		this->format = format;
		this->data = new unsigned char[this->getByteSize()];
		hlog::write(logTag, "Registering manual texture: " + this->_getInternalName()); // print here because type and format need to be assigned first
		Image::write(0, 0, this->width, this->height, 0, 0, data, this->width, this->height, format, this->data, this->width, this->height, this->format);
		this->_checkMaxTextureSize();
		this->_assignFormat();
		return true;
	}

	bool Texture::_create(int width, int height, const Color& color, Image::Format format, Type type)
	{
		if (width <= 0 || height <= 0)
		{
			hlog::errorf(logTag, "Cannot create texture with dimensions %d,%d!", width, height);
			return false;
		}
		this->width = width;
		this->height = height;
		this->type = type;
		this->format = format;
		this->data = new unsigned char[this->getByteSize()];
		hlog::write(logTag, "Registering manual texture: " + this->_getInternalName()); // print here because type and format need to be assigned first
		Image::fillRect(0, 0, this->width, this->height, color, this->data, this->width, this->height, this->format);
		this->_checkMaxTextureSize();
		this->_assignFormat();
		return true;
	}

	bool Texture::_createRenderTarget(int width, int height)
	{
		if (width <= 0 || height <= 0)
		{
			hlog::errorf(logTag, "Cannot create texture with dimensions %d,%d!", width, height);
			return false;
		}
		this->width = width;
		this->height = height;
		this->type = Type::RenderTarget;
		this->format = april::rendersys->getNativeTextureFormat(Image::Format::RGBX);
		hlog::write(logTag, "Registering manual texture: " + this->_getInternalName()); // print here because type and format need to be assigned first
		this->_checkMaxTextureSize();
		this->_assignFormat();
		return true;
	}

	void Texture::_checkMaxTextureSize()
	{
		int maxTextureSize = april::rendersys->getCaps().maxTextureSize;
		if (maxTextureSize > 0 && (this->width > maxTextureSize || this->height > maxTextureSize))
		{
			hlog::warnf(logTag, "Texture size for '%s' is %d,%d while the reported system max texture size is %d!", this->_getInternalName().cStr(), this->width, this->height, maxTextureSize);
		}
	}

	Texture::~Texture()
	{
		// this mutex order must remain the same, because of the order of locking in other places
		hmutex::ScopeLock lock(&this->asyncLoadMutex);
		this->asyncLoadQueued = false;
		this->asyncLoadDiscarded = false;
		if (this->dataAsync != NULL)
		{
			delete[] this->dataAsync;
		}
		hmutex::ScopeLock lockData(&this->asyncDataMutex);
		if (this->data != NULL)
		{
			delete[] this->data;
		}
	}

	void Texture::_deviceUnloadTexture()
	{
		this->_deviceDestroyTexture();
		hmutex::ScopeLock lock(&this->asyncLoadMutex);
		this->uploaded = false;
		if (this->asyncLoadQueued)
		{
			this->asyncLoadDiscarded = true;
		}
		if (this->dataAsync != NULL)
		{
			delete[] this->dataAsync;
			this->dataAsync = NULL;
		}
		this->firstUpload = true;
	}

	int Texture::getWidth() const
	{
		if (this->width == 0)
		{
			hlog::warnf(logTag, "Texture '%s' has width = 0 (possibly not loaded yet?)", this->filename.cStr());
		}
		return this->width;
	}

	int Texture::getHeight() const
	{
		if (this->height == 0)
		{
			hlog::warnf(logTag, "Texture '%s' has height = 0 (possibly not loaded yet?)", this->filename.cStr());
		}
		return this->height;
	}

	int Texture::getBpp() const
	{
		if (this->format == Image::Format::Invalid)
		{
			hlog::warnf(logTag, "Texture '%s' has bpp = 0 (possibly not loaded yet?)", this->filename.cStr());
		}
		return this->format.getBpp();
	}

	int Texture::getByteSize() const
	{
		if (this->width == 0 || this->height == 0 || this->format == Image::Format::Invalid)
		{
			hlog::warnf(logTag, "Texture '%s' has byteSize = 0 (possibly not loaded yet?)", this->filename.cStr());
		}
		return this->_getByteSize();
	}

	int Texture::_getByteSize() const
	{
		if (this->compressedSize > 0)
		{
			return this->compressedSize;
		}
		return (this->width * this->height * this->format.getBpp());
	}

	int Texture::getCurrentAllRamSize()
	{
		int byteSize = this->_getByteSize();
		int result = 0;
		hmutex::ScopeLock lock(&this->asyncLoadMutex);
		// VRAM
		if (this->width > 0 && this->height > 0 && this->format != Image::Format::Invalid && this->uploaded)
		{
			result += byteSize;
		}
		// async RAM
		bool asyncRamUsed = (!this->asyncLoadQueued && this->dataAsync != NULL && !this->uploaded);
		lock.release();
		if (asyncRamUsed && this->width > 0 && this->height > 0 && this->format != Image::Format::Invalid)
		{
			result += byteSize;
		}
		// RAM
		if (this->type != Type::Immutable && this->type != Type::RenderTarget)
		{
			result += byteSize;
		}
		return result;
	}

	int Texture::getCurrentVRamSize()
	{
		hmutex::ScopeLock lock(&this->asyncLoadMutex);
		if (this->width == 0 || this->height == 0 || this->format == Image::Format::Invalid || !this->uploaded)
		{
			return 0;
		}
		lock.release();
		if (this->compressedSize > 0)
		{
			return this->compressedSize;
		}
		return (this->width * this->height * this->format.getBpp());
	}

	int Texture::getCurrentRamSize()
	{
		if (this->type == Type::Immutable || this->type == Type::RenderTarget)
		{
			return 0;
		}
		if (this->compressedSize > 0)
		{
			return this->compressedSize;
		}
		return (this->width * this->height * this->format.getBpp());
	}

	int Texture::getCurrentAsyncRamSize()
	{
		hmutex::ScopeLock lock(&this->asyncLoadMutex);
		if (this->asyncLoadQueued || this->dataAsync == NULL || this->uploaded)
		{
			return 0;
		}
		lock.release();
		if (this->width == 0 || this->height == 0 || this->format == Image::Format::Invalid)
		{
			return 0;
		}
		if (this->compressedSize > 0)
		{
			return this->compressedSize;
		}
		return (this->width * this->height * this->format.getBpp());
	}

	bool Texture::isUploaded()
	{
		hmutex::ScopeLock lock(&this->asyncLoadMutex);
		return this->uploaded;
	}

	bool Texture::isReadyForUpload()
	{
		hmutex::ScopeLock lock(&this->asyncLoadMutex);
		return (!this->asyncLoadQueued && (this->filename == "" || this->dataAsync != NULL) && !this->uploaded);
	}

	bool Texture::isAsyncLoadQueued()
	{
		hmutex::ScopeLock lock(&this->asyncLoadMutex);
		return this->asyncLoadQueued;
	}

	bool Texture::isUnloaded()
	{
		hmutex::ScopeLock lock(&this->asyncLoadMutex);
		return (!this->uploaded && !this->asyncLoadQueued && this->dataAsync == NULL);
	}

	bool Texture::_isReadable() const
	{
		return (this->type == Type::Managed);
	}

	bool Texture::_isWritable() const
	{
		return (this->type == Type::Managed);
	}

	bool Texture::_isAlterable() const
	{
		return (this->_isReadable() && this->_isWritable());
	}

	hstr Texture::_getInternalName() const
	{
		hstr result;
		if (this->name != "")
		{
			result += "'" + this->name + "'";
		}
		else if (this->filename != "")
		{
			result += "'" + this->filename + "'";
		}
		else
		{
#if defined(_WIN32) && !defined(_UWP)
			result += hsprintf("<0x%p>", this); // only basic Win32 doesn't add 0x to %p
#else
			result += hsprintf("<%p>", this);
#endif
		}
		result += " (" + this->type.getName() + ")";
		return result;
	}
	
	bool Texture::isAsyncUploadQueued()
	{
		if (this->loadMode == Texture::LoadMode::AsyncDeferredUpload)
		{
			return false;
		}
		hmutex::ScopeLock lock(&this->asyncLoadMutex);
		return (!this->asyncLoadQueued && (this->filename == "" || this->dataAsync != NULL) && !this->uploaded);
	}

	bool Texture::_isAsyncUploadQueued()
	{
		if (this->loadMode == Texture::LoadMode::AsyncDeferredUpload)
		{
			return false;
		}
		return (!this->asyncLoadQueued && (this->filename == "" || this->dataAsync != NULL) && !this->uploaded);
	}

	bool Texture::loadAsync()
	{
		hmutex::ScopeLock lock(&this->asyncLoadMutex);
		return this->_loadAsync();
	}

	bool Texture::_loadAsync()
	{
		if (this->dataAsync != NULL || this->uploaded || this->filename == "")
		{
			return false;
		}
		this->_loadMetaData();
		this->asyncLoadDiscarded = false;
		if (!this->asyncLoadQueued && this->type != Type::RenderTarget)
		{
			this->asyncLoadQueued = TextureAsync::queueLoad(this);
		}
		return this->asyncLoadQueued;
	}

	bool Texture::loadMetaData()
	{
		hmutex::ScopeLock lock(&this->asyncLoadMutex);
		if (this->uploaded || this->dataAsync != NULL)
		{
			return true;
		}
		hmutex::ScopeLock lockData(&this->asyncDataMutex);
		if (this->data != NULL)
		{
			return true;
		}
		lockData.release();
		return this->_loadMetaData();
	}

	bool Texture::_loadMetaData()
	{
		if (this->width <= 0 || this->height <= 0)
		{
			if (this->filename == "")
			{
				hlog::error(logTag, "No filename for texture specified!");
				return false;
			}
			Image* image = (this->fromResource ? Image::readMetaDataFromResource(this->filename) : Image::readMetaDataFromFile(this->filename));
			if (image != NULL)
			{
				image = this->_processImageFormatSupport(image);
			}
			if (image == NULL)
			{
				hlog::error(logTag, "Failed to load texture: " + this->_getInternalName());
				return false;
			}
			this->width = image->w;
			this->height = image->h;
			this->format = image->format;
			this->dataFormat = image->internalFormat;
			if (this->dataFormat != 0)
			{
				this->compressedSize = image->compressedSize;
			}
			delete image;
		}
		return true;
	}

	bool Texture::_tryAsyncFinalUpload()
	{
		if (!april::rendersys->canUseLowLevelCalls())
		{
			return false;
		}
		hmutex::ScopeLock lock(&this->asyncLoadMutex);
		if (!this->_isAsyncUploadQueued())
		{
			return false;
		}
		this->_upload(lock);
		return true;
	}

	bool Texture::_upload(hmutex::ScopeLock& lock)
	{
		if (!april::rendersys->canUseLowLevelCalls())
		{
			return false;
		}
		this->asyncLoadDiscarded = false; // a possible previous unload call must be canceled
		int size = 0;
		unsigned char* currentData = NULL;
		// no lock required since it only checks for existence, not for manipulation of data
		if (this->data != NULL) // reload from memory
		{
			hlog::write(logTag, "Uploading texture: " + this->_getInternalName());
			currentData = this->data;
			size = this->getByteSize();
		}
		else if (this->dataAsync != NULL) // load from asynchronically loaded data
		{
			hlog::write(logTag, "Uploading async texture: " + this->_getInternalName());
			currentData = this->dataAsync;
			size = this->getByteSize();
		}
		else
		{
			hlog::write(logTag, "Uploading texture: " + this->_getInternalName());
		}
		lock.release();
		// if no cached data was previously loaded
		if (currentData == NULL && this->type != Type::RenderTarget)
		{
			if (this->filename == "")
			{
				hlog::error(logTag, "No filename for texture specified!");
				return false;
			}
			// must not call createFromResource() or createFromFile() that converts automatically, because _processImageFormatSupport() needs to be called first
			Image* image = (this->fromResource ? Image::createFromResource(this->filename) : Image::createFromFile(this->filename));
			if (image != NULL)
			{
				image = this->_processImageFormatSupport(image);
			}
			if (image != NULL && this->format != Image::Format::Invalid && Image::needsConversion(image->format, this->format))
			{
				unsigned char* data = NULL;
				if (Image::convertToFormat(image->w, image->h, image->data, image->format, &data, this->format))
				{
					delete[] image->data;
					image->format = this->format;
					image->data = data;
				}
			}
			if (image == NULL)
			{
				hlog::error(logTag, "Failed to load texture: " + this->_getInternalName());
				return false;
			}
			this->width = image->w;
			this->height = image->h;
			this->format = image->format;
			this->dataFormat = image->internalFormat;
			if (this->dataFormat != 0)
			{
				size = image->compressedSize;
				this->compressedSize = size;
			}
			int maxTextureSize = april::rendersys->getCaps().maxTextureSize;
			if (maxTextureSize > 0 && (this->width > maxTextureSize || this->height > maxTextureSize))
			{
				hlog::warnf(logTag, "Texture size for '%s' is %d,%d while the reported system max texture size is %d!", this->_getInternalName().cStr(), this->width, this->height, maxTextureSize);
			}
			currentData = image->data;
			image->data = NULL;
			delete image;
		}
		this->_assignFormat();
		bool result = this->_deviceCreateTexture(currentData, size);
		if (!result)
		{
			lock.acquire(&this->asyncDataMutex);
			if (currentData != NULL && this->data != currentData)
			{
				delete[] currentData;
			}
			lock.release();
			lock.acquire(&this->asyncLoadMutex);
			this->dataAsync = NULL; // not needed anymore and makes isReadyForUpload() return false now
			this->uploaded = result;
			return false;
		}
		if (currentData != NULL && this->type != Type::RenderTarget)
		{
			lock.acquire(&this->asyncDataMutex);
			this->dirty = false;
			if (this->firstUpload)
			{
				if (Image::needsConversion(this->format, april::rendersys->getNativeTextureFormat(this->format)) ||
					!this->_uploadToGpu(0, 0, this->width, this->height, 0, 0, currentData, this->width, this->height, format))
				{
					Lock lock = this->_tryLockSystem(0, 0, this->width, this->height);
					if (!lock.failed)
					{
						Image::write(0, 0, this->width, this->height, 0, 0, currentData, this->width, this->height, format, lock.data, lock.dataWidth, lock.dataHeight, lock.format);
						this->_unlockSystem(lock, true);
					}
				}
			}
			if (this->type != Type::Immutable || this->filename == "")
			{
				if (this->data != currentData)
				{
					if (this->data != NULL)
					{
						delete[] this->data;
					}
					this->data = currentData;
				}
				lock.release();
			}
			else
			{
				lock.release();
				delete[] currentData;
				// the used format will be the native format, because there is no intermediate data
				this->format = april::rendersys->getNativeTextureFormat(this->format);
			}
		}
		this->_tryUploadDataToGpu(); // upload any additional changes
		lock.acquire(&this->asyncLoadMutex);
		this->dataAsync = NULL; // not needed anymore and makes isReadyForUpload() return false now
		this->uploaded = result;
		return true;
	}

	void Texture::unload()
	{
		if (!this->isUnloaded())
		{
			hlog::write(logTag, "Unloading texture: " + this->_getInternalName());
		}
		april::rendersys->_addUnloadTextureCommand(new UnloadTextureCommand(this));
	}

	void Texture::waitForAsyncLoad(float timeout)
	{
		TextureAsync::prioritizeLoad(this);
		float time = timeout;
		hmutex::ScopeLock lock;
		while (time > 0.0f || timeout <= 0.0f)
		{
			lock.acquire(&this->asyncLoadMutex);
			if (!this->asyncLoadQueued && this->uploaded)
			{
				break;
			}
			lock.release();
			hthread::sleep(0.1f);
			time -= 0.0001f;
		}
	}

	void Texture::_ensureAsyncCompleted()
	{
		hmutex::ScopeLock lock(&this->asyncLoadMutex);
		if (!this->asyncLoadQueued || this->asyncLoadDiscarded || this->filename == "" || this->uploaded)
		{
			return;
		}
		lock.release();
		TextureAsync::prioritizeLoad(this);
		while (true)
		{
			TextureAsync::updateSingleTexture(this);
			lock.acquire(&this->asyncLoadMutex);
			if (!this->asyncLoadQueued && this->uploaded)
			{
				break;
			}
			lock.release();
			hthread::sleep(0.001f);
		}
	}

	bool Texture::_ensureUploaded()
	{
		hmutex::ScopeLock lock(&this->asyncLoadMutex);
		if (this->uploaded)
		{
			this->_tryUploadDataToGpu(); // upload any additional changes
			return true;
		}
		return this->_upload(lock);
	}

	hstream* Texture::_prepareAsyncStream()
	{
		hmutex::ScopeLock lock(&this->asyncLoadMutex);
		if (!this->asyncLoadQueued || this->asyncLoadDiscarded || this->filename == "" || this->uploaded)
		{
			this->asyncLoadQueued = false;
			this->asyncLoadDiscarded = false;
			return NULL;
		}
		lock.release();
		hstream* stream = new hstream();
		if (this->fromResource)
		{
			hresource file;
			file.open(this->filename);
			stream->writeRaw(file);
		}
		else
		{
			hfile file;
			file.open(this->filename);
			stream->writeRaw(file);
		}
		stream->rewind();
		lock.acquire(&this->asyncLoadMutex);
		if (!this->asyncLoadQueued || this->asyncLoadDiscarded)
		{
			this->asyncLoadQueued = false;
			this->asyncLoadDiscarded = false;
			delete stream;
			return NULL;
		}
		return stream;
	}

	void Texture::_decodeFromAsyncStream(hstream* stream)
	{
		hmutex::ScopeLock lock(&this->asyncLoadMutex);
		if (!this->asyncLoadQueued || this->asyncLoadDiscarded || this->filename == "" || this->dataAsync != NULL || this->uploaded)
		{
			this->asyncLoadQueued = false;
			this->asyncLoadDiscarded = false;
			return;
		}
		lock.release();
		hlog::write(logTag, "Loading async texture: " + this->_getInternalName());
		// must not call createFromStream() that converts automatically, because _processImageFormatSupport() needs to be called first
		Image* image = Image::createFromStream(*(hsbase*)stream, "." + hfile::extensionOf(this->filename));
		if (image != NULL)
		{
			image = this->_processImageFormatSupport(image);
		}
		if (image != NULL && this->format != Image::Format::Invalid && Image::needsConversion(image->format, this->format))
		{
			unsigned char* data = NULL;
			if (Image::convertToFormat(image->w, image->h, image->data, image->format, &data, this->format))
			{
				delete[] image->data;
				image->format = this->format;
				image->data = data;
			}
		}
		if (image == NULL)
		{
			hlog::error(logTag, "Failed to load async texture: " + this->_getInternalName());
			lock.acquire(&this->asyncLoadMutex);
			this->asyncLoadQueued = false;
			this->asyncLoadDiscarded = false;
			return;
		}
		this->width = image->w;
		this->height = image->h;
		this->format = image->format;
		this->dataFormat = image->internalFormat;
		if (this->dataFormat != 0)
		{
			this->compressedSize = image->compressedSize;
		}
		int maxTextureSize = april::rendersys->getCaps().maxTextureSize;
		if (maxTextureSize > 0 && (this->width > maxTextureSize || this->height > maxTextureSize))
		{
			hlog::warnf(logTag, "Texture size for '%s' is %d,%d while the reported system max texture size is %d!", this->_getInternalName().cStr(), this->width, this->height, maxTextureSize);
		}
		lock.acquire(&this->asyncLoadMutex);
		if (this->asyncLoadQueued && !this->asyncLoadDiscarded)
		{
			this->_assignFormat();
			this->dataAsync = image->data;
			image->data = NULL;
		}
		this->asyncLoadQueued = false;
		this->asyncLoadDiscarded = false;
		delete image;
	}

	Image* Texture::_processImageFormatSupport(Image* image)
	{
		if (!april::rendersys->getCaps().textureFormats.has(image->format))
		{
			hlog::warn(logTag, "Texture format not supported, trying to convert to an RGBA format: " + this->_getInternalName());
			Image::Format nativeFormat = april::rendersys->getNativeTextureFormat(Image::Format::RGBA);
			if (image->data != NULL)
			{
				bool result = false;
				Image* newImage = NULL;
				if (image->format == Image::Format::Alpha)
				{
					newImage = Image::create(image->w, image->h, april::Color::White, nativeFormat);
					result = newImage->insertAlphaMap(image);
				}
				else
				{
					newImage = Image::create(image->w, image->h, april::Color::Clear, nativeFormat);
					result = newImage->write(0, 0, image->w, image->h, 0, 0, image);
				}
				delete image;
				image = newImage;
				if (!result)
				{
					hlog::error(logTag, "Could not write format: " + this->_getInternalName());
					delete image;
					image = NULL;
				}
			}
			else // might have been a meta data load
			{
				image->format = nativeFormat;
			}
		}
		return image;
	}

	bool Texture::clear()
	{
		if (!this->_isWritable())
		{
			hlog::warn(logTag, "Cannot write texture: " + this->_getInternalName());
			return false;
		}
		return this->_rawClear();
	}

	bool Texture::_rawClear()
	{
		this->waitForAsyncLoad();
		hmutex::ScopeLock lock(&this->asyncDataMutex);
		memset(this->data, 0, this->getByteSize());
		this->dirty = true;
		return true;
	}

	Color Texture::getPixel(int x, int y)
	{
		Color color = Color::Clear;
		if (!this->_isReadable())
		{
			hlog::warn(logTag, "Cannot read texture: " + this->_getInternalName());
			return color;
		}
		this->waitForAsyncLoad();
		hmutex::ScopeLock lock(&this->asyncDataMutex);
		if (this->data != NULL)
		{
			color = Image::getPixel(x, y, this->data, this->width, this->height, this->format);
		}
		return color;
	}

	bool Texture::setPixel(int x, int y, const Color& color)
	{
		if (!this->_isWritable())
		{
			hlog::warn(logTag, "Cannot write texture: " + this->_getInternalName());
			return false;
		}
		return this->_rawSetPixel(x, y, color);
	}

	bool Texture::_rawSetPixel(int x, int y, const Color& color)
	{
		this->waitForAsyncLoad();
		hmutex::ScopeLock lock(&this->asyncDataMutex);
		bool result = Image::setPixel(x, y, color, this->data, this->width, this->height, this->format);
		this->dirty |= result;
		return result;
	}

	Color Texture::getInterpolatedPixel(float x, float y)
	{
		Color color = Color::Clear;
		if (!this->_isReadable())
		{
			hlog::warn(logTag, "Cannot read texture: " + this->_getInternalName());
			return color;
		}
		this->waitForAsyncLoad();
		hmutex::ScopeLock lock(&this->asyncDataMutex);
		if (this->data != NULL)
		{
			color = Image::getInterpolatedPixel(x, y, this->data, this->width, this->height, this->format);
		}
		return color;
	}

	bool Texture::fillRect(int x, int y, int w, int h, const Color& color)
	{
		if (!this->_isWritable())
		{
			hlog::warn(logTag, "Cannot write texture: " + this->_getInternalName());
			return false;
		}
		return this->_rawFillRect(x, y, w, h, color);
	}

	bool Texture::_rawFillRect(int x, int y, int w, int h, const Color& color)
	{
		if (w == 1 && h == 1)
		{
			return this->_rawSetPixel(x, y, color);
		}
		this->waitForAsyncLoad();
		hmutex::ScopeLock lock(&this->asyncDataMutex);
		bool result = Image::fillRect(x, y, w, h, color, this->data, this->width, this->height, this->format);
		this->dirty |= result;
		return result;
	}

	bool Texture::blitRect(int x, int y, int w, int h, const Color& color)
	{
		if (!this->_isWritable())
		{
			hlog::warn(logTag, "Cannot write texture: " + this->_getInternalName());
			return false;
		}
		return this->_rawBlitRect(x, y, w, h, color);
	}

	bool Texture::_rawBlitRect(int x, int y, int w, int h, const Color& color)
	{
		this->waitForAsyncLoad();
		hmutex::ScopeLock lock(&this->asyncDataMutex);
		bool result = Image::blitRect(x, y, w, h, color, this->data, this->width, this->height, this->format);
		this->dirty |= result;
		return result;
	}

	bool Texture::copyPixelData(unsigned char** output, Image::Format format)
	{
		if (!this->_isReadable())
		{
			hlog::warn(logTag, "Cannot read texture: " + this->_getInternalName());
			return false;
		}
		this->waitForAsyncLoad();
		hmutex::ScopeLock lock(&this->asyncDataMutex);
		return Image::convertToFormat(this->width, this->height, this->data, this->format, output, format, false);
	}

	Image* Texture::createImage(Image::Format format)
	{
		if (!this->_isReadable())
		{
			hlog::warn(logTag, "Cannot read texture: " + this->_getInternalName());
			return NULL;
		}
		this->waitForAsyncLoad();
		hmutex::ScopeLock lock(&this->asyncDataMutex);
		unsigned char* data = NULL;
		Image* image = NULL;
		if (Image::convertToFormat(this->width, this->height, this->data, this->format, &data, format, false))
		{
			image = Image::create(this->width, this->height, NULL, format);
			image->data = data;
		}
		return image;
	}

	bool Texture::write(int sx, int sy, int sw, int sh, int dx, int dy, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat)
	{
		if (!this->_isWritable())
		{
			hlog::warn(logTag, "Cannot write texture: " + this->_getInternalName());
			return false;
		}
		return this->_rawWrite(sx, sy, sw, sh, dx, dy, srcData, srcWidth, srcHeight, srcFormat);
	}

	bool Texture::_rawWrite(int sx, int sy, int sw, int sh, int dx, int dy, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat)
	{
		this->waitForAsyncLoad();
		hmutex::ScopeLock lock(&this->asyncDataMutex);
		bool result = Image::write(sx, sy, sw, sh, dx, dy, srcData, srcWidth, srcHeight, srcFormat, this->data, this->width, this->height, this->format);
		this->dirty |= result;
		return result;
	}

	bool Texture::write(int sx, int sy, int sw, int sh, int dx, int dy, Texture* texture)
	{
		if (!this->_isWritable())
		{
			hlog::warn(logTag, "Cannot write texture: " + this->_getInternalName());
			return false;
		}
		if (texture == NULL)
		{
			hlog::warn(logTag, "Cannot read texture: NULL");
			return false;
		}
		if (!texture->_isReadable())
		{
			hlog::warn(logTag, "Cannot read texture: " + texture->_getInternalName());
			return false;
		}
		texture->waitForAsyncLoad();
		hmutex::ScopeLock lock(&texture->asyncDataMutex);
		return this->write(sx, sy, sw, sh, dx, dy, texture->data, texture->width, texture->height, texture->format);
	}

	bool Texture::writeStretch(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat)
	{
		if (!this->_isWritable())
		{
			hlog::warn(logTag, "Cannot write texture: " + this->_getInternalName());
			return false;
		}
		return this->_rawWriteStretch(sx, sy, sw, sh, dx, dy, dw, dh, srcData, srcWidth, srcHeight, srcFormat);
	}

	bool Texture::_rawWriteStretch(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat)
	{
		this->waitForAsyncLoad();
		hmutex::ScopeLock lock(&this->asyncDataMutex);
		bool result = Image::writeStretch(sx, sy, sw, sh, dx, dy, dw, dh, srcData, srcWidth, srcHeight, srcFormat, this->data, this->width, this->height, this->format);
		this->dirty |= result;
		return result;
	}

	bool Texture::writeStretch(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, Texture* texture)
	{
		if (!this->_isWritable())
		{
			hlog::warn(logTag, "Cannot write texture: " + this->_getInternalName());
			return false;
		}
		if (texture == NULL)
		{
			hlog::warn(logTag, "Cannot read texture: NULL");
			return false;
		}
		if (!texture->_isReadable())
		{
			hlog::warn(logTag, "Cannot read texture: " + texture->_getInternalName());
			return false;
		}
		texture->waitForAsyncLoad();
		hmutex::ScopeLock lock(&texture->asyncDataMutex);
		return this->writeStretch(sx, sy, sw, sh, dx, dy, dw, dh, texture->data, texture->width, texture->height, texture->format);
	}

	bool Texture::blit(int sx, int sy, int sw, int sh, int dx, int dy, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat, unsigned char alpha)
	{
		if (!this->_isAlterable())
		{
			hlog::warn(logTag, "Cannot alter texture: " + this->_getInternalName());
			return false;
		}
		return this->_rawBlit(sx, sy, sw, sh, dx, dy, srcData, srcWidth, srcHeight, srcFormat);
	}

	bool Texture::_rawBlit(int sx, int sy, int sw, int sh, int dx, int dy, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat, unsigned char alpha)
	{
		this->waitForAsyncLoad();
		hmutex::ScopeLock lock(&this->asyncDataMutex);
		bool result = Image::blit(sx, sy, sw, sh, dx, dy, srcData, srcWidth, srcHeight, srcFormat, this->data, this->width, this->height, this->format);
		this->dirty |= result;
		return result;
	}

	bool Texture::blit(int sx, int sy, int sw, int sh, int dx, int dy, Texture* texture, unsigned char alpha)
	{
		if (!this->_isAlterable())
		{
			hlog::warn(logTag, "Cannot alter texture: " + this->_getInternalName());
			return false;
		}
		if (texture == NULL)
		{
			hlog::warn(logTag, "Cannot read texture: NULL");
			return false;
		}
		if (!texture->_isReadable())
		{
			hlog::warn(logTag, "Cannot read texture: " + texture->_getInternalName());
			return false;
		}
		texture->waitForAsyncLoad();
		hmutex::ScopeLock lock(&texture->asyncDataMutex);
		return this->blit(sx, sy, sw, sh, dx, dy, texture->data, texture->width, texture->height, texture->format);
	}

	bool Texture::blitStretch(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat, unsigned char alpha)
	{
		if (!this->_isAlterable())
		{
			hlog::warn(logTag, "Cannot alter texture: " + this->_getInternalName());
			return false;
		}
		return this->_rawBlitStretch(sx, sy, sw, sh, dx, dy, dw, dh, srcData, srcWidth, srcHeight, srcFormat);
	}

	bool Texture::_rawBlitStretch(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat, unsigned char alpha)
	{
		this->waitForAsyncLoad();
		hmutex::ScopeLock lock(&this->asyncDataMutex);
		bool result = Image::blitStretch(sx, sy, sw, sh, dx, dy, dw, dh, srcData, srcWidth, srcHeight, srcFormat, this->data, this->width, this->height, this->format);
		this->dirty |= result;
		return result;
	}

	bool Texture::blitStretch(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, Texture* texture, unsigned char alpha)
	{
		if (!this->_isAlterable())
		{
			hlog::warn(logTag, "Cannot alter texture: " + this->_getInternalName());
			return false;
		}
		if (texture == NULL)
		{
			hlog::warn(logTag, "Cannot read texture: NULL");
			return false;
		}
		if (!texture->_isReadable())
		{
			hlog::warn(logTag, "Cannot read texture: " + texture->_getInternalName());
			return false;
		}
		texture->waitForAsyncLoad();
		hmutex::ScopeLock lock(&texture->asyncDataMutex);
		return this->blitStretch(sx, sy, sw, sh, dx, dy, dw, dh, texture->data, texture->width, texture->height, texture->format);
	}

	bool Texture::rotateHue(int x, int y, int w, int h, float degrees)
	{
		if (!this->_isAlterable())
		{
			hlog::warn(logTag, "Cannot alter texture: " + this->_getInternalName());
			return false;
		}
		return this->_rawRotateHue(x, y, w, h, degrees);
	}

	bool Texture::_rawRotateHue(int x, int y, int w, int h, float degrees)
	{
		this->waitForAsyncLoad();
		hmutex::ScopeLock lock(&this->asyncDataMutex);
		bool result = Image::rotateHue(x, y, w, h, degrees, this->data, this->width, this->height, this->format);
		this->dirty |= result;
		return result;
	}

	bool Texture::saturate(int x, int y, int w, int h, float factor)
	{
		if (!this->_isAlterable())
		{
			hlog::warn(logTag, "Cannot alter texture: " + this->_getInternalName());
			return false;
		}
		return this->_rawSaturate(x, y, w, h, factor);
	}

	bool Texture::_rawSaturate(int x, int y, int w, int h, float factor)
	{
		this->waitForAsyncLoad();
		hmutex::ScopeLock lock(&this->asyncDataMutex);
		bool result = Image::saturate(x, y, w, h, factor, this->data, this->width, this->height, this->format);
		this->dirty |= result;
		return result;
	}

	bool Texture::invert(int x, int y, int w, int h)
	{
		if (!this->_isAlterable())
		{
			hlog::warn(logTag, "Cannot alter texture: " + this->_getInternalName());
			return false;
		}
		return this->_rawInvert(x, y, w, h);
	}

	bool Texture::_rawInvert(int x, int y, int w, int h)
	{
		this->waitForAsyncLoad();
		hmutex::ScopeLock lock(&this->asyncDataMutex);
		bool result = Image::invert(x, y, w, h, this->data, this->width, this->height, this->format);
		this->dirty |= result;
		return result;
	}

	bool Texture::insertAlphaMap(unsigned char* srcData, Image::Format srcFormat, unsigned char median, int ambiguity)
	{
		if (!this->_isAlterable())
		{
			hlog::warn(logTag, "Cannot alter texture: " + this->_getInternalName());
			return false;
		}
		return this->_rawInsertAlphaMap(srcData, srcFormat, median, ambiguity);
	}

	bool Texture::_rawInsertAlphaMap(unsigned char* srcData, Image::Format srcFormat, unsigned char median, int ambiguity)
	{
		this->waitForAsyncLoad();
		hmutex::ScopeLock lock(&this->asyncDataMutex);
		bool result = Image::insertAlphaMap(this->width, this->height, srcData, srcFormat, this->data, this->format, median, ambiguity);
		this->dirty |= result;
		return result;
	}

	bool Texture::insertAlphaMap(Texture* texture, unsigned char median, int ambiguity)
	{
		if (!this->_isAlterable())
		{
			hlog::warn(logTag, "Cannot alter texture: " + this->_getInternalName());
			return false;
		}
		if (texture == NULL)
		{
			hlog::warn(logTag, "Cannot read texture: NULL");
			return false;
		}
		if (!texture->_isReadable())
		{
			hlog::warn(logTag, "Cannot read texture: " + texture->_getInternalName());
			return false;
		}
		if (texture->width != this->width || texture->height != this->height)
		{
			hlog::errorf(logTag, "Cannot insert alpha map, texture sizes don't match: '%s'@%d,%d and '%s'@%d,%d",
				this->_getInternalName().cStr(), this->width, this->height, texture->_getInternalName().cStr(), texture->width, texture->height);
			return false;
		}
		texture->waitForAsyncLoad();
		hmutex::ScopeLock lock(&texture->asyncDataMutex);
		return this->insertAlphaMap(texture->data, texture->format, median, ambiguity);
	}

	// overloads

	Color Texture::getPixel(cgvec2i position)
	{
		return this->getPixel(position.x, position.y);
	}

	bool Texture::setPixel(cgvec2i position, const Color& color)
	{
		return this->setPixel(position.x, position.y, color);
	}

	Color Texture::getInterpolatedPixel(cgvec2f position)
	{
		return this->getInterpolatedPixel(position.x, position.y);
	}

	bool Texture::fillRect(cgrecti rect, const Color& color)
	{
		return this->fillRect(rect.x, rect.y, rect.w, rect.h, color);
	}

	bool Texture::blitRect(cgrecti rect, const Color& color)
	{
		return this->blitRect(rect.x, rect.y, rect.w, rect.h, color);
	}

	bool Texture::copyPixelData(unsigned char** output)
	{
		return this->copyPixelData(output, this->format);
	}

	Image* Texture::createImage()
	{
		return this->createImage(this->format);
	}

	bool Texture::write(int sx, int sy, int sw, int sh, int dx, int dy, Image* image)
	{
		return this->write(sx, sy, sw, sh, dx, dy, image->data, image->w, image->h, image->format);
	}

	bool Texture::write(cgrecti srcRect, cgvec2i destPosition, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat)
	{
		return this->write(srcRect.x, srcRect.y, srcRect.w, srcRect.h, destPosition.x, destPosition.y, srcData, srcWidth, srcHeight, srcFormat);
	}

	bool Texture::write(cgrecti srcRect, cgvec2i destPosition, Texture* texture)
	{
		return this->write(srcRect.x, srcRect.y, srcRect.w, srcRect.h, destPosition.x, destPosition.y, texture);
	}

	bool Texture::write(cgrecti srcRect, cgvec2i destPosition, Image* image)
	{
		return this->write(srcRect.x, srcRect.y, srcRect.w, srcRect.h, destPosition.x, destPosition.y, image->data, image->w, image->h, image->format);
	}

	bool Texture::writeStretch(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, Image* image)
	{
		return this->writeStretch(sx, sy, sw, sh, dx, dy, dw, dh, image->data, image->w, image->h, image->format);
	}

	bool Texture::writeStretch(cgrecti srcRect, cgrecti destRect, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat)
	{
		return this->writeStretch(srcRect.x, srcRect.y, srcRect.w, srcRect.h, destRect.x, destRect.y, destRect.w, destRect.h, srcData, srcWidth, srcHeight, srcFormat);
	}

	bool Texture::writeStretch(cgrecti srcRect, cgrecti destRect, Texture* texture)
	{
		return this->writeStretch(srcRect.x, srcRect.y, srcRect.w, srcRect.h, destRect.x, destRect.y, destRect.w, destRect.h, texture);
	}

	bool Texture::writeStretch(cgrecti srcRect, cgrecti destRect, Image* image)
	{
		return this->writeStretch(srcRect.x, srcRect.y, srcRect.w, srcRect.h, destRect.x, destRect.y, destRect.w, destRect.h, image->data, image->w, image->h, image->format);
	}

	bool Texture::blit(int sx, int sy, int sw, int sh, int dx, int dy, Image* image, unsigned char alpha)
	{
		return this->blit(sx, sy, sw, sh, dx, dy, image->data, image->w, image->h, image->format, alpha);
	}

	bool Texture::blit(cgrecti srcRect, cgvec2i destPosition, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat, unsigned char alpha)
	{
		return this->blit(srcRect.x, srcRect.y, srcRect.w, srcRect.h, destPosition.x, destPosition.y, srcData, srcWidth, srcHeight, srcFormat, alpha);
	}

	bool Texture::blit(cgrecti srcRect, cgvec2i destPosition, Texture* texture, unsigned char alpha)
	{
		return this->blit(srcRect.x, srcRect.y, srcRect.w, srcRect.h, destPosition.x, destPosition.y, texture, alpha);
	}

	bool Texture::blit(cgrecti srcRect, cgvec2i destPosition, Image* image, unsigned char alpha)
	{
		return this->blit(srcRect.x, srcRect.y, srcRect.w, srcRect.h, destPosition.x, destPosition.y, image->data, image->w, image->h, image->format, alpha);
	}

	bool Texture::blitStretch(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, Image* image, unsigned char alpha)
	{
		return this->blitStretch(sx, sy, sw, sh, dx, dy, dw, dh, image->data, image->w, image->h, image->format, alpha);
	}

	bool Texture::blitStretch(cgrecti srcRect, cgrecti destRect, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat, unsigned char alpha)
	{
		return this->blitStretch(srcRect.x, srcRect.y, srcRect.w, srcRect.h, destRect.x, destRect.y, destRect.w, destRect.h, srcData, srcWidth, srcHeight, srcFormat, alpha);
	}

	bool Texture::blitStretch(cgrecti srcRect, cgrecti destRect, Texture* texture, unsigned char alpha)
	{
		return this->blitStretch(srcRect.x, srcRect.y, srcRect.w, srcRect.h, destRect.x, destRect.y, destRect.w, destRect.h, texture, alpha);
	}

	bool Texture::blitStretch(cgrecti srcRect, cgrecti destRect, Image* image, unsigned char alpha)
	{
		return this->blitStretch(srcRect.x, srcRect.y, srcRect.w, srcRect.h, destRect.x, destRect.y, destRect.w, destRect.h, image->data, image->w, image->h, image->format, alpha);
	}

	bool Texture::rotateHue(cgrecti rect, float degrees)
	{
		return this->rotateHue(rect.x, rect.y, rect.w, rect.h, degrees);
	}

	bool Texture::saturate(cgrecti rect, float factor)
	{
		return this->saturate(rect.x, rect.y, rect.w, rect.h, factor);
	}

	bool Texture::invert(cgrecti rect)
	{
		return this->invert(rect.x, rect.y, rect.w, rect.h);
	}

	bool Texture::insertAlphaMap(Image* image, unsigned char median, int ambiguity)
	{
		// safety checks are done in the other insertAlphaMap()
		if (image->w != this->width || image->h != this->height)
		{
			return false;
		}
		return this->insertAlphaMap(image->data, image->format, median, ambiguity);
	}

	void Texture::_setupPot(int& outWidth, int& outHeight)
	{
		outWidth = hpotCeil(this->width);
		outHeight = hpotCeil(this->height);
		if (this->width < outWidth || this->height < outHeight)
		{
			// software NPOT textures do not support anything other than AddressMode::Clamp
			this->addressMode = AddressMode::Clamp;
			// effective addressing area needs to be changed
			this->effectiveWidth = (float)this->width / outWidth;
			this->effectiveHeight = (float)this->height / outHeight;
		}
	}

	unsigned char* Texture::_createPotData(int& outWidth, int& outHeight, unsigned char* data)
	{
		this->_setupPot(outWidth, outHeight);
		unsigned char* newData = new unsigned char[outWidth * outHeight * this->format.getBpp()];
		Image::write(0, 0, this->width, this->height, 0, 0, data, this->width, this->height, this->format, newData, outWidth, outHeight, this->format);
		if (this->width < outWidth)
		{
			Image::writeStretch(this->width - 1, 0, 1, this->height, this->width, 0, outWidth - this->width, this->height, newData, outWidth, outHeight, this->format, newData, outWidth, outHeight, this->format);
		}
		if (this->height < outHeight)
		{
			Image::writeStretch(0, this->height - 1, outWidth, 1, 0, this->height, outWidth, outHeight - this->height, newData, outWidth, outHeight, this->format, newData, outWidth, outHeight, this->format);
		}
		return newData;
	}

	unsigned char* Texture::_createPotClearData(int& outWidth, int& outHeight)
	{
		this->_setupPot(outWidth, outHeight);
		int size = outWidth * outHeight * this->format.getBpp();
		unsigned char* newData = new unsigned char[size];
		memset(newData, 0, size);
		return newData;
	}

	bool Texture::_tryUploadDataToGpu()
	{
		hmutex::ScopeLock lock(&this->asyncDataMutex);
		if (this->dirty)
		{
			this->dirty = false;
			this->_uploadDataToGpu(0, 0, this->width, this->height);
			return true;
		}
		return false;
	}

	bool Texture::_uploadDataToGpu(int x, int y, int w, int h)
	{
		if ((!Image::needsConversion(this->format, april::rendersys->getNativeTextureFormat(this->format)) &&
			this->_uploadToGpu(x, y, w, h, x, y, this->data, this->width, this->height, this->format)) || this->dirty)
		{
			return true;
		}
		Lock lock = this->_tryLockSystem(x, y, w, h);
		if (lock.failed)
		{
			return false;
		}
		bool result = Image::write(x, y, w, h, lock.x, lock.y, this->data, this->width, this->height, this->format, lock.data, lock.dataWidth, lock.dataHeight, lock.format);
		this->_unlockSystem(lock, true);
		return result;
	}

}
