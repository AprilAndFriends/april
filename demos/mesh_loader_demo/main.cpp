/// @file
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @version 1.4
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include <iostream>

#include <april/main.h>
#include <april/RenderSystem.h>
#include <april/Window.h>
#include <april/Timer.h>
#include <aprilutil/StaticMesh.h>
#include <gtypes/Rectangle.h>
#include <gtypes/Vector3.h>

april::Texture* texture;
april::StaticMesh* mesh;
grect drawRect(0.0f, 0.0f, 800.0f, 600.0f);
float angle = 0;

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
	april::init();
	april::createRenderSystem("");
	april::createRenderTarget((int)drawRect.w, (int)drawRect.h, false, "april: 3D Demo");
	april::rendersys->getWindow()->setUpdateCallback(update);
	mesh = new april::StaticMesh("../media/testobject.obj");
	texture = april::rendersys->loadTexture("../media/texture.jpg");
}

void april_destroy()
{
    delete mesh;
	april::destroy();
}
