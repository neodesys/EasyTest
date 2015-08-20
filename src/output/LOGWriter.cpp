/**
 * EasyTest++
 *
 * Copyright (c) 2015, Loïc Le Page
 *
 * This project is released under the MIT license.
 * See: http://opensource.org/licenses/MIT
 */

#include "LOGWriter.h"

#include "../../include/TestSuite.h"
#include "../stats/CodeTimer.h"
#include "FormattedBuffer.h"
#include "../i18n/i18n.h"

namespace
{
	easyTest::output::FormattedBuffer& formatCodeTimer(easyTest::output::FormattedBuffer& buffer, const easyTest::stats::CodeTimer* pTimer)
	{
		if (pTimer)
		{
			const char* const* units = easyTest::i18n::getSequence(easyTest::i18n::TIME_UNITS_SEQ, 5);
			const char* const* prefix = easyTest::i18n::getSequence(easyTest::i18n::TIME_PREFIXES_SEQ, 3);

			buffer.append(prefix[0]).formatTime(pTimer->getElapsedRealTime(), units, false)
					.append(prefix[1]).formatTime(pTimer->getElapsedProcessTime(), units, false)
					.append(prefix[2]).formatTime(pTimer->getElapsedThreadTime(), units, false);
		}
		else
			buffer.append(easyTest::i18n::getString(easyTest::i18n::STATS_NOT_AVAILABLE));

		return buffer;
	}

	const char* const s_bigSeparator = "--------------------------------------------------------------------------------\n";
	const char* const s_tinySeparator = "----------------\n";
}

