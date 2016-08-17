/// @file
/// @version 4.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Defines a generic window.

#ifndef APRIL_WINDOW_H
#define APRIL_WINDOW_H

#include <gtypes/Vector2.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hmap.h>
#include <hltypes/hstring.h>

#include "aprilExport.h"
#include "Keys.h"
#include "Timer.h"

namespace april
{
	class Cursor;
	class ControllerDelegate;
	class KeyboardDelegate;
	class MouseDelegate;
	class RenderSystem;
	class SystemDelegate;
	class TouchDelegate;
	class UpdateDelegate;

	/// @brief Defines a generic window.
	class aprilExport Window
	{
	public:
		/// @brief Defines the input mode.
		enum InputMode
		{
			/// @brief Using a mouse for input.
			MOUSE,
			/// @brief Using a touch-based interface for input.
			TOUCH,
			/// @brief Using a controller for input.
			CONTROLLER
		};

		/// @brief Defines mouse event types.
		enum MouseEventType
		{
			/// @brief Mouse button was pressed.
			MOUSE_DOWN = 0,
			/// @brief Mouse button was released.
			MOUSE_UP = 1,
			/// @brief Mouse button was canceled without an "up" event.
			MOUSE_CANCEL = 2,
			/// @brief Mouse was moved.
			MOUSE_MOVE = 3,
			/// @brief Mouse scroll was changed (usually a scroll wheel).
			MOUSE_SCROLL = 4
		};

		/// @brief Defines keyboard key event types.
		enum KeyEventType
		{
			/// @brief Key was pressed.
			KEY_DOWN = 0,
			/// @brief Key was released.
			KEY_UP = 1
		};

		/// @brief Defines controller input event types.
		enum ControllerEventType
		{
			/// @brief Controller button was pressed.
			CONTROLLER_DOWN = 0,
			/// @brief Controller button was released.
			CONTROLLER_UP = 1,
			/// @brief Controller axis position was changed.
			CONTROLLER_AXIS = 2,
			/// @brief Controller connected.
			CONTROLLER_CONNECTED = 3,
			/// @brief Controller disconnected.
			CONTROLLER_DISCONNECTED = 4
		};

		/// @brief Defines mouse input event data.
		struct MouseInputEvent
		{
			/// @brief The event type.
			MouseEventType type;
			/// @brief The pointer position.
			gvec2 position;
			/// @brief The key code.
			Key keyCode;
			
			/// @brief Basic constructor.
			MouseInputEvent();
			/// @brief Constructor.
			/// @param[in] type The event type.
			/// @param[in] position The pointer position.
			/// @param[in] keyCode The key code.
			MouseInputEvent(MouseEventType type, gvec2 position, Key keyCode);
		
		};

		/// @brief Defines keyboard input event data.
		struct KeyInputEvent
		{
			/// @brief The event type.
			KeyEventType type;
			/// @brief The key code.
			Key keyCode;
			/// @brief The character Unicode value.
			unsigned int charCode;
			
			/// @brief Basic constructor.
			KeyInputEvent();
			/// @brief Constructor.
			/// @param[in] type The event type.
			/// @param[in] keyCode The key code.
			/// @param[in] charCode The character Unicode value.
			KeyInputEvent(KeyEventType type, Key keyCode, unsigned int charCode);
		
		};

		/// @brief Defines touch-interface input event data.
		struct TouchInputEvent
		{
			/// @brief Active touch pointers.
			harray<gvec2> touches;
			
			/// @brief Basic constructor.
			TouchInputEvent();
			/// @brief Constructor.
			/// @param[in] touches Active touch pointers.
			TouchInputEvent(harray<gvec2>& touches);
		
		};

		/// @brief Defines controller input event data.
		struct ControllerInputEvent
		{
			/// @brief The event type.
			ControllerEventType type;
			/// @brief Index of the controller.
			int controllerIndex;
			/// @brief The button code.
			Button buttonCode;
			/// @brief axisValue The axis value.
			float axisValue;
			
			/// @brief Basic constructor.
			ControllerInputEvent();
			/// @brief Constructor.
			/// @param[in] type The event type.
			/// @param[in] buttonCode The button code.
			/// @param[in] axisValue The axis value.
			ControllerInputEvent(ControllerEventType type, int controllerIndex, Button buttonCode, float axisValue);
			
		};

