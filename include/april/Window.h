/// @file
/// @version 5.2
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
#include <gtypes/Vector3.h>
#include <hltypes/henum.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hmap.h>
#include <hltypes/hmutex.h>
#include <hltypes/hstring.h>

#include "aprilExport.h"
#include "Events.h"
#include "InputMode.h"
#include "Keys.h"
#include "Timer.h"

namespace april
{
	class Application;
	class CreateWindowCommand;
	class ControllerDelegate;
	class Cursor;
	class DestroyWindowCommand;
	class KeyDelegate;
	class MotionDelegate;
	class MouseDelegate;
	class RenderSystem;
	class SetWindowResolutionCommand;
	class SystemDelegate;
	class TouchDelegate;
	class UnassignWindowCommand;
	class UpdateDelegate;
	class VirtualKeyboard;

	/// @brief Defines a generic window.
	class aprilExport Window
	{
	public:
		friend class Application;
		friend class CreateWindowCommand;
		friend class DestroyWindowCommand;
		friend class RenderSystem;
		friend class SetWindowResolutionCommand;
		friend class UnassignWindowCommand;

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
			/// @brief Whether the the window should start minimized.
			bool minimized;
			/// @brief Whether the update thread should be suspended with a mutex lock.
			bool suspendUpdateThread;
			/// @brief Whether the fullscreen hotkey should be allowed.
			april::Key keyPause;
			/// @brief The factor used for downscaling a window size when not running in fullscreen.
			float defaultWindowModeResolutionFactor;
			/// @brief Special hack for Mac implementations.
			bool mac_displayLinkIgnoreSystemRedraw; // TODOa - probably should be refactored or removed
			/// @brief toggle to enable/disable touch input on platforms that support it, enabled by default
			bool enableTouchInput;

			/// @brief Basic constructor.
			Options();

			/// @brief Creates a string representation of the object.
			/// @return A string representation of the object.
			hstr toString() const;

		};

		/// @brief Basic constructor.
		Window();
		/// @brief Destructor.
		virtual ~Window();
		/// @brief Creates the Window.
		/// @param[in] width Width of the window's rendering area.
		/// @param[in] height Height of the window's rendering area.
		/// @param[in] fullscreen Whether the window should be created in fullscreen or not.
		/// @param[in] title The title to be displayed on the window title bar.
		/// @param[in] options The Options object.
		/// @return True if successful.
		bool create(int width, int height, bool fullscreen, chstr title, Window::Options options);
		/// @brief Destroys the Window.
		/// @return True if successful.
		bool destroy();
		/// @brief Unassigns the Window from a RenderSystem.
		/// @note This is usually used only internally and is needed for some internal call ordering purposes.
		void unassign();

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
		void setFullscreen(const bool& value);
		/// @brief Whether the Window is focused.
		HL_DEFINE_IS(focused, Focused);
		/// @brief Whether presentFrame() will do proper processing.
		HL_DEFINE_ISSET(presentFrameEnabled, PresentFrameEnabled);
		/// @brief The system cursor.
		HL_DEFINE_GET(Cursor*, cursor, Cursor);
		/// @brief The cursor position.
		HL_DEFINE_GET(gvec2f, cursorPosition, CursorPosition);
		/// @brief Whether the virtual keyboard is visible.
		HL_DEFINE_IS(virtualKeyboardVisible, VirtualKeyboardVisible);
		/// @brief The ratio of how much height of the Window the virtual keyboard takes up.
		HL_DEFINE_GET(float, virtualKeyboardHeightRatio, VirtualKeyboardHeightRatio);
		/// @brief Gets the current input mode.
		HL_DEFINE_GET(InputMode, inputMode, InputMode);
		/// @brief Sets the input mode.
		/// @param[in] value The new input mode.
		void setInputMode(const InputMode& value);
		/// @brief Gets the input mode translation map.
		HL_DEFINE_GET2(hmap, InputMode, InputMode, inputModeTranslations, InputModeTranslations);
		/// @brief Sets the input mode translation map.
		/// @param[in] value The new input mode translation map.
		void setInputModeTranslations(const hmap<InputMode, InputMode>& value);
		/// @brief Gets the Window size.
		/// @return The Window size.
		gvec2i getSize() const;
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

