/**
 * EasyTest++
 *
 * Copyright (c) 2015, Loïc Le Page
 *
 * This project is released under the MIT license.
 * See: http://opensource.org/licenses/MIT
 */

#include "CodeTimer.h"

#include "Chrono.h"

namespace
{
	const easyTest::stats::Chrono s_chrono[] = {{easyTest::stats::Chrono::Type::REAL_TIME}, {easyTest::stats::Chrono::Type::PROCESS_TIME}, {easyTest::stats::Chrono::Type::THREAD_TIME}};
}

namespace easyTest
{
	namespace stats
	{
		bool CodeTimer::start()
		{
			if (m_bRunning)
				return false;

			for (int i = 0; i < 3; ++i)
				m_timestamp[i] = s_chrono[i].tick();

			m_bRunning = true;
			return true;
		}

		void CodeTimer::stop()
		{
			if (m_bRunning)
			{
				for (int i = 2; i >= 0; --i)
				{
					m_timestamp[i] = s_chrono[i].tick() - m_timestamp[i];
					if (m_timestamp[i] < 0)
						m_timestamp[i] = 0;
				}

				m_bRunning = false;
			}
		}
	}
}