		/// @brief Defines options for creation of the window.
		struct aprilExport Options
		{
		public:
			/// @brief Whether the window is resizable.
			bool resizable;
			/// @brief Whether the FPS counter should be displayed in the window title.
			bool fpsCounter;
			/// @brief Whether the fullscreen hotkey should be allowed.
			bool hotkeyFullscreen;
			/// @brief The factor used for downscaling a window size when not running in fullscreen.
			float defaultWindowModeResolutionFactor;
			/// @brief Special hack for Mac implementations.
			bool mac_displayLinkIgnoreSystemRedraw; // TODOa - probably should be refactored or removed

			/// @brief Basic constructor.
			Options();
			/// @brief Destructor.
			~Options();

			/// @brief Creates a string representation of the object.
			/// @return A string representation of the object.
			hstr toString() const;

		};

		/// @brief Basic constructor.
		Window();
		/// @brief Destructor.
		virtual ~Window();
		/// @brief Creates the Window.
		/// @param[in] w Width of the window's rendering area.
		/// @param[in] h Height of the window's rendering area.
		/// @param[in] fullscreen Whether the window should be created in fullscreen or not.
		/// @param[in] title The title to be displayed on the window title bar.
		/// @param[in] options The Options object.
		/// @return True if successful.
		virtual bool create(int w, int h, bool fullscreen, chstr title, Window::Options options);
		/// @brief Destroys the Window.
		/// @return True if successful.
		virtual bool destroy();
		/// @brief Unassigns the Window from a RenderSystem.
		/// @note This is usually used only internally and is needed for some internal call ordering purposes.
		virtual void unassign();

		/// @brief Window name.
		HL_DEFINE_GET(hstr, name, Name);
		/// @brief Window creation options.
		HL_DEFINE_GET(Options, options, Options);
		/// @brief Whether the Window was created.
		HL_DEFINE_IS(created, Created);
		/// @brief The title.
		HL_DEFINE_GET(hstr, title, Title);
		/// @brief Whether the Window is displayed in fullscreen or not.
		HL_DEFINE_IS(fullscreen, Fullscreen);
		/// @brief Sets the Window fullscreen display.
		/// @param[in] value Whether the Window should switch to fullscreen (true) or windowed (false).
		void setFullscreen(bool value);
		/// @brief Whether the Window is focused.
		HL_DEFINE_IS(focused, Focused);
		/// @brief Whether the Window is running.
		HL_DEFINE_IS(running, Running);
		/// @brief The current FPS.
		HL_DEFINE_GETSET(int, fps, Fps);
		/// @brief The FPS resolution.
		HL_DEFINE_GETSET(float, fpsResolution, FpsResolution);
		/// @brief The maximum allowed time-delta between frames.
		/// @note Limiting this makes sense, because on weak hardware configurations it allows that large frameskips don't result in too large time skips.
		HL_DEFINE_GETSET(float, timeDeltaMaxLimit, TimeDeltaMaxLimit);
		/// @brief The system cursor.
		HL_DEFINE_GET(Cursor*, cursor, Cursor);
		/// @brief The cursor position.
		HL_DEFINE_GET(gvec2, cursorPosition, CursorPosition);
		/// @brief Whether the virtual keyboard is visible.
		HL_DEFINE_IS(virtualKeyboardVisible, VirtualKeyboardVisible);
		/// @brief The ratio of how much height of the Window the virtual keyboard takes up.
		HL_DEFINE_GET(float, virtualKeyboardHeightRatio, VirtualKeyboardHeightRatio);
		/// @brief Gets the current input mode.
		HL_DEFINE_GET(InputMode, inputMode, InputMode);
		/// @brief Sets the input mode.
		/// @param[in] value The new input mode.
		void setInputMode(InputMode value);
		/// @brief Gets the input mode translation map.
		HL_DEFINE_GET2(hmap, InputMode, InputMode, inputModeTranslations, InputModeTranslations);
		/// @brief Sets the input mode translation map.
		/// @param[in] value The new input mode translation map.
		void setInputModeTranslations(hmap<InputMode, InputMode> value);
		/// @brief Gets the Window size.
		/// @return The Window size.
		gvec2 getSize() const;
		/// @brief Gets the Window size aspect ratio.
		/// @return The Window size aspect ratio.
		float getAspectRatio() const;
		/// @brief The controller emulation keys.
		/// @note This is useful when testing controller input functionality without actually using a controller.
		HL_DEFINE_GETSET2(hmap, Key, Button, controllerEmulationKeys, ControllerEmulationKeys);
		/// @brief The controller emulation keys for positive axis values.
		/// @note This is useful when testing controller input functionality without actually using a controller.
		HL_DEFINE_GETSET2(hmap, Key, Button, controllerEmulationAxisesPositive, ControllerEmulationAxisesPositive);
		/// @brief The controller emulation keys for negative axis values.
		/// @note This is useful when testing controller input functionality without actually using a controller.
		HL_DEFINE_GETSET2(hmap, Key, Button, controllerEmulationAxisesNegative, ControllerEmulationAxisesNegative);

