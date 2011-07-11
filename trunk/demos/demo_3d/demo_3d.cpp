/************************************************************************************\
This source file is part of the Awesome Portable Rendering Interface Library         *
For latest info, see http://libapril.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes, Boris Mikic                                        *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#include <april/main.h>
#include <april/RenderSystem.h>
#include <april/Window.h>

april::Texture* tex;

bool render(float time_increase)
{
	april::rendersys->clear(true, true);
	static float angle=0;
	angle+=time_increase*90;
	april::rendersys->setPerspective(60.0f,800/600.0f,1.0f,1000.0f);
	april::rendersys->setTexture(tex);
	
	april::rendersys->lookAt(gtypes::Vector3(2,2,-5),gtypes::Vector3(0,0,0),gtypes::Vector3(0,1,0));
	april::rendersys->rotate(angle,0,1,0);
	
	april::TexturedVertex v[4];
	
	v[0].x=-1;   v[0].y=1;   v[0].z=0; v[0].u=0; v[0].v=0;
	v[1].x=1; v[1].y=1;   v[1].z=0; v[1].u=1; v[1].v=0;
	v[2].x=-1;   v[2].y=-1; v[2].z=0; v[2].u=0; v[2].v=1;
	v[3].x=1; v[3].y=-1; v[3].z=0; v[3].u=1; v[3].v=1;
	
	april::rendersys->render(april::TriangleStrip,v,4);
	return true;
}

void april_init(const harray<hstr>& args)
{
	april::init("april",800,600,0,"april: 3D Demo");
	april::rendersys->getWindow()->setUpdateCallback(render);

	tex=april::rendersys->loadTexture("../media/texture.jpg");
}

void april_destroy()
{
	april::destroy();
}
