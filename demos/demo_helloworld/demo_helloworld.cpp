/// @file
/// @version 3.6
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

#include <april/april.h>
#include <april/Cursor.h>
#include <april/KeyboardDelegate.h>
#include <april/main.h>
#include <april/MouseDelegate.h>
#include <april/Platform.h>
#include <april/RenderSystem.h>
#include <april/SystemDelegate.h>
#include <april/UpdateDelegate.h>
#include <april/Window.h>
#include <gtypes/Rectangle.h>
#include <hltypes/hlog.h>
#include <hltypes/hstring.h>

#define LOG_TAG "demo_helloworld"

april::Cursor* cursor = NULL;
april::Texture* ball = NULL;
april::TexturedVertex v[4];

#if !defined(_ANDROID) && !defined(_IOS) && !defined(_WINP8)
grect drawRect(0.0f, 0.0f, 800.0f, 600.0f);
#else
grect drawRect(0.0f, 0.0f, 480.0f, 320.0f);
#endif
gvec2 size = drawRect.getSize() * 5 / 16;

class Ball
{
private:
	gvec2 position;
	gvec2 velocity;

	const float size = 96;
	const float speed = 256;
public:
	Ball()
	{
		position.set(hrand(0, april::window->getSize().x - size), 
			hrand(0, april::window->getSize().y - size));

		velocity.set(speed, speed);
	}

	void update(float timeDelta)
	{
		position += velocity * timeDelta;

		if (position.x < 0 || position.x > april::window->getSize().x - size)
		{
			position -= velocity * timeDelta;
			velocity.x = -velocity.x;
		}

		if (position.y < 0 || position.y > april::window->getSize().y - size)
		{
			position -= velocity * timeDelta;
			velocity.y = -velocity.y;
		}
	}

	void render()
	{
		float x1 = position.x;
		float x2 = position.x + size;
		float y1 = position.y;
		float y2 = position.y + size;

		april::rendersys->setTexture(ball);

		v[0].x = x1; v[0].y = y1; v[0].z = 0; v[0].u = 0; v[0].v = 0;
		v[1].x = x2; v[1].y = y1; v[1].z = 0; v[1].u = 1; v[1].v = 0;
		v[2].x = x1; v[2].y = y2; v[2].z = 0; v[2].u = 0; v[2].v = 1;
		v[3].x = x2; v[3].y = y2; v[3].z = 0; v[3].u = 1; v[3].v = 1;
		april::rendersys->render(april::RO_TRIANGLE_STRIP, v, 4);
	}
};

harray<Ball> balls;

class UpdateDelegate : public april::UpdateDelegate
{
	bool onUpdate(float timeDelta)
	{	
		april::rendersys->clear();
		april::rendersys->setOrthoProjection(drawRect);	

		foreach(Ball, it, balls)
		{
			it->update(timeDelta);
			it->render();
		}
	
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

static UpdateDelegate* updateDelegate = NULL;
static SystemDelegate* systemDelegate = NULL;

void april_init(const harray<hstr>& args)
{
#ifdef __APPLE__
	// On MacOSX, the current working directory is not set by
	// the Finder, since you are expected to use Core Foundation
	// or ObjC APIs to find files. 
	// So, when porting you probably want to set the current working
	// directory to something sane (e.g. .../Resources/ in the app
	// bundle).
	// In this case, we set it to parent of the .app bundle.
	{	// curly braces in order to localize variables 

		CFURLRef url = CFBundleCopyBundleURL(CFBundleGetMainBundle());
		CFStringRef path = CFURLCopyFileSystemPath(url, kCFURLPOSIXPathStyle);
		// let's hope chdir() will be happy with utf8 encoding
		const char* cpath = CFStringGetCStringPtr(path, kCFStringEncodingUTF8);
		char* cpath_alloc = NULL;
		if (cpath == NULL)
		{
			// CFStringGetCStringPtr is allowed to return NULL. bummer.
			// we need to use CFStringGetCString instead.
			cpath_alloc = (char*)malloc(CFStringGetLength(path) + 1);
			CFStringGetCString(path, cpath_alloc, CFStringGetLength(path) + 1, kCFStringEncodingUTF8);
		}
		else
		{
			// even though it didn't return NULL, we still want to slice off bundle name.
			cpath_alloc = (char*)malloc(CFStringGetLength(path) + 1);
			strcpy(cpath_alloc, cpath);
		}
		// just in case / is appended to .app path for some reason
		if (cpath_alloc[CFStringGetLength(path) - 1] == '/')
		{
			cpath_alloc[CFStringGetLength(path) - 1] = 0;
		}
		// replace pre-.app / with a null character, thus
		// cutting off .app's name and getting parent of .app.
		strrchr(cpath_alloc, '/')[0] = 0;
		// change current dir using posix api
		chdir(cpath_alloc);
		free(cpath_alloc); // even if null, still ok
		CFRelease(path);
		CFRelease(url);
	}
#endif
	srand((unsigned int)htime());
	updateDelegate = new UpdateDelegate();
	systemDelegate = new SystemDelegate();	
#if defined(_ANDROID) || defined(_IOS) || defined(_WINRT)
	drawRect.setSize(april::getSystemInfo().displayResolution);
#endif
	april::init(april::RS_DEFAULT, april::WS_DEFAULT);
	april::createRenderSystem();

	april::Window::Options windowOptions;
	windowOptions.resizable = true;

	april::createWindow((int)drawRect.w, (int)drawRect.h, false, "APRIL: Hello World Demo", windowOptions);
#ifdef _WINRT
	april::window->setParam("cursor_mappings", "101 " RESOURCE_PATH "cursor\n102 " RESOURCE_PATH "simple");
#endif
	april::window->setUpdateDelegate(updateDelegate);
	april::window->setSystemDelegate(systemDelegate);
	
	cursor = april::window->createCursor(RESOURCE_PATH "cursor");
	april::window->setCursor(cursor);
	ball = april::rendersys->createTextureFromResource(RESOURCE_PATH "logo");	

	balls.add(Ball());
}

void april_destroy()
{
	april::window->setCursor(NULL);
	delete cursor;
	april::rendersys->destroyTexture(ball);
	ball = NULL;
	
	delete updateDelegate;
	updateDelegate = NULL;
	delete systemDelegate;
	systemDelegate = NULL;	
}
