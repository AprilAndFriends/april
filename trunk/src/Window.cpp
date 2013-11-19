/// @file
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @author  Ivan Vucica
/// @version 3.14
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include <gtypes/Rectangle.h>
#include <gtypes/Vector2.h>
#include <hltypes/hlog.h>
#include <hltypes/hstring.h>
#include <hltypes/hthread.h>

#include "april.h"
#include "ControllerDelegate.h"
#include "KeyboardDelegate.h"
#include "Keys.h"
#include "MouseDelegate.h"
#include "Platform.h"
#include "RenderSystem.h"
#include "SystemDelegate.h"
#include "TouchDelegate.h"
#include "UpdateDelegate.h"
#include "Window.h"

#define INPUT_MODE_NAME(value) \
	hstr(value == MOUSE ? "MOUSE" : \
	(value == TOUCH ? "TOUCH" : \
	(value == CONTROLLER ? "CONTROLLER" : "UNDEFINED")))

namespace april
{
	// TODO - refactor
	void (*Window::msLaunchCallback)(void*) = NULL;
	void Window::handleLaunchCallback(void* args)
	{
		if (msLaunchCallback != NULL)
		{
			(*msLaunchCallback)(args);
		}
	}
	//////////////////

	Window::KeyInputEvent::KeyInputEvent(Window::KeyEventType type, Key keyCode, unsigned int charCode)
	{
		this->type = type;
		this->keyCode = keyCode;
		this->charCode = charCode;
	}

	Window::MouseInputEvent::MouseInputEvent(Window::MouseEventType type, gvec2 position, Key keyCode)
	{
		this->type = type;
		this->position = position;
		this->keyCode = keyCode;
	}
		
	Window::TouchInputEvent::TouchInputEvent(harray<gvec2>& touches)
	{
		this->touches = touches;
	}
		
	Window::ControllerInputEvent::ControllerInputEvent(Window::ControllerEventType type, Button buttonCode)
	{
		this->type = type;
		this->buttonCode = buttonCode;
	}

	Window* window = NULL;
	
	Window::Options::Options()
	{
		this->resizable = false;
		this->fpsCounter = false;
	}
	
	Window::Options::~Options()
	{
	}

	hstr Window::Options::toString()
	{
		harray<hstr> options;
		if (this->resizable)
		{
			options += "resizable";
		}
		if (options.size() == 0)
		{
			options += "none";
		}
		return options.join(',');
	}
	
	Window::Window() : created(false), fullscreen(true), focused(true), running(true),
		fps(0), fpsCount(0), fpsTimer(0.0f), fpsResolution(0.5f), cursorVisible(false),
		virtualKeyboardVisible(false), virtualKeyboardHeightRatio(0.0f),
		multiTouchActive(false), inputMode(MOUSE)
	{
		april::window = this;
		this->name = "Generic";
		this->updateDelegate = NULL;
		this->keyboardDelegate = NULL;
		this->mouseDelegate = NULL;
		this->touchDelegate = NULL;
		this->controllerDelegate = NULL;
		this->systemDelegate = NULL;
	}
	
	Window::~Window()
	{
		this->destroy();
	}

	bool Window::create(int w, int h, bool fullscreen, chstr title, Window::Options options)
	{
		if (!this->created)
		{
			hlog::writef(april::logTag, "Creating window: '%s' (%d, %d) %s, '%s', (options: %s)",
				this->name.c_str(), w, h, fullscreen ? "fullscreen" : "windowed", title.c_str(), options.toString().c_str());
			this->fullscreen = fullscreen;
			this->title = title;
			this->options = options;
			this->created = true;
			this->fps = 0;
			this->fpsCount = 0;
			this->fpsTimer = 0.0f;
			this->fpsResolution = 0.5f;
			this->multiTouchActive = false;
			this->virtualKeyboardVisible = false;
			this->virtualKeyboardHeightRatio = 0.0f;
			this->inputMode = MOUSE;
			return true;
		}
		return false;
	}
	
