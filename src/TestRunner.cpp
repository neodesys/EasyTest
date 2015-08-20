/**
 * EasyTest++
 *
 * Copyright (c) 2015, Loïc Le Page
 *
 * This project is released under the MIT license.
 * See: http://opensource.org/licenses/MIT
 */

#include "TestRunner.h"

#include <cassert>

#ifdef __linux__
#ifndef EASYTEST_NO_THREADS
#include <unistd.h>
#endif //!EASYTEST_NO_THREADS

#elif defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif //__linux__

#include "../include/TestSuiteRegistrar.h"
#include "ITestListener.h"
#include "RTErrorProtector.h"

namespace
{
	unsigned int getHardwareThreadCount()
	{
#ifndef EASYTEST_NO_THREADS
#ifdef __linux__
		long n = sysconf(_SC_NPROCESSORS_ONLN);
		if (n > 1)
			return n;
		else
			return 1;

#elif defined(_WIN32)
		SYSTEM_INFO sysInfo = {};
		GetNativeSystemInfo(&sysInfo);
		if (sysInfo.dwNumberOfProcessors > 1)
			return sysInfo.dwNumberOfProcessors;
		else
			return 1;

#endif //__linux__
#else //EASYTEST_NO_THREADS is defined
		return 1;
#endif //!EASYTEST_NO_THREADS
	}
}

namespace easyTest
{
	const unsigned int TestRunner::NB_HARDWARE_THREADS = getHardwareThreadCount();

	TestRunner::~TestRunner()
	{
		stopTestSuites();
		waitTestsTermination();
	}

	bool TestRunner::startTestSuites(const TestSuiteRegistrar** testSuiteList, size_t nbTestSuites, unsigned int nbThreads, ITestListener* pTestListener)
	{
		if (m_testSuiteList || !testSuiteList || !nbTestSuites)
			return false;

		if (!m_executionTimer.start())
			return false;

		if (!RTErrorProtector::initProtectors())
		{
			m_executionTimer.stop();
			return false;
		}

		m_pTestListener = pTestListener;
		m_testSuiteList = testSuiteList;
		m_nbTestSuites = nbTestSuites;
		m_bStopRequested.store(false, std::memory_order_relaxed);

#ifndef EASYTEST_NO_THREADS
		assert(!m_nbThreads);
		assert(!m_threadList);

		m_nextTestSuiteIdx.store(0, std::memory_order_relaxed);
		m_nbFailedTestSuites.store(0, std::memory_order_relaxed);

		if (nbThreads)
		{
			if (nbThreads > m_nbTestSuites)
				nbThreads = static_cast<unsigned int>(m_nbTestSuites);

			m_threadList = new(std::nothrow) ThreadInfo[nbThreads]();
			if (m_threadList)
				m_nbThreads = nbThreads;
			else
				nbThreads = 0;
		}

#else //EASYTEST_NO_THREADS is defined
		m_nextTestSuiteIdx = 0;
		m_nbFailedTestSuites = 0;
		nbThreads = 0;
#endif //!EASYTEST_NO_THREADS

		if (m_pTestListener)
		{
			size_t nbTotalTestCases = 0;
			for (size_t i = 0; i < m_nbTestSuites; ++i)
			{
				const TestSuiteRegistrar* pRegistrar = m_testSuiteList[i];
				if (pRegistrar)
					nbTotalTestCases += pRegistrar->getTestCaseCount();
			}

			TestEvent event;
			event.type = TestEventType::TESTRUNNER_START;
			event.testRunnerStart.nbMaxWorkerThreads = nbThreads;
			event.testRunnerStart.nbTotalSuites = m_nbTestSuites;
			event.testRunnerStart.nbTotalTestCases = nbTotalTestCases;
			m_pTestListener->onEvent(event);
		}

#ifndef EASYTEST_NO_THREADS
		if (m_threadList)
		{
			bool bRunningInThread = false;
			for (size_t i = 0; i < m_nbThreads; ++i)
			{
				ThreadInfo* pInfo = m_threadList + i;
				pInfo->threadIdx = static_cast<unsigned int>(i);
				pInfo->pRunner = this;

#ifdef __linux__
				if (!pthread_create(&pInfo->threadHandle, nullptr, &run, pInfo))
					bRunningInThread = true;
				else
					pInfo->threadHandle = 0;

#elif defined(_WIN32)
				pInfo->threadHandle = _beginthreadex(nullptr, 0, &run, pInfo, 0, nullptr);
				if (pInfo->threadHandle)
					bRunningInThread = true;
#endif //__linux__
			}

			if (bRunningInThread)
				return true;
			else
			{
				m_nbThreads = 0;
				delete[] m_threadList;
				m_threadList = nullptr;
			}
		}
#endif //!EASYTEST_NO_THREADS

		//Execute all test suites in calling thread
		ThreadInfo info = {};
		info.pRunner = this;
		run(&info);

		return true;
	}