		/// @brief Gets custom virtual keyboard implementation.
		HL_DEFINE_GET(VirtualKeyboard*, virtualKeyboard, VirtualKeyboard);
		/// @brief Sets custom virtual keyboard implementation.
		void setVirtualKeyboard(VirtualKeyboard* value);
		/// @brief The update delegate.
		HL_DEFINE_GETSET(UpdateDelegate*, updateDelegate, UpdateDelegate);
		/// @brief The mouse input delegate.
		HL_DEFINE_GETSET(MouseDelegate*, mouseDelegate, MouseDelegate);
		/// @brief The keyboard input delegate.
		HL_DEFINE_GETSET(KeyDelegate*, keyDelegate, KeyDelegate);
		/// @brief The touch input delegate.
		HL_DEFINE_GETSET(TouchDelegate*, touchDelegate, TouchDelegate);
		/// @brief The controller input delegate.
		HL_DEFINE_GETSET(ControllerDelegate*, controllerDelegate, ControllerDelegate);
		/// @brief The motion input delegate.
		HL_DEFINE_GETSET(MotionDelegate*, motionDelegate, MotionDelegate);
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
		/// @param[in] width Width of the window's rendering area.
		/// @param[in] height Height of the window's rendering area.
		virtual void setResolution(int width, int height);
		/// @brief Sets the Window resolution/size with fullscreen manipulation.
		/// @param[in] width Width of the window's rendering area.
		/// @param[in] height Height of the window's rendering area.
		/// @param[in] fullscreen Whether the window should be switched to fullscreen or windowed.
		virtual void setResolution(int width, int height, bool fullscreen);
		/// @brief Toggles fullscreen/window mode.
		/// @note Remembers the last windowed size and returns to it. Useful when using directly with a fullscreen hotkey.
		/// @see setFullscreen
		virtual void toggleFullscreen();

		/// @brief Gets the Window's rendering area width.
		/// @return The Window's rendering area width.
		virtual int getWidth() const = 0;
		/// @brief Gets the Window's rendering area height.
		/// @return The Window's rendering area height.
		virtual int getHeight() const = 0;
		/// @brief Gets the Window's internal backend ID.
		/// @return The Window's internal backend ID.
		virtual void* getBackendId() const = 0;

		/// @brief Updates the entire application by one frame.
		/// @param[in] timeDelta Time since last frame.
		/// @return True if the application should continue to run.
		virtual bool update(float timeDelta);
		/// @brief Processed queued system events.
		virtual void checkEvents();
		/// @brief Aborts execution and forces the application to exit after the current frame is complete.
		/// @note It is safer to return false in your implementation of UpdateDelegate::onUpdate().
		HL_DEPRECATED("Deprecated API. Use Application::finish() instead.")
		virtual void terminateMainLoop(); // TODOx - move to Application class
		/// @brief Displays a virtual keyboard if necessary.
		/// @note Some systems don't support this while on other this is the only way to handle any kind of keyboard input.
		virtual void showVirtualKeyboard();
		/// @brief Hides the virtual keyboard if necessary.
		virtual void hideVirtualKeyboard();
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
		
