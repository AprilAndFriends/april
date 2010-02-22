/************************************************************************************
This source file is part of the Awesome Portable Rendering Interface Library
For latest info, see http://libatres.sourceforge.net/
*************************************************************************************
Copyright (c) 2010 Kresimir Spes (kreso@cateia.com)

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License (LGPL) as published by the
Free Software Foundation; either version 2 of the License, or (at your option)
any later version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.
*************************************************************************************/
#ifndef RENDERSYSTEM_GL_H
#define RENDERSYSTEM_GL_H

#include "RenderSystem.h"

namespace April
{
	class GLTexture : public Texture
	{
	public:
		unsigned int mTexId;
		
		GLTexture(std::string filename,bool dynamic);
		GLTexture(unsigned char* rgba,int w,int h);
		~GLTexture();
		
		void load();
		void unload();
		int getSizeInBytes();
	};

	class GLRenderSystem : public RenderSystem
	{
		bool mTexCoordsEnabled,mColorEnabled;
	public:
		GLRenderSystem(int w,int h);
		~GLRenderSystem();
		std::string getName();
		
		// object creation
		Texture* loadTexture(std::string filename,bool dynamic);
		Texture* createTextureFromMemory(unsigned char* rgba,int w,int h);

		// modelview matrix transformation
		void setIdentityTransform();
		void translate(float x,float y);
		void rotate(float angle); // degrees!
		void scale(float s);
		void pushTransform();
		void popTransform();
		void setBlendMode(BlendMode mode);

		// projection matrix transformation
		void setViewport(float w,float h,float x_offset,float y_offset);
		
		// rendering
		void clear(bool color,bool depth);
		void setTexture(Texture* t);
		void render(RenderOp renderOp,TexturedVertex* v,int nVertices);
		void render(RenderOp renderOp,TexturedVertex* v,int nVertices,float r,float g,float b,float a);
		void render(RenderOp renderOp,PlainVertex* v,int nVertices);
		void render(RenderOp renderOp,PlainVertex* v,int nVertices,float r,float g,float b,float a);
		void render(RenderOp renderOp,ColoredVertex* v,int nVertices);

		void setAlphaMultiplier(float value);
		void presentFrame();
		
		bool triggerUpdate(float time_increase);
		bool triggerKeyEvent(bool down,unsigned int keycode,unsigned int charcode);
		bool triggerMouseEvent(int event,float x,float y,int button);
		
		void enterMainLoop();
	};

	void createGLRenderSystem(int w,int h,bool fullscreen,std::string title);

	void destroyGLRenderSystem();
}
#endif
