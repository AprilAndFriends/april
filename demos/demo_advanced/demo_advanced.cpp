/// @file
/// @version 4.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#ifndef __ANDROID__
	#ifndef _UWP
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
#include <april/KeyDelegate.h>
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

#define CIRCLE_VERTEX_COUNT 32
#define CIRCLE_RADIUS 8

april::Cursor* cursor = NULL;
april::Texture* textures[4] = { NULL, NULL, NULL, NULL};

// vertices for render test
april::TexturedVertex dv[4];
april::PlainVertex pv[4];
april::TexturedVertex tv[4];
april::ColoredVertex cv[3];
april::ColoredTexturedVertex ctv[3];

#define SHAPE_VERTICES 8
#define SHAPE_POLYGONS 6

// shape vertices for demonstrating different
// rendering methods
april::TexturedVertex sf[8];
april::TexturedVertex sw[SHAPE_POLYGONS*6];

//quad vertices for demonstrating blend-modes
april::ColoredTexturedVertex ctvQuad[4];
april::TexturedVertex tvQuad[4];

#define STAR_VERTICES 9
april::TexturedVertex starStrip[10];

#if !defined(__ANDROID__) && !defined(_IOS) && !defined(_WINP8)
grectf drawRect(0.0f, 0.0f, 800.0f, 600.0f);
#else
grectf drawRect(0.0f, 0.0f, 480.0f, 320.0f);
#endif
gvec2f offset = drawRect.getSize() * 0.5f;
grectf textureRect;
grectf src(0.0f, 0.0f, 1.0f, 1.0f);
bool mousePressed = false;

class Bone
{
public:
	gvec2f position;
	gvec2f point;
	float length;

	bool dragging;

	april::PlainVertex lineVertices[2];
	april::PlainVertex circleVertices[CIRCLE_VERTEX_COUNT*3];

	Bone(gvec2f position, float length)
	{
		this->position = position;
		this->dragging = false;
		this->length = length;
		this->point = this->position + gvec2f(this->length, 0);
	};

	void update()
	{
		gvec2f direction = point - position;
		direction.normalize();
		if (direction.length() <= 0.0f)
		{
			direction = gvec2f(1, 0);
		}
		direction *= length;
		this->lineVertices[0].x = this->position.x; 
		this->lineVertices[0].y = this->position.y; 
		this->lineVertices[1].x = this->position.x + direction.x;
		this->lineVertices[1].y = this->position.y + direction.y;
		for (int i = 0, j = 0; i < CIRCLE_VERTEX_COUNT * 3; i += 3, ++j)
		{
			circleVertices[i].x = this->lineVertices[1].x;
			circleVertices[i].y = this->lineVertices[1].y;

			circleVertices[i + 1].x = this->lineVertices[1].x + (float)hcos((360.0f / CIRCLE_VERTEX_COUNT) * j) * CIRCLE_RADIUS;
			circleVertices[i + 1].y = this->lineVertices[1].y + (float)hsin((360.0f / CIRCLE_VERTEX_COUNT) * j) * CIRCLE_RADIUS;

			circleVertices[i + 2].x = this->lineVertices[1].x + (float)hcos((360.0f / CIRCLE_VERTEX_COUNT) * (j + 1)) * CIRCLE_RADIUS;
			circleVertices[i + 2].y = this->lineVertices[1].y + (float)hsin((360.0f / CIRCLE_VERTEX_COUNT) * (j + 1)) * CIRCLE_RADIUS;
		}
	}

	void onClick(gvec2f mousePosition)
	{
		gvec2f direction = point - position;
		direction.normalize();
		direction *= length;
		gvec2f circleCenter = this->position + direction;
		gvec2f delta = (mousePosition - circleCenter);
		if (delta.length() < CIRCLE_RADIUS)
		{
			dragging = true;
		}
	}

	void onDrag(gvec2f mousePosition)
	{
		if (dragging)
		{
			this->point = mousePosition;
		}
	}

	void onRelease()
	{
		dragging = false;
	}

	void render()
	{
		april::rendersys->render(april::RenderOperation::LineList, lineVertices, 2);
		april::rendersys->render(april::RenderOperation::TriangleList, circleVertices, CIRCLE_VERTEX_COUNT*3);
	}

};
Bone bone(gvec2f(256, 256), 160);

