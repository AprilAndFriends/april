/// @file
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @version 2.5
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include <hltypes/hplatform.h>
#ifndef _ANDROID
#if !_HL_WINRT
#define RESOURCE_PATH "../media/"
#else
#define RESOURCE_PATH "media/"
#endif
#else
#define RESOURCE_PATH "./"
#endif

#include <stdio.h>
#include <time.h>

#include <april/april.h>
#include <april/main.h>
#include <april/RenderSystem.h>
#include <april/Window.h>
#include <gtypes/Rectangle.h>
#include <gtypes/Vector2.h>
#include <hltypes/hlog.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hstring.h>

#define LOG_TAG "demo_simple"

april::Texture* texture = NULL;
april::Texture* manualTexture = NULL;
april::TexturedVertex dv[4];
#ifndef _ANDROID
grect drawRect(0.0f, 0.0f, 800.0f, 600.0f);
#else
grect drawRect(0.0f, 0.0f, 480.0f, 320.0f);
#endif
gvec2 offset(240.0f, 128.0f);
grect textureRect;
grect src(0.0f, 0.0f, 1.0f, 1.0f);
bool mousePressed = false;

bool update(float k)
{
	april::rendersys->clear();
	april::rendersys->setOrthoProjection(drawRect);
	april::rendersys->drawFilledRect(drawRect, april::Color::Grey);
	manualTexture->fillRect(hrand(manualTexture->getWidth()), hrand(manualTexture->getHeight()), hrand(1, 9), hrand(1, 9), april::Color(hrand(255), hrand(255), hrand(255)));
	april::rendersys->setTexture(manualTexture);
	april::rendersys->render(april::TriangleStrip, dv, 4);
	april::rendersys->setTexture(texture);
	april::rendersys->drawTexturedRect(textureRect + offset, src);
	april::rendersys->drawFilledRect(grect(0.0f, 0.0f, 100.0f, 75.0f), april::Color::Yellow);
	return true;
}

void onMouseDown(int button)
{
	offset = april::window->getCursorPosition();
	hlog::writef(LOG_TAG, "    - DOWN x: %4.0f y: %4.0f button: %d", offset.x, offset.y, button);
	mousePressed = true;
}

void onMouseUp(int button)
{
	gvec2 cursor = april::window->getCursorPosition();
	hlog::writef(LOG_TAG, "    - UP   x: %4.0f y: %4.0f button: %d", cursor.x, cursor.y, button);
	mousePressed = false;
}

void onMouseMove()
{
	gvec2 cursor = april::window->getCursorPosition();
	hlog::writef(LOG_TAG, "    - MOVE x: %4.0f y: %4.0f", cursor.x, cursor.y);
	if (mousePressed)
	{
		offset = cursor;
	}
}

void april_init(const harray<hstr>& args)
{
	srand((unsigned int)time(NULL));
	dv[0].x = 0.0f;			dv[0].y = 0.0f;			dv[0].z = 0.0f;	dv[0].u = 0.0f;	dv[0].v = 0.0f;
	dv[1].x = drawRect.w;	dv[1].y = 0.0f;			dv[1].z = 0.0f;	dv[1].u = 1.0f;	dv[1].v = 0.0f;
	dv[2].x = 0.0f;			dv[2].y = drawRect.h;	dv[2].z = 0.0f;	dv[2].u = 0.0f;	dv[2].v = 1.0f;
	dv[3].x = drawRect.w;	dv[3].y = drawRect.h;	dv[3].z = 0.0f;	dv[3].u = 1.0f;	dv[3].v = 1.0f;
	april::init(april::RS_DEFAULT, april::WS_DEFAULT);
	april::createRenderSystem();
	april::createWindow((int)drawRect.w, (int)drawRect.h, false, "april: Simple Demo");
	april::window->setUpdateCallback(&update);
	april::window->setMouseCallbacks(&onMouseDown, &onMouseUp, &onMouseMove, NULL);
	texture = april::rendersys->loadTexture(RESOURCE_PATH "jpt_final");
	textureRect.setSize(texture->getWidth() * 0.5f, texture->getHeight() * 0.5f);
	textureRect.x = -textureRect.w / 2;
	textureRect.y = -textureRect.h / 2;
	manualTexture = april::rendersys->createTexture((int)drawRect.w, (int)drawRect.h, april::Texture::FORMAT_ARGB);
	manualTexture->blit(100, 100, texture, 0, 0, texture->getWidth(), texture->getHeight());
	manualTexture->stretchBlit(0, 100, 900, 200, texture, 0, 0, texture->getWidth() / 2, texture->getHeight() / 2);
}

void april_destroy()
{
	delete texture;
	delete manualTexture;
	april::destroy();
}
