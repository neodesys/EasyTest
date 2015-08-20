/**
 * EasyTest++
 *
 * Copyright (c) 2015, Loïc Le Page
 *
 * This project is released under the MIT license.
 * See: http://opensource.org/licenses/MIT
 */

#ifndef _CHRONO_H_
#define	_CHRONO_H_

#ifdef __linux__
#include <ctime>
#elif defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#error Unsupported platform: currently supported platforms are\
       Linux and Windows
#endif //__linux__

namespace easyTest
{
	namespace stats
	{
		class Chrono final
		{
		public:
			//Under Windows, process and thread timers have a really bad
			//resolution (superior to tens of ms), so their values are not
			//really meaningful for normal test cases which execute in a few
			//nano or micro-seconds.
			enum struct Type : unsigned char
			{
				REAL_TIME,
				PROCESS_TIME,
				THREAD_TIME
			};

			Chrono(Type type);

			long long tick() const; //elapsed time in ns since Chrono construction.

		private:
			Chrono(const Chrono&) = delete;
			Chrono& operator=(const Chrono&) = delete;

#ifdef __linux__
			clockid_t m_clockId = 0;
#elif defined(_WIN32)
			Type m_type = Type::REAL_TIME;
			LARGE_INTEGER m_counterFreq = {};
#endif //__linux

			long long m_timestamp = 0;
		};
	}
}

#endif //_CHRONO_H_
