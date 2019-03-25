/// @file
/// @version 5.2
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

#include "Application.h"
#include "april.h"
#include "AsyncCommands.h"
#include "ControllerDelegate.h"
#include "Cursor.h"
#include "KeyDelegate.h"
#include "Keys.h"
#include "MotionDelegate.h"
#include "MouseDelegate.h"
#include "Platform.h"
#include "RenderSystem.h"
#include "SetWindowResolutionCommand.h"
#include "SystemDelegate.h"
#include "TextureAsync.h"
#include "TouchDelegate.h"
#include "UpdateDelegate.h"
#include "VirtualKeyboard.h"
#include "Window.h"

namespace april
{
	Window* window = NULL;
	
	Window::Options::Options()
	{
#ifndef _UWP
		this->resizable = false;
#else // UWP has resizable turned on by default and cannot be turned off
		this->resizable = true;
#endif
		this->fpsCounter = false;
		this->hotkeyFullscreen = false;
		this->minimized = false;
		this->suspendUpdateThread = true;
		this->keyPause = april::Key::None;
		this->mac_displayLinkIgnoreSystemRedraw = false;
		this->enableTouchInput = true;
		this->defaultWindowModeResolutionFactor = 0.8f;
	}
	
	hstr Window::Options::toString() const
	{
		harray<hstr> options;
		if (this->resizable)
		{
			options += "Resizable";
		}
		if (this->hotkeyFullscreen)
		{
			options += "Fullscreen Hotkey";
		}
		if (this->minimized)
		{
			options += "Minimized";
		}
		if (!this->suspendUpdateThread)
		{
			options += "No Update-Suspend";
		}
		if (options.size() == 0)
		{
			options += "None";
		}
		return options.joined(',');
	}
	
	Window::Window()
	{
		this->name = "Generic";
		this->created = false;
		this->fullscreen = true;
		this->focused = true;
		this->presentFrameEnabled = true;
		this->paused = false;
		this->lastWidth = 0;
		this->lastHeight = 0;
		this->cursor = NULL;
		this->cursorVisible = false;
		this->virtualKeyboardVisible = false;
		this->virtualKeyboardHeightRatio = 0.0f;
		this->multiTouchActive = false;
		this->inputMode = InputMode::Mouse;
		this->virtualKeyboard = NULL;
		this->updateDelegate = NULL;
		this->mouseDelegate = NULL;
		this->keyDelegate = NULL;
		this->touchDelegate = NULL;
		this->controllerDelegate = NULL;
		this->motionDelegate = NULL;
		this->systemDelegate = NULL;
	}
	
	Window::~Window()
	{
		this->destroy();
	}

	bool Window::create(int width, int height, bool fullscreen, chstr title, Window::Options options)
	{
		if (!this->created)
		{
			this->created = true;
			april::rendersys->_addAsyncCommand(new CreateWindowCommand(width, height, fullscreen, title, options));
			return true;
		}
		return false;
	}
	
	void Window::_systemCreate(int width, int height, bool fullscreen, chstr title, Window::Options options)
	{
		hlog::writef(logTag, "Creating window: '%s' (%d, %d) %s, '%s', (options: %s)",
			this->name.cStr(), width, height, fullscreen ? "fullscreen" : "windowed", title.cStr(), options.toString().cStr());
		this->fullscreen = fullscreen;
		this->title = title;
		this->options = options;
		this->paused = false;
		if (options.hotkeyFullscreen)
		{
			if (!fullscreen)
			{
				this->lastWidth = width;
				this->lastHeight = height;
			}
			else
			{
				SystemInfo info = april::getSystemInfo();
				this->lastWidth = hround(info.displayResolution.x * 0.6666667f);
				this->lastHeight = hround(info.displayResolution.y * 0.6666667f);
			}
		}
		this->multiTouchActive = false;
		this->cursor = NULL;
		this->virtualKeyboardVisible = false;
		this->virtualKeyboardHeightRatio = 0.0f;
		this->inputMode = InputMode::Mouse;
	}

	bool Window::destroy()
	{
		if (this->created)
		{
			this->created = false;
			april::rendersys->_addAsyncCommand(new DestroyWindowCommand());
			return true;
		}
		return false;
	}

