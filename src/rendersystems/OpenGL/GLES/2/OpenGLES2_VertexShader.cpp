/// @file
/// @version 3.6
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#ifdef _OPENGLES2
#include <d3d11.h>

#include <hltypes/hlog.h>
#include <hltypes/hresource.h>
#include <hltypes/hstring.h>

#include "april.h"
#include "OpenGLES2_RenderSystem.h"
#include "OpenGLES2_VertexShader.h"

#if _IOS
	#include <OpenGLES/ES2/gl.h>
	#include <OpenGLES/ES2/glext.h>
	extern GLint _positionSlot;
#else
	#include <GLES2/gl2.h>
	#ifdef _ANDROID
		#define GL_GLEXT_PROTOTYPES
		#include <GLES2/glext.h>
	#endif
#endif

namespace april
{
	OpenGLES2_VertexShader::OpenGLES2_VertexShader(chstr filename) : VertexShader(), glShader(0)
	{
		this->loadResource(filename);
	}

	OpenGLES2_VertexShader::OpenGLES2_VertexShader() : VertexShader(), glShader(0)
	{
	}

	OpenGLES2_VertexShader::~OpenGLES2_VertexShader()
	{
		if (this->glShader != 0)
		{
			glDeleteShader(this->glShader);
		}
	}

	bool OpenGLES2_VertexShader::loadFile(chstr filename)
	{
		if (this->glShader != 0)
		{
			hlog::error(logTag, "Shader already loaded.");
			return false;
		}
		unsigned char* data = NULL;
		int size = 0;
		if (!this->_loadFileData(filename, &data, &size))
		{
			hlog::error(logTag, "Shader file not found: " + filename);
			return false;
		}
		return this->_compileShader(filename, data, size);
	}

	bool OpenGLES2_VertexShader::loadResource(chstr filename)
	{
		if (this->glShader != 0)
		{
			hlog::error(logTag, "Shader already loaded.");
			return false;
		}
		unsigned char* data = NULL;
		int size = 0;
		if (!this->_loadResourceData(filename, &data, &size))
		{
			hlog::error(logTag, "Shader file not found: " + filename);
			return false;
		}
		return this->_compileShader(filename, data, size);
	}

	bool OpenGLES2_VertexShader::_compileShader(chstr filename, unsigned char* data, int size)
	{
		this->glShader = glCreateShader(GL_FRAGMENT_SHADER);
		if (this->glShader == 0)
		{
			hlog::error(logTag, "Shader could not be created!");
			return false;
		}
		glShaderSource(this->glShader, 1, (const char**)&data, &size);
		glCompileShader(this->glShader);
		delete[] data;
		GLint compiled = 0;
		glGetShaderiv(this->glShader, GL_COMPILE_STATUS, &compiled);
		if (compiled == 0)
		{
			int messageSize = 0;
			int written = 0;
			glGetShaderiv(this->glShader, GL_INFO_LOG_LENGTH, &messageSize);
			char* message = new char[messageSize];
			glGetShaderInfoLog(this->glShader, messageSize, &written, message);
			hlog::error(logTag, "Shader '" + filename + "' could not be compiled! Error:\n" + hstr(message));
			delete[] message;
			glDeleteShader(this->glShader);
			this->glShader = 0;
			return false;
		}
		return true;
	}

	void OpenGLES2_VertexShader::setConstantsB(const int* quads, unsigned int quadCount)
	{
	}

	void OpenGLES2_VertexShader::setConstantsI(const int* quads, unsigned int quadCount)
	{
	}

	void OpenGLES2_VertexShader::setConstantsF(const float* quads, unsigned int quadCount)
	{
	}

}

#endif