		/// @brief The update delegate.
		HL_DEFINE_GETSET(UpdateDelegate*, updateDelegate, UpdateDelegate);
		/// @brief The mouse input delegate.
		HL_DEFINE_GETSET(MouseDelegate*, mouseDelegate, MouseDelegate);
		/// @brief The keyboard input delegate.
		HL_DEFINE_GETSET(KeyboardDelegate*, keyboardDelegate, KeyboardDelegate);
		/// @brief The touch input delegate.
		HL_DEFINE_GETSET(TouchDelegate*, touchDelegate, TouchDelegate);
		/// @brief The controller input delegate.
		HL_DEFINE_GETSET(ControllerDelegate*, controllerDelegate, ControllerDelegate);
		/// @brief The system handling delegate.
		HL_DEFINE_GETSET(SystemDelegate*, systemDelegate, SystemDelegate);

		/// @brief Sets the Window title.
		/// @param[in] value The new title.
		virtual inline void setTitle(chstr value) { this->title = value; }
		/// @brief Gets the cursor visibility flag.
		/// @return The cursor visibility flag.
		virtual inline bool isCursorVisible() const { return this->cursorVisible; }
		/// @brief Sets the cursor visibility flag.
		/// @param[in] value The new cursor visibility flag.
		virtual void setCursorVisible(bool value);
		/// @brief Sets the system cursor.
		/// @param[in] value The new system cursor.
		virtual void setCursor(Cursor* value);
		/// @brief Checks if the cursor is inside the client/rendering area of the Window.
		/// @return True if the cursor is inside the client/rendering area of the Window.
		virtual bool isCursorInside() const;

		/// @brief Sets the Window resolution/size.
		/// @param[in] w Width of the window's rendering area.
		/// @param[in] h Height of the window's rendering area.
		virtual void setResolution(int w, int h);
		/// @brief Sets the Window resolution/size with fullscreen manipulation.
		/// @param[in] w Width of the window's rendering area.
		/// @param[in] h Height of the window's rendering area.
		/// @param[in] fullscreen Whether the window should be switched to fullscreen or windowed.
		virtual void setResolution(int w, int h, bool fullscreen);
		/// @brief Toggles fullscreen/window mode.
		/// @note Remembers the last windowed size and returns to it. Useful when using directly with a fullscreen hotkey.
		/// @see setFullscreen
		virtual void toggleHotkeyFullscreen();

		/// @brief Gets the Window's rendering area width.
		/// @return The Window's rendering area width.
		virtual int getWidth() const = 0;
		/// @brief Gets the Window's rendering area height.
		/// @return The Window's rendering area height.
		virtual int getHeight() const = 0;
		/// @brief Gets the Window's internal backend ID.
		/// @return The Window's internal backend ID.
		virtual void* getBackendId() const = 0;

		/// @brief Flushes the currently rendered data to the backbuffer for display.
		/// @note Usually this doesn't need to be called manually.
		virtual void presentFrame() = 0;

		/// @brief Updates the entire application by one frame.
		/// @return True if the application should continue to run.
		virtual bool updateOneFrame();
		/// @brief Processed queued system events.
		virtual void checkEvents();
		/// @brief Aborts execution and forces the application to exit after the current frame is complete.
		/// @note It is safer to return false in your implementation of UpdateDelegate::onUpdate().
		virtual void terminateMainLoop();
		/// @brief Displays a virtual keyboard if necessary.
		/// @note Some systems don't support this while on other this is the only way to handle any kind of keyboard input.
		virtual inline void showVirtualKeyboard() { }
		/// @brief Hides the virtual keyboard if necessary.
		virtual inline void hideVirtualKeyboard() { }
		/// @brief Finds the actual filename of a texture resource file.
		/// @param[in] filename Resource filename without the extension.
		/// @return The detected resource filename or an empty string if no resource file could be found.
		virtual hstr findCursorResource(chstr filename) const;
		/// @brief Finds the actual filename of a texture file.
		/// @param[in] filename Filename without the extension.
		/// @return The detected filename or an empty string if no file could be found.
		virtual hstr findCursorFile(chstr filename) const;