	void Window::_systemDestroy()
	{
		hlog::writef(logTag, "Destroying window '%s'.", this->name.cStr());
		this->setVirtualKeyboard(NULL);
		this->paused = false;
		this->multiTouchActive = false;
		this->cursor = NULL;
		this->virtualKeyboardVisible = false;
		this->virtualKeyboardHeightRatio = 0.0f;
		this->inputMode = InputMode::Mouse;
		this->virtualKeyboard = NULL;
		this->updateDelegate = NULL;
		this->mouseDelegate = NULL;
		this->keyDelegate = NULL;
		this->touchDelegate = NULL;
		this->controllerDelegate = NULL;
		this->motionDelegate = NULL;
		this->systemDelegate = NULL;
		this->mouseEvents.clear();
		this->keyEvents.clear();
		this->touchEvents.clear();
		this->touchesEvents.clear();
		this->controllerEvents.clear();
		this->controllerEmulationKeys.clear();
	}

	void Window::unassign()
	{
		april::rendersys->_addAsyncCommand(new UnassignWindowCommand());
	}

	void Window::_systemUnassign()
	{
	}

	void Window::setInputMode(const InputMode& value)
	{
		InputMode finalValue = value;
		if (this->inputModeTranslations.hasKey(finalValue))
		{
			finalValue = this->inputModeTranslations[finalValue];
		}
		if (this->inputMode != finalValue)
		{
			this->inputMode = finalValue;
			hlog::write(logTag, "Changing Input Mode to: " + this->inputMode.getName());
			if (this->inputMode == InputMode::Controller)
			{
				this->cursorPosition.set(-10000.0f, -10000.0f);
			}
		}
	}

