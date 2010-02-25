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
#ifndef RENDERSYSTEM_H
#define RENDERSYSTEM_H

#include <string>
#include "AprilExport.h"

namespace April
{
	// render operations
	enum RenderOp
	{
		TriangleList=1,
		TriangleStrip=2,
		TriangleFan=3,
		LineList=4,
		LineStrip=5,
		LineLoop=6,
	};
	
	class AprilExport Vector
	{
	public:
		float x,y,z;
	};
	
	class AprilExport TexturedVertex : public Vector
	{
	public:
		float u,v;
	};

	struct AprilExport PlainVertex : public Vector
	{
	public:
	};

	struct AprilExport ColoredVertex : public Vector
	{
	public:
		unsigned int color;
	};

	enum BlendMode
	{
		ALPHA_BLEND,
		ADD,
		DEFAULT
	};

	struct AprilExport Color
	{
		unsigned char r,g,b,a;
		Color(float r,float g,float b,float a=1);
		Color();

		void setHex(std::string hex);

		float r_float() { return (float) r/255.0f; }
		float g_float() { return (float) g/255.0f; }
		float b_float() { return (float) b/255.0f; }
		float a_float() { return (float) a/255.0f; }

	};


	class AprilExport Texture
	{
	protected:
		bool mDynamic;
		std::string mFilename;
		int mWidth,mHeight;
	public:
		Texture();
		virtual ~Texture();
		virtual void unload()=0;
		virtual int getSizeInBytes()=0;
		
		int getWidth() { return mWidth; };
		int getHeight() { return mHeight; };
		/// only used with dynamic textures since at chapter load you need it's dimensions for images, but you don't know them yet
		void _setDimensions(int w,int h) { mWidth=w; mHeight=h; }
		bool isDynamic() { return mDynamic; }
		std::string getFilename() { return mFilename; }
	};

	class AprilExport RenderSystem
	{
	protected:
		float mAlphaMultiplier;
		
		bool (*mUpdateCallback)(float);
		void (*mMouseDownCallback)(float,float,int);
		void (*mMouseUpCallback)(float,float,int);
		void (*mMouseMoveCallback)(float,float);
		void (*mKeyDownCallback)(unsigned int,unsigned int);
		void (*mKeyUpCallback)(unsigned int,unsigned int);
		
		
	public:
		virtual std::string getName()=0;

		RenderSystem();

		// object creation
		virtual Texture* loadTexture(std::string filename,bool dynamic=false)=0;
		virtual Texture* createTextureFromMemory(unsigned char* rgba,int w,int h)=0;

		// modelview matrix transformation
		virtual void setIdentityTransform()=0;
		virtual void translate(float x,float y)=0;
		virtual void rotate(float angle)=0; // degrees!
		virtual void scale(float s)=0;
		virtual void pushTransform()=0;
		virtual void popTransform()=0;
		virtual void setBlendMode(BlendMode mode)=0;
		
		// projection matrix tronsformation
		virtual void setViewport(float w,float h,float x_offset=0,float y_offset=0)=0;
		
		// rendering
		virtual void clear(bool color=true,bool depth=false)=0;
		virtual void setTexture(Texture* t)=0;
		virtual void render(RenderOp renderOp,TexturedVertex* v,int nVertices)=0;
		virtual void render(RenderOp renderOp,TexturedVertex* v,int nVertices,float r,float g,float b,float a)=0;
		virtual void render(RenderOp renderOp,PlainVertex* v,int nVertices)=0;
		virtual void render(RenderOp renderOp,PlainVertex* v,int nVertices,float r,float g,float b,float a)=0;
		virtual void render(RenderOp renderOp,ColoredVertex* v,int nVertices)=0;
		
		void drawColoredQuad(float x,float y,float w,float h,float r,float g,float b,float a=1);
		
		void logMessage(std::string message);
		
		virtual void setAlphaMultiplier(float value)=0;
		float getAlphaMultiplier() { return mAlphaMultiplier; }

		virtual void presentFrame()=0;
		virtual void enterMainLoop()=0;
	
		void registerUpdateCallback(bool (*callback)(float));
		void registerMouseCallbacks(void (*mouse_dn)(float,float,int),
									void (*mouse_up)(float,float,int),
									void (*mouse_move)(float,float));
		void registerKeyboardCallbacks(void (*key_dn)(unsigned int,unsigned int),
									   void (*key_up)(unsigned int,unsigned int));
	};
	
	void AprilExport init(std::string rendersystem_name,int w,int h,bool fullscreen,std::string title);
	void AprilExport destroy();

}
// global rendersys shortcut variable
extern AprilExport April::RenderSystem* rendersys;

#endif
