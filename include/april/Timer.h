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
/// Defines a generic timer.

#ifndef APRIL_TIMER_H
#define APRIL_TIMER_H

#define __HL_INCLUDE_PLATFORM_HEADERS
#include <hltypes/hplatform.h>
#ifndef _WIN32
#include <stdint.h>
#ifndef __int64
#define __int64 uint64_t
#endif
#endif
#include <stdio.h>

#include "aprilExport.h"

namespace april
{
	/// @brief Defines a generic timer.
	class aprilExport Timer
	{
	public:
		/// @brief Basic constructor.
		Timer();
		/// @brief Destructor.
		~Timer();
		
		/// @brief Gets current time.
		float getTime();

		/// @brief Calculates the difference since the last measurement.
		/// @param[in] update Whether to update the measurement before retrieving.
		/// @return Time since the last measurement.
		float diff(bool update = true);
		/// @brief Updates the timer manually.
		void update();
		
	protected:
		/// @brief Current difference.
		float dt;
		/// @brief Previous time.
		float td;
		/// @brief Current time.
		float td2;
		/// @brief Frequency of the timer.
		__int64 frequency;
		/// @brief Resolution of the timer.
		float resolution;
		/// @brief When the timer started.
		__int64 start;
		/// @brief How much time elapsed.
		unsigned long elapsed;
		/// @brief Whether the internal performance timer is used.
		bool performanceTimer;
		/// @brief Start time of performance timer.
		__int64 performanceTimerStart;
		/// @brief Elapsed time of performance timer.
		__int64 performanceTimerElapsed;
		
	};

}

#endif
