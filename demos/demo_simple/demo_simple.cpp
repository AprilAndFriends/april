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

#define LOG_TAG "demo_simple"

#define _ENGINE_RENDER_TEST

april::Cursor* cursor = NULL;
april::Texture* texture = NULL;
april::Texture* manualTexture = NULL;
april::TexturedVertex dv[4];
april::PlainVertex pv[4];
april::TexturedVertex tv[4];
april::ColoredVertex cv[3];
april::ColoredTexturedVertex ctv[3];

#if !defined(_ANDROID) && !defined(_IOS) && !defined(_WINP8)
grect drawRect(0.0f, 0.0f, 800.0f, 600.0f);
#else
grect drawRect(0.0f, 0.0f, 480.0f, 320.0f);
#endif
gvec2 offset = drawRect.getSize() * 0.5f;
grect textureRect;
grect src(0.0f, 0.0f, 1.0f, 1.0f);
bool mousePressed = false;

class UpdateDelegate : public april::UpdateDelegate
{
	bool onUpdate(float timeDelta)
	{
		april::rendersys->clear();
		april::rendersys->setOrthoProjection(drawRect);
		/// some general texture testing
		april::rendersys->drawFilledRect(drawRect, april::Color(96, 96, 96));
		manualTexture->fillRect(hrand(manualTexture->getWidth()), hrand(manualTexture->getHeight()), hrand(1, 9), hrand(1, 9), april::Color(hrand(255), hrand(255), hrand(255)));
		april::rendersys->setTexture(manualTexture);
		april::rendersys->render(april::RenderOperation::TriangleStrip, dv, 4);
		april::rendersys->setTexture(texture);
		april::rendersys->drawTexturedRect(textureRect + offset, src);
		april::rendersys->drawFilledRect(grect(0.0f, drawRect.h - 75.0f, 100.0f, 75.0f), april::Color::Yellow);
		april::rendersys->drawFilledRect(grect(70.0f, drawRect.h - 65.0f, 80.0f, 55.0f), april::Color::Red);
#ifdef _ENGINE_RENDER_TEST
		// testing all render methods
		april::rendersys->drawFilledRect(grect(drawRect.w - 110.0f, drawRect.h - 310.0f, 110.0f, 310.0f), april::Color::Black);
		april::rendersys->render(april::RenderOperation::TriangleList, pv, 3);
		april::rendersys->render(april::RenderOperation::TriangleList, &pv[1], 3, april::Color::Yellow);
		april::rendersys->render(april::RenderOperation::TriangleList, tv, 3);
		april::rendersys->render(april::RenderOperation::TriangleList, &tv[1], 3, april::Color::Green);
		april::rendersys->render(april::RenderOperation::TriangleList, cv, 3);
		april::rendersys->render(april::RenderOperation::TriangleList, ctv, 3);
#endif
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
		gvec2 position = april::window->getCursorPosition();
		hlog::writef(LOG_TAG, "- UP   x: %4.0f y: %4.0f button: %d", position.x, position.y, key.value);
		mousePressed = false;
	}

