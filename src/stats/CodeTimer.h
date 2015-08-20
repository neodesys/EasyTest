/**
 * EasyTest++
 *
 * Copyright (c) 2015, Loïc Le Page
 *
 * This project is released under the MIT license.
 * See: http://opensource.org/licenses/MIT
 */

#ifndef _CODETIMER_H_
#define _CODETIMER_H_

namespace easyTest
{
	namespace stats
	{
		class CodeTimer final
		{
		public:
			CodeTimer() = default;

			bool start();
			void stop();

			bool isRunning() const
			{
				return m_bRunning;
			}

			long long getElapsedRealTime() const //in ns
			{
				return m_bRunning ? 0 : m_timestamp[0];
			}

			long long getElapsedProcessTime() const //in ns
			{
				return m_bRunning ? 0 : m_timestamp[1];
			}

			long long getElapsedThreadTime() const //in ns
			{
				return m_bRunning ? 0 : m_timestamp[2];
			}

		private:
			CodeTimer(const CodeTimer&) = delete;
			CodeTimer& operator=(const CodeTimer&) = delete;

			bool m_bRunning = false;
			long long m_timestamp[3] = {};
		};
	}
}

#endif //_CODETIMER_H_
