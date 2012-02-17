/// @file
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @version 1.5
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef _ANDROID
#define APRIL_ANDROID_PACKAGE_NAME "com/example/april/demo3d"
#define RESOURCE_PATH "./"
#else
#define RESOURCE_PATH "../media/"
#endif

#include <april/main.h>
#include <april/RenderSystem.h>
#include <april/Window.h>

april::Texture* texture = NULL;
april::TexturedVertex v[4];

#ifndef _ANDROID
grect drawRect(0.0f, 0.0f, 800.0f, 600.0f);
#else
grect drawRect(0.0f, 0.0f, 480.0f, 320.0f);
#endif
	
bool render(float k)
{
	static float angle = 0.0f;
	angle += k * 90.0f;
	april::rendersys->clear();
	april::rendersys->setPerspective(60.0f, drawRect.getAspect(), 1.0f, 1000.0f);
	april::rendersys->lookAt(gvec3(2, 2, -5), gvec3(0, 0, 0), gvec3(0, 1, 0));
	april::rendersys->rotate(angle, 0, 1, 0);
	april::rendersys->setTexture(texture);
	april::rendersys->render(april::TriangleStrip, v, 4);
	return true;
}

void april_init(const harray<hstr>& args)
{
	april::init();
	april::createRenderSystem("");
	april::createRenderTarget((int)drawRect.w, (int)drawRect.h, 0, "april: Simple 3D");
	april::rendersys->getWindow()->setUpdateCallback(render);
	texture = april::rendersys->loadTexture(RESOURCE_PATH "texture.png");
	v[0].x = -1.0f;	v[0].y = 1.0f;	v[0].z = 0.0f;	v[0].u = 0.0f;	v[0].v = 0.0f;
	v[1].x = 1.0f;	v[1].y = 1.0f;	v[1].z = 0.0f;	v[1].u = 1.0f;	v[1].v = 0.0f;
	v[2].x = -1.0f;	v[2].y = -1.0f;	v[2].z = 0.0f;	v[2].u = 0.0f;	v[2].v = 1.0f;
	v[3].x = 1.0f;	v[3].y = -1.0f;	v[3].z = 0.0f;	v[3].u = 1.0f;	v[3].v = 1.0f;
}

void april_destroy()
{
	delete texture;
	april::destroy();
}
