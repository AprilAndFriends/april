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
		void render(int renderOp,TexturedVertex* v,int nVertices);
		void render(int renderOp,TexturedVertex* v,int nVertices,float r,float g,float b,float a);
		void render(int renderOp,PlainVertex* v,int nVertices);
		void render(int renderOp,PlainVertex* v,int nVertices,float r,float g,float b,float a);
		void render(int renderOp,ColoredVertex* v,int nVertices);

		void setAlphaMultiplier(float value);
		void presentFrame();
		
		bool triggerUpdate(float time_increase);
		
		void enterMainLoop();
	};

	void createGLRenderSystem(int w,int h,bool fullscreen,std::string title);

	void destroyGLRenderSystem();
}
#endif
