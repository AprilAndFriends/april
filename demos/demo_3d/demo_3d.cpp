/// @file
/// @version 4.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#ifndef __ANDROID__
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

#include <april/april.h>
#include <april/Cursor.h>
#include <april/main.h>
#include <april/MouseDelegate.h>
#include <april/Platform.h>
#include <april/RenderSystem.h>
#include <april/SystemDelegate.h>
#include <april/UpdateDelegate.h>
#include <april/Window.h>
#include <hltypes/hlog.h>

#define LOG_TAG "demo_3d"

#define _COPY_VERTICES(dest, src, start) \
	memcpy(&dest[start], &src[0], 3 * sizeof(april::TexturedVertex)); \
	memcpy(&dest[start + 3], &src[1], 3 * sizeof(april::TexturedVertex));

april::Cursor* cursor = NULL;
april::Texture* texture = NULL;
april::Texture* logo = NULL;
april::TexturedVertex v[36];
gvec2f cameraPosition(90.0f, 60.0f);
gvec2f clickPosition;

#if !defined(__ANDROID__) && !defined(_IOS) && !defined(_WINP8)
grectf drawRect(0.0f, 0.0f, 800.0f, 600.0f);
#else
grectf drawRect(0.0f, 0.0f, 480.0f, 320.0f);
#endif

class MouseDelegate : public april::MouseDelegate
{
public:
	MouseDelegate() : april::MouseDelegate(), pressed(false)
	{
	}

	HL_DEFINE_IS(pressed, Pressed);

	void onMouseDown(april::Key keyCode)
	{
		this->pressed = true;
		clickPosition = april::window->getCursorPosition();
	}

	void onMouseUp(april::Key keyCode)
	{
		this->pressed = false;
		cameraPosition += april::window->getCursorPosition() - clickPosition;
		cameraPosition.y = hclamp(cameraPosition.y, -90.0f, 90.0f);
	}

	void onMouseMove()
	{
	}

protected:
	bool pressed;

};

static MouseDelegate* mouseDelegate = NULL;

class UpdateDelegate : public april::UpdateDelegate
{
public:
	UpdateDelegate() : april::UpdateDelegate()
	{
	}

	bool onUpdate(float timeDelta)
	{
		static gvec3f eye(0.0f, 0.0f, 6.0f);
		static gvec3f target(0.0f, 0.0f, 0.0f);
		static gvec3f up(0.0f, 1.0f, 0.0f);
		april::rendersys->clear(april::Color::Clear, true);
		april::rendersys->lookAt(eye, target, up);
		april::rendersys->setPerspective(60.0f, 1.0f / drawRect.getAspect(), 1.0f, 1000.0f);
		gvec2f position = cameraPosition;
		if (mouseDelegate->isPressed())
		{
			position += april::window->getCursorPosition() - clickPosition;
		}
		april::rendersys->rotate(1.0f, 0.0f, 0.0f, hclamp(position.y * 0.5f, -90.0f, 90.0f));
		april::rendersys->rotate(0.0f, 1.0f, 0.0f, position.x * 0.5f);
		april::rendersys->setTexture(texture);
		april::rendersys->render(april::RenderOperation::TriangleList, v, 36);
		april::rendersys->setTexture(logo);
		april::rendersys->render(april::RenderOperation::TriangleList, v, 36);
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

void __aprilApplicationInit()
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
	mouseDelegate = new MouseDelegate();
#if defined(__ANDROID__) || defined(_IOS)
	drawRect.setSize(april::getSystemInfo().displayResolution);
#endif
	april::init(april::RenderSystemType::Default, april::WindowType::Default);
	april::RenderSystem::Options options;
	options.depthBuffer = true;
	april::createRenderSystem(options);
	april::createWindow((int)drawRect.w, (int)drawRect.h, false, "APRIL: Simple 3D");
#ifdef _WINRT
	april::window->setParam("cursor_mappings", "101 " RESOURCE_PATH "cursor\n102 " RESOURCE_PATH "simple");
#endif
	april::rendersys->setDepthBuffer(true, true);
	april::window->setUpdateDelegate(updateDelegate);
	april::window->setSystemDelegate(systemDelegate);
	april::window->setMouseDelegate(mouseDelegate);
	cursor = april::window->createCursorFromResource(RESOURCE_PATH "cursor");
	april::window->setCursor(cursor);
	texture = april::rendersys->createTextureFromResource(RESOURCE_PATH "texture");
	logo = april::rendersys->createTextureFromResource(RESOURCE_PATH "logo");
	gvec3f _v[8];
	_v[0].x = -1.0f;	_v[0].y = -1.0f;	_v[0].z = 1.0f;
	_v[1].x = 1.0f;		_v[1].y = -1.0f;	_v[1].z = 1.0f;
	_v[2].x = -1.0f;	_v[2].y = 1.0f;		_v[2].z = 1.0f;
	_v[3].x = 1.0f;		_v[3].y = 1.0f;		_v[3].z = 1.0f;
	_v[4].x = -1.0f;	_v[4].y = -1.0f;	_v[4].z = -1.0f;
	_v[5].x = 1.0f;		_v[5].y = -1.0f;	_v[5].z = -1.0f;
	_v[6].x = -1.0f;	_v[6].y = 1.0f;		_v[6].z = -1.0f;
	_v[7].x = 1.0f;		_v[7].y = 1.0f;		_v[7].z = -1.0f;
	april::TexturedVertex _side[4];
	_side[0].u = 0.0f;	_side[0].v = 0.0f;
	_side[1].u = 1.0f;	_side[1].v = 0.0f;
	_side[2].u = 0.0f;	_side[2].v = 1.0f;
	_side[3].u = 1.0f;	_side[3].v = 1.0f;
	// front
	_side[0].set(_v[0]);	_side[1].set(_v[1]);	_side[2].set(_v[2]);	_side[3].set(_v[3]);	_COPY_VERTICES(v, _side, 0);
	// back
	_side[0].set(_v[5]);	_side[1].set(_v[4]);	_side[2].set(_v[7]);	_side[3].set(_v[6]);	_COPY_VERTICES(v, _side, 6);
	// top
	_side[0].set(_v[4]);	_side[1].set(_v[5]);	_side[2].set(_v[0]);	_side[3].set(_v[1]);	_COPY_VERTICES(v, _side, 12);
	// bottom
	_side[0].set(_v[6]);	_side[1].set(_v[7]);	_side[2].set(_v[2]);	_side[3].set(_v[3]);	_COPY_VERTICES(v, _side, 18);
	// left
	_side[0].set(_v[4]);	_side[1].set(_v[0]);	_side[2].set(_v[6]);	_side[3].set(_v[2]);	_COPY_VERTICES(v, _side, 24);
	// right
	_side[0].set(_v[5]);	_side[1].set(_v[1]);	_side[2].set(_v[7]);	_side[3].set(_v[3]);	_COPY_VERTICES(v, _side, 30);
}

void __aprilApplicationDestroy()
{
	april::window->setCursor(NULL);
	april::window->destroyCursor(cursor);
	april::rendersys->destroyTexture(texture);
	texture = NULL;
	april::rendersys->destroyTexture(logo);
	logo = NULL;
	april::destroy();
	delete updateDelegate;
	updateDelegate = NULL;
	delete systemDelegate;
	systemDelegate = NULL;
}
