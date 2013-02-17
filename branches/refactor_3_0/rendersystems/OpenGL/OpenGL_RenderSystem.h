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
/// 
/// @section DESCRIPTION
/// 
/// Defines a generic OpenGL render system.

#ifdef _OPENGL
#ifndef APRIL_OPENGL_RENDER_SYSTEM_H
#define APRIL_OPENGL_RENDER_SYSTEM_H

#include <hltypes/hstring.h>

#include "OpenGL_State.h"
#include "RenderSystem.h"

namespace april
{
	class OpenGL_Texture;

	class OpenGL_RenderSystem : public RenderSystem
	{
	public:
		OpenGL_RenderSystem();
		~OpenGL_RenderSystem();
		bool create(chstr options);
		bool destroy();

		void reset();

		void clear(bool useColor = true, bool depth = false);
		void clear(bool depth, grect rect, Color color = Color::Clear);

		void bindTexture(unsigned int textureId);
		void setResolution(int w, int h);

		void setMatrixMode(unsigned int mode);
		void setTexture(Texture* texture);
		Texture* getRenderTarget();
		void setRenderTarget(Texture* texture);
		void setPixelShader(PixelShader* pixelShader);
		void setVertexShader(VertexShader* vertexShader);
		void setTextureBlendMode(BlendMode mode);
		void setTextureColorMode(ColorMode textureColorMode, unsigned char alpha = 255);
		void setTextureFilter(Texture::Filter textureFilter);
		void setTextureAddressMode(Texture::AddressMode textureAddressMode);

		PixelShader* createPixelShader();
		PixelShader* createPixelShader(chstr filename);
		VertexShader* createVertexShader();
		VertexShader* createVertexShader(chstr filename);

	protected:
		OpenGL_State deviceState;
		OpenGL_State state;
		hstr options;
		OpenGL_Texture* activeTexture;

		virtual void _setupDefaultParameters();
		virtual void _applyStateChanges();
		void _setClientState(unsigned int type, bool enabled);

		virtual void _setTextureBlendMode(BlendMode textureBlendMode);
		virtual void _setTextureColorMode(ColorMode mode, unsigned char alpha = 255);
		virtual void _setTextureFilter(Texture::Filter textureFilter);
		virtual void _setTextureAddressMode(Texture::AddressMode textureAddressMode);

	};
	
}
#endif
#endif