	void TestRunner::stopTestSuites()
	{
		m_bStopRequested.store(true, std::memory_order_relaxed);
	}

	size_t TestRunner::waitTestsTermination()
	{
		unsigned int nbUsedWorkerThreads = 0;

#ifndef EASYTEST_NO_THREADS
		if (m_threadList)
		{
			for (size_t i = 0; i < m_nbThreads; ++i)
			{
				ThreadInfo* pInfo = m_threadList + i;
				if (pInfo->threadHandle)
				{
					nbUsedWorkerThreads++;
#ifdef __linux__
					pthread_join(pInfo->threadHandle, nullptr);

#elif defined(_WIN32)
					HANDLE hThread = reinterpret_cast<HANDLE>(pInfo->threadHandle);
					WaitForSingleObject(hThread, INFINITE);
					CloseHandle(hThread);
#endif //__linux__
				}
			}

			m_nbThreads = 0;
			delete[] m_threadList;
			m_threadList = nullptr;
		}

		size_t nbTestSuites = m_nextTestSuiteIdx.load(std::memory_order_relaxed);
		const size_t nbFailedSuites = m_nbFailedTestSuites.load(std::memory_order_relaxed);

#else //EASYTEST_NO_THREADS is defined
		size_t nbTestSuites = m_nextTestSuiteIdx;
		const size_t nbFailedSuites = m_nbFailedTestSuites;
#endif //!EASYTEST_NO_THREADS

		if (m_testSuiteList)
		{
			RTErrorProtector::shutProtectors();
			m_executionTimer.stop();

			if (m_pTestListener)
			{
				TestEvent event;
				event.type = TestEventType::TESTRUNNER_FINISH;
				event.testRunnerFinish.nbUsedWorkerThreads = nbUsedWorkerThreads;
				event.testRunnerFinish.nbFailedSuites = nbFailedSuites;

				if (nbTestSuites > m_nbTestSuites)
					nbTestSuites = m_nbTestSuites;

				if (nbTestSuites > nbFailedSuites)
					event.testRunnerFinish.nbSuccessSuites = nbTestSuites - nbFailedSuites;
				else
					event.testRunnerFinish.nbSuccessSuites = 0;

				event.testRunnerFinish.nbTotalSuites = m_nbTestSuites;
				event.testRunnerFinish.pTimer = &m_executionTimer;
				m_pTestListener->onEvent(event);
			}

			m_nbTestSuites = 0;
			m_testSuiteList = nullptr;
			m_pTestListener = nullptr;
		}

		return nbFailedSuites;
	}

