/// @file
/// @version 4.1
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include <hltypes/hfile.h>
#include <hltypes/hlog.h>
#include <hltypes/hresource.h>
#include <hltypes/hstring.h>

#include "april.h"
#include "PixelShader.h"

namespace april
{
	PixelShader::PixelShader()
	{
	}

	PixelShader::~PixelShader()
	{
	}

	bool PixelShader::load(const hstream& stream)
	{
		if (this->isLoaded())
		{
			hlog::error(logTag, "Shader already loaded.");
			return false;
		}
		return this->_createShader("[raw]", stream);
	}

	bool PixelShader::loadFile(chstr filename)
	{
		if (this->isLoaded())
		{
			hlog::error(logTag, "Shader already loaded.");
			return false;
		}
		if (!hfile::exists(filename))
		{
			hlog::error(logTag, "Shader file not found: " + filename);
			return false;
		}
		hstream stream;
		hfile file;
		file.open(filename);
		stream.writeRaw(file);
		file.close();
		stream.rewind();
		return this->_createShader(filename, stream);
	}

	bool PixelShader::loadResource(chstr filename)
	{
		if (this->isLoaded())
		{
			hlog::error(logTag, "Shader already loaded.");
			return false;
		}
		if (!hresource::exists(filename))
		{
			hlog::error(logTag, "Shader file not found: " + filename);
			return false;
		}
		hstream stream;
		hresource file;
		file.open(filename);
		stream.writeRaw(file);
		file.close();
		stream.rewind();
		return this->_createShader(filename, stream);
	}

}
