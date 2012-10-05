/// @file
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @version 1.7
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef _ANDROID
#define RESOURCE_PATH "./"
#else
#define RESOURCE_PATH "../media/"
#endif

#include <april/april.h>
#include <april/main.h>
#include <april/RenderSystem.h>
#include <april/Timer.h>
#include <april/Window.h>
#include <aprilutil/StaticMesh.h>
#include <gtypes/Rectangle.h>
#include <gtypes/Vector3.h>

april::Texture* texture;
april::StaticMesh* mesh;
float angle = 0;
#ifndef _ANDROID
grect drawRect(0.0f, 0.0f, 800.0f, 600.0f);
#else
grect drawRect(0.0f, 0.0f, 480.0f, 320.0f);
#endif

bool update(float k)
{
	april::rendersys->clear(true, true);
	angle += k * 90;
	april::rendersys->setPerspective(60, drawRect.getAspect(), 0.1f, 100.0f);
	april::rendersys->setTexture(texture);
	april::rendersys->lookAt(gvec3(2, 2, -5), gvec3(), gvec3(0, 1, 0));
	april::rendersys->rotate(angle, 0, 1, 0);
    mesh->draw(april::TriangleList);
	return true;
}

void april_init(const harray<hstr>& args)
{
	april::init(april::RS_DEFAULT, april::WS_DEFAULT);
	april::createRenderSystem();
	april::createWindow((int)drawRect.w, (int)drawRect.h, false, "april: Mesh Demo");
	april::window->setUpdateCallback(update);
	mesh = new april::StaticMesh(RESOURCE_PATH "testobject.obj");
	texture = april::rendersys->loadTexture(RESOURCE_PATH "texture");
}

void april_destroy()
{
    delete mesh;
	april::destroy();
}