	threadproc_ret _callconv TestRunner::run(void* pUserData)
	{
		const ThreadInfo* pInfo = static_cast<const ThreadInfo*>(pUserData);
		assert(pInfo);
		assert(pInfo->pRunner);

		TestRunner* pRunner = pInfo->pRunner;
		assert(pRunner->m_testSuiteList);
		assert(pRunner->m_nbTestSuites);

		RTErrorProtector errorProtector(pRunner->m_pTestListener, pInfo->threadIdx);

		TestEvent event;
		stats::CodeTimer suiteTimer;
		stats::CodeTimer caseTimer;
		for (;;)
		{
			bool bStopRequested = pRunner->m_bStopRequested.load(std::memory_order_relaxed);
			if (bStopRequested)
				break;

#ifndef EASYTEST_NO_THREADS
			size_t idx = pRunner->m_nextTestSuiteIdx.fetch_add(1, std::memory_order_relaxed);
#else //EASYTEST_NO_THREADS is defined
			size_t idx = pRunner->m_nextTestSuiteIdx++;
#endif //!EASYTEST_NO_THREADS

			if (idx >= pRunner->m_nbTestSuites)
				break;

			suiteTimer.start();

			const TestSuiteRegistrar* pRegistrar = pRunner->m_testSuiteList[idx];
			bool bTestSuiteSuccess = false;
			TestSuite* pTestSuite = errorProtector.createProtectedTestSuite(pRegistrar);
			if (pTestSuite)
			{
				assert(pRegistrar);
				bTestSuiteSuccess = true;

				const size_t nbTotalCases = pTestSuite->getTestCaseCount();
				size_t nbSuccessCases = 0;
				size_t nbFailedCases = 0;

				if (pRunner->m_pTestListener)
				{
					event.type = TestEventType::TESTSUITE_START;
					event.testSuiteStart.workerThreadIdx = pInfo->threadIdx;
					event.testSuiteStart.testSuiteName = pRegistrar->getTestSuiteName();
					event.testSuiteStart.nbTotalCases = nbTotalCases;
					pRunner->m_pTestListener->onEvent(event);
				}

				const TestCaseRegistrar** caseRegListBegin = pTestSuite->getTestCaseList();
				const TestCaseRegistrar** caseRegListEnd = caseRegListBegin + nbTotalCases;
				for (const TestCaseRegistrar** ppCaseReg = caseRegListBegin; ppCaseReg < caseRegListEnd; ++ppCaseReg)
				{
					assert(*ppCaseReg);

					bStopRequested = pRunner->m_bStopRequested.load(std::memory_order_relaxed);
					if (bStopRequested)
					{
						bTestSuiteSuccess = false;
						break;
					}

					caseTimer.start();

					if (pRunner->m_pTestListener)
					{
						event.type = TestEventType::TESTCASE_START;
						event.testCaseStart.testCaseName = (*ppCaseReg)->getTestName();
						pRunner->m_pTestListener->onEvent(event);
					}

					if (errorProtector.executeProtectedTestCase(*ppCaseReg))
					{
						event.testCaseFinish.bSuccess = true;
						nbSuccessCases++;
					}
					else
					{
						event.testCaseFinish.bSuccess = false;
						nbFailedCases++;
						bTestSuiteSuccess = false;
					}

					caseTimer.stop();

					if (pRunner->m_pTestListener)
					{
						event.type = TestEventType::TESTCASE_FINISH;
						event.testCaseFinish.pTimer = &caseTimer;
						pRunner->m_pTestListener->onEvent(event);
					}
				}

				errorProtector.deleteProtectedTestSuite();

				suiteTimer.stop();

				if (pRunner->m_pTestListener)
				{
					event.type = TestEventType::TESTSUITE_FINISH;
					event.testSuiteFinish.nbTotalCases = nbTotalCases;
					event.testSuiteFinish.nbSuccessCases = nbSuccessCases;
					event.testSuiteFinish.nbFailedCases = nbFailedCases;
					event.testSuiteFinish.pTimer = &suiteTimer;
					pRunner->m_pTestListener->onEvent(event);
				}
			}
			else
				suiteTimer.stop();

			if (!bTestSuiteSuccess)
			{
#ifndef EASYTEST_NO_THREADS
				pRunner->m_nbFailedTestSuites.fetch_add(1, std::memory_order_relaxed);

#else //EASYTEST_NO_THREADS is defined
				pRunner->m_nbFailedTestSuites++;
#endif //!EASYTEST_NO_THREADS
			}

			if (bStopRequested)
				break;
		}

		return 0;
	}
}
