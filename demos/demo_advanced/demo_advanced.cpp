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

#ifdef __APPLE__
#include <stdlib.h>
#include <unistd.h>
#import <Foundation/Foundation.h>
#import <CoreFoundation/CoreFoundation.h>
#endif

#include <hltypes/hdir.h>
#include <april/april.h>
#include <april/Cursor.h>
#include <april/main.h>
#include <april/KeyboardDelegate.h>
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
april::Texture* texture = NULL;
april::Texture* texture2 = NULL;
april::Texture* texture3 = NULL;

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
april::ColoredTexturedVertex ctv_quad[4];

#define STAR_VERTICES 9
april::TexturedVertex starStrip[10];

#if !defined(_ANDROID) && !defined(_IOS) && !defined(_WINP8)
grect drawRect(0.0f, 0.0f, 800.0f, 600.0f);
#else
grect drawRect(0.0f, 0.0f, 480.0f, 320.0f);
#endif
gvec2 offset = drawRect.getSize() * 0.5f;
grect textureRect;
grect src(0.0f, 0.0f, 1.0f, 1.0f);
bool mousePressed = false;

class Bone
{
public:
	gvec2 position;
	gvec2 point;
	float length;

	bool dragging = false;

	april::PlainVertex lineVertices[2];
	april::PlainVertex circleVertices[CIRCLE_VERTEX_COUNT*3];

	Bone(gvec2 position, float length)
	{
		this->position = position;
		this->length = length;
		this->point = this->position + gvec2(this->length, 0);
	};

	void update()
	{
		gvec2 direction = point - position;
		direction.normalize();
		if (direction.length() == 0)
			direction = gvec2(1, 0);
		direction *= length;

		this->lineVertices[0].x = this->position.x; 
		this->lineVertices[0].y = this->position.y; 
		this->lineVertices[1].x = this->position.x + direction.x;
		this->lineVertices[1].y = this->position.y + direction.y;		
		
		for (int i = 0, j = 0; i < CIRCLE_VERTEX_COUNT * 3; i += 3, j++)
		{
			circleVertices[i].x = this->lineVertices[1].x;
			circleVertices[i].y = this->lineVertices[1].y;

			circleVertices[i + 1].x = this->lineVertices[1].x + hcos((360.0f / CIRCLE_VERTEX_COUNT) * j) * CIRCLE_RADIUS;
			circleVertices[i + 1].y = this->lineVertices[1].y + hsin((360.0f / CIRCLE_VERTEX_COUNT) * j) * CIRCLE_RADIUS;

			circleVertices[i + 2].x = this->lineVertices[1].x + hcos((360.0f / CIRCLE_VERTEX_COUNT) * (j + 1)) * CIRCLE_RADIUS;
			circleVertices[i + 2].y = this->lineVertices[1].y + hsin((360.0f / CIRCLE_VERTEX_COUNT) * (j + 1)) * CIRCLE_RADIUS;
		}
	}

	void onClick(gvec2 mousePosition)
	{
		gvec2 direction = point - position;
		direction.normalize();
		direction *= length;

		gvec2 circleCenter = this->position + direction;
		gvec2 delta = (mousePosition - circleCenter);

		if (delta.length() < CIRCLE_RADIUS)
		{
			dragging = true;
		}
	}
	void onDrag(gvec2 mousePosition)
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
		april::rendersys->render(april::RO_LINE_LIST, lineVertices, 2);
		april::rendersys->render(april::RO_TRIANGLE_LIST, circleVertices, CIRCLE_VERTEX_COUNT*3);
	}
};
Bone bone(gvec2(256, 256), 160);