	bool Window::destroy()
	{
		if (this->created)
		{
			hlog::writef(april::logTag, "Destroying window '%s'.", this->name.c_str());
			this->created = false;
			this->updateDelegate = NULL;
			this->keyboardDelegate = NULL;
			this->mouseDelegate = NULL;
			this->touchDelegate = NULL;
			this->controllerDelegate = NULL;
			this->systemDelegate = NULL;
			this->keyEvents.clear();
			this->mouseEvents.clear();
			this->touchEvents.clear();
			this->controllerEvents.clear();
			this->touches.clear();
			this->controllerEmulationKeys.clear();
			return true;
		}
		return false;
	}

	void Window::unassign()
	{
	}

	void Window::setInputMode(Window::InputMode value)
	{
		if (this->inputModeTranslations.has_key(value))
		{
			value = this->inputModeTranslations[value];
		}
		if (this->inputMode != value)
		{
			this->inputMode = value;
			hlog::write(april::logTag, "Changing Input Mode to: " + INPUT_MODE_NAME(this->inputMode));
			if (this->inputMode == CONTROLLER)
			{
				this->cursorPosition.set(-10000.0f, -10000.0f);
			}
			if (this->systemDelegate != NULL)
			{
				this->systemDelegate->onInputModeChanged(value);
			}
		}
	}
	
	gvec2 Window::getSize()
	{
		return gvec2((float)this->getWidth(), (float)this->getHeight());
	}
	
	float Window::getAspectRatio()
	{
		return ((float)this->getWidth() / this->getHeight());
	}
	
	bool Window::isCursorInside()
	{
		return grect(0.0f, 0.0f, this->getSize()).isPointInside(this->getCursorPosition());
	}

	void Window::setFullscreen(bool value)
	{
		SystemInfo info = april::getSystemInfo();
		int w = hround(info.displayResolution.x);
		int h = hround(info.displayResolution.y);
		if (!value)
		{
			w = (int)(w * 0.6666667f);
			h = (int)(h * 0.6666667f);
		}
		this->setResolution(w, h, value);
		this->fullscreen = value;
	}

	void Window::setResolution(int w, int h)
	{
		this->setResolution(w, h, this->isFullscreen());
	}

	void Window::setResolution(int w, int h, bool fullscreen)
	{
		hlog::warnf(april::logTag, "setResolution() is not available in '%s'.", this->name.c_str());
	}

	void Window::_setRenderSystemResolution()
	{
		this->_setRenderSystemResolution(this->getWidth(), this->getHeight(), this->fullscreen);
	}
	
	void Window::_setRenderSystemResolution(int w, int h, bool fullscreen)
	{
		hlog::writef(april::logTag, "Setting window resolution: (%d,%d); fullscreen: %s", w, h, fullscreen ? "yes" : "no");
		april::rendersys->_setResolution(w, h, fullscreen);
		if (this->systemDelegate != NULL)
		{
			this->systemDelegate->onWindowSizeChanged(w, h, fullscreen);
		}
	}
	
	void Window::enterMainLoop()
	{
		this->fps = 0;
		this->fpsCount = 0;
		this->fpsTimer = 0.0f;
		this->running = true;
		while (this->running)
		{
			if (!this->updateOneFrame())
			{
				this->running = false;
			}
			april::rendersys->presentFrame();
		}
	}

	bool Window::updateOneFrame()
	{
		static float k;
		k = this->_calcTimeSinceLastFrame();
		if (!this->focused)
		{
			hthread::sleep(40.0f);
		}
		this->checkEvents();
		return (this->performUpdate(k) && this->running);
	}
	
	void Window::checkEvents()
	{
		while (this->keyEvents.size() > 0)
		{
			KeyInputEvent e = this->keyEvents.remove_first();
			this->handleKeyEvent(e.type, e.keyCode, e.charCode);
		}
		while (this->mouseEvents.size() > 0)
		{
			MouseInputEvent e = this->mouseEvents.remove_first();
			if (e.type != Window::AMOUSEEVT_CANCEL && e.type != Window::AMOUSEEVT_SCROLL)
			{
				this->cursorPosition = e.position;
			}
			this->handleMouseEvent(e.type, e.position, e.keyCode);
		}
		while (this->touchEvents.size() > 0)
		{
			TouchInputEvent e = this->touchEvents.remove_first();
			this->handleTouchEvent(e.touches);
		}
		while (this->controllerEvents.size() > 0)
		{
			ControllerInputEvent e = this->controllerEvents.remove_first();
			this->handleControllerEvent(e.type, e.buttonCode);
		}
	}

