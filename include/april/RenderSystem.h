/************************************************************************************\
This source file is part of the Awesome Portable Rendering Interface Library         *
For latest info, see http://libapril.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes (kreso@cateia.com)                                  *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#ifndef RENDERSYSTEM_H
#define RENDERSYSTEM_H

#include <hltypes/hstring.h>
#include <hltypes/harray.h>
#include <gtypes/Vector3.h>
#include <gtypes/Matrix4.h>
#include "AprilExport.h"

namespace gtypes
{
	class Vector2;
}

enum TextureType
{
	AT_NORMAL=1,
	AT_RENDER_TARGET=2
};

enum TextureFormat
{
	AT_XRGB=1,
	AT_ARGB=2
};

namespace April
{
		
	class ImageSource;
	// render operations
	enum RenderOp
	{
		TriangleList=1,
		TriangleStrip=2,
		TriangleFan=3,
		LineList=4,
		LineStrip=5,
	};
	
	struct AprilExport PlainVertex : public gtypes::Vector3
	{
	public:
		void operator=(const gtypes::Vector3& v);
	};

	struct AprilExport ColoredVertex : public PlainVertex
	{
	public:
		unsigned int color;
	};

	class AprilExport TexturedVertex : public PlainVertex
	{
	public:
		float u,v;
	};

	class AprilExport ColoredTexturedVertex : public PlainVertex
	{
	public:
		unsigned int color;
		float u,v;
	};

	enum BlendMode
	{
		ALPHA_BLEND,
		ADD,
		DEFAULT
	};

	class AprilExport Color
	{
	public:
		unsigned char r,g,b,a;
		Color(float r,float g,float b,float a=1);
		Color(unsigned int color);
		Color(chstr hex);
		Color();

		void setColor(float r,float g,float b,float a=1);
		void setColor(unsigned int color);
		void setColor(chstr hex);

		float r_float() { return r/255.0f; }
		float g_float() { return g/255.0f; }
		float b_float() { return b/255.0f; }
		float a_float() { return a/255.0f; }

	};

	class AprilExport Texture
	{
	protected:
		bool mDynamic;
		hstr mFilename;
		int mWidth,mHeight;
		float mUnusedTimer;
		harray<Texture*> mDynamicLinks;
	public:
		Texture();
		virtual ~Texture();
		virtual void unload()=0;
		virtual int getSizeInBytes()=0;
		
		virtual Color getPixel(int x,int y);
		virtual Color getInterpolatedPixel(float x,float y);
		
		void addDynamicLink(Texture* lnk);
		void removeDynamicLink(Texture* lnk);
		void _resetUnusedTimer(bool recursive=1);
		
		int getWidth() { return mWidth; };
		int getHeight() { return mHeight; };
		/// only used with dynamic textures since at chapter load you need it's dimensions for images, but you don't know them yet
		void _setDimensions(int w,int h) { mWidth=w; mHeight=h; }
		bool isDynamic() { return mDynamic; }
		virtual bool isLoaded()=0;
		
		void update(float time_increase);
		hstr getFilename() { return mFilename; }
	};
	
	class AprilExport RAMTexture : public Texture
	{
	protected:
		ImageSource* mBuffer;
	public:
		RAMTexture(chstr filename,bool dynamic);
		virtual ~RAMTexture();
		void load();
		void unload();
		bool isLoaded();
		Color getPixel(int x,int y);
		Color getInterpolatedPixel(float x,float y);
		int getSizeInBytes();
		
	};

	class AprilExport RenderSystem
	{
	protected:
		float mAlphaMultiplier;
		float mIdleUnloadTime;
		bool mDynamicLoading;
		
		bool (*mUpdateCallback)(float);
		void (*mMouseDownCallback)(float,float,int);
		void (*mMouseUpCallback)(float,float,int);
		void (*mMouseMoveCallback)(float,float);
		void (*mKeyDownCallback)(unsigned int);
		void (*mKeyUpCallback)(unsigned int);
		void (*mCharCallback)(unsigned int);

		gtypes::Matrix4 mModelviewMatrix,mProjectionMatrix;

		virtual void _setModelviewMatrix(const gtypes::Matrix4& matrix)=0;
		virtual void _setProjectionMatrix(const gtypes::Matrix4& matrix)=0;
	public:
		virtual hstr getName()=0;

		RenderSystem();
		virtual ~RenderSystem();

		// object creation
		virtual Texture* loadTexture(chstr filename,bool dynamic=false)=0;
		Texture* loadRAMTexture(chstr filename,bool dynamic=false);
		virtual Texture* createTextureFromMemory(unsigned char* rgba,int w,int h)=0;
		virtual Texture* createEmptyTexture(int w,int h,TextureFormat fmt=AT_XRGB,TextureType type=AT_NORMAL)=0;

		// modelview matrix transformation
		void setIdentityTransform();
		void translate(float x,float y,float z=0);
		void rotate(float angle,float ax=0,float ay=0,float az=-1);
		void scale(float s);
		void scale(float sx,float sy,float sz);
		// camera functions
		void lookAt(const gtypes::Vector3 &eye, const gtypes::Vector3 &direction, const gtypes::Vector3 &up);
		// projection matrix tronsformation
		void setOrthoProjection(float w,float h,float x_offset=0,float y_offset=0);
		void setPerspective(float fov, float aspect, float near, float far);
		// rendersys matrix operations
		void setModelviewMatrix(const gtypes::Matrix4& matrix);
		void setProjectionMatrix(const gtypes::Matrix4& matrix);
		virtual bool isFullscreen() { return false; } //2DO - implement in derived classes
		virtual void setFullscreen(bool fullscreen) { } //2DO - implement in derived classes
		
		const gtypes::Matrix4& getModelviewMatrix();
		const gtypes::Matrix4& getProjectionMatrix();
		// render state
		virtual void setBlendMode(BlendMode mode)=0;
		
		
		// rendering
		virtual void clear(bool color=true,bool depth=false)=0;
		virtual void setTexture(Texture* t)=0;
		virtual void render(RenderOp renderOp,TexturedVertex* v,int nVertices)=0;
		virtual void render(RenderOp renderOp,ColoredTexturedVertex* v,int nVertices)=0;
		virtual void render(RenderOp renderOp,TexturedVertex* v,int nVertices,float r,float g,float b,float a)=0;
		virtual void render(RenderOp renderOp,PlainVertex* v,int nVertices)=0;
		virtual void render(RenderOp renderOp,PlainVertex* v,int nVertices,float r,float g,float b,float a)=0;
		virtual void render(RenderOp renderOp,ColoredVertex* v,int nVertices)=0;
		
		virtual void setRenderTarget(Texture* source)=0;
		
		void drawColoredQuad(float x,float y,float w,float h,float r,float g,float b,float a=1);
		void drawTexturedQuad(float x,float y,float w,float h,float sx,float sy,float sw,float sh);
		void drawTexturedQuad(float x,float y,float w,float h,float sx,float sy,float sw,float sh,float r,float g,float b,float a);

		
		float getIdleTextureUnloadTime() { return mIdleUnloadTime; }
		void setIdleTextureUnloadTime(float time) { mIdleUnloadTime=time; }

		void logMessage(chstr message,chstr prefix="[april] ");

		virtual void setAlphaMultiplier(float value)=0;
		float getAlphaMultiplier() { return mAlphaMultiplier; }

		virtual int getWindowWidth()=0;
		virtual int getWindowHeight()=0;

		virtual void setWindowTitle(chstr title)=0;
		virtual gtypes::Vector2 getCursorPos()=0;
		virtual void showSystemCursor(bool b)=0;
		virtual bool isSystemCursorShown()=0;

		virtual void beginFrame()=0;
		virtual void presentFrame()=0;
		virtual void enterMainLoop()=0;

		void registerUpdateCallback(bool (*callback)(float));
		void registerMouseCallbacks(void (*mouse_dn)(float,float,int),
									void (*mouse_up)(float,float,int),
									void (*mouse_move)(float,float));
		void registerKeyboardCallbacks(void (*key_dn)(unsigned int),
									   void (*key_up)(unsigned int),
									   void (*char_callback)(unsigned int));

		void forceDynamicLoading(bool value) { mDynamicLoading=value; }
		bool isDynamicLoadingForced() { return mDynamicLoading; }

		virtual void terminateMainLoop()=0;
	};

	AprilFnExport void setLogFunction(void (*fnptr)(chstr));
	AprilFnExport void init(chstr rendersystem_name,int w,int h,bool fullscreen,chstr title);
	AprilFnExport void destroy();
}
// global rendersys shortcut variable
AprilFnExport extern April::RenderSystem* rendersys;

#endif
