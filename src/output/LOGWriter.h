/**
 * EasyTest++
 *
 * Copyright (c) 2015, Loïc Le Page
 *
 * This project is released under the MIT license.
 * See: http://opensource.org/licenses/MIT
 */

#ifndef _LOGWRITER_H_
#define	_LOGWRITER_H_

#include "TestWriter.h"

namespace easyTest
{
	namespace output
	{
		class LOGWriter final : public TestWriter
		{
		public:
			LOGWriter(std::FILE* pOut, bool bStats, bool bVerbose) : TestWriter(pOut, bStats, bVerbose) {}

			virtual void onEvent(const TestEvent& event) override final;

			virtual void onTrace(unsigned int workerThreadIdx, const SrcInfo& info, const char* format, std::va_list& varArgs) override final;

			virtual void onUnaryAssertFailure(unsigned int workerThreadIdx, const SrcInfo& info, AssertType type, const char* varName) override final;
			virtual void onBinaryAssertFailure(unsigned int workerThreadIdx, const SrcInfo& info, AssertType type, const char* varAName, const char* varBName) override final;

			virtual void onRuntimeError(unsigned int workerThreadIdx, RTErrorType type, const char* exceptionDetails) override final;
		};
	}
}

#endif //_LOGWRITER_H_
