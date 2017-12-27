/// @file
/// @version 4.5
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
		/// @brief Destructor.
		~Application();

		/// @brief Get launch arguments.
		HL_DEFINE_GETSET(harray<hstr>, args, Args);
		/// @brief Get running flag.
		HL_DEFINE_GET(State, state, State);
		/// @brief The current FPS.
		HL_DEFINE_GETSET(int, fps, Fps);
		/// @brief The FPS resolution.
		HL_DEFINE_GETSET(float, fpsResolution, FpsResolution);
		/// @brief The maximum allowed time-delta between frames.
		/// @note Limiting this makes sense, because on weak hardware configurations it allows that large frameskips don't result in too large time skips.
		HL_DEFINE_GETSET(float, timeDeltaMaxLimit, TimeDeltaMaxLimit);

		/// @brief Calls the initialization procedure defined by the user.
		/// @param[in] args Launch arguments.
		void init();
		/// @brief Calls the destruction procedure defined by the user.
		void destroy();

		/// @brief Starts the main loop.
		/// @note This is usually called internally in some implementations, but it's possible to call it manually if a custom april::__mainStandard implementation is used.
		void enterMainLoop();
		/// @brief Performs the update of one frame.
		/// @note This is usually called internally in some implementations, but it's possible to call it manually if a custom april::__mainStandard implementation is used.
		void update();
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
		/// @brief Processes and render a single frame.
		/// @note This is usually called internally in some implementations, due to specific OS limitations.
		void renderFrameSync();

	protected:
		/// @brief Launch arguments.
		harray<hstr> args;
		/// @brief Initialization callback.
		void (*aprilApplicationInit)();
		/// @brief Destruction callback.
		void (*aprilApplicationDestroy)();
		/// @brief The current application state.
		State state;
		/// @brief Whether automatic presentFrame() implementation is used by the underlying system.
		bool autoPresentFrame;
		/// @brief Whether the application has been suspended. Required by some OSes.
		bool suspended;
		/// @brief The Timer object used for timing purposes.
		Timer timer;
		/// @brief The current time since the last frame.
		float timeDelta;
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
		/// @brief The update thread.
		hthread updateThread;
		/// @brief The update mutex.
		hmutex updateMutex;
		/// @brief The time-delta mutex.
		hmutex timeDeltaMutex;

		/// @brief Performs the update of one frame, but without any timer.
		/// @note This is usually called internally in some implementations, but it's possible to call it manually if a custom april::__mainStandard implementation is used.
		void _updateSystem();
		/// @brief Performs FPS counter update.
		/// @note This is usually called internally.
		void _updateFps();

		/// @brief Updates the application asynchronously.
		/// @param[in] thread The thread on which the asynchronous update is running.
		static void _asyncUpdate(hthread* thread);

	};

	/// @brief The global Application instance.
	aprilExport extern april::Application* application;

}
#endif
