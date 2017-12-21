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

#include <april/april.h>
#include <april/Cursor.h>
#include <april/KeyDelegate.h>
#include <april/main.h>
#include <april/MotionDelegate.h>
#include <april/MouseDelegate.h>
#include <april/Platform.h>
#include <april/RenderSystem.h>
#include <april/SystemDelegate.h>
#include <april/UpdateDelegate.h>
#include <april/Window.h>
#include <gtypes/Rectangle.h>
#include <hltypes/hlog.h>
#include <hltypes/hstring.h>

#define LOG_TAG "demo_motion"

april::Cursor* cursor = NULL;
april::Texture* ball = NULL;
april::TexturedVertex v[4];

#if !defined(_ANDROID) && !defined(_IOS) && !defined(_WINP8)
grect drawRect(0.0f, 0.0f, 800.0f, 600.0f);
#else
grect drawRect(0.0f, 0.0f, 480.0f, 320.0f);
#endif
gvec2 size = drawRect.getSize() * 5 / 16;
april::Color backgroundColor = april::Color::Black;
gvec3 accelerometer(0.0f, 0.0f, 0.0f);
gvec2 sizeFactor(0.001f, 0.001f); // 1 px is 1mm
float ballElasticityFactor = 0.5f;

class Ball
{
public:
	Ball()
	{
		this->position.set((float)(int)((drawRect.w - size) * 0.5f), (float)(int)((drawRect.h - size) * 0.5f));
	}

	void update(float timeDelta)
	{
		gvec2 screenSize(drawRect.w - size, drawRect.h - size);
		this->velocity += gvec2(accelerometer.x, -accelerometer.y) / sizeFactor * timeDelta; // (x, -y) because of screen space and accelerometer vector direction
		this->velocity *= 0.995f; // friction
		this->position += this->velocity * timeDelta;
		if (this->position.x < 0)
		{
			this->position.x = -this->position.x * ballElasticityFactor;
			this->velocity.x = -this->velocity.x * ballElasticityFactor;
		}
		else if (this->position.x > screenSize.x)
		{
			this->position.x = screenSize.x + (screenSize.x - this->position.x) * ballElasticityFactor;
			this->velocity.x = -this->velocity.x * ballElasticityFactor;
		}
		if (this->position.y < 0)
		{
			this->position.y = -this->position.y * ballElasticityFactor;
			this->velocity.y = -this->velocity.y * ballElasticityFactor;
		}
		else if (this->position.y > screenSize.y)
		{
			this->position.y = screenSize.y + (screenSize.y - this->position.y) * ballElasticityFactor;
			this->velocity.y = -this->velocity.y * ballElasticityFactor;
		}
	}

	void render()
	{
		float x1 = this->position.x;
		float x2 = this->position.x + size;
		float y1 = this->position.y;
		float y2 = this->position.y + size;
		april::rendersys->setTexture(ball);
		v[0].x = x1; v[0].y = y1; v[0].z = 0; v[0].u = 0; v[0].v = 0;
		v[1].x = x2; v[1].y = y1; v[1].z = 0; v[1].u = 1; v[1].v = 0;
		v[2].x = x1; v[2].y = y2; v[2].z = 0; v[2].u = 0; v[2].v = 1;
		v[3].x = x2; v[3].y = y2; v[3].z = 0; v[3].u = 1; v[3].v = 1;
		april::rendersys->render(april::RenderOperation::TriangleStrip, v, 4);
	}

protected:
	gvec2 position;
	gvec2 velocity;

	static const int size = 96;

};

harray<Ball> balls;

class UpdateDelegate : public april::UpdateDelegate
{
	bool onUpdate(float timeDelta)
	{
		april::rendersys->clear();
		april::rendersys->setOrthoProjection(drawRect);
		april::rendersys->drawFilledRect(drawRect, backgroundColor);
		foreach (Ball, it, balls)
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

class MotionDelegate : public april::MotionDelegate
{
public:
	MotionDelegate() : april::MotionDelegate()
	{
		this->accelerometerEnabled = true;
	}

	void onAccelerometer(cgvec3 motionVector)
	{
		hlog::writef(LOG_TAG, "motion vector: %g,%g,%g", motionVector.x, motionVector.y, motionVector.z);
		accelerometer = motionVector;
	}

};

static UpdateDelegate* updateDelegate = NULL;
static SystemDelegate* systemDelegate = NULL;
static MotionDelegate* motionDelegate = NULL;

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
	motionDelegate = new MotionDelegate();
#if defined(_ANDROID) || defined(_IOS)
	gvec2 resolution = april::getSystemInfo().displayResolution;
	hswap(resolution.x, resolution.y);
	drawRect.setSize(resolution);
#endif
	april::init(april::RenderSystemType::Default, april::WindowType::Default);
	april::createRenderSystem();
	april::Window::Options windowOptions;
	windowOptions.resizable = true;
	april::createWindow((int)drawRect.w, (int)drawRect.h, false, "APRIL: Motion Demo", windowOptions);
#ifdef _WINRT
	april::window->setParam("cursor_mappings", "101 " RESOURCE_PATH "cursor\n102 " RESOURCE_PATH "simple");
#endif
	april::window->setUpdateDelegate(updateDelegate);
	april::window->setSystemDelegate(systemDelegate);
	april::window->setMotionDelegate(motionDelegate);
	cursor = april::window->createCursorFromResource(RESOURCE_PATH "cursor");
	april::window->setCursor(cursor);
	ball = april::rendersys->createTextureFromResource(RESOURCE_PATH "logo");
	balls.add(Ball());
}

void __aprilApplicationDestroy()
{
	april::window->setCursor(NULL);
	april::window->destroyCursor(cursor);
	cursor = NULL;
	april::rendersys->destroyTexture(ball);
	ball = NULL;
	delete motionDelegate;
	motionDelegate = NULL;
	delete systemDelegate;
	systemDelegate = NULL;
	delete updateDelegate;
	updateDelegate = NULL;
}