	void Window::terminateMainLoop()
	{
		this->running = false;
	}
	
	bool Window::performUpdate(float k)
	{
		this->fpsTimer += k;
		if (this->fpsTimer > 0.0f)
		{
			this->fpsCount++;
			if (this->fpsTimer >= this->fpsResolution)
			{
				this->fps = (int)(this->fpsCount / this->fpsTimer);
				this->fpsCount = 0;
				this->fpsTimer = 0.0f;
			}
		}
		else
		{
			this->fps = 0;
			this->fpsCount = 0;
		}
		// returning true: continue execution
		// returning false: abort execution
		if (this->updateDelegate != NULL)
		{
			return this->updateDelegate->onUpdate(k);
		}
		april::rendersys->clear();
		return true;
	}
	
	void Window::handleKeyEvent(KeyEventType type, Key keyCode, unsigned int charCode)
	{
		this->handleKeyOnlyEvent(type, keyCode); // doesn't do anything if keyCode is AK_NONE
		if (type == AKEYEVT_DOWN && charCode > 0) // ignores invalid chars
		{
			this->handleCharOnlyEvent(charCode);
		}
	}
	
	void Window::handleKeyOnlyEvent(KeyEventType type, Key keyCode)
	{
		if (keyCode == AK_UNKNOWN)
		{
			keyCode = AK_NONE;
		}
		if (this->keyboardDelegate != NULL && keyCode != AK_NONE)
		{
			switch (type)
			{
			case AKEYEVT_DOWN:
				this->keyboardDelegate->onKeyDown(keyCode);
				break;
			case AKEYEVT_UP:
				this->keyboardDelegate->onKeyUp(keyCode);
				break;
			}
			// emulation of buttons using keyboard
			if (this->controllerEmulationKeys.has_key(keyCode))
			{
				ControllerEventType buttonType = (type == AKEYEVT_DOWN ? ACTRLEVT_DOWN : ACTRLEVT_UP);
				this->handleControllerEvent(buttonType, this->controllerEmulationKeys[keyCode]);
			}
		}
	}
	
	void Window::handleCharOnlyEvent(unsigned int charCode)
	{
		if (this->keyboardDelegate != NULL && charCode >= 32 && charCode != 127) // special hack, backspace induces a character in some implementations
		{
			this->keyboardDelegate->onChar(charCode);
		}
	}
	
	void Window::handleMouseEvent(MouseEventType type, gvec2 position, Key keyCode)
	{
		if (this->mouseDelegate != NULL)
		{
			switch (type)
			{
			case AMOUSEEVT_DOWN:
				this->mouseDelegate->onMouseDown(keyCode);
				break;
			case AMOUSEEVT_UP:
				this->mouseDelegate->onMouseUp(keyCode);
				break;
			case AMOUSEEVT_CANCEL:
				this->mouseDelegate->onMouseCancel(keyCode);
				break;
			case AMOUSEEVT_MOVE:
				this->mouseDelegate->onMouseMove();
				break;
			case AMOUSEEVT_SCROLL:
				this->mouseDelegate->onMouseScroll(position.x, position.y);
				break;
			}
		}
	}
	
	void Window::handleTouchEvent(const harray<gvec2>& touches)
	{
		if (this->touchDelegate != NULL)
		{
			this->touchDelegate->onTouch(touches);
		}
	}

	void Window::handleControllerEvent(ControllerEventType type, Button buttonCode)
	{
		if (this->controllerDelegate != NULL && buttonCode != AB_NONE)
		{
			switch (type)
			{
			case ACTRLEVT_DOWN:
				this->controllerDelegate->onButtonDown(buttonCode);
				break;
			case ACTRLEVT_UP:
				this->controllerDelegate->onButtonUp(buttonCode);
				break;
			}
		}
	}
	
