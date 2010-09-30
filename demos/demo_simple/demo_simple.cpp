/************************************************************************************\
This source file is part of the Awesome Portable Rendering Interface Library         *
For latest info, see http://libapril.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes (kreso@cateia.com)                                  *
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

April::Texture* tex;

#if TARGET_OS_IPHONE

void paintRect(GLfloat vertices[])
{
	// temp: testing render on iOS
	glDisable(GL_TEXTURE_2D);
	
	
	
	float r=1,g=1,b=1,a=0.4;
	
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
	
	
	glEnable(GL_TEXTURE_2D);
	
}

#endif

bool render(float time_increase)
{
	// always pick up the window width and height
	// iOS has different size than what is requested
	int w = rendersys->getWindowWidth(), h = rendersys->getWindowHeight();
		
	rendersys->setOrthoProjection(w,h);
	rendersys->setTexture(tex);
	
	rendersys->clear();
	
	April::TexturedVertex v[4];
	
	v[0].x=0;   v[0].y=0;   v[0].z=0; v[0].u=0; v[0].v=0;
	v[1].x=w;   v[1].y=0;   v[1].z=0; v[1].u=1; v[1].v=0;
	v[2].x=0;   v[2].y=h;   v[2].z=0; v[2].u=0; v[2].v=1;
	v[3].x=w;   v[3].y=h;   v[3].z=0; v[3].u=1; v[3].v=1;
	
	rendersys->render(April::TriangleStrip,v,4);
	
//	April::PlainVertex pv[3];
//	pv[0].set(-1,0,0); pv[1].set(0,1,0); pv[2].set(1,0,0);
//	rendersys->render(April::TriangleList,pv,3);
	
	
	
#if TARGET_OS_IPHONE
	// testing painting on iOS
	GLfloat vertices[] = {	
		0, 0, 0,
		0, h, 0,
		w, 0, 0,
		
		0, h, 0,
		w, h, 0,
		w, 0, 0};
	//paintRect(vertices);
	
#endif
	
	rendersys->presentFrame();
	return true;
}



int main(int argc, char** argv)
{
	April::init("April",800,600,0,"April: Simple Demo");
	April::rendersys->registerUpdateCallback(render);
	
#if !TARGET_OS_IPHONE
	tex=April::rendersys->loadTexture("../media/texture.jpg");
#else
	tex=April::rendersys->loadTexture("media/texture.jpg");
#endif
	
	rendersys->enterMainLoop();
	April::destroy();
	return 0;
}
