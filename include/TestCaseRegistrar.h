/**
 * EasyTest++
 *
 * Copyright (c) 2015, Loïc Le Page
 *
 * This project is released under the MIT license.
 * See: http://opensource.org/licenses/MIT
 */

#ifndef _TESTCASEREGISTRAR_H_
#define _TESTCASEREGISTRAR_H_

namespace easyTest
{
	class TestSuite;
	class TestCaseRegistrar
	{
	public:
		TestCaseRegistrar(const char* testName)
		{
			if (testName)
				m_testName = testName;
		}

		const char* getTestName() const
		{
			return m_testName;
		}

		virtual bool executeTest(TestSuite* pSuite) const = 0;

	protected:
		~TestCaseRegistrar() = default;

	private:
		TestCaseRegistrar(const TestCaseRegistrar&) = delete;
		TestCaseRegistrar& operator=(const TestCaseRegistrar&) = delete;

		const char* m_testName = "";
	};

	template<class C> class SpecTestSuite;
	template<class C> class TestCaseReg final : TestCaseRegistrar
	{
	public:
		typedef bool (C::*TestCaseFunc)();

		TestCaseReg(const char* testName, TestCaseFunc testFunc) : TestCaseRegistrar(testName)
		{
			if (testFunc)
				m_testFunc = testFunc;

			SpecTestSuite<C>::getTestRefList().addRegistrarRef(this);
		}

		virtual bool executeTest(TestSuite* pSuite) const override final
		{
			return (m_testFunc && pSuite) ? (static_cast<C*>(pSuite)->*m_testFunc)() : false;
		}

	private:
		TestCaseFunc m_testFunc = nullptr;
	};
}

#endif //_TESTCASEREGISTRAR_H_
