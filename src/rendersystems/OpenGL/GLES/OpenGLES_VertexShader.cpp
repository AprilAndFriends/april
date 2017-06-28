/// @file
/// @version 4.4
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
#include "OpenGLES_VertexShader.h"
#include "OpenGL_RenderSystem.h"

namespace april
{
	OpenGLES_VertexShader::OpenGLES_VertexShader() : VertexShader(), glShader(0)
	{
	}

	OpenGLES_VertexShader::~OpenGLES_VertexShader()
	{
		if (this->glShader != 0)
		{
			glDeleteShader(this->glShader);
		}
	}

	bool OpenGLES_VertexShader::isLoaded() const
	{
		return (this->glShader != 0);
	}

	bool OpenGLES_VertexShader::_createShader(chstr filename, const hstream& stream)
	{
		this->glShader = glCreateShader(GL_VERTEX_SHADER);
		if (this->glShader == 0)
		{
			hlog::error(logTag, "Shader could not be created!");
			return false;
		}
		unsigned char* data = (unsigned char*)stream;
		int size = (int)stream.size();
		glShaderSource(this->glShader, 1, (const char**)&data, &size);
		glCompileShader(this->glShader);
		GLint compiled = 0;
		glGetShaderiv(this->glShader, GL_COMPILE_STATUS, &compiled);
		if (compiled == 0)
		{
			int messageSize = 0;
			int written = 0;
			glGetShaderiv(this->glShader, GL_INFO_LOG_LENGTH, &messageSize);
			char* message = new char[messageSize];
			glGetShaderInfoLog(this->glShader, messageSize, &written, message);
			hstr context = filename;
			if (filename == "[raw]")
			{
				context = hstr((char*)data, size);
			}
			hlog::error(logTag, "Shader could not be compiled!\n" + context + "\n" + hstr(message));
			delete[] message;
			glDeleteShader(this->glShader);
			this->glShader = 0;
			return false;
		}
		return true;
	}

	void OpenGLES_VertexShader::setConstantsB(const int* quads, unsigned int quadCount)
	{
		// TODO
	}

	void OpenGLES_VertexShader::setConstantsI(const int* quads, unsigned int quadCount)
	{
		// TODO
	}

	void OpenGLES_VertexShader::setConstantsF(const float* quads, unsigned int quadCount)
	{
		// TODO
	}

}
#endif