		/// @brief Creates a Cursor object from a resource file.
		/// @param[in] filename The filename of the resource.
		/// @return The created Cursor object or NULL if failed.
		Cursor* createCursorFromResource(chstr filename);
		/// @brief Creates a Cursor object from a file.
		/// @param[in] filename The filename of the file.
		/// @return The created Cursor object or NULL if failed.
		Cursor* createCursorFromFile(chstr filename);
		/// @brief Destroys a Cursor object from a file.
		/// @param[in] cursor The cursor to be destroyed.
		void destroyCursor(Cursor* cursor);

		/// @brief Gets the screen rotation flag.
		/// @return The screen rotation flag.
		/// @note This is mostly used internally.
		// TODOa - this doesn't seem to be used anywhere, maybe it should be removed
		virtual inline bool isRotating() const { return false; } // iOS/Android devices for example
		/// @brief Gets an internal system parameter.
		/// @param[in] parameter Name of the parameter.
		/// @return Internal system parameter.
		// TODOaa - this should be refactored to have all parameters available in the header
		virtual inline hstr getParam(chstr parameter) { return ""; }
		/// @brief Sets an internal system parameter.
		/// @param[in] parameter Name of the parameter.
		/// @param[in] value Value to be set.
		// TODOaa - this should be refactored to have all parameters available in the header
		virtual inline void setParam(chstr parameter, chstr value) { }
		
		/// @brief Handles a mouse event and propagates it to the delegate.
		/// @param[in] type The event type.
		/// @param[in] position The pointer position.
		/// @param[in] keyCode The key code.
		virtual void handleMouseEvent(MouseEventType type, gvec2 position, Key keyCode);
		/// @brief Handles a keyboard event and propagates it to the delegate.
		/// @param[in] type The event type.
		/// @param[in] keyCode The key code.
		/// @param[in] charCode The character Unicode value.
		virtual void handleKeyEvent(KeyEventType type, Key keyCode, unsigned int charCode);
		/// @brief Handles a touch event and propagates it to the delegate.
		/// @param[in] touches Active touch pointers.
		virtual void handleTouchEvent(const harray<gvec2>& touches);
		/// @brief Handles a controller event and propagates it to the delegate.
		/// @param[in] type The event type.
		/// @param[in] controllerIndex Index of the controller.
		/// @param[in] buttonCode The button code.
		/// @param[in] axisValue The axis value.
		virtual void handleControllerEvent(ControllerEventType type, int controllerIndex, Button buttonCode, float axisValue);
		/// @brief Handles a quit event and propagates it to the delegate.
		/// @param[in] canCancel Whether the window quitting can be canceled.
		/// @return True if the system is allowed to actually close the window.
		virtual bool handleQuitRequestEvent(bool canCancel);
		/// @brief Handles a focus-change event and propagates it to the delegate.
		/// @param[in] focused Whether the window is focused now.
		virtual void handleFocusChangeEvent(bool focused);
		/// @brief Handles a virtual-keyboard-change event and propagates it to the delegate.
		/// @param[in] visible Whether the virtual keyboard is visible.
		/// @param[in] heightRatio The ratio of the screen height that the keyboard takes up.
		virtual void handleVirtualKeyboardChangeEvent(bool visible, float heightRatio);
		/// @brief Handles a low memory warning event and propagates it to the delegate.
		virtual void handleLowMemoryWarningEvent();

		/// @brief Handles a keyboard key press event and propagates it to the delegate.
		/// @param[in] type The event type.
		/// @param[in] keyCode The key code.
		/// @note This is a utility function.
		/// @see handleKeyEvent
		void handleKeyOnlyEvent(KeyEventType type, Key keyCode);
		/// @brief Handles a keyboard character event and propagates it to the delegate.
		/// @param[in] charCode The character Unicode value.
		/// @note This is a utility function.
		/// @see handleKeyEvent
		void handleCharOnlyEvent(unsigned int charCode);