	bool Window::handleQuitRequest(bool canCancel)
	{
		// returns whether or not the windowing system is permitted to close the window
		if (this->systemDelegate != NULL)
		{
			return this->systemDelegate->onQuit(canCancel);
		}
		return true;
	}
	
	void Window::handleFocusChangeEvent(bool focused)
	{
		this->focused = focused;
		hlog::write(april::logTag, "Window " + hstr(focused ? "gained focus." : "lost focus."));
		if (this->systemDelegate != NULL)
		{
			this->systemDelegate->onWindowFocusChanged(focused);
		}
	}

	void Window::handleActivityChangeEvent(bool active)
	{
		hlog::warn(april::logTag, this->name + " does not implement activity change events!");
	}
	
	void Window::handleVirtualKeyboardVisibilityChange(bool visible, float heightRatio)
	{
		this->virtualKeyboardVisible = visible;
		this->virtualKeyboardHeightRatio = heightRatio;
		if (this->systemDelegate != NULL)
		{
			this->systemDelegate->onVirtualKeyboardVisibilityChanged(this->virtualKeyboardVisible, this->virtualKeyboardHeightRatio);
		}
	}

	void Window::handleLowMemoryWarning()
	{
		if (this->systemDelegate != NULL)
		{
			this->systemDelegate->onLowMemoryWarning();
		}
	}

	void Window::queueKeyEvent(KeyEventType type, Key keyCode, unsigned int charCode)
	{
		this->keyEvents += KeyInputEvent(type, keyCode, charCode);
	}

	void Window::queueMouseEvent(MouseEventType type, gvec2 position, Key keyCode)
	{
		this->mouseEvents += MouseInputEvent(type, position, keyCode);
	}

	void Window::queueTouchEvent(MouseEventType type, gvec2 position, int index)
	{
		int previousTouchesSize = this->touches.size();
		switch (type)
		{
		case AMOUSEEVT_DOWN:
			if (index < this->touches.size()) // DOWN event of an already indexed touch, never happened so far
			{
				return;
			}
			this->touches += position;
			break;
		case AMOUSEEVT_UP:
			if (index >= this->touches.size()) // redundant UP event, can happen
			{
				return;
			}
			this->touches.remove_at(index);
			break;
		case AMOUSEEVT_MOVE:
			if (index >= this->touches.size()) // MOVE event of an unindexed touch, never happened so far
			{
				return;
			}
			this->touches[index] = position;
			break;
		case AMOUSEEVT_CANCEL: // canceling a particular pointer, required by specific systems (e.g. WinRT)
			if (index < this->touches.size())
			{
				this->touches.remove_at(index);
				if (this->touches.size() == 0)
				{
					this->multiTouchActive = false;
				}
			}
			return;
		default:
			break;
		}
		if (this->multiTouchActive || this->touches.size() > 1)
		{
			if (!this->multiTouchActive && previousTouchesSize == 1)
			{
				// cancel (notify the app) the previously called mousedown event so we can begin the multi touch event properly
				this->queueMouseEvent(AMOUSEEVT_CANCEL, position, AK_LBUTTON);
			}
			this->multiTouchActive = (this->touches.size() > 0);
		}
		else
		{
			this->queueMouseEvent(type, position, AK_LBUTTON);
		}
		this->touchEvents.clear();
		this->touchEvents += TouchInputEvent(this->touches);
	}

	void Window::queueControllerEvent(ControllerEventType type, Button buttonCode)
	{
		this->controllerEvents += ControllerInputEvent(type, buttonCode);
	}

	float Window::_calcTimeSinceLastFrame()
	{
		float k = this->timer.diff(true);
		if (k > 0.5f)
		{
			k = 0.05f; // prevent jumps. from eg, waiting on device reset or super low framerate
		}
		if (!this->focused)
		{
			k = 0.0f;
		}
		return k;
	}
	
}