class UpdateDelegate : public april::UpdateDelegate
{
	bool onUpdate(float timeDelta)
	{
		// clear the screen and set orthographic projection
		april::rendersys->clear();
		april::rendersys->setOrthoProjection(drawRect);
		// reset the blend mode
		april::rendersys->setBlendMode(april::BlendMode::BlendMode::Alpha);
		// save the default matrices so they can be restored later
		gmat4 modelviewMatrix = april::rendersys->getModelviewMatrix();
		gmat4 projectionMatrix = april::rendersys->getProjectionMatrix();
		// line list polygon
		april::rendersys->setTexture(NULL);
		april::rendersys->translate(gvec2f(256, 128));
		april::rendersys->render(april::RenderOperation::LineList, sw, SHAPE_POLYGONS * 6);
		// filled polygon
		april::rendersys->setTexture(NULL);
		april::rendersys->translate(gvec2f(64, 0));
		april::rendersys->render(april::RenderOperation::TriangleStrip, sf, SHAPE_VERTICES);
		// textured polygon
		april::rendersys->setTexture(textures[1]);
		april::rendersys->translate(gvec2f(64, 0));
		april::rendersys->render(april::RenderOperation::TriangleStrip, sf, SHAPE_VERTICES);
		// line strip polygon
		april::rendersys->setTexture(NULL);
		static float rotationAngle = 0.0f;
		rotationAngle += timeDelta * 512;
		april::rendersys->translate(gvec2f(96, 32));
		april::rendersys->rotate(rotationAngle);
		april::rendersys->render(april::RenderOperation::LineStrip, starStrip, STAR_VERTICES);
		// reset the modelview matrix
		april::rendersys->setModelviewMatrix(modelviewMatrix);
		april::rendersys->setColorMode(april::ColorMode::Multiply, 1.0f);
		// blending examples
		april::rendersys->translate(gvec2f(-50, 300));
		for_iter (i, 0, april::BlendMode::getValues().size())
		{
			april::rendersys->setTexture(textures[1]);
			april::rendersys->setBlendMode(april::BlendMode::Alpha);
			april::rendersys->translate(gvec2f(90, -32));
			april::rendersys->render(april::RenderOperation::TriangleStrip, ctvQuad, 4);
			april::rendersys->translate(gvec2f(32, 32));
			april::rendersys->setTexture(textures[2]);
			april::rendersys->setBlendMode(april::BlendMode::fromInt(i));
			april::rendersys->render(april::RenderOperation::TriangleStrip, ctvQuad, 4);
		}
		april::rendersys->setModelviewMatrix(modelviewMatrix);
		april::rendersys->setBlendMode(april::BlendMode::Alpha);
		//color mode examples
		april::rendersys->translate(gvec2f(-50, 450));
		for_iter (i, 0, april::ColorMode::getValues().size())
		{
			april::rendersys->setTexture(textures[1]);
			april::rendersys->setColorMode(april::ColorMode::Multiply, 1.0f);
			april::rendersys->translate(gvec2f(90, -32));
			april::rendersys->render(april::RenderOperation::TriangleStrip, ctvQuad, 4);
			april::rendersys->translate(gvec2f(32, 32));
			april::rendersys->setTexture(textures[2]);
			april::rendersys->setColorMode(april::ColorMode::fromInt(i), 0.5f);
			april::rendersys->render(april::RenderOperation::TriangleStrip, tvQuad, 4);
		}
		april::rendersys->setColorMode(april::ColorMode::Multiply, 1.0f);
		april::rendersys->setModelviewMatrix(modelviewMatrix);
		april::rendersys->setTexture(textures[3]);
		april::rendersys->translate(gvec2f(0, 0));
		april::rendersys->render(april::RenderOperation::TriangleStrip, ctvQuad, 4);
		april::rendersys->translate(gvec2f(200, 0));
		april::rendersys->scale(gvec2f(-1, 1));		
		april::rendersys->render(april::RenderOperation::TriangleStrip, ctvQuad, 4);
		april::rendersys->translate(gvec2f(0, 200));
		april::rendersys->scale(gvec2f(1, -1));
		april::rendersys->render(april::RenderOperation::TriangleStrip, ctvQuad, 4);
		april::rendersys->translate(gvec2f(200, 0));
		april::rendersys->scale(gvec2f(-1, 1));
		april::rendersys->render(april::RenderOperation::TriangleStrip, ctvQuad, 4);
		april::rendersys->setTexture(NULL);
		// reset the matrices
		april::rendersys->setModelviewMatrix(modelviewMatrix);
		april::rendersys->setProjectionMatrix(projectionMatrix);
		bone.update();
		bone.render();	
#ifdef _ENGINE_RENDER_TEST
		// testing all render methods
		april::rendersys->setTexture(textures[0]);
		april::rendersys->drawFilledRect(grectf(drawRect.w - 110.0f, drawRect.h - 310.0f, 110.0f, 310.0f), april::Color::Black);
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
		//this is called when the window size is changed
		april::rendersys->setViewport(drawRect);
	}

};

