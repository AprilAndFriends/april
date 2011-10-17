/************************************************************************************\
This source file is part of the Awesome Portable Rendering Interface Library         *
For latest info, see http://libapril.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes, Boris Mikic                                        *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#ifndef APRIL_RENDERSYSTEM_H
#define APRIL_RENDERSYSTEM_H

#include <hltypes/hstring.h>
#include <hltypes/harray.h>
#include <gtypes/Rectangle.h>
#include <gtypes/Vector2.h>
#include <gtypes/Vector3.h>
#include <gtypes/Matrix4.h>

#include "Color.h"
#include "Texture.h"
#include "aprilExport.h"

namespace april
{
	class Window;
	class ImageSource;
	// render operations
	enum RenderOp
	{
		TriangleList = 1,
		TriangleStrip = 2,
		TriangleFan = 3,
		LineList = 4,
		LineStrip = 5,
		PointList = 6,
	};
	
	enum BlendMode
	{
		ALPHA_BLEND,
		ADD,
		SUBTRACT,
		OVERWRITE,
		DEFAULT
	};

	struct aprilExport DisplayMode
	{
		int width;
		int height;
		int refreshRate;

		bool operator==(const DisplayMode& other)
		{
			return (this->width == other.width && this->height == other.height && this->refreshRate == other.refreshRate);
		}

		bool operator!=(const DisplayMode& other)
		{
			return !(*this == other);
		}
	};
	
	struct aprilExport PlainVertex : public gvec3
	{
	public:
		void operator=(const gvec3& v);
	};

	struct aprilExport ColoredVertex : public PlainVertex
	{
	public:
		unsigned int color;
		void operator=(const gvec3& v);
	};

	class aprilExport TexturedVertex : public PlainVertex
	{
	public:
		float u;
		float v;
		void operator=(const gvec3& v);
	};

	class aprilExport ColoredTexturedVertex : public ColoredVertex
	{
	public:
		float u;
		float v;
		void operator=(const gvec3& v);
	};
    
    class aprilExport ColoredTexturedNormalVertex : public ColoredTexturedVertex
    {
    public:
        gvec3 normal;
		void operator=(const gvec3& v);
    };
    
    class aprilExport TexturedNormalVertex : public TexturedVertex
    {
    public:
        gvec3 normal;
		void operator=(const gvec3& v);
    };
    
    class aprilExport ColoredNormalVertex : public ColoredVertex
    {
    public:
        gvec3 normal;
		void operator=(const gvec3& v);
    };

	class aprilExport RenderSystem
	{
	public:
		RenderSystem();
		virtual ~RenderSystem();

		virtual void assignWindow(Window* window) = 0;

		// object creation
		hstr findTextureFile(chstr filename);
		virtual Texture* loadTexture(chstr filename, bool dynamic = false) = 0;
		RAMTexture* loadRAMTexture(chstr filename, bool dynamic = false);
		virtual Texture* createTextureFromMemory(unsigned char* rgba, int w, int h) = 0;
		virtual Texture* createEmptyTexture(int w, int h, TextureFormat fmt = AT_XRGB, TextureType type = AT_NORMAL) = 0;
		Texture* createBlankTexture(int w, int h, TextureFormat fmt = AT_XRGB, TextureType type = AT_NORMAL);

		// modelview matrix transformation
		void setIdentityTransform();
		void translate(float x, float y, float z = 0.0f);
		void rotate(float angle, float ax = 0.0f, float ay = 0.0f, float az = -1.0f);
		void scale(float s);
		void scale(float sx, float sy, float sz);
		// camera functions
		void lookAt(const gvec3 &eye, const gvec3 &direction, const gvec3 &up);
		// projection matrix transformation
		void setOrthoProjection(gvec2 size);
		void setOrthoProjection(grect rect);
		void setPerspective(float fov, float aspect, float nearClip, float farClip);
		// rendersys matrix operations
		void setModelviewMatrix(const gmat4& matrix);
		void setProjectionMatrix(const gmat4& matrix);
		virtual bool isFullscreen();
		virtual void setFullscreen(bool fullscreen) { } //TODO - implement in derived classes
		virtual void setResolution(int w, int h);
		
		const gmat4& getModelviewMatrix();
		const gmat4& getProjectionMatrix();
		// render state
		virtual void setBlendMode(BlendMode mode) = 0;
		// caps
		virtual float getPixelOffset() = 0;
		virtual hstr getName() = 0;
		// rendering
		virtual void clear(bool useColor = true, bool depth = false) = 0;
		virtual void clear(bool useColor, bool depth, grect rect, Color color = APRIL_COLOR_CLEAR) = 0;
		virtual void setTexture(Texture* t) = 0;
		virtual void render(RenderOp renderOp, PlainVertex* v, int nVertices) = 0;
		virtual void render(RenderOp renderOp, PlainVertex* v, int nVertices, Color color) = 0;
		virtual void render(RenderOp renderOp, TexturedVertex* v, int nVertices) = 0;
		virtual void render(RenderOp renderOp, TexturedVertex* v, int nVertices, Color color) = 0;
		virtual void render(RenderOp renderOp, ColoredVertex* v, int nVertices) = 0;
		virtual void render(RenderOp renderOp, ColoredTexturedVertex* v, int nVertices) = 0;
		
		virtual Texture* getRenderTarget() = 0;
		virtual void setRenderTarget(Texture* source) = 0;
		
		void drawColoredQuad(grect rect, Color color);
		void drawTexturedQuad(grect rect, grect src);
		void drawTexturedQuad(grect rect, grect src, Color color);

		float getIdleTextureUnloadTime() { return mIdleUnloadTime; }
		void setIdleTextureUnloadTime(float time) { mIdleUnloadTime = time; }

		virtual void beginFrame() = 0;

		void forceDynamicLoading(bool value) { mDynamicLoading = value; }
		bool isDynamicLoadingForced() { return mDynamicLoading; }
		
		Window* getWindow() { return mWindow; }
		
        virtual ImageSource* grabScreenshot(int bpp = 3) = 0;
        
		virtual void presentFrame();
		virtual harray<DisplayMode> getSupportedDisplayModes() = 0;

		DEPRECATED_ATTRIBUTE void setOrthoProjection(float w, float h, float x_offset = 0.0f, float y_offset = 0.0f);

	protected:
		Window* mWindow;
		float mIdleUnloadTime;
		bool mDynamicLoading;
		TextureFilter mTextureFilter;
		bool mTextureWrapping;
		gmat4 mModelviewMatrix;
		gmat4 mProjectionMatrix;
		
		virtual void _setModelviewMatrix(const gmat4& matrix) = 0;
		virtual void _setProjectionMatrix(const gmat4& matrix) = 0;
		
	};

	aprilFnExport void setLogFunction(void (*fnptr)(chstr));
	aprilFnExport void init();
	aprilFnExport void createRenderSystem(chstr options);
	aprilFnExport void createRenderTarget(int w, int h, bool fullscreen, chstr title);
	aprilFnExport void destroy();
	aprilFnExport void addTextureExtension(chstr extension);
	aprilFnExport void log(chstr message, chstr prefix = "[april] ");
	
	// global rendersys shortcut variable
	aprilFnExport extern april::RenderSystem* rendersys;
	
}

#endif
