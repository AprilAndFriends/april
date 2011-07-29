/************************************************************************************\
This source file is part of the Awesome Portable Rendering Interface Library         *
For latest info, see http://libapril.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes                                                     *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#if defined(HAVE_MARMELADE)
#ifndef MARMELADE_RENDERSYSTEM_H
#define MARMELADE_RENDERSYSTEM_H

#include <s3e.h>
#include <GLES/gl.h>
#include <GLES/egl.h>

#include "RenderSystem.h"

namespace april
{
	class Marmelade_RenderSystem : public RenderSystem
	{
	public:
		Marmelade_RenderSystem();
		~Marmelade_RenderSystem();

		void assignWindow(Window* window);
		
		// object creation
		Texture* loadTexture(chstr filename, bool dynamic);
		Texture* createTextureFromMemory(unsigned char* rgba, int w, int h);
		Texture* createEmptyTexture(int w, int h, TextureFormat fmt, TextureType type);


		// modelview matrix transformation
		void setBlendMode(BlendMode mode);
		void setTextureFilter(TextureFilter filter);
		void setTextureWrapping(bool wrap);
		void setResolution(int w, int h);
		// caps
		float getPixelOffset();
		hstr getName();
        
        ImageSource* grabScreenshot(int bpp = 3);

		// rendering
		void clear(bool color, bool depth);
		void setTexture(Texture* t);
		void render(RenderOp renderOp, TexturedVertex* v, int nVertices);
		void render(RenderOp renderOp, ColoredTexturedVertex* v, int nVertices);
		void render(RenderOp renderOp, TexturedVertex* v, int nVertices, Color color);
		void render(RenderOp renderOp, PlainVertex* v, int nVertices);
		void render(RenderOp renderOp, PlainVertex* v, int nVertices, Color color);
		void render(RenderOp renderOp, ColoredVertex* v, int nVertices);

		void setAlphaMultiplier(float value);
		void setRenderTarget(Texture* source);
		void beginFrame();

		harray<DisplayMode> getSupportedDisplayModes();
		
		static Marmelade_RenderSystem* create();

	protected:
		bool mTexCoordsEnabled;
		bool mColorEnabled;
		void _setModelviewMatrix(const gmat4& matrix);
		void _setProjectionMatrix(const gmat4& matrix);
		
	};
	
}

#endif
#endif
