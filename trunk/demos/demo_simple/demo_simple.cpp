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
#include <time.h>

#include <april/main.h>
#include <april/RenderSystem.h>
#include <april/Window.h>
#include <gtypes/Rectangle.h>
#include <gtypes/Vector2.h>
#include <hltypes/util.h>

april::Texture* texture = NULL;
april::Texture* manualTexture = NULL;
april::TexturedVertex dv[4];
april::TexturedVertex v[4];
grect drawRect(0.0f, 0.0f, 800.0f, 600.0f);
gvec2 offset(drawRect.w / 2, drawRect.h / 2);
gvec2 size;
bool mousePressed = false;

bool update(float k)
{
	april::rendersys->clear();
	april::rendersys->setOrthoProjection(drawRect);
	manualTexture->fillRect(hrand(manualTexture->getWidth()), hrand(manualTexture->getHeight()), hrand(1, 9), hrand(1, 9), april::Color(hrand(255), hrand(255), hrand(255)));
	april::rendersys->setTexture(manualTexture);
	april::rendersys->render(april::TriangleStrip, dv, 4);
	april::rendersys->setTexture(texture);
	v[0].x = offset.x - size.x;	v[0].y = offset.y - size.y;
	v[1].x = offset.x + size.x;	v[1].y = offset.y - size.y;
	v[2].x = offset.x - size.x;	v[2].y = offset.y + size.y;
	v[3].x = offset.x + size.x;	v[3].y = offset.y + size.y;
	april::rendersys->render(april::TriangleStrip, v, 4);
	april::rendersys->drawColoredQuad(grect(600, 450, 100, 75), APRIL_COLOR_RED);
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

void april_init(const harray<hstr>& args)
{
	srand((unsigned int)time(NULL));
	dv[0].x = 0.0f;			dv[0].y = 0.0f;			dv[0].z = 0.0f;	dv[0].u = 0.0f;	dv[0].v = 0.0f;
	dv[1].x = drawRect.w;	dv[1].y = 0.0f;			dv[1].z = 0.0f;	dv[1].u = 1.0f;	dv[1].v = 0.0f;
	dv[2].x = 0.0f;			dv[2].y = drawRect.h;	dv[2].z = 0.0f;	dv[2].u = 0.0f;	dv[2].v = 1.0f;
	dv[3].x = drawRect.w;	dv[3].y = drawRect.h;	dv[3].z = 0.0f;	dv[3].u = 1.0f;	dv[3].v = 1.0f;
	v[0].z = 0.0f;	v[0].u = 0.0f;	v[0].v = 0.0f;
	v[1].z = 0.0f;	v[1].u = 1.0f;	v[1].v = 0.0f;
	v[2].z = 0.0f;	v[2].u = 0.0f;	v[2].v = 1.0f;
	v[3].z = 0.0f;	v[3].u = 1.0f;	v[3].v = 1.0f;
	april::init();
	april::createRenderSystem("");
	april::createRenderTarget((int)drawRect.w, (int)drawRect.h, false, "april: Simple Demo");
	april::rendersys->getWindow()->setUpdateCallback(update);
	april::rendersys->getWindow()->setMouseCallbacks(onMouseDown, onMouseUp, onMouseMove);
	texture = april::rendersys->loadTexture("../media/texture.jpg");
	manualTexture = april::rendersys->createEmptyTexture((int)drawRect.w, (int)drawRect.h);
	manualTexture->blit(100, 100, texture, 0, 0, texture->getWidth(), texture->getHeight());
	manualTexture->stretchBlit(0, 100, 900, 200, texture, 0, 0, texture->getWidth() / 2, texture->getHeight() / 2);
	size.x = texture->getWidth() / 4.0f;
	size.y = texture->getHeight() / 4.0f;
}

void april_destroy()
{
	delete texture;
	delete manualTexture;
	april::destroy();
}
