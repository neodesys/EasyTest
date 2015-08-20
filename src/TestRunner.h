/**
 * EasyTest++
 *
 * Copyright (c) 2015, Loïc Le Page
 *
 * This project is released under the MIT license.
 * See: http://opensource.org/licenses/MIT
 */

#ifndef _TESTRUNNER_H_
#define _TESTRUNNER_H_

#include <atomic>

#ifndef EASYTEST_NO_THREADS
//We cannot use std::thread because it is not exception-safe. When built without
//exception support, std::thread throws systematically an uncaught exception at
//thread start. This exception ends up in the global terminate_handler set by
//RTErrorProtector::initProtectors() but with an uninitialized jump buffer
//which leads to a segmentation fault. In order to be able to use EasyTest++
//with or without exception support, we have to rely on the native OS thread
//API which is exception-safe in all cases.

#ifdef __linux__
#include <pthread.h>
#elif defined(_WIN32)
#include <process.h>
#else
#error Unsupported platform: currently supported platforms are\
       Linux and Windows
#endif //__linux__

#else //EASYTEST_NO_THREADS is defined
#include <cstddef>
#endif

#include "stats/CodeTimer.h"

namespace easyTest
{
#ifndef EASYTEST_NO_THREADS
#ifdef __linux__
	typedef pthread_t thread_t;
	typedef void* threadproc_ret;
	#define _callconv
#elif defined(_WIN32)
	typedef uintptr_t thread_t;
	typedef unsigned threadproc_ret;
	#define _callconv __stdcall
#endif //__linux__

#else //EASYTEST_NO_THREADS is defined
	typedef unsigned int thread_t;
	typedef void* threadproc_ret;
	#define _callconv
#endif //!EASYTEST_NO_THREADS

	class TestSuiteRegistrar;
	class ITestListener;

	class TestRunner final
	{
	public:
		//TestRunner launches worker-threads but it is NOT thread-safe in
		//itself. All TestRunner methods MUST all be called from the same
		//control thread (main), with the only exception of stopTestSuites()
		//which can be called from any thread.
		TestRunner() = default;
		~TestRunner();

		static const unsigned int NB_HARDWARE_THREADS;

		//startTestSuites may launch less threads than requested and still
		//succeed. At worst, the method may process test suites in the calling
		//control thread (which always happens if nbThreads is 0 or
		//EASYTEST_NO_THREADS has been defined during compilation.
		bool startTestSuites(const TestSuiteRegistrar** testSuiteList, size_t nbTestSuites, unsigned int nbThreads, ITestListener* pTestListener);
		void stopTestSuites();
		size_t waitTestsTermination(); //returns the number of failed test suites

	private:
		TestRunner(const TestRunner&) = delete;
		TestRunner& operator=(const TestRunner&) = delete;

		size_t m_nbTestSuites = 0;
		const TestSuiteRegistrar** m_testSuiteList = nullptr;
		ITestListener* m_pTestListener = nullptr;
		stats::CodeTimer m_executionTimer;

		//Even when not using threads, we need an atomic flag used to stop any
		//running test suite when receiving an asynchronous interruption signal.
		std::atomic_bool m_bStopRequested = ATOMIC_VAR_INIT(false);

		static threadproc_ret _callconv run(void* pUserData);

		struct ThreadInfo
		{
			thread_t threadHandle;
			unsigned int threadIdx;
			TestRunner* pRunner;
		};

#ifndef EASYTEST_NO_THREADS
		size_t m_nbThreads = 0;
		ThreadInfo* m_threadList = nullptr;

		std::atomic_size_t m_nextTestSuiteIdx = ATOMIC_VAR_INIT(0);
		std::atomic_size_t m_nbFailedTestSuites = ATOMIC_VAR_INIT(0);

#else //EASYTEST_NO_THREADS is defined
		size_t m_nextTestSuiteIdx = 0;
		size_t m_nbFailedTestSuites = 0;
#endif //!EASYTEST_NO_THREADS
	};
}

#endif //_TESTRUNNER_H_
