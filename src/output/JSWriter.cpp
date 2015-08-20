/**
 * EasyTest++
 *
 * Copyright (c) 2015, Loïc Le Page
 *
 * This project is released under the MIT license.
 * See: http://opensource.org/licenses/MIT
 */

#include "JSWriter.h"

#include "../../include/TestSuite.h"
#include "../stats/CodeTimer.h"
#include "FormattedBuffer.h"

namespace
{
	easyTest::output::FormattedBuffer& formatCodeTimer(easyTest::output::FormattedBuffer& buffer, const easyTest::stats::CodeTimer* pTimer, bool bVerbose)
	{
		if (pTimer)
		{
			if (bVerbose)
				buffer.appendFormat("\n\t{\n\t\treal: %lld,\n\t\tproc: %lld,\n\t\tthread: %lld\n\t}\n", pTimer->getElapsedRealTime(), pTimer->getElapsedProcessTime(), pTimer->getElapsedThreadTime());
			else
				buffer.appendFormat("{\"real\":%lld,\"proc\":%lld,\"thread\":%lld}", pTimer->getElapsedRealTime(), pTimer->getElapsedProcessTime(), pTimer->getElapsedThreadTime());
		}
		else
		{
			if (bVerbose)
				buffer.append(" null\n");
			else
				buffer.append("null");
		}

		return buffer;
	}
}