		/// @brief Handles a activity-change event.
		/// @param[in] active Whether the window is active now.
		/// @note This is a different concept from focus-change that is usually only used in certain implementations.
		virtual void handleActivityChange(bool active);

		/// @brief Queues a mouse event for processing before the start of the next frame.
		/// @param[in] type The event type.
		/// @param[in] position The pointer position.
		/// @param[in] keyCode The key code.
		/// @note This is mostly used internally, but it can also be used to simulate input.
		virtual void queueMouseEvent(MouseEventType type, gvec2 position, Key keyCode);
		/// @brief Queues a keyboard event for processing before the start of the next frame.
		/// @param[in] type The event type.
		/// @param[in] keyCode The key code.
		/// @param[in] charCode The character Unicode value.
		/// @note This is mostly used internally, but it can also be used to simulate input.
		virtual void queueKeyEvent(KeyEventType type, Key keyCode, unsigned int charCode);
		/// @brief Queues a touch event for processing before the start of the next frame.
		/// @param[in] type The event type.
		/// @param[in] position The pointer position.
		/// @param[in] index The pointer index.
		/// @note This is mostly used internally, but it can also be used to simulate input.
		virtual void queueTouchEvent(MouseEventType type, gvec2 position, int index);
		/// @brief Queues a controller event for processing before the start of the next frame.
		/// @param[in] type The event type.
		/// @param[in] controllerIndex Index of the controller.
		/// @param[in] buttonCode The button code.
		/// @param[in] axisValue The axis value.
		/// @note This is mostly used internally, but it can also be used to simulate input.
		virtual void queueControllerEvent(ControllerEventType type, int controllerIndex, Button buttonCode, float axisValue);

		/// @brief Starts the main loop.
		/// @note This is usually called internally in some implementations, but it's possible to call it manually if a custom april_main implementation is used.
		virtual void enterMainLoop();
		/// @brief Performs the update of one frame.
		/// @param[in] timeDelta Time that has passed since the last frame.
		/// @note This is usually called internally in some implementations, but it's possible to call it manually if a custom april_main implementation is used.
		virtual bool performUpdate(float timeDelta);
		
#ifndef DOXYGEN_SHOULD_SKIP_THIS
		// TODOaa - refactor or maybe even remove this
		// the following functions should be temporary, it was added because I needed access to
		// iOS early initialization process. When april will be refactored this needs to be changed --kspes
		static inline void setLaunchCallback(void (*callback)(void*)) { msLaunchCallback = callback; }
		static void handleLaunchCallback(void* args);

		DEPRECATED_ATTRIBUTE inline Cursor* createCursor(chstr filename) { return this->createCursorFromResource(filename); }
		DEPRECATED_ATTRIBUTE inline bool isTouchEnabled() { return (this->inputMode == TOUCH); }
		DEPRECATED_ATTRIBUTE inline bool isVirtualKeyboardActive() { return this->isVirtualKeyboardVisible(); }
		DEPRECATED_ATTRIBUTE inline void beginKeyboardHandling() { this->showVirtualKeyboard(); }
		DEPRECATED_ATTRIBUTE inline void terminateKeyboardHandling() { this->hideVirtualKeyboard(); }
#endif