class MouseDelegate : public april::MouseDelegate
{
	void onMouseDown(april::Key key)
	{
		offset = april::window->getCursorPosition();
		bone.onClick(offset);
		mousePressed = true;
	}

	void onMouseUp(april::Key key)
	{
		bone.onRelease();
		mousePressed = false;
	}

	void onMouseMove()
	{
		gvec2f position = april::window->getCursorPosition();
		bone.onDrag(position);
		if (mousePressed)
		{
			offset = position;
		}
	}

	void onMouseCancel(april::Key key)
	{
	}

};

class KeyDelegate : public april::KeyDelegate
{
	void onKeyUp(april::Key keyCode)
	{
		if (keyCode == april::Key::F4)
		{
			april::window->setFullscreen(!april::window->isFullscreen());
		}
	}

};

static UpdateDelegate* updateDelegate = NULL;
static SystemDelegate* systemDelegate = NULL;
static MouseDelegate* mouseDelegate = NULL;
static KeyDelegate* keyDelegate = NULL;

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
	keyDelegate = new KeyDelegate();
#if defined(__ANDROID__) || defined(_IOS)
	drawRect.setSize(april::getSystemInfo().displayResolution);
#endif
	// init
	april::init(april::RenderSystemType::Default, april::WindowType::Default);
	april::createRenderSystem();
	april::createWindow((int)drawRect.w, (int)drawRect.h, false, "APRIL: Advanced Demo");
#ifdef _UWP
	april::window->setParam("cursor_mappings", "101 " RESOURCE_PATH "cursor\n102 " RESOURCE_PATH "simple");