	void onMouseMove()
	{
		gvec2 position = april::window->getCursorPosition();
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
	// init
	april::init(april::RenderSystemType::Default, april::WindowType::Default);
	april::createRenderSystem();
	april::createWindow((int)drawRect.w, (int)drawRect.h, false, "APRIL: Simple Demo");
	// background
	dv[0].x = 0.0f;			dv[0].y = 0.0f;			dv[0].z = 0.0f;	dv[0].u = 0.0f;	dv[0].v = 0.0f;
	dv[1].x = drawRect.w;	dv[1].y = 0.0f;			dv[1].z = 0.0f;	dv[1].u = 1.0f;	dv[1].v = 0.0f;
	dv[2].x = 0.0f;			dv[2].y = drawRect.h;	dv[2].z = 0.0f;	dv[2].u = 0.0f;	dv[2].v = 1.0f;
	dv[3].x = drawRect.w;	dv[3].y = drawRect.h;	dv[3].z = 0.0f;	dv[3].u = 1.0f;	dv[3].v = 1.0f;
	// plain
	pv[0].x = drawRect.w - 100.0f;	pv[0].y = drawRect.h - 300.0f;	pv[0].z = 0.0f;
	pv[1].x = drawRect.w;			pv[1].y = drawRect.h - 300.0f;	pv[1].z = 0.0f;
	pv[2].x = drawRect.w - 100.0f;	pv[2].y = drawRect.h - 200.0f;	pv[2].z = 0.0f;
	pv[3].x = drawRect.w;			pv[3].y = drawRect.h - 200.0f;	pv[3].z = 0.0f;
	// textured
	tv[0].x = drawRect.w - 100.0f;	tv[0].y = drawRect.h - 200.0f;	tv[0].z = 0.0f;	tv[0].u = 0.0f;	tv[0].v = 0.0f;
	tv[1].x = drawRect.w;			tv[1].y = drawRect.h - 200.0f;	tv[1].z = 0.0f;	tv[1].u = 1.0f;	tv[1].v = 0.0f;
	tv[2].x = drawRect.w - 100.0f;	tv[2].y = drawRect.h - 100.0f;	tv[2].z = 0.0f;	tv[2].u = 0.0f;	tv[2].v = 1.0f;
	tv[3].x = drawRect.w;			tv[3].y = drawRect.h - 100.0f;	tv[3].z = 0.0f;	tv[3].u = 1.0f;	tv[3].v = 1.0f;
	// colored
	cv[0].x = drawRect.w - 100.0f;	cv[0].y = drawRect.h - 100.0f;	cv[0].z = 0.0f;	cv[0].color = april::rendersys->getNativeColorUInt(april::Color::Yellow);
	cv[1].x = drawRect.w;			cv[1].y = drawRect.h - 100.0f;	cv[1].z = 0.0f;	cv[1].color = april::rendersys->getNativeColorUInt(april::Color::Red);
	cv[2].x = drawRect.w - 100.0f;	cv[2].y = drawRect.h - 0.0f;	cv[2].z = 0.0f;	cv[2].color = april::rendersys->getNativeColorUInt(april::Color::Green);
	// colored-textured
	ctv[0].x = drawRect.w;			ctv[0].y = drawRect.h - 100.0f;	ctv[0].z = 0.0f;	ctv[0].u = 1.0f;	ctv[0].v = 0.0f;	ctv[0].color = april::rendersys->getNativeColorUInt(april::Color::Red);
	ctv[1].x = drawRect.w - 100.0f;	ctv[1].y = drawRect.h - 0.0f;	ctv[1].z = 0.0f;	ctv[1].u = 0.0f;	ctv[1].v = 1.0f;	ctv[1].color = april::rendersys->getNativeColorUInt(april::Color::Green);
	ctv[2].x = drawRect.w;			ctv[2].y = drawRect.h - 0.0f;	ctv[2].z = 0.0f;	ctv[2].u = 1.0f;	ctv[2].v = 1.0f;	ctv[2].color = april::rendersys->getNativeColorUInt(april::Color::White);
#ifdef _WINRT
	april::window->setParam("cursor_mappings", "101 " RESOURCE_PATH "cursor\n102 " RESOURCE_PATH "simple");
#endif
	april::window->setUpdateDelegate(updateDelegate);
	april::window->setSystemDelegate(systemDelegate);
	april::window->setMouseDelegate(mouseDelegate);
	cursor = april::window->createCursorFromResource(RESOURCE_PATH "cursor");
	april::window->setCursor(cursor);
	texture = april::rendersys->createTextureFromResource(RESOURCE_PATH "logo", april::Texture::Type::Managed);
	textureRect.setSize(texture->getWidth() * 0.5f, texture->getHeight() * 0.5f);
	textureRect.x = -textureRect.w * 0.5f;
	textureRect.y = -textureRect.h * 0.5f;
	// demonstrating some of the image manipulation methods
	manualTexture = april::rendersys->createTexture((int)drawRect.w, (int)drawRect.h, april::Color::Clear, april::Image::Format::RGBA, april::Texture::Type::Managed);
	manualTexture->write(0, 0, texture->getWidth(), texture->getHeight(), 0, 0, texture);
	manualTexture->invert(0, 0, 256, 128);
	manualTexture->saturate(0, 128, 128, 128, 0.0f);
	manualTexture->rotateHue(128, 0, 128, 128, 180.0f);
	manualTexture->blit(0, 0, texture->getWidth(), texture->getHeight(), 128, 128, texture, 96);
	manualTexture->blitStretch(texture->getWidth() / 2, 0, texture->getWidth() / 2, texture->getHeight(), 64, 128, 700, 200, texture, 208);
}

void __aprilApplicationDestroy()
{
	april::window->setCursor(NULL);
	april::window->destroyCursor(cursor);
	cursor = NULL;
	april::rendersys->destroyTexture(texture);
	texture = NULL;
	april::rendersys->destroyTexture(manualTexture);
	manualTexture = NULL;
	april::destroy();
	delete updateDelegate;
	updateDelegate = NULL;
	delete systemDelegate;
	systemDelegate = NULL;
	delete mouseDelegate;
	mouseDelegate = NULL;
}
