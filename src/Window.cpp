/// @file
/// @version 4.4
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include <gtypes/Rectangle.h>
#include <gtypes/Vector2.h>
#include <hltypes/hfile.h>
#include <hltypes/hlog.h>
#include <hltypes/hresource.h>
#include <hltypes/hstring.h>
#include <hltypes/hthread.h>

#include "april.h"
#include "ControllerDelegate.h"
#include "Cursor.h"
#include "KeyboardDelegate.h"
#include "Keys.h"
#include "MouseDelegate.h"
#include "Platform.h"
#include "RenderSystem.h"
#include "SystemDelegate.h"
#include "TextureAsync.h"
#include "TouchDelegate.h"
#include "UpdateDelegate.h"
#include "VirtualKeyboard.h"
#include "Window.h"

namespace april
{
	// TODOaa - refactor
	void (*Window::msLaunchCallback)(void*) = NULL;
	void Window::handleLaunchCallback(void* args)
	{
		if (msLaunchCallback != NULL)
		{
			(*msLaunchCallback)(args);
		}
	}
	//////////////////

	HL_ENUM_CLASS_DEFINE(Window::MouseInputEvent::Type,
	(
		HL_ENUM_DEFINE_VALUE(Window::MouseInputEvent::Type, Down, 0);
		HL_ENUM_DEFINE_VALUE(Window::MouseInputEvent::Type, Up, 1);
		HL_ENUM_DEFINE_VALUE(Window::MouseInputEvent::Type, Cancel, 2);
		HL_ENUM_DEFINE_VALUE(Window::MouseInputEvent::Type, Move, 3);
		HL_ENUM_DEFINE_VALUE(Window::MouseInputEvent::Type, Scroll, 4);
	));

	HL_ENUM_CLASS_DEFINE(Window::KeyInputEvent::Type,
	(
		HL_ENUM_DEFINE_VALUE(Window::KeyInputEvent::Type, Down, 0);
		HL_ENUM_DEFINE_VALUE(Window::KeyInputEvent::Type, Up, 1);
	));

	HL_ENUM_CLASS_DEFINE(Window::ControllerInputEvent::Type,
	(
		HL_ENUM_DEFINE_VALUE(Window::ControllerInputEvent::Type, Down, 0);
		HL_ENUM_DEFINE_VALUE(Window::ControllerInputEvent::Type, Up, 1);
		HL_ENUM_DEFINE_VALUE(Window::ControllerInputEvent::Type, Axis, 2);
		HL_ENUM_DEFINE_VALUE(Window::ControllerInputEvent::Type, Connected, 3);
		HL_ENUM_DEFINE_VALUE(Window::ControllerInputEvent::Type, Disconnected, 4);
	));

	Window::MouseInputEvent::MouseInputEvent()
	{
		this->type = Type::Move;
		this->keyCode = Key::None;
	}
		
	Window::MouseInputEvent::MouseInputEvent(Window::MouseInputEvent::Type type, cgvec2 position, Key keyCode)
	{
		this->type = type;
		this->position = position;
		this->keyCode = keyCode;
	}
		
	Window::KeyInputEvent::KeyInputEvent()
	{
		this->type = Type::Up;
		this->keyCode = Key::None;
		this->charCode = 0;
	}

	Window::KeyInputEvent::KeyInputEvent(Window::KeyInputEvent::Type type, Key keyCode, unsigned int charCode)
	{
		this->type = type;
		this->keyCode = keyCode;
		this->charCode = charCode;
	}

	Window::TouchInputEvent::TouchInputEvent()
	{
	}
		
	Window::TouchInputEvent::TouchInputEvent(harray<gvec2>& touches)
	{
		this->touches = touches;
	}
		
	Window::ControllerInputEvent::ControllerInputEvent()
	{
		this->type = Type::Up;
		this->buttonCode = Button::None;
	}

	Window::ControllerInputEvent::ControllerInputEvent(Window::ControllerInputEvent::Type type, int controllerIndex, Button buttonCode, float axisValue)
	{
		this->type = type;
		this->controllerIndex = controllerIndex;
		this->buttonCode = buttonCode;
		this->axisValue = axisValue;
	}

	Window* window = NULL;
	
	Window::Options::Options()
	{
		this->resizable = false;
		this->fpsCounter = false;
		this->hotkeyFullscreen = false;
		this->keyPause = april::Key::None;
		this->mac_displayLinkIgnoreSystemRedraw = false;
		this->defaultWindowModeResolutionFactor = 0.8f;
	}
	
