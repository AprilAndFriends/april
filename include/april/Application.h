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
/// Defines a generic application.

#ifndef APRIL_APPLICATION_H
#define APRIL_APPLICATION_H

#include <hltypes/harray.h>
#include <hltypes/henum.h>
#include <hltypes/hmutex.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hstring.h>
#include <hltypes/hthread.h>

#include "aprilExport.h"
#include "Platform.h"
#include "Timer.h"

namespace april
{
	class Window;

	/// @brief Defines a generic application.
	class aprilExport Application
	{
	public:
		friend class Window;

		/// @class State
		/// @brief Defines application states.
		HL_ENUM_CLASS_PREFIX_DECLARE(aprilExport, State,
		(
			/// @var static const Type Type::Idle
			/// @brief Initial state.
			HL_ENUM_DECLARE(State, Idle);
			/// @var static const Type Type::Starting
			/// @brief Launching process is in progress.
			HL_ENUM_DECLARE(State, Starting);
			/// @var static const Type Type::StartedWaiting
			/// @brief Launching process finished, waiting to run.
			HL_ENUM_DECLARE(State, StartedWaiting);
			/// @var static const Type Type::Running
			/// @brief While the applications is running.
			HL_ENUM_DECLARE(State, Running);
			/// @var static const Type Type::Stopping
			/// @brief Stopping process in progress.
			HL_ENUM_DECLARE(State, Stopping);
			/// @var static const Type Type::Stopped
			/// @brief Stopping process has finished.
			HL_ENUM_DECLARE(State, Stopped);
		));

		/// @brief Basic constructor.
		/// @param[in] aprilApplicationInit Initialization callback.
		/// @param[in] aprilApplicationDestroy Destruction callback.
		Application(void (*aprilApplicationInit)(), void (*aprilApplicationDestroy)());

		/// @brief Get/Set launch arguments.
		HL_DEFINE_GETSET(harray<hstr>, args, Args);
		/// @brief Get running flag.
		State getState();
		/// @brief Whether the application is suspended right now.
		HL_DEFINE_IS(suspended, Suspended);
		/// @brief The current FPS.
		HL_DEFINE_GETSET(int, fps, Fps);
		/// @brief The FPS resolution.
		HL_DEFINE_GETSET(double, fpsResolution, FpsResolution);
		/// @brief The maximum allowed time-delta between frames.
		/// @note Limiting this makes sense, because on weak hardware configurations it allows that large frameskips don't result in too large time skips.
		HL_DEFINE_GETSET(double, timeDeltaMaxLimit, TimeDeltaMaxLimit);
		/// @brief Checks whether any OS message boxes are queued for display.
		bool isAnyMessageBoxQueued();

		/// @brief Calls the initialization procedure defined by the user.
		void init();
		/// @brief Calls the destruction procedure defined by the user.
		void destroy();

		/// @brief Starts the main loop.
		/// @note This is usually called internally in some implementations, but it's possible to call it manually if a custom april::__mainStandard implementation is used.
		void enterMainLoop();
		/// @brief Performs the update when initializing.
		/// @param[in] singleUpdateOnly Required for some platform due to how applications are handled internally.
		/// @note This is usually called internally in some implementations, but it's possible to call it manually if a custom april::__mainStandard implementation is used.
		void updateInitializing(bool singleUpdateOnly = false);
		/// @brief Performs the update of one frame.
		/// @return True if a system command was executed.
		/// @note This is usually called internally in some implementations, but it's possible to call it manually if a custom april::__mainStandard implementation is used.
		bool update();
		/// @brief Performs the update when finishing.
		/// @note This is usually called internally in some implementations, but it's possible to call it manually if a custom april::__mainStandard implementation is used.
		void updateFinishing();
		/// @brief Finishes the main loop.
		void finish();
		/// @brief Finalizes the application process so all threads can finish up.
		void finalize();
		/// @brief Suspends application.
		/// @note This is usually called internally in some implementations.
		void suspend();
		/// @brief Resumes application.
		/// @note This is usually called internally in some implementations.
		void resume();
		/// @brief Processes and render a single frame without stopping the update thread.
		/// @param[in] systemEnabled Whether the system present is actually enabled.
		/// @note This is usually called internally in some implementations, due to specific OS limitations.
		void repeatLastFrame(bool systemEnabled);
		/// @brief Processes and render a single frame and stops the update thread.
		/// @note This is usually called internally in some implementations, due to specific OS limitations.
		void renderFrameSync();
		/// @brief Queues a message box display.
		/// @param[in] data The data for the message box display.
		/// @note This is usually called internally only.
		void queueMessageBox(const MessageBoxData& data);
		/// @brief Waits until all OS message boxes have finished displaying.
		void waitForMessageBoxes();
		/// @brief Forces a time-delta of 0 for the next frame update.
		/// @note Useful to prevent stutters if the current frame is doing lots of work.
		void forceNoTimeDeltaNextFrame();

		static void messageBoxCallback(const MessageBoxButton& button);

	protected:
		/// @brief Launch arguments.
		harray<hstr> args;
		/// @brief Initialization callback.
		void (*aprilApplicationInit)();
		/// @brief Destruction callback.
		void (*aprilApplicationDestroy)();
		/// @brief The current application state.
		State state;
		/// @brief Whether the application has been suspended. Required by some OSes.
		bool suspended;
		/// @brief Whether the update-thread is waiting to be suspended. Required by some OSes.
		bool updateSuspendQueued;
		/// @brief The Timer object used for general timing purposes.
		Timer timer;
		/// @brief The current time since the last frame.
		double timeDelta;
		/// @brief The Timer object used for timing frame rendering to adjust the update thread sleep time.
		Timer frameTimer;
		/// @brief FPS of the last mesaure.
		int fps;
		/// @brief Current counter for FPS calculation.
		int fpsCount;
		/// @brief Current timer for FPS calculation.
		double fpsTimer;
		/// @brief FPS update resolution.
		double fpsResolution;
		/// @brief Maximum allowed time-delta that are propagated into the UpdateDelegate.
		/// @note Limiting this makes sense, because on weak hardware configurations it allows that large frameskips don't result in too large time skips.
		double timeDeltaMaxLimit;
		/// @brief Whether to force a time-delta of 0 for the next frame update.
		bool noTimeDeltaNextFrame;
		/// @brief Queue for message box displays.
		harray<MessageBoxData> messageBoxQueue;
		/// @brief Whether OS message box is being displayed at this moment or not.
		bool displayingMessageBox;
		/// @brief The update thread.
		hthread updateThread;
		/// @brief The update mutex.
		hmutex updateMutex;
		/// @brief The update mutex.
		hmutex stateMutex;
		/// @brief The time-delta mutex.
		hmutex timeDeltaMutex;
		/// @brief The frame timer mutex.
		hmutex frameTimerMutex;
		/// @brief The message-box queue mutex.
		hmutex messageBoxMutex;

		/// @brief Set State.
		/// @param[in] value New State.
		void _setState(const State& value);
		/// @brief Performs the update of one frame, but without any timer.
		bool _updateSystem();
		/// @brief Processes message box displays.
		void _updateMessageBoxQueue();
		/// @brief Performs FPS counter update.
		void _updateFps();

		/// @brief Updates the application asynchronously.
		/// @param[in] thread The thread on which the asynchronous update is running.
		static void _asyncUpdate(hthread* thread);

	};

	/// @brief The global Application instance.
	aprilExport extern april::Application* application;

}
#endif