		/// @brief Handles a quit event and propagates it to the delegate.
		/// @param[in] canCancel Whether the window quitting can be canceled.
		/// @return True if the system is allowed to actually close the window.
		virtual bool handleQuitRequest(bool canCancel);
		/// @brief Handles a focus-change event and propagates it to the delegate.
		/// @param[in] focused Whether the window is focused now.
		virtual void handleFocusChange(bool focused);
		/// @brief Handles a activity-change event.
		/// @param[in] active Whether the window is active now.
		/// @note This is a different concept from focus-change that is usually only used in certain implementations.
		virtual void handleActivityChange(bool active);
		/// @brief Handles a size-change event and propagates it to the delegate.
		/// @param[in] width New width of the resolutin.
		/// @param[in] height New height of the resolutin.
		/// @param[in] fullscreen Whether the display is now fullscreen or windowed.
		virtual void handleSizeChange(int width, int height, bool fullscreen);
		/// @brief Handles a input-mode-change event and propagates it to the delegate.
		/// @param[in] inputMode New input mode.
		virtual void handleInputModeChange(const InputMode& inputMode);
		/// @brief Handles a virtual-keyboard-change event and propagates it to the delegate.
		/// @param[in] visible Whether the virtual keyboard is visible.
		/// @param[in] heightRatio The ratio of the screen height that the keyboard takes up.
		virtual void handleVirtualKeyboardChange(bool visible, float heightRatio);
		/// @brief Handles a low memory warning event and propagates it to the delegate.
		virtual void handleLowMemoryWarning();
		/// @brief Handles a screenshot event and propagates it to the delegate.
		/// @param[in] image The Image objet containing the screenshot bitmap data.
		virtual void handleScreenshot(Image* image);
		/// @brief Handles a mouse event and propagates it to the delegate.
		/// @param[in] type The event type.
		/// @param[in] position The pointer position.
		/// @param[in] keyCode The key code.
		virtual void handleMouseInput(MouseEvent::Type type, cgvec2f position, Key keyCode);
		/// @brief Handles a keyboard event and propagates it to the delegate.
		/// @param[in] type The event type.
		/// @param[in] keyCode The key code.
		/// @param[in] charCode The character Unicode value.
		virtual void handleKeyInput(KeyEvent::Type type, Key keyCode, unsigned int charCode);
		/// @brief Handles a touch event and propagates it to the delegate.
		/// @param[in] type The event type.
		/// @param[in] index The touch index.
		/// @param[in] position The touch position.
		virtual void handleTouchInput(TouchEvent::Type type, int index, cgvec2f position);
		/// @brief Handles an indiscriminate touches event and propagates it to the delegate.
		/// @param[in] touches Active touch pointers.
		virtual void handleTouchesInput(const harray<gvec2f>& touches);
		/// @brief Handles a controller event and propagates it to the delegate.
		/// @param[in] type The event type.
		/// @param[in] controllerIndex Index of the controller.
		/// @param[in] buttonCode The button code.
		/// @param[in] axisValue The axis value.
		virtual void handleControllerInput(ControllerEvent::Type type, int controllerIndex, Button buttonCode, float axisValue);
		/// @brief Handles a motion event and propagates it to the delegate.
		/// @param[in] type The event type.
		/// @param[in] motionVector Motion data vector.
		virtual void handleMotionInput(MotionEvent::Type type, cgvec3f motionVector);

		/// @brief Handles a keyboard key press event and propagates it to the delegate.
		/// @param[in] type The event type.
		/// @param[in] keyCode The key code.
		/// @note This is a utility function.
		/// @see handleKeyEvent
		void handleKeyOnlyInput(KeyEvent::Type type, Key keyCode);
		/// @brief Handles a keyboard character event and propagates it to the delegate.
		/// @param[in] charCode The character Unicode value.
		/// @note This is a utility function.
		/// @see handleKeyEvent
		void handleCharOnlyInput(unsigned int charCode);

