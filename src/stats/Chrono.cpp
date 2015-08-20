/**
 * EasyTest++
 *
 * Copyright (c) 2015, Loïc Le Page
 *
 * This project is released under the MIT license.
 * See: http://opensource.org/licenses/MIT
 */

#include "Chrono.h"

#ifdef __linux__
#if !defined(CLOCK_MONOTONIC_RAW) || !defined(CLOCK_PROCESS_CPUTIME_ID) || !defined(CLOCK_THREAD_CPUTIME_ID)
#error Some clocks are not defined, you must compile on a linux kernel >= 2.6.28
#endif //!CLOCK_MONOTONIC_RAW || !CLOCK_PROCESS_CPUTIME_ID || !CLOCK_THREAD_CPUTIME_ID
#endif //__linux__

namespace easyTest
{
	namespace stats
	{
		Chrono::Chrono(Type type)
		{
#ifdef __linux__
			switch (type)
			{
			case Type::REAL_TIME:
				m_clockId = CLOCK_MONOTONIC_RAW;
				break;

			case Type::PROCESS_TIME:
				m_clockId = CLOCK_PROCESS_CPUTIME_ID;
				break;

			case Type::THREAD_TIME:
				m_clockId = CLOCK_THREAD_CPUTIME_ID;
				break;
			}

			timespec now = {};
			if (clock_gettime(m_clockId, &now))
			{
				m_clockId = CLOCK_MONOTONIC;
				if (clock_gettime(m_clockId, &now))
				{
					m_clockId = CLOCK_REALTIME;
					if (clock_gettime(m_clockId, &now))
					{
						m_clockId = -1;
						m_timestamp = 0;
						return;
					}
				}
			}

			m_timestamp = now.tv_sec * 1000000000 + now.tv_nsec;

#elif defined(_WIN32)
			m_type = type;

			switch (m_type)
			{
			case Type::REAL_TIME:
				if (QueryPerformanceFrequency(&m_counterFreq))
				{
					LARGE_INTEGER counter;
					if (QueryPerformanceCounter(&counter))
					{
						m_timestamp = counter.QuadPart;
						return;
					}
				}
				break;

			case Type::PROCESS_TIME:
				{
					FILETIME time[4];
					if (GetProcessTimes(GetCurrentProcess(), time, time + 1, time + 2, time + 3))
					{
						m_timestamp = static_cast<long long>(time[2].dwHighDateTime) << 32 | time[2].dwLowDateTime;
						m_timestamp += static_cast<long long>(time[3].dwHighDateTime) << 32 | time[3].dwLowDateTime;
						return;
					}
				}
				break;

			case Type::THREAD_TIME:
				{
					FILETIME time[4];
					if (GetThreadTimes(GetCurrentThread(), time, time + 1, time + 2, time + 3))
					{
						m_timestamp = static_cast<long long>(time[2].dwHighDateTime) << 32 | time[2].dwLowDateTime;
						m_timestamp += static_cast<long long>(time[3].dwHighDateTime) << 32 | time[3].dwLowDateTime;
						return;
					}
				}
				break;
			}

			m_counterFreq.QuadPart = 0;
			m_timestamp = 0;
#endif //__linux__
		}

		long long Chrono::tick() const
		{
#ifdef __linux__
			if (m_clockId >= 0)
			{
				timespec now = {};
				if (!clock_gettime(m_clockId, &now))
					return now.tv_sec * 1000000000 + now.tv_nsec - m_timestamp;
			}

#elif defined(_WIN32)
			switch (m_type)
			{
			case Type::REAL_TIME:
				if (m_counterFreq.QuadPart > 0)
				{
					LARGE_INTEGER counter;
					if (QueryPerformanceCounter(&counter))
						return (counter.QuadPart - m_timestamp) * 1000000000 / m_counterFreq.QuadPart;
				}
				break;

			case Type::PROCESS_TIME:
				{
					FILETIME time[4];
					if (GetProcessTimes(GetCurrentProcess(), time, time + 1, time + 2, time + 3))
					{
						long long now = static_cast<long long>(time[2].dwHighDateTime) << 32 | time[2].dwLowDateTime;
						now += static_cast<long long>(time[3].dwHighDateTime) << 32 | time[3].dwLowDateTime;
						return (now - m_timestamp) * 100;
					}
				}
				break;

			case Type::THREAD_TIME:
				{
					FILETIME time[4];
					if (GetThreadTimes(GetCurrentThread(), time, time + 1, time + 2, time + 3))
					{
						long long now = static_cast<long long>(time[2].dwHighDateTime) << 32 | time[2].dwLowDateTime;
						now += static_cast<long long>(time[3].dwHighDateTime) << 32 | time[3].dwLowDateTime;
						return (now - m_timestamp) * 100;
					}
				}
				break;
			}
#endif //__linux__

			return 0;
		}
	}
}
