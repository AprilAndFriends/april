/// @file
/// @version 3.5
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include <april/april.h>
#include <april/Cursor.h>
#include <april/main.h>
#include <april/MouseDelegate.h>
#include <april/Platform.h>
#include <april/RenderSystem.h>
#include <april/SystemDelegate.h>
#include <april/UpdateDelegate.h>
#include <april/Window.h>
#include <gtypes/Rectangle.h>
#include <gtypes/Vector2.h>
#include <hltypes/hlog.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hstring.h>

#include "CustomRenderSystem.h"
#include "CustomWindow.h"

#define RESOURCE_PATH "../../demos/media/"

static april::Texture* texture = NULL;
static grect drawRect(0.0f, 0.0f, 800.0f, 600.0f);
static gvec2 offset = drawRect.getSize() * 0.5f;
static grect textureRect;
static bool mousePressed = false;

class UpdateDelegate : public april::UpdateDelegate
{
	bool onUpdate(float timeDelta)
	{
		april::rendersys->clear();
		april::rendersys->setOrthoProjection(drawRect);
		april::rendersys->drawFilledRect(drawRect, april::Color(64, 64, 64));
		april::rendersys->setTexture(texture);
		april::rendersys->drawTexturedRect(textureRect + offset, grect(0.0f, 0.0f, 1.0f, 1.0f));
		april::rendersys->drawFilledRect(grect(0.0f, 0.0f, 100.0f, 75.0f), april::Color::Orange);
		april::rendersys->drawFilledRect(grect(10.0f, 10.0f, 80.0f, 55.0f), april::Color::Yellow);
		return true;
	}

} updateDelegate;

class SystemDelegate : public april::SystemDelegate
{
public:
	SystemDelegate() : april::SystemDelegate()
	{
	}

} systemDelegate;

class MouseDelegate : public april::MouseDelegate
{
	void onMouseDown(april::Key key)
	{
		offset = april::window->getCursorPosition();
		mousePressed = true;
	}

	void onMouseUp(april::Key key)
	{
		mousePressed = false;
	}

	void onMouseMove()
	{
		if (mousePressed)
		{
			offset = april::window->getCursorPosition();
		}
	}

} mouseDelegate;

void april_init(const harray<hstr>& args)
{
	srand((unsigned int)htime());
	april::init(new CustomRenderSystem(), new CustomWindow());
	april::createRenderSystem();
	april::createWindow((int)drawRect.w, (int)drawRect.h, false, "APRIL: Simple Interface");
	april::window->setUpdateDelegate(&updateDelegate);
	april::window->setSystemDelegate(&systemDelegate);
	april::window->setMouseDelegate(&mouseDelegate);
	texture = april::rendersys->createTextureFromResource(RESOURCE_PATH "jpt_final", april::Texture::TYPE_MANAGED);
	textureRect.setSize(texture->getWidth() * 0.5f, texture->getHeight() * 0.5f);
	textureRect.x = -textureRect.w / 2;
	textureRect.y = -textureRect.h / 2;
}

void april_destroy()
{
	delete texture;
	april::destroy();
}