namespace easyTest
{
	namespace output
	{
		void JSWriter::onEvent(const TestEvent& event)
		{
			switch (event.type)
			{
			case TestEventType::TESTRUNNER_START:
				TestWriter::onEvent(event);
				{
					FormattedBuffer* pBuffer = getControlThreadBuffer();
					if (pBuffer)
					{
						if (m_bVerbose)
						{
							pBuffer->append("var g_testResult = [\n{\n\ttype: \"runner_start\",\n")
									.appendFormat("\tnbMaxWorkerThreads: %u,\n\tnbTotalSuites: %zu,\n\tnbTotalTestCases: %zu\n},\n", event.testRunnerStart.nbMaxWorkerThreads, event.testRunnerStart.nbTotalSuites, event.testRunnerStart.nbTotalTestCases);
						}
						else
						{
							pBuffer->append("{\"type\":\"runner_start\",")
									.appendFormat("\"nbMaxWorkerThreads\":%u,\"nbTotalSuites\":%zu,\"nbTotalTestCases\":%zu}\n", event.testRunnerStart.nbMaxWorkerThreads, event.testRunnerStart.nbTotalSuites, event.testRunnerStart.nbTotalTestCases);
						}

						pBuffer->flushAndClear(m_pOut);
					}
				}
				break;

			case TestEventType::TESTRUNNER_FINISH:
				{
					FormattedBuffer* pBuffer = getControlThreadBuffer();
					if (pBuffer)
					{
						if (m_bVerbose)
						{
							pBuffer->append("{\n\ttype: \"runner_finish\",\n")
									.appendFormat("\tnbUsedWorkerThreads: %u,\n\tnbTotalSuites: %zu,\n\tnbSuccessSuites: %zu,\n\tnbFailedSuites: %zu,\n\ttimer:", event.testRunnerFinish.nbUsedWorkerThreads, event.testRunnerFinish.nbTotalSuites, event.testRunnerFinish.nbSuccessSuites, event.testRunnerFinish.nbFailedSuites);
							formatCodeTimer(*pBuffer, m_bStats ? event.testRunnerFinish.pTimer : nullptr, true)
									.append("}];\n");
						}
						else
						{
							pBuffer->append("{\"type\":\"runner_finish\",")
									.appendFormat("\"nbUsedWorkerThreads\":%u,\"nbTotalSuites\":%zu,\"nbSuccessSuites\":%zu,\"nbFailedSuites\":%zu,\"timer\":", event.testRunnerFinish.nbUsedWorkerThreads, event.testRunnerFinish.nbTotalSuites, event.testRunnerFinish.nbSuccessSuites, event.testRunnerFinish.nbFailedSuites);
							formatCodeTimer(*pBuffer, m_bStats ? event.testRunnerFinish.pTimer : nullptr, false)
									.append("}\n");
						}

						pBuffer->flushAndClear(m_pOut);
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
						{
							pBuffer->append("{\n\ttype: \"suite_error\",\n")
									.appendFormat("\tworkerThreadIdx: %u,\n\ttestSuiteName: \"%s\",\n\tnbTestCases: %zu\n},\n", event.testSuiteError.workerThreadIdx, event.testSuiteError.testSuiteName, event.testSuiteError.nbTestCases);
						}
						else
						{
							pBuffer->append("{\"type\":\"suite_error\",")
									.appendFormat("\"workerThreadIdx\":%u,\"testSuiteName\":\"%s\",\"nbTestCases\":%zu}\n", event.testSuiteError.workerThreadIdx, event.testSuiteError.testSuiteName, event.testSuiteError.nbTestCases);
						}

						pBuffer->flushAndClear(m_pOut);
					}
				}
				break;

			case TestEventType::TESTSUITE_START:
				{
					FormattedBuffer* pBuffer = getWorkerThreadBuffer(event.testSuiteStart.workerThreadIdx);
					if (pBuffer)
					{
						if (m_bVerbose)
						{
							pBuffer->append("{\n\ttype: \"suite_start\",\n")
									.appendFormat("\tworkerThreadIdx: %u,\n\ttestSuiteName: \"%s\",\n\tnbTotalCases: %zu\n},\n", event.testSuiteStart.workerThreadIdx, event.testSuiteStart.testSuiteName, event.testSuiteStart.nbTotalCases);
						}
						else
						{
							pBuffer->append("{\"type\":\"suite_start\",")
									.appendFormat("\"workerThreadIdx\":%u,\"testSuiteName\":\"%s\",\"nbTotalCases\":%zu}\n", event.testSuiteStart.workerThreadIdx, event.testSuiteStart.testSuiteName, event.testSuiteStart.nbTotalCases)
									.flushAndClear(m_pOut);
						}
					}
				}
				break;

			case TestEventType::TESTSUITE_FINISH:
				{
					FormattedBuffer* pBuffer = getWorkerThreadBuffer(event.testSuiteFinish.workerThreadIdx);
					if (pBuffer)
					{
						if (m_bVerbose)
						{
							pBuffer->append("{\n\ttype: \"suite_finish\",\n")
									.appendFormat("\tworkerThreadIdx: %u,\n\ttestSuiteName: \"%s\",\n\tnbTotalCases: %zu,\n\tnbSuccessCases: %zu,\n\tnbFailedCases: %zu,\n\ttimer:", event.testSuiteFinish.workerThreadIdx, event.testSuiteFinish.testSuiteName, event.testSuiteFinish.nbTotalCases, event.testSuiteFinish.nbSuccessCases, event.testSuiteFinish.nbFailedCases);
							formatCodeTimer(*pBuffer, m_bStats ? event.testSuiteFinish.pTimer : nullptr, true)
									.append("},\n");
						}
						else
						{
							pBuffer->append("{\"type\":\"suite_finish\",")
									.appendFormat("\"workerThreadIdx\":%u,\"testSuiteName\":\"%s\",\"nbTotalCases\":%zu,\"nbSuccessCases\":%zu,\"nbFailedCases\":%zu,\"timer\":", event.testSuiteFinish.workerThreadIdx, event.testSuiteFinish.testSuiteName, event.testSuiteFinish.nbTotalCases, event.testSuiteFinish.nbSuccessCases, event.testSuiteFinish.nbFailedCases);
							formatCodeTimer(*pBuffer, m_bStats ? event.testSuiteFinish.pTimer : nullptr, false)
									.append("}\n");
						}

						pBuffer->flushAndClear(m_pOut);
					}
				}
				break;

			case TestEventType::TESTCASE_START:
				{
					FormattedBuffer* pBuffer = getWorkerThreadBuffer(event.testCaseStart.workerThreadIdx);
					if (pBuffer)
					{
						if (m_bVerbose)
						{
							pBuffer->append("{\n\ttype: \"case_start\",\n")
									.appendFormat("\tworkerThreadIdx: %u,\n\ttestSuiteName: \"%s\",\n\ttestCaseName: \"%s\"\n},\n", event.testCaseStart.workerThreadIdx, event.testCaseStart.testSuiteName, event.testCaseStart.testCaseName);
						}
						else
						{
							pBuffer->append("{\"type\":\"case_start\",")
									.appendFormat("\"workerThreadIdx\":%u,\"testSuiteName\":\"%s\",\"testCaseName\":\"%s\"}\n", event.testCaseStart.workerThreadIdx, event.testCaseStart.testSuiteName, event.testCaseStart.testCaseName)
									.flushAndClear(m_pOut);
						}
					}
				}
				break;

			case TestEventType::TESTCASE_FINISH:
				{
					FormattedBuffer* pBuffer = getWorkerThreadBuffer(event.testCaseFinish.workerThreadIdx);
					if (pBuffer)
					{
						if (m_bVerbose)
						{
							pBuffer->append("{\n\ttype: \"case_finish\",\n")
									.appendFormat("\tworkerThreadIdx: %u,\n\ttestSuiteName: \"%s\",\n\ttestCaseName: \"%s\",\n\tbSuccess: %s,\n\ttimer:", event.testCaseFinish.workerThreadIdx, event.testCaseFinish.testSuiteName, event.testCaseFinish.testCaseName, event.testCaseFinish.bSuccess ? "true" : "false");
							formatCodeTimer(*pBuffer, m_bStats ? event.testCaseFinish.pTimer : nullptr, true)
									.append("},\n");
						}
						else
						{
							pBuffer->append("{\"type\":\"case_finish\",")
									.appendFormat("\"workerThreadIdx\":%u,\"testSuiteName\":\"%s\",\"testCaseName\":\"%s\",\"bSuccess\":%s,\"timer\":", event.testCaseFinish.workerThreadIdx, event.testCaseFinish.testSuiteName, event.testCaseFinish.testCaseName, event.testCaseFinish.bSuccess ? "true" : "false");
							formatCodeTimer(*pBuffer, m_bStats ? event.testCaseFinish.pTimer : nullptr, false)
									.append("}\n")
									.flushAndClear(m_pOut);
						}
					}
				}
				break;
			}
		}

