#ifndef RENDERSYSTEM_H
#define RENDERSYSTEM_H

#include <string>

// render operations
#define TRIANGLE_LIST 1
#define TRIANGLE_STRIP 2
#define TRIANGLE_FAN 3
#define LINE_LIST 4
#define LINE_STRIP 5
#define LINE_LOOP 6

namespace April
{
	class Vector
	{
	public:
		float x,y,z;
	};
	
	class TexturedVertex : public Vector
	{
	public:
		float u,v;
	};

	struct PlainVertex : public Vector
	{
	public:
	};

	struct ColoredVertex : public Vector
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

	struct Color
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


	class Texture
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

	class RenderSystem
	{
	protected:
		float mAlphaMultiplier;
		
		bool (*mUpdateCallback)(float);
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
		virtual void render(int renderOp,TexturedVertex* v,int nVertices)=0;
		virtual void render(int renderOp,TexturedVertex* v,int nVertices,float r,float g,float b,float a)=0;
		virtual void render(int renderOp,PlainVertex* v,int nVertices)=0;
		virtual void render(int renderOp,PlainVertex* v,int nVertices,float r,float g,float b,float a)=0;
		virtual void render(int renderOp,ColoredVertex* v,int nVertices)=0;
		
		void drawColoredQuad(float x,float y,float w,float h,float r,float g,float b,float a=1);
		
		void logMessage(std::string message);
		
		virtual void setAlphaMultiplier(float value)=0;
		float getAlphaMultiplier() { return mAlphaMultiplier; }

		virtual void presentFrame()=0;
		virtual void enterMainLoop()=0;
	
		void registerUpdateCallback(bool (*update)(float));
	};
	
	void init(std::string rendersystem_name,int w,int h,bool fullscreen,std::string title);
	void destroy();

}
// global rendersys shortcut variable
extern April::RenderSystem* rendersys;

#endif
