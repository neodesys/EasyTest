/**
 * EasyTest++
 *
 * Copyright (c) 2015, Loïc Le Page
 *
 * This project is released under the MIT license.
 * See: http://opensource.org/licenses/MIT
 */

#ifndef _TESTSUITEREGISTRAR_H_
#define _TESTSUITEREGISTRAR_H_

#include <new>

#include "TestSuite.h"

namespace easyTest
{
	class TestSuiteRegistrar
	{
	public:
		TestSuiteRegistrar(const char* testSuiteName)
		{
			if (testSuiteName)
				m_testSuiteName = testSuiteName;

			getSuiteRefList().addRegistrarRef(this);
		}

		const char* getTestSuiteName() const
		{
			return m_testSuiteName;
		}

		virtual size_t getTestCaseCount() const = 0;

		virtual TestSuite* createTestSuite(ITestListener* pTestListener, unsigned int workerThreadIdx) const = 0;
		virtual void deleteTestSuite(TestSuite* p) const = 0;

		static size_t getTestSuiteCount()
		{
			return getSuiteRefList().getRegistrarCount();
		}

		static const TestSuiteRegistrar** getTestSuiteList()
		{
			return getSuiteRefList().getRegistrarList();
		}

	protected:
		~TestSuiteRegistrar() = default;

	private:
		TestSuiteRegistrar(const TestSuiteRegistrar&) = delete;
		TestSuiteRegistrar& operator=(const TestSuiteRegistrar&) = delete;

		const char* m_testSuiteName = "";

		//We use a static function and not a simple static instance in order to
		//prevent any "static initialization order fiasco" as a simple instance
		//may be initialized AFTER TestSuiteRegistrar constructor.
		static RegistrarRefStorage<TestSuiteRegistrar>& getSuiteRefList()
		{
			static RegistrarRefStorage<TestSuiteRegistrar> s_suiteRefList;
			return s_suiteRefList;
		}
	};

	template<class C> class TestSuiteReg final : public TestSuiteRegistrar
	{
	public:
		TestSuiteReg(const char* testSuiteName) : TestSuiteRegistrar(testSuiteName) {}

		virtual size_t getTestCaseCount() const override final
		{
			return SpecTestSuite<C>::getTestRefList().getRegistrarCount();
		}

		virtual TestSuite* createTestSuite(ITestListener* pTestListener, unsigned int workerThreadIdx) const override final
		{
			TestSuite* pSuite = new(std::nothrow) C();
			if (pSuite)
			{
				pSuite->m_workerThreadIdx = workerThreadIdx;
				pSuite->m_pTestListener = pTestListener;
				return pSuite;
			}

			return nullptr;
		}

		virtual void deleteTestSuite(TestSuite* p) const override final
		{
			if (p)
			{
				//We reset the listener before destroying the suite in order
				//to prevent trace and assert outputs during destruction.
				p->m_pTestListener = nullptr;
				delete p;
			}
		}
	};
}

#endif //_TESTSUITEREGISTRAR_H_
