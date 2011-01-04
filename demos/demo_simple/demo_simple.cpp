/************************************************************************************\
This source file is part of the Awesome Portable Rendering Interface Library         *
For latest info, see http://libapril.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes                                                     *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#include <april/RenderSystem.h>
#include <april/main.h>
#ifdef __APPLE__
#include <TargetConditionals.h>
#endif

#if TARGET_OS_IPHONE
#include <OpenGLES/ES1/gl.h>
#endif

april::Texture* tex;

#if TARGET_OS_IPHONE

float offx=0, offy=0;

void paintRect(GLfloat vertices[])
{
	// temp: testing render on iOS
	glDisable(GL_TEXTURE_2D);
	
	
	
	float r=0.5,g=1,b=1,a=0.4;
	
	GLfloat colors[] = {
		r,g,b,a,
		r,g,b,a,
		r,g,b,a,
		
		r,g,b,a,
		r,g,b,a,
		r,g,b,a};
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	//	glEnable(GL_POLYGON_SMOOTH);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	
	glVertexPointer(3, GL_FLOAT, 0, vertices);
	glColorPointer(4, GL_FLOAT, 0, colors);
	
	glDrawArrays(GL_TRIANGLES, 0, 6);
	
	glDisable(GL_BLEND);
	glDisableClientState(GL_COLOR_ARRAY);
	
	glEnable(GL_TEXTURE_2D);
	
}

#endif

bool render(float time_increase)
{
	// always pick up the window width and height
	// iOS has different size than what is requested
	float w = rendersys->getWindowWidth(), h = rendersys->getWindowHeight();
		
	rendersys->setOrthoProjection(w,h);
	rendersys->setTexture(tex);
	
	rendersys->clear();
	

	
	april::TexturedVertex v[4];
	
	v[0].x=0;   v[0].y=0;   v[0].z=0; v[0].u=0; v[0].v=0;
	v[1].x=offx;   v[1].y=0;   v[1].z=0; v[1].u=1; v[1].v=0;
	v[2].x=0;   v[2].y=offy;   v[2].z=0; v[2].u=0; v[2].v=1;
	v[3].x=offx;   v[3].y=offy;   v[3].z=0; v[3].u=1; v[3].v=1;
	
	glEnable(GL_TEXTURE_2D);
	rendersys->render(april::TriangleStrip,v,4);
	
	
#if TARGET_OS_IPHONE
	// testing painting on iOS
	
	offx+=time_increase;
	offy+=time_increase;
	w=10; h=10;
	GLfloat vertices[] = {	
		offx+0, offy+0, 0,
		offx+0, offy+h, 0,
		offx+w, offy+0, 0,
		
		offx+0, offy+h, 0,
		offx+w, offy+h, 0,
		offx+w, offy+0, 0};
	paintRect(vertices);
	
#endif
	
	rendersys->presentFrame();
	return true;
}


void mousedn(float x,float y,int btn)
{
	printf("dn x: %g y: %g btn: %d\n", x, y, btn);
	offx = x; offy = y;
}
void mouseup(float x,float y,int btn)
{
	printf("up x: %g y: %g btn: %d\n", x, y, btn);
	offx = x; offy = y;
}
void mousemove(float x,float y)
{
	printf("mv x: %g y: %g\n", x, y);
	offx = x; offy = y;
}


int main(int argc, char** argv)
{
	april::init("april",800,600,0,"april: Simple Demo");
	april::rendersys->registerUpdateCallback(render);
	april::rendersys->registerMouseCallbacks(mousedn, mouseup, mousemove);
#if !TARGET_OS_IPHONE
	tex=april::rendersys->loadTexture("../media/texture.jpg");
#else
	tex=april::rendersys->loadTexture("media/texture.jpg");
#endif
	
	rendersys->enterMainLoop();
	april::destroy();
	return 0;
}
