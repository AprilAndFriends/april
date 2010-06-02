/************************************************************************************\
This source file is part of the Awesome Portable Rendering Interface Library         *
For latest info, see http://libapril.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes (kreso@cateia.com)                                  *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#include "april/RenderSystem.h"

April::Texture* tex;

bool render(float time_increase)
{
	rendersys->setOrthoProjection(800,600);
	rendersys->setTexture(tex);
	
	rendersys->clear();
	
	April::TexturedVertex v[4];
	
	v[0].x=0;   v[0].y=0;   v[0].z=0; v[0].u=0; v[0].v=0;
	v[1].x=800; v[1].y=0;   v[1].z=0; v[1].u=1; v[1].v=0;
	v[2].x=0;   v[2].y=600; v[2].z=0; v[2].u=0; v[2].v=1;
	v[3].x=800; v[3].y=600; v[3].z=0; v[3].u=1; v[3].v=1;
	
	rendersys->render(April::TriangleStrip,v,4);
	return true;
}

int main()
{
	April::init("April",800,600,0,"April: Simple Demo");
	rendersys->registerUpdateCallback(render);

	tex=rendersys->loadTexture("../media/texture.jpg");

	rendersys->enterMainLoop();
	April::destroy();
	return 0;
}
