/// @file
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @version 1.4
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include <april/main.h>
#include <april/RenderSystem.h>
#include <april/Window.h>

april::Texture* tex;
april::TexturedVertex v[4];
	
bool render(float time_increase)
{
	april::rendersys->clear(true, true);
	static float angle = 0.0f;
	angle += time_increase * 90.0f;
	april::rendersys->setPerspective(60.0f, 800 / 600.0f, 1.0f, 1000.0f);
	april::rendersys->setTexture(tex);
	
	april::rendersys->lookAt(gvec3(2, 2, -5), gvec3(0, 0, 0), gvec3(0, 1, 0));
	april::rendersys->rotate(angle, 0, 1, 0);
	
	april::rendersys->render(april::TriangleStrip, v, 4);
	return true;
}

void april_init(const harray<hstr>& args)
{
	april::init();
	april::createRenderSystem("");
	april::createRenderTarget(800, 600, 0, "april: Simple Demo");
	april::rendersys->getWindow()->setUpdateCallback(render);

	tex = april::rendersys->loadTexture("../media/texture.jpg");

	v[0].x = -1.0f;	v[0].y = 1.0f;	v[0].z = 0.0f;	v[0].u = 0.0f;	v[0].v = 0.0f;
	v[1].x = 1.0f;	v[1].y = 1.0f;	v[1].z = 0.0f;	v[1].u = 1.0f;	v[1].v = 0.0f;
	v[2].x = -1.0f;	v[2].y = -1.0f;	v[2].z = 0.0f;	v[2].u = 0.0f;	v[2].v = 1.0f;
	v[3].x = 1.0f;	v[3].y = -1.0f;	v[3].z = 0.0f;	v[3].u = 1.0f;	v[3].v = 1.0f;
}

void april_destroy()
{
	april::destroy();
}
