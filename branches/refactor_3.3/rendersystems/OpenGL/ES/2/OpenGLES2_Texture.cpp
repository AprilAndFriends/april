#ifdef _OPENGLES2
#include <hltypes/hstring.h>

#include "april.h"
#include "Color.h"
#include "OpenGLES2_Texture.h"

namespace april
{
	OpenGLES2_Texture::OpenGLES2_Texture(chstr filename) : OpenGLES_Texture(filename)
	{
	}

	OpenGLES2_Texture::OpenGLES2_Texture(int w, int h, unsigned char* rgba) : OpenGLES_Texture(w, h, rgba)
	{
	}

	OpenGLES2_Texture::OpenGLES2_Texture(int w, int h, Format format, Type type, Color color) :
		OpenGLES_Texture(w, h, format, type, color)
	{
	}

	OpenGLES2_Texture::~OpenGLES2_Texture()
	{
	}

}

#endif