namespace easyTest
{
	namespace output
	{
		void LOGWriter::onEvent(const TestEvent& event)
		{
			switch (event.type)
			{
			case TestEventType::TESTRUNNER_START:
				TestWriter::onEvent(event);

				if (m_bVerbose)
				{
					FormattedBuffer* pBuffer = getControlThreadBuffer();
					if (pBuffer)
					{
						pBuffer->appendFormat(i18n::getString(i18n::LOG_RUNNER_START), event.testRunnerStart.nbTotalSuites, event.testRunnerStart.nbTotalTestCases)
#ifndef EASYTEST_NO_THREADS
								.appendFormat(i18n::getString(i18n::LOG_RUNNER_USING_THREADS), event.testRunnerStart.nbMaxWorkerThreads)
#endif //!EASYTEST_NO_THREADS
								.append("...\n").flushAndClear(m_pOut);
					}
				}
				break;

			case TestEventType::TESTRUNNER_FINISH:
				{
					FormattedBuffer* pBuffer = getControlThreadBuffer();
					if (pBuffer)
					{
						if (m_bVerbose)
							pBuffer->append(s_bigSeparator).appendFormat(i18n::getString(i18n::LOG_RUNNER_FINISH_VERB), event.testRunnerFinish.nbSuccessSuites, event.testRunnerFinish.nbFailedSuites, event.testRunnerFinish.nbTotalSuites);
						else if (event.testRunnerFinish.nbSuccessSuites + event.testRunnerFinish.nbFailedSuites < event.testRunnerFinish.nbTotalSuites)
						{
							if (event.testRunnerFinish.nbFailedSuites)
								pBuffer->appendFormat(i18n::getString(i18n::LOG_RUNNER_FINISH_INTERR), event.testRunnerFinish.nbFailedSuites, event.testRunnerFinish.nbTotalSuites - event.testRunnerFinish.nbSuccessSuites - event.testRunnerFinish.nbFailedSuites);
							else
								pBuffer->appendFormat(i18n::getString(i18n::LOG_RUNNER_FINISH_INTOK), event.testRunnerFinish.nbTotalSuites - event.testRunnerFinish.nbSuccessSuites);
						}
						else if (event.testRunnerFinish.nbFailedSuites)
							pBuffer->appendFormat(i18n::getString(i18n::LOG_RUNNER_FINISH_ERR), event.testRunnerFinish.nbFailedSuites);
						else
							pBuffer->appendFormat(i18n::getString(i18n::LOG_RUNNER_FINISH_OK), event.testRunnerFinish.nbTotalSuites);

						formatCodeTimer(pBuffer->append(i18n::getString(i18n::LOG_TOTAL_EXEC_TIME)), event.testRunnerFinish.pTimer)
#ifndef EASYTEST_NO_THREADS
								.appendFormat(i18n::getString(i18n::LOG_RUNNER_USING_THREADS), event.testRunnerFinish.nbUsedWorkerThreads)
#endif //!EASYTEST_NO_THREADS
								.append('\n').flushAndClear(m_pOut);
					}
				}

				TestWriter::onEvent(event);
				break;

			case TestEventType::TESTSUITE_ERROR:
				{
					FormattedBuffer* pBuffer = getWorkerThreadBuffer(event.testSuiteError.workerThreadIdx);
					if (pBuffer)
					{
						if (m_bVerbose)
							pBuffer->append(s_bigSeparator);

						pBuffer->appendFormat(i18n::getString(i18n::LOG_FAILED_SUITE), event.testSuiteError.testSuiteName, event.testSuiteError.nbTestCases).flushAndClear(m_pOut);
					}
				}
				break;

			case TestEventType::TESTSUITE_START:
				if (m_bVerbose)
				{
					FormattedBuffer* pBuffer = getWorkerThreadBuffer(event.testSuiteStart.workerThreadIdx);
					if (pBuffer)
						pBuffer->append(s_bigSeparator).appendFormat(i18n::getString(i18n::LOG_SUITE_START), event.testSuiteStart.testSuiteName, event.testSuiteStart.nbTotalCases);
				}
				break;

			case TestEventType::TESTSUITE_FINISH:
				{
					FormattedBuffer* pBuffer = getWorkerThreadBuffer(event.testSuiteFinish.workerThreadIdx);
					if (pBuffer)
					{
						if (m_bVerbose)
							pBuffer->append(s_tinySeparator).appendFormat(i18n::getString(i18n::LOG_SUITE_FINISH_VERB), event.testSuiteFinish.testSuiteName, event.testSuiteFinish.nbSuccessCases, event.testSuiteFinish.nbFailedCases, event.testSuiteFinish.nbTotalCases);
						else if (event.testSuiteFinish.nbSuccessCases + event.testSuiteFinish.nbFailedCases < event.testSuiteFinish.nbTotalCases)
						{
							if (event.testSuiteFinish.nbFailedCases)
								pBuffer->appendFormat(i18n::getString(i18n::LOG_SUITE_FINISH_INTERR), event.testSuiteFinish.testSuiteName, event.testSuiteFinish.nbFailedCases, event.testSuiteFinish.nbTotalCases - event.testSuiteFinish.nbSuccessCases - event.testSuiteFinish.nbFailedCases);
							else
								pBuffer->appendFormat(i18n::getString(i18n::LOG_SUITE_FINISH_INTOK), event.testSuiteFinish.testSuiteName, event.testSuiteFinish.nbTotalCases - event.testSuiteFinish.nbSuccessCases);
						}
						else if (event.testSuiteFinish.nbFailedCases)
							pBuffer->appendFormat(i18n::getString(i18n::LOG_SUITE_FINISH_ERR), event.testSuiteFinish.testSuiteName, event.testSuiteFinish.nbFailedCases);
						else
							pBuffer->appendFormat(i18n::getString(i18n::LOG_SUITE_FINISH_OK), event.testSuiteFinish.testSuiteName, event.testSuiteFinish.nbTotalCases);

						if (m_bStats)
						{
							if (m_bVerbose)
								pBuffer->append('\n');
							else
								pBuffer->append(' ');

							formatCodeTimer(pBuffer->append(i18n::getString(i18n::LOG_EXEC_TIME)), event.testSuiteFinish.pTimer);
						}

						pBuffer->append('\n').flushAndClear(m_pOut);
					}
				}
				break;

			case TestEventType::TESTCASE_START:
				if (m_bVerbose)
				{
					FormattedBuffer* pBuffer = getWorkerThreadBuffer(event.testCaseStart.workerThreadIdx);
					if (pBuffer)
						pBuffer->append(s_tinySeparator).appendFormat(i18n::getString(i18n::LOG_CASE_START), event.testCaseStart.testCaseName, event.testCaseStart.testSuiteName);
				}
				break;

			case TestEventType::TESTCASE_FINISH:
				{
					FormattedBuffer* pBuffer = getWorkerThreadBuffer(event.testCaseFinish.workerThreadIdx);
					if (pBuffer)
					{
						if (m_bVerbose)
							pBuffer->appendFormat(i18n::getString(i18n::LOG_CASE_FINISH_VERB), event.testCaseFinish.testCaseName, event.testCaseFinish.testSuiteName, i18n::getString(event.testCaseFinish.bSuccess ? i18n::LOG_CASE_SUCCESS : i18n::LOG_CASE_FAILURE));
						else if (!event.testCaseFinish.bSuccess)
							pBuffer->appendFormat(i18n::getString(i18n::LOG_CASE_FINISH_ERR), event.testCaseFinish.testSuiteName, event.testCaseFinish.testCaseName);
						else
							pBuffer->appendFormat(i18n::getString(i18n::LOG_CASE_FINISH_OK), event.testCaseFinish.testSuiteName, event.testCaseFinish.testCaseName);

						if (m_bStats)
						{
							if (m_bVerbose)
								pBuffer->append('\n');
							else
								pBuffer->append(' ');

							formatCodeTimer(pBuffer->append(i18n::getString(i18n::LOG_EXEC_TIME)), event.testCaseFinish.pTimer);
						}

						pBuffer->append('\n');
					}
				}
				break;
			}
		}

