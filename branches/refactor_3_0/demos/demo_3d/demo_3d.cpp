/// @file
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @version 3.0
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

#include <april/april.h>
#include <april/main.h>
#include <april/Platform.h>
#include <april/RenderSystem.h>
#include <april/UpdateDelegate.h>
#include <april/Window.h>

april::Texture* texture = NULL;
april::TexturedVertex v[4];

grect drawRect(0.0f, 0.0f, 800.0f, 600.0f);

class UpdateDelegate : public april::UpdateDelegate
{
public:
	UpdateDelegate::UpdateDelegate() : april::UpdateDelegate(), angle(0.0f)
	{
	}

	bool onUpdate(float timeSinceLastFrame)
	{
		this->angle += timeSinceLastFrame * 90.0f;
		april::rendersys->clear();
		april::rendersys->setPerspective(60.0f, 1.0f / drawRect.getAspect(), 1.0f, 1000.0f);
		april::rendersys->lookAt(gvec3(2.0f, 2.0f, -5.0f), gvec3(0.0f, 0.0f, 0.0f), gvec3(0.0f, 1.0f, 0.0f));
		april::rendersys->rotate(this->angle, 0.0f, 1.0f, 0.0f);
		april::rendersys->setTexture(texture);
		april::rendersys->render(april::TriangleStrip, v, 4);
		return true;
	}

protected:
	float angle;

};

static UpdateDelegate* updateDelegate = NULL;

void april_init(const harray<hstr>& args)
{
	updateDelegate = new UpdateDelegate();
#if defined(_ANDROID) || defined(_IOS)
	drawRect.setSize(april::getSystemInfo().displayResolution);
#endif
	april::init(april::RS_DEFAULT, april::WS_DEFAULT);
	april::createRenderSystem();
	april::createWindow((int)drawRect.w, (int)drawRect.h, false, "APRIL: Simple 3D");
	april::window->setUpdateDelegate(updateDelegate);
	texture = april::rendersys->createTexture(RESOURCE_PATH "texture");
	v[0].x = -1.0f;	v[0].y = 1.0f;	v[0].z = 0.0f;	v[0].u = 0.0f;	v[0].v = 0.0f;
	v[1].x = 1.0f;	v[1].y = 1.0f;	v[1].z = 0.0f;	v[1].u = 1.0f;	v[1].v = 0.0f;
	v[2].x = -1.0f;	v[2].y = -1.0f;	v[2].z = 0.0f;	v[2].u = 0.0f;	v[2].v = 1.0f;
	v[3].x = 1.0f;	v[3].y = -1.0f;	v[3].z = 0.0f;	v[3].u = 1.0f;	v[3].v = 1.0f;
}

void april_destroy()
{
	delete texture;
	april::destroy();
	delete updateDelegate;
}
