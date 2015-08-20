/**
 * EasyTest++
 *
 * Copyright (c) 2015, Loïc Le Page
 *
 * This project is released under the MIT license.
 * See: http://opensource.org/licenses/MIT
 */

#ifndef _RTERRORPROTECTOR_H_
#define _RTERRORPROTECTOR_H_

namespace easyTest
{
	class TestSuite;
	class TestSuiteRegistrar;
	class ITestListener;
	class TestCaseRegistrar;
	enum struct RTErrorType : unsigned char;

	class RTErrorProtector final
	{
	public:
		//initProtectors() MUST be called BEFORE any test suite worker-thread
		//creation and shutProtectors() MUST be called AFTER all these worker
		//threads have terminated.
		static bool initProtectors();
		static void shutProtectors();

		//RTErrorProtector instances are to be used within each test suite
		//worker-thread. There should be a local instance of RTErrorProtector
		//in each worker-thread which wants to protect tests execution.
		RTErrorProtector(ITestListener* pTestListener, unsigned int workerThreadIdx) : m_pTestListener(pTestListener), m_workerThreadIdx(workerThreadIdx) {}
		~RTErrorProtector();

		TestSuite* createProtectedTestSuite(const TestSuiteRegistrar* pRegistrar);
		bool executeProtectedTestCase(const TestCaseRegistrar* pCaseReg);
		void deleteProtectedTestSuite();

	private:
		RTErrorProtector(const RTErrorProtector&) = delete;
		RTErrorProtector& operator=(const RTErrorProtector&) = delete;

		void fireRuntimeError(RTErrorType type, const char* exceptionDetails) const;

		ITestListener* const m_pTestListener;
		const unsigned int m_workerThreadIdx;

		const TestSuiteRegistrar* m_pRegistrar = nullptr;
		TestSuite* m_pTestSuite = nullptr;

		static bool s_hasProtectors;
	};
}

#endif //_RTERRORPROTECTOR_H_
