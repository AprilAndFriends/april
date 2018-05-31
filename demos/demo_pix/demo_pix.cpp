/// @file
/// @version 4.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#ifndef _ANDROID
#ifndef _WINRT
#define RESOURCE_PATH "../../demos/media/"
#else
#define RESOURCE_PATH "media/"
#endif
#elif defined(__APPLE__)
#define RESOURCE_PATH "media/"
#else
#define RESOURCE_PATH "./"
#endif

#include <stdlib.h>

#ifdef __APPLE__
#include <unistd.h>
#import <Foundation/Foundation.h>
#import <CoreFoundation/CoreFoundation.h>
#endif

#include <hltypes/hdir.h>
#include <april/april.h>
#include <aprilpix/aprilpix.h>
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

#define LOG_TAG "demo_pix"

april::Cursor* cursor = NULL;
april::Texture* texture = NULL;
#if !defined(_ANDROID) && !defined(_IOS) && !defined(_WINP8)
grectf drawRect(0.0f, 0.0f, 800.0f, 600.0f);
#else
grectf drawRect(0.0f, 0.0f, 480.0f, 320.0f);
#endif
gvec2f offset = drawRect.getSize() * 0.5f;
grectf textureRect;
grectf src(0.0f, 0.0f, 1.0f, 1.0f);
bool mousePressed = false;

class UpdateDelegate : public april::UpdateDelegate
{
	bool onUpdate(float timeDelta)
	{
		april::rendersys->clear();
		april::rendersys->setOrthoProjection(drawRect);
		april::rendersys->drawFilledRect(drawRect, april::Color(96, 96, 96));
		april::rendersys->setTexture(texture);
		april::rendersys->drawTexturedRect(textureRect + offset, src);
		return true;
	}

};

class SystemDelegate : public april::SystemDelegate
{
public:
	SystemDelegate() : april::SystemDelegate()
	{
	}

	void onWindowSizeChanged(int width, int height, bool fullScreen)
	{
		hlog::writef(LOG_TAG, "window size changed: %dx%d", width, height);
		april::rendersys->setViewport(drawRect);
	}

};

class MouseDelegate : public april::MouseDelegate
{
	void onMouseDown(april::Key key)
	{
		offset = april::window->getCursorPosition();
		hlog::writef(LOG_TAG, "- DOWN x: %4.0f y: %4.0f button: %d", offset.x, offset.y, key.value);
		mousePressed = true;
	}

	void onMouseUp(april::Key key)
	{
		gvec2f position = april::window->getCursorPosition();
		hlog::writef(LOG_TAG, "- UP   x: %4.0f y: %4.0f button: %d", position.x, position.y, key.value);
		mousePressed = false;
	}

	void onMouseMove()
	{
		gvec2f position = april::window->getCursorPosition();
		hlog::writef(LOG_TAG, "- MOVE x: %4.0f y: %4.0f", position.x, position.y);
		if (mousePressed)
		{
			offset = position;
		}
	}

	void onMouseCancel(april::Key key)
	{
		hlog::writef(LOG_TAG, "- CANCEL button: %d", key.value);
	}

};

static UpdateDelegate* updateDelegate = NULL;
static SystemDelegate* systemDelegate = NULL;
static MouseDelegate* mouseDelegate = NULL;

#ifdef __APPLE__
void ObjCUtil_setCWD(const char* override_default_dir)
{
	static bool set = 0;
	if (!set || override_default_dir != NULL)
	{
		if (override_default_dir == NULL)
		{
			NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
			const char* dir = [[[NSBundle mainBundle] resourcePath] UTF8String];
			hdir::chdir(dir);
			[pool release];
		}
		else
		{
			hdir::chdir(override_default_dir);
		}
		set = 1;
	}
}
#endif

void __aprilApplicationInit()
{
#ifdef __APPLE__
	// On MacOSX, the current working directory is not set by
	// the Finder, since you are expected to use Core Foundation
	// or ObjC APIs to find files. 
	ObjCUtil_setCWD(NULL);
#endif
	srand((unsigned int)htime());
	updateDelegate = new UpdateDelegate();
	systemDelegate = new SystemDelegate();
	mouseDelegate = new MouseDelegate();
#if defined(_ANDROID) || defined(_IOS)
	drawRect.setSize(april::getSystemInfo().displayResolution);
#endif
	april::init(april::RenderSystemType::Default, april::WindowType::Default);
	april::createRenderSystem();
	april::createWindow((int)drawRect.w, (int)drawRect.h, false, "APRIL: Pix Demo");
	aprilpix::init();
	april::setTextureExtensions(april::getTextureExtensions() + aprilpix::getExtensions());
#ifdef _WINRT
	april::window->setParam("cursor_mappings", "101 " RESOURCE_PATH "cursor\n102 " RESOURCE_PATH "simple");
#endif
	april::window->setUpdateDelegate(updateDelegate);
	april::window->setSystemDelegate(systemDelegate);
	april::window->setMouseDelegate(mouseDelegate);
	cursor = april::window->createCursorFromResource(RESOURCE_PATH "cursor");
	april::window->setCursor(cursor);
	texture = april::rendersys->createTextureFromResource(RESOURCE_PATH "pix", april::Texture::Type::Managed);
	//texture = april::rendersys->createTextureFromResource(RESOURCE_PATH "pvr_RGB4", april::Texture::Type::Managed);
	//texture = april::rendersys->createTextureFromResource(RESOURCE_PATH "pvr_RGBA4", april::Texture::Type::Managed);
	//texture = april::rendersys->createTextureFromResource(RESOURCE_PATH "pvr_RGB2", april::Texture::Type::Managed);
	//texture = april::rendersys->createTextureFromResource(RESOURCE_PATH "pvr_RGBA2", april::Texture::Type::Managed);
	textureRect.setSize(texture->getWidth() * 0.5f, texture->getHeight() * 0.5f);
	textureRect.x = -textureRect.w * 0.5f;
	textureRect.y = -textureRect.h * 0.5f;
}

void __aprilApplicationDestroy()
{
	april::window->setCursor(NULL);
	april::window->destroyCursor(cursor);
	cursor = NULL;
	april::rendersys->destroyTexture(texture);
	texture = NULL;
	aprilpix::destroy();
	april::destroy();
	delete updateDelegate;
	updateDelegate = NULL;
	delete systemDelegate;
	systemDelegate = NULL;
	delete mouseDelegate;
	mouseDelegate = NULL;
}
