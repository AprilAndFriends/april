/// @file
/// @version 5.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include <hltypes/hlog.h>
#include <hltypes/hstring.h>

#ifdef _ANDROID
#include "androidUtilJNI.h"
#endif
#include "april.h"
#include "Application.h"
#include "main_base.h"
#include "Platform.h"
#include "RenderSystem.h"
#include "TextureAsync.h"
#include "UpdateDelegate.h"
#include "Window.h"

namespace april
{
#ifdef _ANDROID
	extern void* javaVM;
#endif

	HL_ENUM_CLASS_DEFINE(Application::State,
	(
		HL_ENUM_DEFINE(Application::State, Idle);
		HL_ENUM_DEFINE(Application::State, Starting);
		HL_ENUM_DEFINE(Application::State, Running);
		HL_ENUM_DEFINE(Application::State, Stopping);
		HL_ENUM_DEFINE(Application::State, Stopped);
	));

	Application* application = NULL;
	
	Application::Application(void (*aprilApplicationInit)(), void (*aprilApplicationDestroy)()) : state(State::Idle), suspended(false), timeDelta(0.0f), fps(0), fpsCount(0),
		fpsTimer(0.0f), fpsResolution(0.5f), timeDeltaMaxLimit(0.1f), displayingMessageBox(false), updateThread(&_asyncUpdate, "APRIL Async Update")
	{
		this->aprilApplicationInit = aprilApplicationInit;
		this->aprilApplicationDestroy = aprilApplicationDestroy;
	}
	
	Application::~Application()
	{
	}

	Application::State Application::getState()
	{
		hmutex::ScopeLock lock(&this->stateMutex);
		return this->state;
	}

	void Application::_setState(const State& value)
	{
		hmutex::ScopeLock lock(&this->stateMutex);
		this->state = value;
	}
	
	bool Application::isAnyMessageBoxQueued()
	{
		hmutex::ScopeLock lock(&this->messageBoxMutex);
		return (!this->displayingMessageBox && this->messageBoxQueue.size() > 0);
	}

	void Application::init()
	{
		this->_setState(State::Starting);
		this->updateThread.start();
	}
	
	void Application::updateInitializing(bool singleUpdateOnly)
	{
		while (this->getState() == State::Starting)
		{
			this->_updateSystem();
			if (singleUpdateOnly)
			{
				return;
			}
			hthread::sleep(0.001f);
		}
	}

	void Application::destroy()
	{
		this->_setState(State::Idle);
		this->resume(); // makes sure the update thread can resume
		this->updateThread.join();
	}

	void Application::enterMainLoop()
	{
		this->fps = 0;
		this->fpsCount = 0;
		this->fpsTimer = 0.0f;
		this->timer.update();
		while (this->getState() == State::Running)
		{
			this->update();
		}
		this->updateFinishing();
	}

	void Application::updateFinishing()
	{
		this->resume(); // makes sure the update thread can resume
		// processing remaining commands from other thread
		while (this->getState() == State::Stopping)
		{
			this->_updateSystem();
		}
		// finish everything up
		this->_updateSystem();
		if (april::rendersys != NULL)
		{
			april::rendersys->_flushAsyncCommands();
		}
		// done
		this->_setState(State::Idle);
	}

	bool Application::update()
	{
		this->_updateMessageBoxQueue();
		TextureAsync::update();
		april::window->checkEvents();
		float timeDelta = (float)this->timer.diff(true);
		if (!april::window->isFocused())
		{
			timeDelta = 0.0f;
		}
		this->fpsTimer += timeDelta;
		if (this->timeDeltaMaxLimit > 0.0f)
		{
			timeDelta = hmin(timeDelta, this->timeDeltaMaxLimit);
		}
		hmutex::ScopeLock lock(&this->timeDeltaMutex);
		this->timeDelta += timeDelta;
		lock.release();
		lock.acquire(&this->frameTimerMutex);
		this->frameTimer.update();
		lock.release();
		return april::rendersys->update(timeDelta);
	}

	void Application::_updateMessageBoxQueue()
	{
		hmutex::ScopeLock lock(&this->messageBoxMutex);
		if (!this->displayingMessageBox && this->messageBoxQueue.size() > 0)
		{
			MessageBoxData data = this->messageBoxQueue.first();
			this->displayingMessageBox = true;
			lock.release();
			_processMessageBox(data);
		}
	}

	void Application::_updateSystem()
	{
		this->_updateMessageBoxQueue();
		TextureAsync::update();
		if (april::window != NULL)
		{
			april::window->checkEvents();
		}
		if (april::rendersys != NULL)
		{
			april::rendersys->update(0.0f); // might require some rendering
		}
	}

	void Application::_updateFps()
	{
		if (this->fpsTimer > 0.0f)
		{
			++this->fpsCount;
			if (this->fpsTimer >= this->fpsResolution)
			{
				this->fps = hceil(this->fpsCount / this->fpsTimer);
				this->fpsCount = 0;
				this->fpsTimer -= this->fpsResolution;
			}
		}
		else
		{
			this->fps = 0;
			this->fpsCount = 0;
		}
	}

	void Application::finish()
	{
		hmutex::ScopeLock lock(&this->stateMutex);
		if (this->state == State::Starting || this->state == State::Running)
		{
			this->state = State::Stopping;
		}
	}

