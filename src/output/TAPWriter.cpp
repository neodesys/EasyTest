/**
 * EasyTest++
 *
 * Copyright (c) 2015, Loïc Le Page
 *
 * This project is released under the MIT license.
 * See: http://opensource.org/licenses/MIT
 */

#include "TAPWriter.h"

#include "../../include/TestSuite.h"
#include "../stats/CodeTimer.h"
#include "FormattedBuffer.h"
#include "../i18n/i18n.h"

namespace
{
	easyTest::output::FormattedBuffer& formatCodeTimer(easyTest::output::FormattedBuffer& buffer, const easyTest::stats::CodeTimer* pTimer, bool bInsert)
	{
		if (pTimer)
		{
			const char* const* units = easyTest::i18n::getSequence(easyTest::i18n::TIME_UNITS_SEQ, 5);
			const char* const* prefix = easyTest::i18n::getSequence(easyTest::i18n::TIME_PREFIXES_SEQ, 3);

			if (bInsert)
			{
				buffer.insert(prefix[0]).formatTime(pTimer->getElapsedRealTime(), units, true)
						.insert(prefix[1]).formatTime(pTimer->getElapsedProcessTime(), units, true)
						.insert(prefix[2]).formatTime(pTimer->getElapsedThreadTime(), units, true);
			}
			else
			{
				buffer.append(prefix[0]).formatTime(pTimer->getElapsedRealTime(), units, false)
						.append(prefix[1]).formatTime(pTimer->getElapsedProcessTime(), units, false)
						.append(prefix[2]).formatTime(pTimer->getElapsedThreadTime(), units, false);
			}
		}
		else if (bInsert)
			buffer.insert(easyTest::i18n::getString(easyTest::i18n::STATS_NOT_AVAILABLE));
		else
			buffer.append(easyTest::i18n::getString(easyTest::i18n::STATS_NOT_AVAILABLE));

		return buffer;
	}
}

namespace easyTest
{
	namespace output
	{
		void TAPWriter::onEvent(const TestEvent& event)
		{
			switch (event.type)
			{
			case TestEventType::TESTRUNNER_START:
				TestWriter::onEvent(event);
				{
					FormattedBuffer* pBuffer = getControlThreadBuffer();
					if (pBuffer)
					{
						pBuffer->appendFormat("1..%zu\n", event.testRunnerStart.nbTotalTestCases);
						if (m_bVerbose)
						{
							pBuffer->appendFormat(i18n::getString(i18n::TAP_RUNNER_START), event.testRunnerStart.nbTotalSuites);
#ifndef EASYTEST_NO_THREADS
							pBuffer->appendFormat(i18n::getString(i18n::TAP_RUNNER_USING_THREADS), event.testRunnerStart.nbMaxWorkerThreads);
#endif //!EASYTEST_NO_THREADS
						}

						pBuffer->flushAndClear(m_pOut);
					}
				}
				break;

			case TestEventType::TESTRUNNER_FINISH:
				if (m_bStats)
				{
					FormattedBuffer* pBuffer = getControlThreadBuffer();
					if (pBuffer)
					{
						formatCodeTimer(pBuffer->append(i18n::getString(i18n::TAP_TOTAL_EXEC_TIME)), event.testRunnerFinish.pTimer, false)
								.append('\n')
								.flushAndClear(m_pOut);
					}
				}

				TestWriter::onEvent(event);
				break;

			case TestEventType::TESTSUITE_ERROR:
				{
					FormattedBuffer* pBuffer = getWorkerThreadBuffer(event.testSuiteError.workerThreadIdx);
					if (pBuffer)
					{
						for (size_t i = 0; i < event.testSuiteError.nbTestCases; ++i)
						{
							pBuffer->insert("not ok - ")
									.insertFormat(i18n::getString(i18n::TAP_FAILED_SUITE), event.testSuiteError.testSuiteName);
						}

						pBuffer->flushAndClear(m_pOut);
					}
				}
				break;

			case TestEventType::TESTSUITE_START:
				//Nothing to do
				break;

			case TestEventType::TESTSUITE_FINISH:
				if (m_bVerbose)
				{
					//In verbose mode, we have to flush the worker-thread
					//buffer in order to print out possible runtime errors
					//during test suite destruction.
					FormattedBuffer* pBuffer = getWorkerThreadBuffer(event.testSuiteFinish.workerThreadIdx);
					if (pBuffer)
						pBuffer->flushAndClear(m_pOut);
				}
				break;

			case TestEventType::TESTCASE_START:
				{
					FormattedBuffer* pBuffer = getWorkerThreadBuffer(event.testCaseStart.workerThreadIdx);
					if (pBuffer)
						pBuffer->setInsertMark();
				}
				break;

			case TestEventType::TESTCASE_FINISH:
				{
					FormattedBuffer* pBuffer = getWorkerThreadBuffer(event.testCaseFinish.workerThreadIdx);
					if (pBuffer)
					{
						if (m_bStats)
						{
							pBuffer->insertFormat("%sok - [%s::%s] - ", event.testCaseFinish.bSuccess ? "" : "not ", event.testCaseFinish.testSuiteName, event.testCaseFinish.testCaseName);
							formatCodeTimer(*pBuffer, event.testCaseFinish.pTimer, true).insert('\n');
						}
						else
							pBuffer->insertFormat("%sok - [%s::%s]\n", event.testCaseFinish.bSuccess ? "" : "not ", event.testCaseFinish.testSuiteName, event.testCaseFinish.testCaseName);

						pBuffer->flushAndClear(m_pOut);
					}
				}
				break;
			}
		}