	protected:
		/// @brief Whether the Window was created.
		bool created;
		/// @brief The name.
		hstr name;
		/// @brief The text in the window title bar.
		hstr title;
		/// @brief Whether the Window is fullscreen or windowed.
		bool fullscreen;
		/// @brief The options used to create the Window.
		Options options;
		/// @brief Whether the Window is focused.
		bool focused;
		/// @brief Whether the Window system is running.
		bool running;
		/// @brief Previous width.
		/// @note Used when restoring the window size after switching from fullscreen to windowed.
		int lastWidth;
		/// @brief Previous height.
		/// @note Used when restoring the window size after switching from fullscreen to windowed.
		int lastHeight;
		/// @brief FPS of the last mesaure.
		int fps;
		/// @brief Current counter for FPS calculation.
		int fpsCount;
		/// @brief Current timer for FPS calculation.
		float fpsTimer;
		/// @brief FPS update resolution.
		float fpsResolution;
		/// @brief Maximum allowed time-delta that are propagated into the UpdateDelegate.
		/// @note Limiting this makes sense, because on weak hardware configurations it allows that large frameskips don't result in too large time skips.
		float timeDeltaMaxLimit;
		/// @brief Current cursor position.
		gvec2 cursorPosition;
		/// @brief Current system cursor.
		Cursor* cursor;
		/// @brief Whether the system cursor is visible.
		bool cursorVisible;
		/// @brief Whether the virtual keyboard is currently visible.
		bool virtualKeyboardVisible;
		/// @brief The ratio of how much screen height the virtual keyboard is currently taking up.
		float virtualKeyboardHeightRatio;
		/// @brief The current input mode.
		InputMode inputMode;
		/// @brief The input mode translation map.
		hmap<InputMode, InputMode> inputModeTranslations;
		/// @brief The filename extensions supported for cursor image files.
		harray<hstr> cursorExtensions;
		/// @brief Whether multi-touch mode is currently active.
		bool multiTouchActive;
		/// @brief The current active touch pointers.
		harray<gvec2> touches;
		/// @brief Queued mouse events.
		harray<MouseInputEvent> mouseEvents;
		/// @brief Queued keyboard events.
		harray<KeyInputEvent> keyEvents;
		/// @brief Queued touch events.
		harray<TouchInputEvent> touchEvents;
		/// @brief Queued controller events.
		harray<ControllerInputEvent> controllerEvents;
		/// @brief The Timer object used for timing purposes.
		Timer timer;
		/// @brief The controller emulation keys.
		/// @note This is useful when testing controller input functionality without actually using a controller.
		hmap<Key, Button> controllerEmulationKeys;
		/// @brief The controller emulation keys for positive axis values.
		/// @note This is useful when testing controller input functionality without actually using a controller.
		hmap<Key, Button> controllerEmulationAxisesPositive;
		/// @brief The controller emulation keys for negative axis values.
		/// @note This is useful when testing controller input functionality without actually using a controller.
		hmap<Key, Button> controllerEmulationAxisesNegative;

		/// @brief The current update delegate.
		UpdateDelegate* updateDelegate;
		/// @brief The current mouse delegate.
		MouseDelegate* mouseDelegate;
		/// @brief The current keyboard delegate.
		KeyboardDelegate* keyboardDelegate;
		/// @brief The current touch delegate.
		TouchDelegate* touchDelegate;
		/// @brief The current controller delegate.
		ControllerDelegate* controllerDelegate;
		/// @brief The current system delegate.
		SystemDelegate* systemDelegate;

		/// @brief Internally safe method for creating a Cursor object.
		/// @param[in] fromResource Whether the Cursor should be created from a resource file or a normal file.
		/// @param[in] filename The filename of the cursor.
		/// @return The created Cursor object or NULL if failed.
		Cursor* _createCursorFromSource(bool fromResource, chstr filename);

		/// @brief Calculates the time passed since the render of the last frame using a Timer.
		/// @return The time passed since the render of the last frame.
		virtual float _calcTimeSinceLastFrame();
		/// @brief Calls _setRenderSystemResolution() with the current Window parameters.
		/// @see _setRenderSystemResolution(int w, int h, bool fullscreen)
		void _setRenderSystemResolution();
		/// @brief Calls the RenderSystem method for changing the resolution to synchronize Window and RenderSystem.
		/// @param[in] w New width of the resolutin.
		/// @param[in] h New height of the resolutin.
		/// @param[in] fullscreen Whether the display is now fullscreen or windowed.
		virtual void _setRenderSystemResolution(int w, int h, bool fullscreen);

		/// @brief Creates the actual system Cursor.
		/// @param[in] fromResource Whether the Cursor is created from a resource file or a normal file.
		/// @return The created Cursor object or NULL if not supported on this Window implementation.
		virtual Cursor* _createCursor(bool fromResource);
		/// @brief Sets the internal system cursor.
		virtual void _refreshCursor();

#ifndef DOXYGEN_SHOULD_SKIP_THIS
		// TODOaa - refactor or maybe even remove this
		// the following functions should be temporary, it was added because I needed access to
		// iOS early initialization process. When april will be refactored this needs to be changed --kspes
		static void(*msLaunchCallback)(void*);
#endif
	};

	/// @brief The global Window instance.
	aprilExport extern april::Window* window;

}
#endif
