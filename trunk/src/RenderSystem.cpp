#include <stdio.h>
#include "RenderSystem.h"
#include "RenderSystem_GL.h"

April::RenderSystem* rendersys;

namespace April
{
	int hexstr_to_int(std::string s)
	{
		int i;
		sscanf(s.c_str(),"%x",&i);
		return i;
	}
	
	void hexstr_to_argb(std::string& hex,unsigned char* a,unsigned char* r,unsigned char* g,unsigned char* b)
	{
		if (hex.substr(0,2) != "0x") hex="0x"+hex;
		if (hex.size() == 8)
		{
			*r=hexstr_to_int(hex.substr(2,2));
			*g=hexstr_to_int(hex.substr(4,2));
			*b=hexstr_to_int(hex.substr(6,2));
			*a=255;
		}
		else if (hex.size() == 10)
		{
			*r=hexstr_to_int(hex.substr(4,2));
			*g=hexstr_to_int(hex.substr(6,2));
			*b=hexstr_to_int(hex.substr(8,2));
			*a=hexstr_to_int(hex.substr(2,2));
		}
		else throw "Color format must be either 0xAARRGGBB or 0xRRGGBB";
	}
		
	Color::Color(float r,float g,float b,float a)
	{
		this->r=r*255; this->g=g*255; this->b=b*255; this->a=a*255;
	}

	Color::Color()
	{
		r=g=b=a=1;
	}

	void Color::setHex(std::string hex)
	{
		// this is going to bite me in the arse on a little endian system...
		hexstr_to_argb(hex,&a,&r,&g,&b);
	}

	Texture::Texture()
	{
		
	}


	Texture::~Texture()
	{
		
	}

	RenderSystem::RenderSystem()
	{
		mAlphaMultiplier=1.0f;
		mUpdateCallback=0;
	}

	void RenderSystem::drawColoredQuad(float x,float y,float w,float h,float r,float g,float b,float a)
	{
		PlainVertex v[4];
		v[0].x=x;   v[0].y=y;
		v[1].x=x+w; v[1].y=y;
		v[2].x=x;   v[2].y=y+h;
		v[3].x=x+w; v[3].y=y+h;
		
		render(TRIANGLE_STRIP,v,4,r,g,b,a);
	}
	
	void RenderSystem::logMessage(std::string message)
	{
		
	}
	
	void RenderSystem::registerUpdateCallback(bool (*update)(float))
	{
		mUpdateCallback=update;
	}
/*********************************************************************************/
	
	
	
	void init(std::string rendersystem_name,int w,int h,bool fullscreen,std::string title)
	{
		createGLRenderSystem(w,h,fullscreen,title);
	}
	
	void enterMainLoop()
	{
		
	}
	
	void destroy()
	{
		destroyGLRenderSystem();
	}

}