		void TAPWriter::onTrace(unsigned int workerThreadIdx, const SrcInfo& info, const char* format, std::va_list& varArgs)
		{
			if (m_bVerbose)
			{
				FormattedBuffer* pBuffer = getWorkerThreadBuffer(workerThreadIdx);
				if (pBuffer)
				{
					pBuffer->appendFormat(i18n::getString(i18n::TAP_TRACE_HEADER), info.getFile(), info.getFunction(), info.getLine())
							.appendFormat(format, varArgs)
							.append('\n');
				}
			}
		}

		void TAPWriter::onUnaryAssertFailure(unsigned int workerThreadIdx, const SrcInfo& info, AssertType type, const char* varName)
		{
			if (m_bVerbose)
			{
				FormattedBuffer* pBuffer = getWorkerThreadBuffer(workerThreadIdx);
				if (pBuffer)
				{
					pBuffer->appendFormat(i18n::getString(i18n::TAP_ASSERT_HEADER), info.getFile(), info.getFunction(), info.getLine())
							.appendFormat("%s %s\n", varName, getUnaryAssertFailText(type));
				}
			}
		}

		void TAPWriter::onBinaryAssertFailure(unsigned int workerThreadIdx, const SrcInfo& info, AssertType type, const char* varAName, const char* varBName)
		{
			if (m_bVerbose)
			{
				FormattedBuffer* pBuffer = getWorkerThreadBuffer(workerThreadIdx);
				if (pBuffer)
				{
					pBuffer->appendFormat(i18n::getString(i18n::TAP_ASSERT_HEADER), info.getFile(), info.getFunction(), info.getLine())
							.appendFormat("%s %s %s\n", varAName, getBinaryAssertFailText(type), varBName);
				}
			}
		}

		void TAPWriter::onRuntimeError(unsigned int workerThreadIdx, RTErrorType type, const char* exceptionDetails)
		{
			if (m_bVerbose)
			{
				FormattedBuffer* pBuffer = getWorkerThreadBuffer(workerThreadIdx);
				if (pBuffer)
				{
					pBuffer->append(i18n::getString(i18n::TAP_RTERROR_HEADER));
					if (type == RTErrorType::CPP_EXCEPTION)
						pBuffer->appendFormat(getRuntimeErrorText(type), exceptionDetails);
					else
						pBuffer->append(getRuntimeErrorText(type));
					pBuffer->append('\n');
				}
			}
		}
	}
}