#endif
	textures[0] = april::rendersys->createTextureFromResource(RESOURCE_PATH "jpt_final", april::Texture::Type::Managed);
	textures[1] = april::rendersys->createTextureFromResource(RESOURCE_PATH "camo", april::Texture::Type::Managed);
	textures[2] = april::rendersys->createTextureFromResource(RESOURCE_PATH "logo", april::Texture::Type::Managed);
	textures[3] = april::rendersys->createTextureFromResource(RESOURCE_PATH "bloom", april::Texture::Type::Managed);
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

	// color-textured quad (for blending and color examples)
	tvQuad[0].x = ctvQuad[0].x = 0.0f;
	tvQuad[0].y = ctvQuad[0].y = 0.0f;
	tvQuad[0].z = ctvQuad[0].z = 0.0f;
	tvQuad[0].u = ctvQuad[0].u = 0.0f;
	tvQuad[0].v = ctvQuad[0].v = 0.0f;
	ctvQuad[0].color = april::rendersys->getNativeColorUInt(april::Color::White);

	tvQuad[1].x = ctvQuad[1].x = 100.0f;
	tvQuad[1].y = ctvQuad[1].y = 0.0f;
	tvQuad[1].z = ctvQuad[1].z = 0.0f;
	tvQuad[1].u = ctvQuad[1].u = 1.0f;
	tvQuad[1].v = ctvQuad[1].v = 0.0f;
	ctvQuad[1].color = april::rendersys->getNativeColorUInt(april::Color::White);

	tvQuad[2].x = ctvQuad[2].x = 0.0f;
	tvQuad[2].y = ctvQuad[2].y = 100.0f;
	tvQuad[2].z = ctvQuad[2].z = 0.0f;
	tvQuad[2].u = ctvQuad[2].u = 0.0f;
	tvQuad[2].v = ctvQuad[2].v = 1.0f;
	ctvQuad[2].color = april::rendersys->getNativeColorUInt(april::Color::White);

	tvQuad[3].x = ctvQuad[3].x = 100.0f;
	tvQuad[3].y = ctvQuad[3].y = 100.0f;
	tvQuad[3].z = ctvQuad[3].z = 0.0f;
	tvQuad[3].u = ctvQuad[3].u = 1.0f;
	tvQuad[3].v = ctvQuad[3].v = 1.0f;
	ctvQuad[3].color = april::rendersys->getNativeColorUInt(april::Color::White);

	// set delegates
	april::window->setUpdateDelegate(updateDelegate);
	april::window->setSystemDelegate(systemDelegate);
	april::window->setMouseDelegate(mouseDelegate);
	april::window->setKeyDelegate(keyDelegate);
	cursor = april::window->createCursorFromResource(RESOURCE_PATH "cursor");
	april::window->setCursor(cursor);

	sf[0].x = 0;	sf[0].y = 0; sf[0].u = 0.2f; sf[0].v = 0.0f;
	sf[1].x = -20;	sf[1].y = 20; sf[1].u = 0.0f; sf[1].v = 0.2f;
	sf[2].x = 20;	sf[2].y = 20; sf[2].u = 0.4f; sf[2].v = 0.2f;
	sf[3].x = 0;	sf[3].y = 40; sf[3].u = 0.2f; sf[3].v = 0.4f;
	sf[4].x = 40;	sf[4].y = 40; sf[4].u = 0.6f; sf[4].v = 0.4f;
	sf[5].x = 40;	sf[5].y = 80; sf[5].u = 0.6f; sf[5].v = 0.8f;
	sf[6].x = 20;	sf[6].y = 60; sf[6].u = 0.4f; sf[6].v = 0.6f;
	sf[7].x = 0;	sf[7].y = 80; sf[7].u = 0.2f; sf[7].v = 0.8f;

	sw[0] = sf[0]; sw[1] = sf[1]; sw[2] = sf[1]; sw[3] = sf[2]; sw[4] = sf[2]; sw[5] = sf[0];
	sw[6] = sf[1]; sw[7] = sf[3]; sw[8] = sf[3]; sw[9] = sf[2]; sw[10] = sf[2]; sw[11] = sf[1];
	sw[12] = sf[2]; sw[13] = sf[3]; sw[14] = sf[3]; sw[15] = sf[4]; sw[16] = sf[4]; sw[17] = sf[2];
	sw[18] = sf[3]; sw[19] = sf[6]; sw[20] = sf[6]; sw[21] = sf[4]; sw[22] = sf[4]; sw[23] = sf[3];
	sw[24] = sf[4]; sw[25] = sf[6]; sw[26] = sf[6]; sw[27] = sf[5]; sw[28] = sf[5]; sw[29] = sf[4];
	sw[30] = sf[6]; sw[31] = sf[7]; sw[32] = sf[7]; sw[33] = sf[5]; sw[34] = sf[5]; sw[35] = sf[6];

	gvec2f v[STAR_VERTICES];
	v[0] = gvec2f(0, 0);
	v[1] = gvec2f(10, -40);
	v[2] = gvec2f(20, 0);
	v[3] = gvec2f(60, 10);
	v[4] = gvec2f(20, 20);
	v[5] = gvec2f(10, 60);
	v[6] = gvec2f(0, 20);
	v[7] = gvec2f(-40, 10);
	v[8] = v[0];
	for_iter (i, 0, STAR_VERTICES)
	{
		starStrip[i].x = v[i].x - v[1].x;
		starStrip[i].y = v[i].y - v[3].y;
	}
}

void __aprilApplicationDestroy()
{
	april::window->setCursor(NULL);
	april::window->destroyCursor(cursor);
	cursor = NULL;
	april::rendersys->destroyTexture(textures[0]);
	textures[0] = NULL;
	april::rendersys->destroyTexture(textures[1]);
	textures[1] = NULL;
	april::rendersys->destroyTexture(textures[2]);
	textures[2] = NULL;
	april::rendersys->destroyTexture(textures[3]);
	textures[3] = NULL;
	april::destroy();
	delete updateDelegate;
	updateDelegate = NULL;
	delete systemDelegate;
	systemDelegate = NULL;
	delete mouseDelegate;
	mouseDelegate = NULL;
	delete keyDelegate;
	keyDelegate = NULL;
}