		/// @brief Queues a quit event.
		/// @param[in] canCancel Whether the window quitting can be canceled.
		/// @return True if the system is allowed to actually close the window.
		virtual bool queueQuitRequest(bool canCancel);
		/// @brief Queues a focus change event for processing before the start of the next frame.
		/// @param[in] focused Whether the window is focused now.
		/// @note This is mostly used internally.
		virtual void queueFocusChange(bool focused);
		/// @brief Queues an activity change event for processing before the start of the next frame.
		/// @param[in] active Whether the app is active now.
		/// @note This is mostly used internally.
		virtual void queueActivityChange(bool active);
		/// @brief Queues a size change event for processing before the start of the next frame.
		/// @param[in] width New width of the resolutin.
		/// @param[in] height New height of the resolutin.
		/// @param[in] fullscreen Whether the display is now fullscreen or windowed.
		/// @note This is mostly used internally.
		virtual void queueSizeChange(int width, int height, bool fullscreen);
		/// @brief Queues a size change event for processing before the start of the next frame.
		/// @param[in] inputMode New input mode.
		/// @note This is mostly used internally.
		virtual void queueInputModeChange(const InputMode& inputMode);
		/// @brief Queues a virtual-keyboard-change event.
		/// @param[in] visible Whether the virtual keyboard is visible.
		/// @param[in] heightRatio The ratio of the screen height that the keyboard takes up.
		virtual void queueVirtualKeyboardChange(bool visible, float heightRatio);
		/// @brief Queues a low memory warning event.
		virtual void queueLowMemoryWarning();
		/// @brief Queues a screenshot event.
		/// @param[in] image The Image objet containing the screenshot bitmap data.
		virtual void queueScreenshot(Image* image);
		/// @brief Queues a mouse event for processing before the start of the next frame.
		/// @param[in] type The event type.
		/// @param[in] position The pointer position.
		/// @param[in] keyCode The key code.
		/// @note This is mostly used internally, but it can also be used to simulate input.
		virtual void queueMouseInput(MouseEvent::Type type, cgvec2f position, Key keyCode);
		/// @brief Queues a keyboard event for processing before the start of the next frame.
		/// @param[in] type The event type.
		/// @param[in] keyCode The key code.
		/// @param[in] charCode The character Unicode value.
		/// @note This is mostly used internally, but it can also be used to simulate input.
		virtual void queueKeyInput(KeyEvent::Type type, Key keyCode, unsigned int charCode);
		/// @brief Queues a touch event for processing before the start of the next frame.
		/// @param[in] type The event type.
		/// @param[in] index The pointer index.
		/// @param[in] position The pointer position.
		/// @note This is mostly used internally, but it can also be used to simulate input.
		virtual void queueTouchInput(TouchEvent::Type type, int index, cgvec2f position);
		/// @brief Queues a controller event for processing before the start of the next frame.
		/// @param[in] type The event type.
		/// @param[in] controllerIndex Index of the controller.
		/// @param[in] buttonCode The button code.
		/// @param[in] axisValue The axis value.
		/// @note This is mostly used internally, but it can also be used to simulate input.
		virtual void queueControllerInput(ControllerEvent::Type type, int controllerIndex, Button buttonCode, float axisValue);
		/// @brief Queues a motion event for processing before the start of the next frame.
		/// @param[in] type The event type.
		/// @param[in] motionVector Motion data vector.
		/// @note This is mostly used internally, but it can also be used to simulate input.
		virtual void queueMotionInput(MotionEvent::Type type, cgvec3f motionVector);

		/// @brief Performs the update of one frame.
		/// @param[in] timeDelta Time that has passed since the last frame.
		/// @note This is usually called internally in some implementations, but it's possible to call it manually if a custom april::__mainStandard implementation is used.
		virtual bool performUpdate(float timeDelta);
		
#ifndef DOXYGEN_SHOULD_SKIP_THIS
		// TODOaa - refactor or maybe even remove this
		// the following functions should be temporary, it was added because I needed access to
		// iOS early initialization process. When april will be refactored this needs to be changed --kspes
		static inline void setLaunchCallback(void (*callback)(void*)) { msLaunchCallback = callback; }
		static void handleLaunchCallback(void* args);
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
		/// @brief Whether execution is currently paused.
		bool paused;
		/// @brief Whether presentFrame() will do proper processing.
		/// @note Usually used by Application since some OS implementations do this automatically.
		bool presentFrameEnabled;
		/// @brief Previous width.
		/// @note Used when restoring the window size after switching from fullscreen to windowed.
		int lastWidth;
		/// @brief Previous height.
		/// @note Used when restoring the window size after switching from fullscreen to windowed.
		int lastHeight;
		/// @brief Current cursor position.
		gvec2f cursorPosition;
		/// @brief Current touch positions.
		hmap<int, gvec2f> touchPositions;
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
		/// @brief The mutex for event queueing.
		hmutex eventMutex;
		/// @brief Whether multi-touch mode is currently active.
		bool multiTouchActive;
		/// @brief The current active indexed touches.
		hmap<int, gvec2f> indexedTouches;
		/// @brief The current active touch pointers.
		harray<gvec2f> touches;
		/// @brief Queued mouse events.
		harray<MouseEvent> mouseEvents;
		/// @brief Queued keyboard events.
		harray<KeyEvent> keyEvents;
		/// @brief Queued touch events.
		harray<TouchEvent> touchEvents;
		/// @brief Queued touch events.
		harray<TouchesEvent> touchesEvents;
		/// @brief Queued controller events.
		harray<ControllerEvent> controllerEvents;
		/// @brief Queued motion events.
		harray<MotionEvent> motionEvents;
		/// @brief Queued generic events.
		harray<GenericEvent> genericEvents;
		/// @brief The controller emulation keys.
		/// @note This is useful when testing controller input functionality without actually using a controller.
		hmap<Key, Button> controllerEmulationKeys;
		/// @brief The controller emulation keys for positive axis values.
		/// @note This is useful when testing controller input functionality without actually using a controller.
		hmap<Key, Button> controllerEmulationAxisesPositive;
		/// @brief The controller emulation keys for negative axis values.
		/// @note This is useful when testing controller input functionality without actually using a controller.
		hmap<Key, Button> controllerEmulationAxisesNegative;

