/// @file
/// @version 5.2
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#ifdef _OPENGLES
#include <hltypes/hlog.h>
#include <hltypes/hstream.h>
#include <hltypes/hstring.h>

#include "april.h"
#include "OpenGLES_PixelShader.h"
#include "OpenGL_RenderSystem.h"

namespace april
{
	OpenGLES_PixelShader::OpenGLES_PixelShader() :
		PixelShader(),
		glShader(0)
	{
	}

	OpenGLES_PixelShader::~OpenGLES_PixelShader()
	{
		if (this->glShader != 0 && april::rendersys->canUseLowLevelCalls())
		{
			glDeleteShader(this->glShader);
		}
	}

	bool OpenGLES_PixelShader::isLoaded() const
	{
		return (this->glShader != 0);
	}

	bool OpenGLES_PixelShader::_createShader(chstr filename, const hstream& stream)
	{
		if (!april::rendersys->canUseLowLevelCalls())
		{
			return false;
		}
		GL_SAFE_CALL(this->glShader = glCreateShader, (GL_FRAGMENT_SHADER));
		if (this->glShader == 0)
		{
			hlog::error(logTag, "Shader could not be created!");
			return false;
		}
		unsigned char* data = (unsigned char*)stream;
		int size = (int)stream.size();
		GL_SAFE_CALL(glShaderSource, (this->glShader, 1, (const char**)&data, &size));
		GL_SAFE_CALL(glCompileShader, (this->glShader));
		GLint compiled = 0;
		GL_SAFE_CALL(glGetShaderiv, (this->glShader, GL_COMPILE_STATUS, &compiled));
		if (compiled == 0)
		{
			int messageSize = 0;
			int written = 0;
			GL_SAFE_CALL(glGetShaderiv, (this->glShader, GL_INFO_LOG_LENGTH, &messageSize));
			char* message = new char[messageSize];
			GL_SAFE_CALL(glGetShaderInfoLog, (this->glShader, messageSize, &written, message));
			hstr context = filename;
			if (filename == "[raw]")
			{
				context = hstr((char*)data, size);
			}
			hlog::error(logTag, "Shader could not be compiled!\n" + context + "\n" + hstr(message));
			delete[] message;
			GL_SAFE_CALL(glDeleteShader, (this->glShader));
			this->glShader = 0;
			return false;
		}
		return true;
	}

	void OpenGLES_PixelShader::setConstantsB(const int* quads, unsigned int quadCount)
	{
		// TODO?
	}

	void OpenGLES_PixelShader::setConstantsI(const int* quads, unsigned int quadCount)
	{
		// TODO?
	}

	void OpenGLES_PixelShader::setConstantsF(const float* quads, unsigned int quadCount)
	{
		// TODO?
	}

}
#endif