		void JSWriter::onTrace(unsigned int workerThreadIdx, const SrcInfo& info, const char* format, std::va_list& varArgs)
		{
			FormattedBuffer* pBuffer = getWorkerThreadBuffer(workerThreadIdx);
			if (pBuffer)
			{
				if (m_bVerbose)
				{
					pBuffer->append("{\n\ttype: \"trace\",\n")
							.appendFormat("\tworkerThreadIdx: %u,\n\tfile: \"", workerThreadIdx)
							.appendJSString(info.getFile())
							.appendFormat("\",\n\tfunction: \"%s\",\n\tline: %u,\n\tdetails: \"", info.getFunction(), info.getLine())
							.appendFormatJSString(format, varArgs)
							.append("\"\n},\n");
				}
				else
				{
					pBuffer->append("{\"type\":\"trace\",")
							.appendFormat("\"workerThreadIdx\":%u,\"file\":\"", workerThreadIdx)
							.appendJSString(info.getFile())
							.appendFormat("\",\"function\":\"%s\",\"line\":%u,\"details\":\"", info.getFunction(), info.getLine())
							.appendFormatJSString(format, varArgs)
							.append("\"}\n")
							.flushAndClear(m_pOut);
				}
			}
		}

		void JSWriter::onUnaryAssertFailure(unsigned int workerThreadIdx, const SrcInfo& info, AssertType type, const char* varName)
		{
			FormattedBuffer* pBuffer = getWorkerThreadBuffer(workerThreadIdx);
			if (pBuffer)
			{
				if (m_bVerbose)
				{
					pBuffer->append("{\n\ttype: \"assert\",\n")
							.appendFormat("\tworkerThreadIdx: %u,\n\tfile: \"", workerThreadIdx)
							.appendJSString(info.getFile())
							.appendFormat("\",\n\tfunction: \"%s\",\n\tline: %u,\n\tdetails: \"%s %s\"\n},\n", info.getFunction(), info.getLine(), varName, getUnaryAssertFailText(type));
				}
				else
				{
					pBuffer->append("{\"type\":\"assert\",")
							.appendFormat("\"workerThreadIdx\":%u,\"file\":\"", workerThreadIdx)
							.appendJSString(info.getFile())
							.appendFormat("\",\"function\":\"%s\",\"line\":%u,\"details\":\"%s %s\"}\n", info.getFunction(), info.getLine(), varName, getUnaryAssertFailText(type))
							.flushAndClear(m_pOut);
				}
			}
		}