	Window::Options::~Options()
	{
	}

	hstr Window::Options::toString() const
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
		return options.joined(',');
	}
	
	Window::Window()
	{
		this->name = "Generic";
		this->created = false;
		this->fullscreen = true;
		this->focused = true;
		this->running = true;
		this->paused = false;
		this->lastWidth = 0;
		this->lastHeight = 0;
		this->fps = 0;
		this->fpsCount = 0;
		this->fpsTimer = 0.0f;
		this->fpsResolution = 0.5f;
		this->timeDeltaMaxLimit = 0.2f;
		this->cursor = NULL;
		this->cursorVisible = false;
		this->virtualKeyboardVisible = false;
		this->virtualKeyboardHeightRatio = 0.0f;
		this->multiTouchActive = false;
		this->inputMode = InputMode::Mouse;
		this->virtualKeyboard = NULL;
		this->updateDelegate = NULL;
		this->mouseDelegate = NULL;
		this->keyboardDelegate = NULL;
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
			hlog::writef(logTag, "Creating window: '%s' (%d, %d) %s, '%s', (options: %s)",
				this->name.cStr(), w, h, fullscreen ? "fullscreen" : "windowed", title.cStr(), options.toString().cStr());
			this->fullscreen = fullscreen;
			this->title = title;
			this->options = options;
			this->created = true;
			this->paused = false;
			if (options.hotkeyFullscreen)
			{
				if (!fullscreen)
				{
					this->lastWidth = w;
					this->lastHeight = h;
				}
				else
				{
					SystemInfo info = april::getSystemInfo();
					this->lastWidth = hround(info.displayResolution.x * 0.6666667f);
					this->lastHeight = hround(info.displayResolution.y * 0.6666667f);
				}
			}
			this->fps = 0;
			this->fpsCount = 0;
			this->fpsTimer = 0.0f;
			this->fpsResolution = 0.5f;
			this->multiTouchActive = false;
			this->cursor = NULL;
			this->virtualKeyboardVisible = false;
			this->virtualKeyboardHeightRatio = 0.0f;
			this->inputMode = InputMode::Mouse;
			return true;
		}
		return false;
	}
	
	bool Window::destroy()
	{
		if (this->created)
		{
			hlog::writef(logTag, "Destroying window '%s'.", this->name.cStr());
			this->setVirtualKeyboard(NULL);
			this->created = false;
			this->paused = false;
			this->fps = 0;
			this->fpsCount = 0;
			this->fpsTimer = 0.0f;
			this->fpsResolution = 0.5f;
			this->multiTouchActive = false;
			this->cursor = NULL;
			this->virtualKeyboardVisible = false;
			this->virtualKeyboardHeightRatio = 0.0f;
			this->inputMode = InputMode::Mouse;
			this->virtualKeyboard = NULL;
			this->updateDelegate = NULL;
			this->mouseDelegate = NULL;
			this->keyboardDelegate = NULL;
			this->touchDelegate = NULL;
			this->controllerDelegate = NULL;
			this->systemDelegate = NULL;
			this->mouseEvents.clear();
			this->keyEvents.clear();
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

	void Window::setInputMode(InputMode value)
	{
		if (this->inputModeTranslations.hasKey(value))
		{
			value = this->inputModeTranslations[value];
		}
		if (this->inputMode != value)
		{
			this->inputMode = value;
			hlog::write(logTag, "Changing Input Mode to: " + this->inputMode.getName());
			if (this->inputMode == InputMode::Controller)
			{
				this->cursorPosition.set(-10000.0f, -10000.0f);
			}
			if (this->systemDelegate != NULL)
			{
				this->systemDelegate->onInputModeChanged(value);
			}
		}
	}

	void Window::setInputModeTranslations(hmap<InputMode, InputMode> value)
	{
		this->inputModeTranslations = value;
		if (this->inputModeTranslations.hasKey(this->inputMode))
		{
			this->inputMode = this->inputModeTranslations[this->inputMode];
			hlog::write(logTag, "Forcing Input Mode to: " + this->inputMode.getName());
			if (this->inputMode == InputMode::Controller)
			{
				this->cursorPosition.set(-10000.0f, -10000.0f);
			}
			if (this->systemDelegate != NULL)
			{
				this->systemDelegate->onInputModeChanged(this->inputMode);
			}
		}
	}

	gvec2 Window::getSize() const
	{
		return gvec2((float)this->getWidth(), (float)this->getHeight());
	}
	
	float Window::getAspectRatio() const
	{
		return ((float)this->getWidth() / this->getHeight());
	}

	void Window::setVirtualKeyboard(VirtualKeyboard* value)
	{
		if (value == NULL && this->virtualKeyboard != NULL)
		{
			bool visible = this->virtualKeyboard->isVisible();
			this->virtualKeyboard->hideKeyboard(true);
			if (visible && !this->virtualKeyboard->isVisible())
			{
				this->handleVirtualKeyboardChangeEvent(false, 0.0f);
			}
		}
		this->virtualKeyboard = value;
	}
	
	void Window::setCursorVisible(bool value)
	{
		this->cursorVisible = value;
		this->_refreshCursor();
	}

	void Window::setCursor(Cursor* value)
	{
		this->cursor = value;
		this->_refreshCursor();
	}

	bool Window::isCursorInside() const
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
			float factor = this->getOptions().defaultWindowModeResolutionFactor;
			w = (int)(w * factor);
			h = (int)(h * factor);
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
		hlog::warnf(logTag, "setResolution() is not available in '%s'.", this->name.cStr());
	}

	void Window::toggleHotkeyFullscreen()
	{
		if (this->options.hotkeyFullscreen)
		{
			SystemInfo info = april::getSystemInfo();
			int width = hround(info.displayResolution.x);
			int height = hround(info.displayResolution.y);
			if (!this->fullscreen)
			{
				this->lastWidth = this->getWidth();
				this->lastHeight = this->getHeight();
			}
			else
			{
				width = this->lastWidth;
				height = this->lastHeight;
			}
			this->setResolution(width, height, !this->fullscreen);
		}
	}

	void Window::_setRenderSystemResolution()
	{
		this->_setRenderSystemResolution(this->getWidth(), this->getHeight(), this->fullscreen);
	}
	
	void Window::_setRenderSystemResolution(int w, int h, bool fullscreen)
	{
		hlog::writef(logTag, "Setting window resolution: (%d,%d); fullscreen: %s", w, h, fullscreen ? "yes" : "no");
		april::rendersys->_deviceChangeResolution(w, h, fullscreen);
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
			if (this->updateDelegate != NULL)
			{
				this->updateDelegate->onPresentFrame();
			}
		}
	}

	bool Window::updateOneFrame()
	{
		TextureAsync::update();
		float timeDelta = this->_calcTimeSinceLastFrame();
		if (!this->focused)
		{
			hthread::sleep(40.0f);
		}
		this->checkEvents();
		return (this->performUpdate(timeDelta) && this->running);
	}
	
	void Window::checkEvents()
	{
		// due to possible problems with multiple scroll events in one frame, consecutive scroll events are merged (and so are move events for convenience)
		MouseInputEvent mouseEvent;
		gvec2 cumulativeScroll;
		while (this->mouseEvents.size() > 0) // required while instead of for, because this loop could modify this->mouseEvents when the event is propagated
		{
			mouseEvent = this->mouseEvents.removeFirst();
			if (mouseEvent.type != MouseInputEvent::Type::Cancel && mouseEvent.type != MouseInputEvent::Type::Scroll)
			{
				this->cursorPosition = mouseEvent.position;
			}
			if (mouseEvent.type == MouseInputEvent::Type::Scroll)
			{
				cumulativeScroll += mouseEvent.position;
				if (this->mouseEvents.size() == 0 || this->mouseEvents.first().type != MouseInputEvent::Type::Scroll)
				{
					this->handleMouseEvent(mouseEvent.type, cumulativeScroll, mouseEvent.keyCode);
					cumulativeScroll.set(0.0f, 0.0f);
				}
			}
			// if not a scroll event or final move event (because of merging)
			else if (mouseEvent.type != MouseInputEvent::Type::Move || (mouseEvent.type == MouseInputEvent::Type::Move &&
				(this->mouseEvents.size() == 0 || this->mouseEvents.first().type != MouseInputEvent::Type::Move)))
			{
				this->handleMouseEvent(mouseEvent.type, mouseEvent.position, mouseEvent.keyCode);
			}
		}
		KeyInputEvent keyEvent;
		while (this->keyEvents.size() > 0) // required while instead of for, because this loop could modify this->keyEvents when the event is propagated
		{
			keyEvent = this->keyEvents.removeFirst();
			this->handleKeyEvent(keyEvent.type, keyEvent.keyCode, keyEvent.charCode);
		}
		TouchInputEvent touchEvent;
		while (this->touchEvents.size() > 0) // required while instead of for, because this loop could modify this->touchEvents when the event is propagated
		{
			touchEvent = this->touchEvents.removeFirst();
			this->handleTouchEvent(touchEvent.touches);
		}
		ControllerInputEvent controllerEvent;
		while (this->controllerEvents.size() > 0) // required while instead of for, because this loop could modify this->controllerEvents when the event is propagated
		{
			controllerEvent = this->controllerEvents.removeFirst();
			this->handleControllerEvent(controllerEvent.type, controllerEvent.controllerIndex, controllerEvent.buttonCode, controllerEvent.axisValue);
		}
	}

	void Window::terminateMainLoop()
	{
		this->running = false;
	}

	void Window::showVirtualKeyboard()
	{
		if (this->virtualKeyboard != NULL)
		{
			bool visible = this->virtualKeyboard->isVisible();
			this->virtualKeyboard->showKeyboard(false);
			if (!visible && this->virtualKeyboard->isVisible())
			{
				this->handleVirtualKeyboardChangeEvent(true, this->virtualKeyboard->getHeightRatio());
			}
		}
	}
	
	void Window::hideVirtualKeyboard()
	{
		if (this->virtualKeyboard != NULL)
		{
			bool visible = this->virtualKeyboard->isVisible();
			this->virtualKeyboard->hideKeyboard(false);
			if (visible && !this->virtualKeyboard->isVisible())
			{
				this->handleVirtualKeyboardChangeEvent(false, 0.0f);
			}
		}
	}

	bool Window::performUpdate(float timeDelta)
	{
		if (this->paused)
		{
			timeDelta = 0.0f;
		}
		if (this->timeDeltaMaxLimit > 0.0f)
		{
			timeDelta = hmin(timeDelta, this->timeDeltaMaxLimit);
		}
		this->fpsTimer += timeDelta;
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
		// returning true: continue execution
		// returning false: abort execution
		if (this->updateDelegate != NULL)
		{
			if (this->updateDelegate->onUpdate(timeDelta))
			{
				if (this->virtualKeyboard != NULL && this->virtualKeyboard->isVisible())
				{
					this->virtualKeyboard->drawKeyboard();
				}
				return true;
			}
			return false;
		}
		april::rendersys->clear();
		return true;
	}
	
	void Window::handleMouseEvent(MouseInputEvent::Type type, cgvec2 position, Key keyCode)
	{
		if (this->mouseDelegate != NULL)
		{
			if (type == MouseInputEvent::Type::Down)
			{
				this->mouseDelegate->setCurrentCursorPosition(position);
				this->mouseDelegate->onMouseDown(keyCode);
			}
			else if (type == MouseInputEvent::Type::Up)
			{
				this->mouseDelegate->setCurrentCursorPosition(position);
				this->mouseDelegate->onMouseUp(keyCode);
			}
			else if (type == MouseInputEvent::Type::Cancel)
			{
				this->mouseDelegate->setCurrentCursorPosition(position);
				this->mouseDelegate->onMouseCancel(keyCode);
			}
			else if (type == MouseInputEvent::Type::Move)
			{
				this->mouseDelegate->setCurrentCursorPosition(position);
				this->mouseDelegate->onMouseMove();
			}
			else if (type == MouseInputEvent::Type::Scroll)
			{
				this->mouseDelegate->onMouseScroll(position.x, position.y);
			}
		}
	}
	
	void Window::handleKeyEvent(KeyInputEvent::Type type, Key keyCode, unsigned int charCode)
	{
		this->handleKeyOnlyEvent(type, keyCode); // doesn't do anything if keyCode is Key::None
		if (type == KeyInputEvent::Type::Down && charCode > 0) // ignores invalid chars
		{
			// according to the unicode standard, this range is undefined and reserved for system codes
			// for example, Mac OSX maps keys up, down, left, right to this key, inducing wrong char calls to the app.
			// source: http://en.wikibooks.org/wiki/Unicode/Character_reference/F000-FFFF
			if (charCode < 0xE000 || charCode > 0xF8FF)
			{
				this->handleCharOnlyEvent(charCode);
			}
		}
	}

	void Window::handleKeyOnlyEvent(KeyInputEvent::Type type, Key keyCode)
	{
		if (this->keyboardDelegate != NULL && keyCode != Key::None)
		{
			if (type == KeyInputEvent::Type::Down)
			{
				if (this->options.keyPause == keyCode)
				{
					this->paused = !this->paused;
				}
				this->keyboardDelegate->onKeyDown(keyCode);
			}
			else if (type == KeyInputEvent::Type::Up)
			{
				this->keyboardDelegate->onKeyUp(keyCode);
			}
			bool processed = false;
			// emulation of buttons using keyboard
			if (this->controllerEmulationKeys.hasKey(keyCode))
			{
				Button button = this->controllerEmulationKeys[keyCode];
				if (button != Button::AxisLX && button != Button::AxisLY && button != Button::AxisRX && button != Button::AxisRY && button != Button::TriggerL && button != Button::TriggerR)
				{
					this->handleControllerEvent((type == KeyInputEvent::Type::Down ? ControllerInputEvent::Type::Down : ControllerInputEvent::Type::Up), 0, button, 0.0f);
					processed = true;
				}
			}
			// emulation of positive axis values using keyboard
			if (!processed && this->controllerEmulationAxisesPositive.hasKey(keyCode))
			{
				Button button = this->controllerEmulationAxisesPositive[keyCode];
				if (button == Button::AxisLX || button == Button::AxisLY || button == Button::AxisRX || button == Button::AxisRY || button == Button::TriggerL || button == Button::TriggerR)
				{
					this->handleControllerEvent(ControllerInputEvent::Type::Axis, 0, button, (type == KeyInputEvent::Type::Down ? 1.0f : 0.0f));
					processed = true;
				}
			}
			// emulation of negative axis values using keyboard
			if (!processed && this->controllerEmulationAxisesNegative.hasKey(keyCode))
			{
				Button button = this->controllerEmulationAxisesNegative[keyCode];
				if (button == Button::AxisLX || button == Button::AxisLY || button == Button::AxisRX || button == Button::AxisRY)
				{
					this->handleControllerEvent(ControllerInputEvent::Type::Axis, 0, button, (type == KeyInputEvent::Type::Down ? -1.0f : 0.0f));
					processed = true;
				}
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

	void Window::handleTouchEvent(const harray<gvec2>& touches)
	{
		if (this->touchDelegate != NULL)
		{
			this->touchDelegate->onTouch(touches);
		}
	}

	void Window::handleControllerEvent(ControllerInputEvent::Type type, int controllerIndex, Button buttonCode, float axisValue)
	{
		if (this->controllerDelegate != NULL)
		{
			if (buttonCode != Button::None)
			{
				if (type == ControllerInputEvent::Type::Down)
				{
					this->controllerDelegate->onButtonDown(controllerIndex, buttonCode);
				}
				else if (type == ControllerInputEvent::Type::Up)
				{
					this->controllerDelegate->onButtonUp(controllerIndex, buttonCode);
				}
				else if (type == ControllerInputEvent::Type::Axis)
				{
					this->controllerDelegate->onControllerAxisChange(controllerIndex, buttonCode, axisValue);
				}
			}
			else // connection change always uses Button::None
			{
				if (type == ControllerInputEvent::Type::Connected)
				{
					this->controllerDelegate->onControllerConnectionChanged(controllerIndex, true);
				}
				else if (type == ControllerInputEvent::Type::Disconnected)
				{
					this->controllerDelegate->onControllerConnectionChanged(controllerIndex, false);
				}
			}
		}
	}

	bool Window::handleQuitRequestEvent(bool canCancel)
	{
		// returns whether or not the windowing system is permitted to close the window
		return (this->systemDelegate == NULL || this->systemDelegate->onQuit(canCancel));
	}
	
	void Window::handleFocusChangeEvent(bool focused)
	{
		this->focused = focused;
		hlog::write(logTag, "Window " + hstr(focused ? "gained focus." : "lost focus."));
		if (this->systemDelegate != NULL)
		{
			this->systemDelegate->onWindowFocusChanged(focused);
		}
	}

	void Window::handleActivityChange(bool active)
	{
		hlog::warn(logTag, this->name + " does not implement activity change events!");
	}
	
	void Window::handleVirtualKeyboardChangeEvent(bool visible, float heightRatio)
	{
		this->virtualKeyboardVisible = visible;
		this->virtualKeyboardHeightRatio = heightRatio;
		if (this->systemDelegate != NULL)
		{
			this->systemDelegate->onVirtualKeyboardChanged(this->virtualKeyboardVisible, this->virtualKeyboardHeightRatio);
		}
	}

	void Window::handleLowMemoryWarningEvent()
	{
		hlog::writef(logTag, "Processing low memory warning. Current RAM: %lld B; Current VRAM: %lld B", april::getRamConsumption(), april::rendersys->getVRamConsumption());
		if (this->systemDelegate != NULL)
		{
			this->systemDelegate->onLowMemoryWarning();
			hlog::writef(logTag, "Low memory warning processed. Current RAM: %lld B; Current VRAM: %lld B", april::getRamConsumption(), april::rendersys->getVRamConsumption());
		}
	}

	void Window::queueMouseEvent(MouseInputEvent::Type type, cgvec2 position, Key keyCode)
	{
		this->mouseEvents += MouseInputEvent(type, position, keyCode);
	}

	void Window::queueKeyEvent(KeyInputEvent::Type type, Key keyCode, unsigned int charCode)
	{
		this->keyEvents += KeyInputEvent(type, keyCode, charCode);
	}

	void Window::queueTouchEvent(MouseInputEvent::Type type, cgvec2 position, int index)
	{
		harray<gvec2> previousTouches = this->touches;
		if (type == MouseInputEvent::Type::Down)
		{
			if (index < this->touches.size()) // DOWN event of an already indexed touch, never happened so far
			{
				return;
			}
			this->touches += position;
		}
		else if (type == MouseInputEvent::Type::Up)
		{
			if (index >= this->touches.size()) // redundant UP event, can happen
			{
				return;
			}
			this->touches.removeAt(index);
		}
		else if (type == MouseInputEvent::Type::Move)
		{
			if (index >= this->touches.size()) // MOVE event of an unindexed touch, never happened so far
			{
				return;
			}
			this->touches[index] = position;
		}
		else if (type == MouseInputEvent::Type::Cancel) // canceling a particular pointer, required by specific systems (e.g. WinRT)
		{
			if (index < this->touches.size())
			{
				this->touches.removeAt(index);
				if (this->touches.size() == 0)
				{
					this->multiTouchActive = false;
				}
			}
			return;
		}
		if (this->multiTouchActive || this->touches.size() > 1)
		{
			if (!this->multiTouchActive && previousTouches.size() == 1)
			{
				// cancel (notify the app) that the previously called mouse-down event is canceled so multi-touch can be properly processed
				this->queueMouseEvent(MouseInputEvent::Type::Cancel, previousTouches.first(), Key::MouseL);
			}
			this->multiTouchActive = (this->touches.size() > 0);
		}
		else
		{
			this->queueMouseEvent(type, position, Key::MouseL);
		}
		this->touchEvents.clear();
		this->touchEvents += TouchInputEvent(this->touches);
	}

	void Window::queueControllerEvent(ControllerInputEvent::Type type, int controllerIndex, Button buttonCode, float axisValue)
	{
		this->controllerEvents += ControllerInputEvent(type, controllerIndex, buttonCode, axisValue);
	}

	float Window::_calcTimeSinceLastFrame()
	{
		float timeDelta = this->timer.diff(true);
		if (!this->focused)
		{
			timeDelta = 0.0f;
		}
		return timeDelta;
	}

	hstr Window::findCursorResource(chstr filename) const
	{
		hstr _filename;
		foreachc (hstr, it, this->cursorExtensions)
		{
			_filename = filename + (*it);
			if (hresource::exists(_filename))
			{
				return _filename;
			}
		}
		return "";
	}
	
	hstr Window::findCursorFile(chstr filename) const
	{
		hstr _filename;
		foreachc (hstr, it, this->cursorExtensions)
		{
			_filename = filename + (*it);
			if (hfile::exists(_filename))
			{
				return _filename;
			}
		}
		return "";
	}

	Cursor* Window::createCursorFromResource(chstr filename)
	{
		return this->_createCursorFromSource(true, filename);
	}

	Cursor* Window::createCursorFromFile(chstr filename)
	{
		return this->_createCursorFromSource(false, filename);
	}

	Cursor* Window::_createCursorFromSource(bool fromResource, chstr filename)
	{
		hstr name = (fromResource ? this->findCursorResource(filename) : this->findCursorFile(filename));
		if (name == "")
		{
			return NULL;
		}
		Cursor* cursor = this->_createCursor(fromResource);
		if (cursor != NULL && !cursor->_create(name))
		{
			delete cursor;
			return NULL;
		}
		return cursor;
	}

	april::Cursor* Window::_createCursor(bool fromResource)
	{
		hlog::warnf(logTag, "Cursors are not available in '%s'.", this->name.cStr());
		return NULL;
	}

	void Window::destroyCursor(Cursor* cursor)
	{
		if (this->cursor == cursor)
		{
			this->setCursor(NULL);
		}
		delete cursor;
	}

	void Window::_refreshCursor()
	{
	}

}