	void Window::setInputModeTranslations(const hmap<InputMode, InputMode>& value)
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
		}
	}

	gvec2i Window::getSize() const
	{
		return gvec2i(this->getWidth(), this->getHeight());
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
				this->queueVirtualKeyboardChange(false, 0.0f);
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
		return grectf(0.0f, 0.0f, this->getSize()).isPointInside(this->getCursorPosition());
	}

	void Window::setFullscreen(const bool& value)
	{
		int width = this->getWidth();
		int height = this->getHeight();
		if (value)
		{
			SystemInfo info = april::getSystemInfo();
			width = info.displayResolution.x;
			height = info.displayResolution.y;
		}
		else
		{
			width = hround(width * this->options.defaultWindowModeResolutionFactor);
			height = hround(height * this->options.defaultWindowModeResolutionFactor);
		}
		this->setResolution(width, height, value);
	}

	void Window::setResolution(int width, int height)
	{
		this->setResolution(width, height, this->isFullscreen());
	}

	void Window::setResolution(int width, int height, bool fullscreen)
	{
		if (this->getWidth() != width || this->getHeight() != height || this->fullscreen != fullscreen)
		{
			april::rendersys->_addAsyncCommand(new SetWindowResolutionCommand(width, height, fullscreen));
		}
	}

	void Window::toggleFullscreen()
	{
		gvec2i newSize = this->_getToggleHotkeyFullscreenSize();
		this->setResolution(newSize.x, newSize.y, !this->fullscreen);
	}

	void Window::_systemToggleHotkeyFullscreen()
	{
		if (this->options.hotkeyFullscreen)
		{
			gvec2i newSize = this->_getToggleHotkeyFullscreenSize();
			this->_systemSetResolution(newSize.x, newSize.y, !this->fullscreen);
			this->queueSizeChange(this->getWidth(), this->getHeight(), this->fullscreen);
		}
	}

	gvec2i Window::_getToggleHotkeyFullscreenSize()
	{
		gvec2i result;
		if (!this->fullscreen)
		{
			SystemInfo info = april::getSystemInfo();
			result = info.displayResolution;
			this->lastWidth = this->getWidth();
			this->lastHeight = this->getHeight();
		}
		else
		{
			result.set(this->lastWidth, this->lastHeight);
		}
		return result;
	}

	void Window::_systemSetResolution(int width, int height, bool fullscreen)
	{
		hlog::warnf(logTag, "Changing the resolution is not available in '%s'.", this->name.cStr());
	}

	void Window::_setRenderSystemResolution()
	{
		this->_setRenderSystemResolution(this->getWidth(), this->getHeight(), this->fullscreen);
	}
	
	void Window::_setRenderSystemResolution(int width, int height, bool fullscreen)
	{
		april::rendersys->_deviceChangeResolution(width, height, fullscreen);
	}
	
	void Window::_presentFrame(bool systemEnabled)
	{
		april::application->_updateFps();
	}

	bool Window::update(float timeDelta)
	{
		this->_processEvents();
		if (!this->focused)
		{
			hthread::sleep(40.0f);
		}
		return (this->performUpdate(timeDelta) && april::application->getState() == Application::State::Running);
	}

	void Window::checkEvents()
	{
	}

	void Window::_processEvents()
	{
		hmutex::ScopeLock lock(&this->eventMutex);
		harray<GenericEvent> genericEvents = this->genericEvents;
		harray<MouseEvent> mouseEvents = this->mouseEvents;
		harray<KeyEvent> keyEvents = this->keyEvents;
		harray<TouchEvent> touchEvents = this->touchEvents;
		harray<TouchesEvent> touchesEvents = this->touchesEvents;
		harray<ControllerEvent> controllerEvents = this->controllerEvents;
		harray<MotionEvent> motionEvents = this->motionEvents;
		this->genericEvents.clear();
		this->mouseEvents.clear();
		this->keyEvents.clear();
		this->touchEvents.clear();
		this->touchesEvents.clear();
		this->controllerEvents.clear();
		this->motionEvents.clear();
		lock.release();
		GenericEvent sizeEvent(GenericEvent::Type::SizeChange);
		bool receivedMemoryWarning = false;
		for_iter (i, 0, genericEvents.size())
		{
			if (genericEvents[i].type == GenericEvent::Type::QuitRequest)
			{
				if (this->handleQuitRequest(genericEvents[i].boolValue))
				{
					april::application->finish();
				}
			}
			else if (genericEvents[i].type == GenericEvent::Type::FocusChange)
			{
				this->handleFocusChange(genericEvents[i].boolValue);
			}
			else if (genericEvents[i].type == GenericEvent::Type::ActivityChange)
			{
				this->handleActivityChange(genericEvents[i].boolValue);
			}
			else if (genericEvents[i].type == GenericEvent::Type::SizeChange)
			{
				sizeEvent = genericEvents[i]; // merging size events
			}
			else if (genericEvents[i].type == GenericEvent::Type::InputModeChange)
			{
				this->handleInputModeChange(InputMode::fromInt(genericEvents[i].intValue));
			}
			else if (genericEvents[i].type == GenericEvent::Type::VirtualKeyboardChange)
			{
				this->handleVirtualKeyboardChange(genericEvents[i].boolValue, genericEvents[i].floatValue);
			}
			else if (genericEvents[i].type == GenericEvent::Type::LowMemoryWarning)
			{
				receivedMemoryWarning = true; // merging low memory warning events
			}
			else if (genericEvents[i].type == GenericEvent::Type::Screenshot)
			{
				this->handleScreenshot(genericEvents[i].image);
			}
		}
		if (sizeEvent.intValue > 0 && sizeEvent.intValueOther > 0)
		{
			this->handleSizeChange(sizeEvent.intValue, sizeEvent.intValueOther, sizeEvent.boolValue);
		}
		if (receivedMemoryWarning)
		{
			this->handleLowMemoryWarning();
		}
		// due to possible problems with multiple scroll events in one frame, consecutive scroll events are merged (and so are move events for convenience)
		gvec2f cumulativeScroll;
		for_iter (i, 0, mouseEvents.size())
		{
			if (mouseEvents[i].type != MouseEvent::Type::Cancel && mouseEvents[i].type != MouseEvent::Type::Scroll)
			{
				this->cursorPosition = mouseEvents[i].position;
			}
			if (mouseEvents[i].type == MouseEvent::Type::Scroll)
			{
				cumulativeScroll += mouseEvents[i].position;
				// if final event or next event is not a scroll event (because of merging)
				if (i == mouseEvents.size() - 1 || mouseEvents[i + 1].type != MouseEvent::Type::Scroll)
				{
					this->handleMouseInput(mouseEvents[i].type, cumulativeScroll, mouseEvents[i].keyCode);
					cumulativeScroll.set(0.0f, 0.0f);
				}
			}
			// if not a move event at all or final move event or next event is not a move event (because of merging)
			else if (mouseEvents[i].type != MouseEvent::Type::Move || (mouseEvents[i].type == MouseEvent::Type::Move &&
				(i == mouseEvents.size() - 1 || mouseEvents[i + 1].type != MouseEvent::Type::Move)))
			{
				this->handleMouseInput(mouseEvents[i].type, mouseEvents[i].position, mouseEvents[i].keyCode);
			}
		}
		for_iter (i, 0, keyEvents.size())
		{
			this->handleKeyInput(keyEvents[i].type, keyEvents[i].keyCode, keyEvents[i].charCode);
		}
		for_iter (i, 0, touchEvents.size())
		{
			this->touchPositions[touchEvents[i].index] = touchEvents[i].position;
			// if not a move event at all or final move event or next event is not a move event (because of merging)
			if (touchEvents[i].type != TouchEvent::Type::Move || (touchEvents[i].type == TouchEvent::Type::Move &&
				(i == touchEvents.size() - 1 || touchEvents[i + 1].type != TouchEvent::Type::Move || touchEvents[i + 1].index != touchEvents[i].index)))
			{
				this->handleTouchInput(touchEvents[i].type, touchEvents[i].index, touchEvents[i].position);
			}
		}
		for_iter (i, 0, touchesEvents.size())
		{
			this->handleTouchesInput(touchesEvents[i].touches);
		}
		for_iter (i, 0, controllerEvents.size())
		{
			this->handleControllerInput(controllerEvents[i].type, controllerEvents[i].controllerIndex, controllerEvents[i].buttonCode, controllerEvents[i].axisValue);
		}
		for_iter (i, 0, motionEvents.size())
		{
			this->handleMotionInput(motionEvents[i].type, motionEvents[i].motionVector);
		}
	}

	void Window::terminateMainLoop()
	{
		april::application->finish();
	}

	void Window::showVirtualKeyboard()
	{
		if (this->virtualKeyboard != NULL)
		{
			bool visible = this->virtualKeyboard->isVisible();
			this->virtualKeyboard->showKeyboard(false);
			if (!visible && this->virtualKeyboard->isVisible())
			{
				this->queueVirtualKeyboardChange(true, this->virtualKeyboard->getHeightRatio());
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
				this->queueVirtualKeyboardChange(false, 0.0f);
			}
		}
	}

	bool Window::performUpdate(float timeDelta)
	{
		if (this->paused)
		{
			timeDelta = 0.0f;
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
	
	bool Window::handleQuitRequest(bool canCancel)
	{
		// returns whether or not the windowing system is permitted to close the window
		return (this->systemDelegate == NULL || this->systemDelegate->onQuit(canCancel));
	}

	void Window::handleFocusChange(bool focused)
	{
		this->focused = focused;
		hlog::write(logTag, "Window focus change: " + hstr(focused ? "true" : "false"));
		if (this->systemDelegate != NULL)
		{
			this->systemDelegate->onWindowFocusChanged(focused);
		}
	}

	void Window::handleActivityChange(bool active)
	{
		hlog::warn(logTag, this->name + " does not implement activity change events!");
	}

	void Window::handleSizeChange(int width, int height, bool fullscreen)
	{
		hlog::writef(logTag, "Setting window resolution: (%d,%d); fullscreen: %s", width, height, fullscreen ? "yes" : "no");
		if (this->systemDelegate != NULL)
		{
			this->systemDelegate->onWindowSizeChanged(width, height, fullscreen);
		}
	}

	void Window::handleInputModeChange(const InputMode& inputMode)
	{
		InputMode newInputMode = inputMode;
		if (this->inputModeTranslations.hasKey(newInputMode))
		{
			newInputMode = this->inputModeTranslations[newInputMode];
		}
		if (this->inputMode != newInputMode)
		{
			this->inputMode = newInputMode;
			hlog::write(logTag, "Changing Input Mode to: " + this->inputMode.getName());
			if (this->inputMode == InputMode::Controller)
			{
				this->cursorPosition.set(-10000.0f, -10000.0f);
			}
			if (this->systemDelegate != NULL)
			{
				this->systemDelegate->onInputModeChanged(newInputMode);
			}
		}
	}

	void Window::handleVirtualKeyboardChange(bool visible, float heightRatio)
	{
		this->virtualKeyboardVisible = visible;
		this->virtualKeyboardHeightRatio = heightRatio;
		if (this->systemDelegate != NULL)
		{
			this->systemDelegate->onVirtualKeyboardChanged(this->virtualKeyboardVisible, this->virtualKeyboardHeightRatio);
		}
	}

	void Window::handleLowMemoryWarning()
	{
		hlog::writef(logTag, "Processing low memory warning. Current RAM: %lld B; Current VRAM: %lld B", april::getRamConsumption(), april::rendersys->getVRamConsumption());
		if (this->systemDelegate != NULL)
		{
			this->systemDelegate->onLowMemoryWarning();
			hlog::writef(logTag, "Low memory warning processed. Current RAM: %lld B; Current VRAM: %lld B", april::getRamConsumption(), april::rendersys->getVRamConsumption());
		}
	}

	void Window::handleScreenshot(Image* image)
	{
		if (this->systemDelegate != NULL)
		{
			this->systemDelegate->onScreenshot(image);
		}
	}

	void Window::handleMouseInput(MouseEvent::Type type, cgvec2f position, Key keyCode)
	{
		if (this->mouseDelegate != NULL)
		{
			if (type == MouseEvent::Type::Down)
			{
				this->mouseDelegate->setCurrentCursorPosition(position);
				this->mouseDelegate->onMouseDown(keyCode);
			}
			else if (type == MouseEvent::Type::Up)
			{
				this->mouseDelegate->setCurrentCursorPosition(position);
				this->mouseDelegate->onMouseUp(keyCode);
			}
			else if (type == MouseEvent::Type::Cancel)
			{
				this->mouseDelegate->setCurrentCursorPosition(position);
				this->mouseDelegate->onMouseCancel(keyCode);
			}
			else if (type == MouseEvent::Type::Move)
			{
				this->mouseDelegate->setCurrentCursorPosition(position);
				this->mouseDelegate->onMouseMove();
			}
			else if (type == MouseEvent::Type::Scroll)
			{
				this->mouseDelegate->onMouseScroll(position.x, position.y);
			}
		}
	}
	
	void Window::handleKeyInput(KeyEvent::Type type, Key keyCode, unsigned int charCode)
	{
		this->handleKeyOnlyInput(type, keyCode); // doesn't do anything if keyCode is Key::None
		if (type == KeyEvent::Type::Down && charCode > 0) // ignores invalid chars
		{
			// according to the unicode standard, this range is undefined and reserved for system codes
			// for example, Mac OSX maps keys up, down, left, right to this key, inducing wrong char calls to the app.
			// source: http://en.wikibooks.org/wiki/Unicode/Character_reference/F000-FFFF
			if (charCode < 0xE000 || charCode > 0xF8FF)
			{
				this->handleCharOnlyInput(charCode);
			}
		}
	}

	void Window::handleKeyOnlyInput(KeyEvent::Type type, Key keyCode)
	{
		if (this->keyDelegate != NULL && keyCode != Key::None)
		{
			if (type == KeyEvent::Type::Down)
			{
				if (this->options.keyPause == keyCode)
				{
					this->paused = !this->paused;
				}
				this->keyDelegate->onKeyDown(keyCode);
			}
			else if (type == KeyEvent::Type::Up)
			{
				this->keyDelegate->onKeyUp(keyCode);
			}
			bool processed = false;
			// emulation of buttons using keyboard
			if (this->controllerEmulationKeys.hasKey(keyCode))
			{
				Button button = this->controllerEmulationKeys[keyCode];
				if (button != Button::AxisLX && button != Button::AxisLY && button != Button::AxisRX && button != Button::AxisRY && button != Button::TriggerL && button != Button::TriggerR)
				{
					this->handleControllerInput((type == KeyEvent::Type::Down ? ControllerEvent::Type::Down : ControllerEvent::Type::Up), 0, button, 0.0f);
					processed = true;
				}
			}
			// emulation of positive axis values using keyboard
			if (!processed && this->controllerEmulationAxisesPositive.hasKey(keyCode))
			{
				Button button = this->controllerEmulationAxisesPositive[keyCode];
				if (button == Button::AxisLX || button == Button::AxisLY || button == Button::AxisRX || button == Button::AxisRY || button == Button::TriggerL || button == Button::TriggerR)
				{
					this->handleControllerInput(ControllerEvent::Type::Axis, 0, button, (type == KeyEvent::Type::Down ? 1.0f : 0.0f));
					processed = true;
				}
			}
			// emulation of negative axis values using keyboard
			if (!processed && this->controllerEmulationAxisesNegative.hasKey(keyCode))
			{
				Button button = this->controllerEmulationAxisesNegative[keyCode];
				if (button == Button::AxisLX || button == Button::AxisLY || button == Button::AxisRX || button == Button::AxisRY)
				{
					this->handleControllerInput(ControllerEvent::Type::Axis, 0, button, (type == KeyEvent::Type::Down ? -1.0f : 0.0f));
					// processed = true; // commenting out since value is not used further in this function
				}
			}
		}
	}

	void Window::handleCharOnlyInput(unsigned int charCode)
	{
		if (this->keyDelegate != NULL && charCode >= 32 && charCode != 127) // special hack, backspace induces a character in some implementations
		{
			this->keyDelegate->onChar(charCode);
		}
	}

	void Window::handleTouchInput(TouchEvent::Type type, int index, cgvec2f position)
	{
		if (this->touchDelegate != NULL)
		{
			hmap<int, gvec2f> currentTouches = this->touchDelegate->getCurrentTouches();
			currentTouches[index] = position;
			this->touchDelegate->setCurrentTouches(currentTouches);
			if (type == TouchEvent::Type::Down)
			{
				this->touchDelegate->onTouchDown(index);
			}
			else if (type == TouchEvent::Type::Up)
			{
				this->touchDelegate->onTouchUp(index);
			}
			else if (type == TouchEvent::Type::Move)
			{
				this->touchDelegate->onTouchMove(index);
			}
		}
	}

	void Window::handleTouchesInput(const harray<gvec2f>& touches)
	{
		if (this->touchDelegate != NULL)
		{
			this->touchDelegate->onTouch(touches);
		}
	}

	void Window::handleControllerInput(ControllerEvent::Type type, int controllerIndex, Button buttonCode, float axisValue)
	{
		if (this->controllerDelegate != NULL)
		{
			if (buttonCode != Button::None)
			{
				if (type == ControllerEvent::Type::Down)
				{
					this->controllerDelegate->onButtonDown(controllerIndex, buttonCode);
				}
				else if (type == ControllerEvent::Type::Up)
				{
					this->controllerDelegate->onButtonUp(controllerIndex, buttonCode);
				}
				else if (type == ControllerEvent::Type::Axis)
				{
					this->controllerDelegate->onControllerAxisChange(controllerIndex, buttonCode, axisValue);
				}
			}
			else // connection change always uses Button::None
			{
				if (type == ControllerEvent::Type::Connected)
				{
					this->controllerDelegate->onControllerConnectionChanged(controllerIndex, true);
				}
				else if (type == ControllerEvent::Type::Disconnected)
				{
					this->controllerDelegate->onControllerConnectionChanged(controllerIndex, false);
				}
			}
		}
	}

	void Window::handleMotionInput(MotionEvent::Type type, cgvec3f motionVector)
	{
		if (this->motionDelegate != NULL)
		{
			if (type == MotionEvent::Type::Accelerometer)
			{
				this->motionDelegate->onAccelerometer(motionVector);
			}
			else if (type == MotionEvent::Type::LinearAccelerometer)
			{
				this->motionDelegate->onLinearAccelerometer(motionVector);
			}
			else if (type == MotionEvent::Type::Gravity)
			{
				this->motionDelegate->onGravity(motionVector);
			}
			else if (type == MotionEvent::Type::Rotation)
			{
				this->motionDelegate->onRotation(motionVector);
			}
			else if (type == MotionEvent::Type::Gyroscope)
			{
				this->motionDelegate->onGyroscope(motionVector);
			}
		}
	}

	bool Window::queueQuitRequest(bool canCancel)
	{
		hmutex::ScopeLock lock(&this->eventMutex);
		this->genericEvents += GenericEvent(GenericEvent::Type::QuitRequest, canCancel);
		return false; // always return false, the app needs to make a decision whether to terminate or not
	}

	void Window::queueFocusChange(bool focused)
	{
		hmutex::ScopeLock lock(&this->eventMutex);
		this->genericEvents += GenericEvent(GenericEvent::Type::FocusChange, focused);
	}

	void Window::queueActivityChange(bool active)
	{
		hmutex::ScopeLock lock(&this->eventMutex);
		this->genericEvents += GenericEvent(GenericEvent::Type::ActivityChange, active);
	}

	void Window::queueSizeChange(int width, int height, bool fullscreen)
	{
		hmutex::ScopeLock lock(&this->eventMutex);
		this->genericEvents += GenericEvent(GenericEvent::Type::SizeChange, width, height, fullscreen);
	}

	void Window::queueInputModeChange(const InputMode& inputMode)
	{
		hmutex::ScopeLock lock(&this->eventMutex);
		this->genericEvents += GenericEvent(GenericEvent::Type::InputModeChange, (int)inputMode.value);
	}

	void Window::queueVirtualKeyboardChange(bool visible, float heightRatio)
	{
		hmutex::ScopeLock lock(&this->eventMutex);
		this->genericEvents += GenericEvent(GenericEvent::Type::VirtualKeyboardChange, visible, heightRatio);
	}

	void Window::queueLowMemoryWarning()
	{
		hmutex::ScopeLock lock(&this->eventMutex);
		this->genericEvents += GenericEvent(GenericEvent::Type::LowMemoryWarning);
	}

	void Window::queueScreenshot(Image* image)
	{
		hmutex::ScopeLock lock(&this->eventMutex);
		this->genericEvents += GenericEvent(GenericEvent::Type::Screenshot, image);
	}

	void Window::queueMouseInput(MouseEvent::Type type, cgvec2f position, Key keyCode)
	{
		hmutex::ScopeLock lock(&this->eventMutex);
		this->mouseEvents += MouseEvent(type, position, keyCode);
	}

	void Window::queueKeyInput(KeyEvent::Type type, Key keyCode, unsigned int charCode)
	{
		hmutex::ScopeLock lock(&this->eventMutex);
		this->keyEvents += KeyEvent(type, keyCode, charCode);
	}

	void Window::queueTouchInput(TouchEvent::Type type, int index, cgvec2f position)
	{
		hmutex::ScopeLock lock(&this->eventMutex);
		harray<int> indices = this->indexedTouches.keys();
		indices.sort();
		harray<gvec2f> previousTouches = this->indexedTouches.values(indices);
		if (type == TouchEvent::Type::Down)
		{
			if (this->indexedTouches.hasKey(index)) // DOWN event of an already indexed touch, never happened so far
			{
				return;
			}
			this->indexedTouches[index] = position;
		}
		else if (type == TouchEvent::Type::Up)
		{
			if (!this->indexedTouches.hasKey(index)) // redundant UP event, can happen
			{
				return;
			}
			this->indexedTouches.removeKey(index);
		}
		else if (type == TouchEvent::Type::Move)
		{
			if (!this->indexedTouches.hasKey(index)) // MOVE event of an unindexed touch, never happened so far
			{
				return;
			}
			this->indexedTouches[index] = position;
		}
		else if (type == TouchEvent::Type::Cancel) // canceling a particular pointer, required by specific systems (e.g. UWP)
		{
			if (!this->indexedTouches.hasKey(index))
			{
				return;
			}
			this->indexedTouches.removeKey(index);
		}
		indices = this->indexedTouches.keys();
		indices.sort();
		harray<gvec2f> currentTouches = this->indexedTouches.values(indices);
		if (this->multiTouchActive || currentTouches.size() > 1)
		{
			if (!this->multiTouchActive && previousTouches.size() == 1)
			{
				// cancel (notify the app) that the previously called mouse-down event is canceled so multi-touch can be properly processed
				this->mouseEvents += MouseEvent(MouseEvent::Type::Cancel, previousTouches.first(), Key::MouseL);
			}
			this->multiTouchActive = (currentTouches.size() > 0);
		}
		else
		{
			this->mouseEvents += MouseEvent(MouseEvent::Type::fromName(type.getName()), position, Key::MouseL);
		}
		this->touchEvents += TouchEvent(type, index, position);
		this->touchesEvents.clear();
		this->touchesEvents += TouchesEvent(currentTouches);
	}

	void Window::queueControllerInput(ControllerEvent::Type type, int controllerIndex, Button buttonCode, float axisValue)
	{
		hmutex::ScopeLock lock(&this->eventMutex);
		this->controllerEvents += ControllerEvent(type, controllerIndex, buttonCode, axisValue);
	}

	void Window::queueMotionInput(MotionEvent::Type type, cgvec3f motionVector)
	{
		hmutex::ScopeLock lock(&this->eventMutex);
		this->motionEvents += MotionEvent(type, motionVector);
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