	void Application::finalize()
	{
		hmutex::ScopeLock lock(&this->stateMutex);
		if (this->state == State::Stopping)
		{
			this->state = State::Stopped;
			while (this->state == State::Stopped)
			{
				lock.release();
				hthread::sleep(0.001f);
				lock.acquire(&this->stateMutex);
			}
		}
	}

	void Application::_asyncUpdate(hthread* thread)
	{
#ifdef _ANDROID
		// attaching the Java thread is neccessary so C++-to-Java calls can be made
		JNIEnv* env = NULL;
		if (((JavaVM*)april::javaVM)->AttachCurrentThread(&env, NULL) != JNI_OK)
		{
			env = NULL;
		}
#endif
		(*april::application->aprilApplicationInit)();
		hmutex::ScopeLock lock(&april::application->stateMutex);
		if (april::window != NULL && april::rendersys != NULL && april::application->state == State::Starting)
		{
			april::application->state = State::Running;
			lock.release();
			float timeDelta = 0.0f;
			UpdateDelegate* updateDelegate = NULL;
			hmutex::ScopeLock lockTimeDelta;
			float sleepTime = 0.0f;
			// if this option can be changed in the future, this code needs to be adjusted for this
			bool vSync = april::rendersys->getOptions().vSync;
			// this might be changed in the future (assuming at least one display mode is available, this might not be safe)
			float frameTime = 1000.0f / april::rendersys->getDisplayModes().first().refreshRate;
			while (april::application->getState() == State::Running)
			{
				lock.acquire(&april::application->updateMutex);
				lockTimeDelta.acquire(&april::application->timeDeltaMutex);
				timeDelta = april::application->timeDelta;
				april::application->timeDelta = 0.0f;
				lockTimeDelta.release();
				if (!april::window->update(timeDelta))
				{
					april::application->finish();
				}
				april::window->setPresentFrameEnabled(false);
				updateDelegate = april::window->getUpdateDelegate(); // constantly getting to make sure not to use an outdated object
				if (updateDelegate != NULL)
				{
					updateDelegate->onPresentFrame();
				}
				else
				{
					april::rendersys->presentFrame();
				}
				april::window->setPresentFrameEnabled(true);
				lock.release();
				while (april::application->getState() == State::Running && april::rendersys->getAsyncQueuesCount() > april::rendersys->getFrameAdvanceUpdates())
				{
					if (vSync)
					{
						lock.acquire(&april::application->frameTimerMutex);
						sleepTime = frameTime - (float)april::application->frameTimer.diff(false) * 1000.0f;
						lock.release();
						hthread::sleep(hmax(sleepTime - 1.0f, 0.001f)); // using safe zone of 1ms
					}
					else
					{
						hthread::sleep(0.001f);
					}
					april::window->_processEvents();
				}
			}
		}
		else
		{
			april::application->state = State::Stopping;
			lock.release();
			if (april::window != NULL)
			{
				april::window->_processEvents();
			}
		}
		(*april::application->aprilApplicationDestroy)();
#ifdef _ANDROID
		if (env != NULL)
		{
			((JavaVM*)april::javaVM)->DetachCurrentThread();
		}
#endif
	}

	void Application::suspend()
	{
		if (!this->suspended && this->getState() == State::Running)
		{
			// TODO - is this still needed?
			/*
#ifndef _ANDROID
			april::rendersys->_flushAsyncCommands(); // this is here for safe-guard on non-Android platforms
#endif
			*/
			if (april::window->getOptions().suspendUpdateThread)
			{
				this->updateMutex.lock();
			}
			hlog::write(logTag, "Application suspend.");
			this->suspended = true;
			april::rendersys->_flushAsyncCommands();
			if (april::rendersys->getOptions().clearOnSuspend)
			{
				april::rendersys->_deviceClear(true);
				april::rendersys->_devicePresentFrame(true);
			}
		}
	}

	void Application::resume()
	{
		if (this->suspended)
		{
			hlog::write(logTag, "Application resume.");
			this->suspended = false;
			if (april::window->getOptions().suspendUpdateThread)
			{
				this->updateMutex.unlock();
			}
		}
	}

	void Application::renderFrameSync()
	{
		// TODO - can this even work?
		hmutex::ScopeLock lock(&this->updateMutex);
		april::rendersys->_repeatLastFrame();
	}

	void Application::queueMessageBox(const MessageBoxData& data)
	{
		hmutex::ScopeLock lock(&this->messageBoxMutex);
		this->messageBoxQueue += data;
	}

	void Application::waitForMessageBoxes()
	{
		// for some reason this freezes Mac apps even though it's running on another thread
#ifndef _MAC
		hmutex::ScopeLock lock(&this->messageBoxMutex);
		while (april::application->displayingMessageBox || this->messageBoxQueue.size() > 0)
		{
			lock.release();
			hthread::sleep(0.001f);
			lock.acquire(&this->messageBoxMutex);
		}
#endif
	}

	void Application::messageBoxCallback(const MessageBoxButton& button)
	{
		hmutex::ScopeLock lock(&april::application->messageBoxMutex);
		MessageBoxData data = april::application->messageBoxQueue.first();
		if (data.callback != NULL)
		{
			lock.release();
			data.callback(button);
			lock.acquire(&april::application->messageBoxMutex);
		}
		april::application->displayingMessageBox = false;
		april::application->messageBoxQueue.removeFirst();
		if (data.applicationFinishAfterDisplay)
		{
			april::application->finish();
		}
	}

}