class UpdateDelegate : public april::UpdateDelegate
{
	bool onUpdate(float timeDelta)
	{
		// clear the screen and set orthographic projection
		april::rendersys->clear();
		april::rendersys->setOrthoProjection(drawRect);

		// reset the blend mode
		april::rendersys->setBlendMode(april::BlendMode::BM_DEFAULT);

		// save the default matrices so they can be restored later
		gmat4 modelviewMatrix = april::rendersys->getModelviewMatrix();
		gmat4 projectionMatrix = april::rendersys->getProjectionMatrix();

		// line list polygon
		april::rendersys->setTexture(0);

		april::rendersys->translate(gvec2(256, 128));
		april::rendersys->render(april::RO_LINE_LIST, sw, SHAPE_POLYGONS * 6);

		// filled polygon
		april::rendersys->setTexture(0);

		april::rendersys->translate(gvec2(64, 0));
		april::rendersys->render(april::RO_TRIANGLE_STRIP, sf, SHAPE_VERTICES);

		// textured polygon
		april::rendersys->setTexture(texture2);

		april::rendersys->translate(gvec2(64, 0));
		april::rendersys->render(april::RO_TRIANGLE_STRIP, sf, SHAPE_VERTICES);

		// line strip polygon
		april::rendersys->setTexture(0);
		static float rot = 0;
		rot += timeDelta*512;

		april::rendersys->translate(gvec2(96, 32));
		april::rendersys->rotate(rot);
		april::rendersys->render(april::RO_LINE_STRIP, starStrip, STAR_VERTICES);

		// reset the modelview matrix
		april::rendersys->setModelviewMatrix(modelviewMatrix);

		// blending examples
		april::rendersys->translate(gvec2(-50, 300));
		for (int i = 0; i < 4; i++)
		{
			april::rendersys->setTexture(texture2);
			april::rendersys->setBlendMode(april::BlendMode::BM_DEFAULT);

			april::rendersys->translate(gvec2(0, -32));
			april::rendersys->translate(gvec2(90, 0));
			april::rendersys->render(april::RO_TRIANGLE_STRIP, ctv_quad, 4);
			april::rendersys->translate(gvec2(32, 32));

			april::rendersys->setTexture(texture3);
			if (i == 0)
				april::rendersys->setBlendMode(april::BlendMode::BM_ADD);
			else if (i == 1)
				april::rendersys->setBlendMode(april::BlendMode::BM_SUBTRACT);
			else if (i == 2)
				april::rendersys->setBlendMode(april::BlendMode::BM_ALPHA);
			else if (i == 3)
				april::rendersys->setBlendMode(april::BlendMode::BM_OVERWRITE);

			april::rendersys->render(april::RO_TRIANGLE_STRIP, ctv_quad, 4);
		}

		april::rendersys->setModelviewMatrix(modelviewMatrix);
		april::rendersys->setBlendMode(april::BlendMode::BM_DEFAULT);

		//color mode examples
		april::rendersys->translate(gvec2(-50, 450));
		for (int i = 0; i < 3; i++)
		{
			april::rendersys->setTexture(texture2);
			april::rendersys->setColorMode(april::ColorMode::CM_DEFAULT, 1.0f);

			april::rendersys->translate(gvec2(0, -32));
			april::rendersys->translate(gvec2(90, 0));
			april::rendersys->render(april::RO_TRIANGLE_STRIP, ctv_quad, 4);
			april::rendersys->translate(gvec2(32, 32));

			april::rendersys->setTexture(texture3);
			if (i == 0)
				april::rendersys->setColorMode(april::ColorMode::CM_ALPHA_MAP, 0.5f);
			else if (i == 1)
				april::rendersys->setColorMode(april::ColorMode::CM_LERP, 0.5f);
			else if (i == 2)
				april::rendersys->setColorMode(april::ColorMode::CM_MULTIPLY, 0.5f);

			april::rendersys->render(april::RO_TRIANGLE_STRIP, ctv_quad, 4);
		}

		april::rendersys->setTexture(0);

		// reset the matrices
		april::rendersys->setModelviewMatrix(modelviewMatrix);
		april::rendersys->setProjectionMatrix(projectionMatrix);

		bone.update();
		bone.render();	
		
#ifdef _ENGINE_RENDER_TEST
		// testing all render methods
		april::rendersys->setTexture(texture);
		april::rendersys->drawFilledRect(grect(drawRect.w - 110.0f, drawRect.h - 310.0f, 110.0f, 310.0f), april::Color::Black);
		april::rendersys->render(april::RO_TRIANGLE_LIST, pv, 3);
		april::rendersys->render(april::RO_TRIANGLE_LIST, &pv[1], 3, april::Color::Yellow);
		april::rendersys->render(april::RO_TRIANGLE_LIST, tv, 3);
		april::rendersys->render(april::RO_TRIANGLE_LIST, &tv[1], 3, april::Color::Green);
		april::rendersys->render(april::RO_TRIANGLE_LIST, cv, 3);
		april::rendersys->render(april::RO_TRIANGLE_LIST, ctv, 3);
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
		gvec2 position = april::window->getCursorPosition();
		bone.onRelease();
		mousePressed = false;
	}

