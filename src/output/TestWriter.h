/**
 * EasyTest++
 *
 * Copyright (c) 2015, Loïc Le Page
 *
 * This project is released under the MIT license.
 * See: http://opensource.org/licenses/MIT
 */

#ifndef _TESTWRITER_H_
#define	_TESTWRITER_H_

#include <cstdio>

#include "../ITestListener.h"

namespace easyTest
{
	namespace output
	{
		class FormattedBuffer;

		class TestWriter : public ITestListener
		{
		public:
			virtual ~TestWriter() override;

			//If overriden, TestWriter::onEvent(event) MUST be called by any
			//child class on:
			//- TESTRUNNER_START event (must be called first to init
			//                         FormatBuffer array)
			//- TESTRUNNER_FINISH event (must be called last to cleanup
			//                          FormatBuffer array)
			virtual void onEvent(const TestEvent& event) override;

		protected:
			//Each instance MUST be used by only ONE runner at the same time.
			TestWriter(std::FILE* pOut, bool bStats, bool bVerbose) : m_pOut(pOut), m_bStats(bStats), m_bVerbose(bVerbose) {}

			std::FILE* const m_pOut;
			const bool m_bStats;
			const bool m_bVerbose;

			FormattedBuffer* getControlThreadBuffer() const;
			FormattedBuffer* getWorkerThreadBuffer(unsigned int workerThreadIdx) const;

			const char* getUnaryAssertFailText(AssertType type) const;
			const char* getBinaryAssertFailText(AssertType type) const;
			const char* getRuntimeErrorText(RTErrorType type) const;

		private:
			TestWriter(const TestWriter&) = delete;
			TestWriter& operator=(const TestWriter&) = delete;

			FormattedBuffer* m_bufferList = nullptr;
			size_t m_nbBuffers = 0;
		};
	}
}

#endif //_TESTWRITER_H_
