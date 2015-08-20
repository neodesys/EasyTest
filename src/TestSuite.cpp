/**
 * EasyTest++
 *
 * Copyright (c) 2015, Loïc Le Page
 *
 * This project is released under the MIT license.
 * See: http://opensource.org/licenses/MIT
 */

#include "../include/TestSuite.h"

#include "ITestListener.h"

namespace easyTest
{
	void TestSuite::trace(const SrcInfo& info, const char* format, ...) const
	{
		if (m_pTestListener && format && (format[0] != '\0'))
		{
			std::va_list varArgs;
			va_start(varArgs, format);
			m_pTestListener->onTrace(m_workerThreadIdx, info, format, varArgs);
			va_end(varArgs);
		}
	}

	bool TestSuite::unaryAssert(bool condition, const SrcInfo& info, AssertType type, const char* varName) const
	{
		if (condition)
			return true;

		if (m_pTestListener)
			m_pTestListener->onUnaryAssertFailure(m_workerThreadIdx, info, type, varName);

		return false;
	}

	bool TestSuite::binaryAssert(bool condition, const SrcInfo& info, AssertType type, const char* varAName, const char* varBName) const
	{
		if (condition)
			return true;

		if (m_pTestListener)
			m_pTestListener->onBinaryAssertFailure(m_workerThreadIdx, info, type, varAName, varBName);

		return false;
	}
}
