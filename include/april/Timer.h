/// @file
/// @version 4.3
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

#include <stdint.h>
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
		
		/// @brief Gets current time of timer.
		double getTime() const;

		/// @brief Calculates the difference since the last measurement.
		/// @param[in] update Whether to update the measurement before retrieving.
		/// @return Time since the last measurement.
		float diff(bool update = true);
		/// @brief Updates the timer manually.
		void update();
		
	protected:
		/// @brief Current difference.
		float difference;
		/// @brief Previous time.
		double td1;
		/// @brief Current time.
		double td2;
		/// @brief Frequency of the timer.
		int64_t frequency;
		/// @brief Resolution of the timer.
		double resolution;
		/// @brief When the timer started.
		int64_t start;
		/// @brief Whether the internal performance timer is used.
		bool performanceTimer;
		/// @brief Start time of performance timer.
		int64_t performanceTimerStart;
		/// @brief Elapsed time of performance timer.
		int64_t performanceTimerElapsed;
		
	};

}

#endif
