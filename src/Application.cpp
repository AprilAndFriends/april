/// @file
/// @version 4.5
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include <hltypes/hlog.h>

#include "Application.h"
#include "main_base.h"
#include "RenderSystem.h"
#include "TextureAsync.h"
#include "UpdateDelegate.h"
#include "Window.h"

namespace april
{
	Application* application = NULL;
	
	Application::Application(void (*aprilApplicationInit)(), void (*aprilApplicationDestroy)()) : running(true), autoPresentFrame(false),
		fps(0), fpsCount(0), fpsTimer(0.0f), fpsResolution(0.5f), timeDeltaMaxLimit(0.1f)
	{
		this->aprilApplicationInit = aprilApplicationInit;
		this->aprilApplicationDestroy = aprilApplicationDestroy;
#ifdef _ANDROID
		this->autoPresentFrame = true;
#endif
	}
	
	Application::~Application()
	{
	}

	void Application::init(const harray<hstr>& args)
	{
		this->args = args;
		(*this->aprilApplicationInit)();
	}

	void Application::destroy()
	{
		(*this->aprilApplicationDestroy)();
	}

	void Application::enterMainLoop()
	{
		this->fps = 0;
		this->fpsCount = 0;
		this->fpsTimer = 0.0f;
		this->running = true;
		while (this->running)
		{
			this->update();
		}
	}

	void Application::update()
	{
		TextureAsync::update();
		float timeDelta = this->timer.diff(true);
		if (!april::window->isFocused())
		{
			timeDelta = 0.0f;
		}
		this->fpsTimer += timeDelta;
		if (this->timeDeltaMaxLimit > 0.0f)
		{
			timeDelta = hmin(timeDelta, this->timeDeltaMaxLimit);
		}
		if (this->fpsTimer > 0.0f)
		{
			++this->fpsCount;
			if (this->fpsTimer >= this->fpsResolution)
			{
				this->fps = hceil(this->fpsCount / this->fpsTimer);
				this->fpsCount = 0;
				this->fpsTimer = 0.0f;
			}
		}
		else
		{
			this->fps = 0;
			this->fpsCount = 0;
		}
		if (!april::window->update(timeDelta))
		{
			this->running = false;
		}
		april::rendersys->update(timeDelta);
		UpdateDelegate* updateDelegate = april::window->getUpdateDelegate();
		if (updateDelegate != NULL)
		{
#ifndef _ANDROID
			updateDelegate->onPresentFrame();
#else // on AndroidJNI the backend does buffer swapping so this single call needs to be ignored
			april::window->setPresentFrameEnabled(false);
			updateDelegate->onPresentFrame();
			april::window->setPresentFrameEnabled(true);
#endif
		}
		// this consumes the timeDelta after the frame is done
		if (!april::window->isFocused())
		{
			this->timer.diff(true);
		}
	}

}