	void onMouseMove()
	{
		gvec2 position = april::window->getCursorPosition();
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

class KeyboardDelegate : public april::KeyboardDelegate
{
	void onKeyUp(april::Key keyCode)
	{
		if(keyCode == april::Key::AK_F4)
			april::window->setFullscreen(!april::window->isFullscreen());
	}
};

static UpdateDelegate* updateDelegate = NULL;
static SystemDelegate* systemDelegate = NULL;
static MouseDelegate* mouseDelegate = NULL;
static KeyboardDelegate* keyboardDelegate = NULL;

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

void april_init(const harray<hstr>& args)
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
	keyboardDelegate = new KeyboardDelegate();

#if defined(_ANDROID) || defined(_IOS) || defined(_WINRT)
	drawRect.setSize(april::getSystemInfo().displayResolution);
#endif
	// init
	april::init(april::RS_DEFAULT, april::WS_DEFAULT);
	april::createRenderSystem();
	april::createWindow((int)drawRect.w, (int)drawRect.h, false, "APRIL: Advanced Demo");
#ifdef _WINRT
	april::window->setParam("cursor_mappings", "101 " RESOURCE_PATH "cursor\n102 " RESOURCE_PATH "simple");
#endif
	texture = april::rendersys->createTextureFromResource(RESOURCE_PATH "jpt_final", april::Texture::TYPE_MANAGED);
	texture2 = april::rendersys->createTextureFromResource(RESOURCE_PATH "camo", april::Texture::TYPE_MANAGED);
	texture3 = april::rendersys->createTextureFromResource(RESOURCE_PATH "logo", april::Texture::TYPE_MANAGED);
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

	//color-textured quad (for blending and color examples)
	ctv_quad[0].x = 0;			
	ctv_quad[0].y = 0;	
	ctv_quad[0].z = 0.0f;	
	ctv_quad[0].u = 0.0f;	
	ctv_quad[0].v = 0.0f;	
	ctv_quad[0].color = april::rendersys->getNativeColorUInt(april::Color::Red);

	ctv_quad[1].x = 100;
	ctv_quad[1].y = 0;
	ctv_quad[1].z = 0.0f;
	ctv_quad[1].u = 1.0f;
	ctv_quad[1].v = 0.0f;
	ctv_quad[1].color = april::rendersys->getNativeColorUInt(april::Color::White);

	ctv_quad[2].x = 0;
	ctv_quad[2].y = 100;
	ctv_quad[2].z = 0.0f;
	ctv_quad[2].u = 0.0f;
	ctv_quad[2].v = 1.0f;
	ctv_quad[2].color = april::rendersys->getNativeColorUInt(april::Color::White);

	ctv_quad[3].x = 100.0f;	
	ctv_quad[3].y = 100;	
	ctv_quad[3].z = 0.0f;	
	ctv_quad[3].u = 1.0f;	
	ctv_quad[3].v = 1.0f;	
	ctv_quad[3].color = april::rendersys->getNativeColorUInt(april::Color::Green);

	// set delegates
	april::window->setUpdateDelegate(updateDelegate);
	april::window->setSystemDelegate(systemDelegate);
	april::window->setMouseDelegate(mouseDelegate);
	april::window->setKeyboardDelegate(keyboardDelegate);
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

	gvec2 v[STAR_VERTICES];
	v[0] = gvec2(0, 0);
	v[1] = gvec2(10, -40);
	v[2] = gvec2(20, 0);
	v[3] = gvec2(60, 10);
	v[4] = gvec2(20, 20);
	v[5] = gvec2(10, 60);
	v[6] = gvec2(0, 20);
	v[7] = gvec2(-40, 10);
	v[8] = v[0];
	for (int i = 0; i < STAR_VERTICES; i++)
	{
		starStrip[i].x = v[i].x;
		starStrip[i].y = v[i].y;
	}
}

void april_destroy()
{
	april::window->setCursor(NULL);
	april::window->destroyCursor(cursor);
	cursor = NULL;
	april::destroy();
	delete updateDelegate;
	updateDelegate = NULL;
	delete systemDelegate;
	systemDelegate = NULL;
	delete mouseDelegate;
	mouseDelegate = NULL;
	delete keyboardDelegate;
	keyboardDelegate = NULL;
}
