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
	rendersys->clear();
	static float angle=0;
	angle+=time_increase*90;
	rendersys->setPerspective(60,800/600.,1,1000);
	rendersys->setTexture(tex);
	
	rendersys->lookAt(gtypes::Vector3(2,2,-5),gtypes::Vector3(0,0,0),gtypes::Vector3(0,1,0));
	rendersys->rotate(angle,0,1,0);
	
	April::TexturedVertex v[4];
	
	v[0].x=-1;   v[0].y=1;   v[0].z=0; v[0].u=0; v[0].v=0;
	v[1].x=1; v[1].y=1;   v[1].z=0; v[1].u=1; v[1].v=0;
	v[2].x=-1;   v[2].y=-1; v[2].z=0; v[2].u=0; v[2].v=1;
	v[3].x=1; v[3].y=-1; v[3].z=0; v[3].u=1; v[3].v=1;
	
	rendersys->render(April::TriangleStrip,v,4);
	return true;
}

int main()
{
	April::init("OpenGL",800,600,0,"April: 3D Demo");
	rendersys->registerUpdateCallback(render);

	tex=rendersys->loadTexture("../media/texture.jpg");

	rendersys->enterMainLoop();
	April::destroy();
	return 0;
}