		void JSWriter::onBinaryAssertFailure(unsigned int workerThreadIdx, const SrcInfo& info, AssertType type, const char* varAName, const char* varBName)
		{
			FormattedBuffer* pBuffer = getWorkerThreadBuffer(workerThreadIdx);
			if (pBuffer)
			{
				if (m_bVerbose)
				{
					pBuffer->append("{\n\ttype: \"assert\",\n")
							.appendFormat("\tworkerThreadIdx: %u,\n\tfile: \"", workerThreadIdx)
							.appendJSString(info.getFile())
							.appendFormat("\",\n\tfunction: \"%s\",\n\tline: %u,\n\tdetails: \"%s %s %s\"\n},\n", info.getFunction(), info.getLine(), varAName, getBinaryAssertFailText(type), varBName);
				}
				else
				{
					pBuffer->append("{\"type\":\"assert\",")
							.appendFormat("\"workerThreadIdx\":%u,\"file\":\"", workerThreadIdx)
							.appendJSString(info.getFile())
							.appendFormat("\",\"function\":\"%s\",\"line\":%u,\"details\":\"%s %s %s\"}\n", info.getFunction(), info.getLine(), varAName, getBinaryAssertFailText(type), varBName)
							.flushAndClear(m_pOut);
				}
			}
		}

		void JSWriter::onRuntimeError(unsigned int workerThreadIdx, RTErrorType type, const char* exceptionDetails)
		{
			FormattedBuffer* pBuffer = getWorkerThreadBuffer(workerThreadIdx);
			if (pBuffer)
			{
				if (m_bVerbose)
				{
					pBuffer->append("{\n\ttype: \"runtime_error\",\n")
							.appendFormat("\tworkerThreadIdx: %u,\n\tdetails: \"", workerThreadIdx);

					if (type == RTErrorType::CPP_EXCEPTION)
						pBuffer->appendFormatJSString(getRuntimeErrorText(type), exceptionDetails);
					else
						pBuffer->append(getRuntimeErrorText(type));

					pBuffer->append("\"\n},\n");
				}
				else
				{
					pBuffer->append("{\"type\":\"runtime_error\",")
							.appendFormat("\"workerThreadIdx\":%u,\"details\":\"", workerThreadIdx);

					if (type == RTErrorType::CPP_EXCEPTION)
						pBuffer->appendFormatJSString(getRuntimeErrorText(type), exceptionDetails);
					else
						pBuffer->append(getRuntimeErrorText(type));

					pBuffer->append("\"}\n")
							.flushAndClear(m_pOut);
				}
			}
		}
	}
}