		void LOGWriter::onTrace(unsigned int workerThreadIdx, const SrcInfo& info, const char* format, std::va_list& varArgs)
		{
			if (m_bVerbose)
			{
				FormattedBuffer* pBuffer = getWorkerThreadBuffer(workerThreadIdx);
				if (pBuffer)
				{
					pBuffer->appendFormat(i18n::getString(i18n::LOG_TRACE_HEADER), info.getFile(), info.getFunction(), info.getLine())
							.appendFormat(format, varArgs)
							.append('\n');
				}
			}
		}

		void LOGWriter::onUnaryAssertFailure(unsigned int workerThreadIdx, const SrcInfo& info, AssertType type, const char* varName)
		{
			if (m_bVerbose)
			{
				FormattedBuffer* pBuffer = getWorkerThreadBuffer(workerThreadIdx);
				if (pBuffer)
				{
					pBuffer->appendFormat(i18n::getString(i18n::LOG_ASSERT_HEADER), info.getFile(), info.getFunction(), info.getLine())
							.appendFormat("%s %s\n", varName, getUnaryAssertFailText(type));
				}
			}
		}

		void LOGWriter::onBinaryAssertFailure(unsigned int workerThreadIdx, const SrcInfo& info, AssertType type, const char* varAName, const char* varBName)
		{
			if (m_bVerbose)
			{
				FormattedBuffer* pBuffer = getWorkerThreadBuffer(workerThreadIdx);
				if (pBuffer)
				{
					pBuffer->appendFormat(i18n::getString(i18n::LOG_ASSERT_HEADER), info.getFile(), info.getFunction(), info.getLine())
							.appendFormat("%s %s %s\n", varAName, getBinaryAssertFailText(type), varBName);
				}
			}
		}

		void LOGWriter::onRuntimeError(unsigned int workerThreadIdx, RTErrorType type, const char* exceptionDetails)
		{
			if (m_bVerbose)
			{
				FormattedBuffer* pBuffer = getWorkerThreadBuffer(workerThreadIdx);
				if (pBuffer)
				{
					pBuffer->append(i18n::getString(i18n::LOG_RTERROR_HEADER));
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
