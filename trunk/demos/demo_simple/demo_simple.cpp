/************************************************************************************\
This source file is part of the Awesome Portable Rendering Interface Library         *
For latest info, see http://libapril.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes, Boris Mikic                                        *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#include <stdio.h>

#include <april/RenderSystem.h>
#include <april/Window.h>
#include <april/main.h>
#include <gtypes/Rectangle.h>
#include <gtypes/Vector2.h>

april::Texture* texture;
april::TexturedVertex v[4];
grect drawRect(0.0f, 0.0f, 800.0f, 600.0f);
gvec2 offset(drawRect.w / 2, drawRect.h / 2);
gvec2 size;
bool mousePressed = false;

bool update(float k)
{
	april::rendersys->clear();
	april::rendersys->setOrthoProjection(drawRect);
	april::rendersys->setTexture(texture);
	v[0].x = offset.x - size.x;	v[0].y = offset.y - size.y;	v[0].z = 0.0f;	v[0].u = 0.0f;	v[0].v = 0.0f;
	v[1].x = offset.x + size.x;	v[1].y = offset.y - size.y;	v[1].z = 0.0f;	v[1].u = 1.0f;	v[1].v = 0.0f;
	v[2].x = offset.x - size.x;	v[2].y = offset.y + size.y;	v[2].z = 0.0f;	v[2].u = 0.0f;	v[2].v = 1.0f;
	v[3].x = offset.x + size.x;	v[3].y = offset.y + size.y;	v[3].z = 0.0f;	v[3].u = 1.0f;	v[3].v = 1.0f;
	april::rendersys->render(april::TriangleStrip, v, 4);
	return true;
}

void onMouseDown(float x, float y, int button)
{
	printf("dn x: %4.0f y: %4.0f button: %d\n", x, y, button);
	mousePressed = true;
	offset.x = x;
	offset.y = y;
}

void onMouseUp(float x, float y, int button)
{
	printf("up x: %4.0f y: %4.0f button: %d\n", x, y, button);
	mousePressed = false;
}

void onMouseMove(float x, float y)
{
	printf("mv x: %4.0f y: %4.0f\n", x, y);
	if (mousePressed)
	{
		offset.x = x;
		offset.y = y;
	}
}

int main(int argc, char** argv)
{
	april::init("april", drawRect.w, drawRect.h, false, "april: Simple Demo");
	april::rendersys->getWindow()->setUpdateCallback(update);
	april::rendersys->getWindow()->setMouseCallbacks(onMouseDown, onMouseUp, onMouseMove);
	texture = april::rendersys->loadTexture("../media/texture.jpg");
	size.x = texture->getWidth() / 4;
	size.y = texture->getHeight() / 4;
	april::rendersys->getWindow()->enterMainLoop();
	april::destroy();
	return 0;
}