		/// @brief A custom virtual keyboard.
		VirtualKeyboard* virtualKeyboard;
		/// @brief The current update delegate.
		UpdateDelegate* updateDelegate;
		/// @brief The current mouse delegate.
		MouseDelegate* mouseDelegate;
		/// @brief The current key delegate.
		KeyDelegate* keyDelegate;
		/// @brief The current touch delegate.
		TouchDelegate* touchDelegate;
		/// @brief The current controller delegate.
		ControllerDelegate* controllerDelegate;
		/// @brief The current motion delegate.
		MotionDelegate* motionDelegate;
		/// @brief The current system delegate.
		SystemDelegate* systemDelegate;

		/// @brief Creates the Window.
		/// @param[in] width Width of the window's rendering area.
		/// @param[in] height Height of the window's rendering area.
		/// @param[in] fullscreen Whether the window should be created in fullscreen or not.
		/// @param[in] title The title to be displayed on the window title bar.
		/// @param[in] options The Options object.
		virtual void _systemCreate(int width, int height, bool fullscreen, chstr title, Window::Options options);
		/// @brief Destroys the Window.
		virtual void _systemDestroy();
		/// @brief Unassigns the Window from a RenderSystem.
		/// @note This is usually used only internally and is needed for some internal call ordering purposes.
		virtual void _systemUnassign();

		/// @brief Processes queued events.
		void _processEvents();

		/// @brief Internally safe method for creating a Cursor object.
		/// @param[in] fromResource Whether the Cursor should be created from a resource file or a normal file.
		/// @param[in] filename The filename of the cursor.
		/// @return The created Cursor object or NULL if failed.
		Cursor* _createCursorFromSource(bool fromResource, chstr filename);

		/// @brief Sets a new fullscreen and resolution state for the Window.
		/// @param[in] width Width of the window's rendering area.
		/// @param[in] height Height of the window's rendering area.
		/// @param[in] fullscreen Whether the window should be created in fullscreen or not.
		virtual void _systemSetResolution(int width, int height, bool fullscreen);
		/// @brief Calls _setRenderSystemResolution() with the current Window parameters.
		/// @see _setRenderSystemResolution(int w, int h, bool fullscreen)
		void _setRenderSystemResolution();
		/// @brief Calls the RenderSystem method for changing the resolution to synchronize Window and RenderSystem.
		/// @param[in] width New width of the resolutin.
		/// @param[in] height New height of the resolutin.
		/// @param[in] fullscreen Whether the display is now fullscreen or windowed.
		virtual void _setRenderSystemResolution(int width, int height, bool fullscreen);
		/// @brief Toggles fullscreen/window mode.
		/// @note Remembers the last windowed size and returns to it. Useful when using directly with a fullscreen hotkey.
		/// @note This is used internally only.
		/// @see setFullscreen
		virtual void _systemToggleHotkeyFullscreen();
		/// @brief Toggles fullscreen/window mode.
		/// @param[out] width The new width of the window.
		/// @param[out] height The new height of the window.
		/// @note Remembers the last windowed size and returns to it. Useful when using directly with a fullscreen hotkey.
		/// @see setFullscreen
		void _getToggleHotkeyFullscreenSize(int& width, int& height);
		/// @brief Gets the window size that a fullscreen toggle should switch to.
		/// @return The window size that a fullscreen toggle should switch to.
		/// @note This is used internally only.
		/// @see setFullscreen
		gvec2i _getToggleHotkeyFullscreenSize();

		/// @brief Creates the actual system Cursor.
		/// @param[in] fromResource Whether the Cursor is created from a resource file or a normal file.
		/// @return The created Cursor object or NULL if not supported on this Window implementation.
		virtual Cursor* _createCursor(bool fromResource);
		/// @brief Sets the internal system cursor.
		virtual void _refreshCursor();

		/// @brief Flushes the currently rendered data to the backbuffer for display.
		/// @param[in] systemEnabled Whether the system call is enabled.
		virtual void _presentFrame(bool systemEnabled);

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
