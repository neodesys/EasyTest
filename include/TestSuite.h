/**
 * EasyTest++
 *
 * Copyright (c) 2015, Loïc Le Page
 *
 * This project is released under the MIT license.
 * See: http://opensource.org/licenses/MIT
 */

#ifndef _TESTSUITE_H_
#define _TESTSUITE_H_

#include <cmath>

#include "TestCaseRegistrar.h"
#include "RegistrarRefStorage.h"

namespace easyTest
{
	class SrcInfo final
	{
	public:
		SrcInfo(const char* file, const char* function, int line)
		{
			if (file)
				m_file = file;

			if (function)
				m_function = function;

			if (line > 0)
				m_line = line;
		}

		const char* getFile() const
		{
			return m_file;
		}

		const char* getFunction() const
		{
			return m_function;
		}

		unsigned int getLine() const
		{
			return m_line;
		}

	private:
		SrcInfo(const SrcInfo&) = delete;
		SrcInfo& operator=(const SrcInfo&) = delete;

		const char* m_file = "";
		const char* m_function = "";
		unsigned int m_line = 0;
	};

	enum struct AssertType : unsigned char
	{
		IS_TRUE,
		IS_FALSE,

		IS_NAN,
		IS_INFINITE,
		IS_FINITE,

		IS_LESS_THAN,
		IS_LESS_OR_EQUAL,
		IS_GREATER_THAN,
		IS_GREATER_OR_EQUAL,

		ARE_EQUAL,
		ARE_DIFFERENT,

		//All string asserts return false if any of the parameters is nullptr
		ARE_STRING_EQUAL,
		ARE_STRING_DIFFERENT,

		ARE_ALMOST_EQUAL,
		ARE_VERY_DIFFERENT,

		ARE_BITWISE_EQUAL,
		ARE_BITWISE_DIFFERENT,

		//All data asserts return false if any of the parameters is nullptr or
		//sizeInBytes is 0
		HAVE_SAME_DATA,
		HAVE_DIFFERENT_DATA
	};

	class ITestListener;

	class TestSuite
	{
	public:
		TestSuite() = default;
		virtual ~TestSuite() = default;

		virtual void setupFixture() {}
		virtual void teardownFixture() {}

		virtual size_t getTestCaseCount() const = 0;
		virtual const TestCaseRegistrar** getTestCaseList() const = 0;

	protected:
		void trace(const SrcInfo& info, const char* format, ...) const;

		bool isTrue(bool var, const char* varName, const SrcInfo& info) const
		{
			return unaryAssert(var, info, AssertType::IS_TRUE, varName);
		}

		bool isFalse(bool var, const char* varName, const SrcInfo& info) const
		{
			return unaryAssert(!var, info, AssertType::IS_FALSE, varName);
		}

		template<typename T> bool isNaN(const T& var, const char* varName, const SrcInfo& info) const
		{
			return unaryAssert(std::isnan(var), info, AssertType::IS_NAN, varName);
		}

		template<typename T> bool isInfinite(const T& var, const char* varName, const SrcInfo& info) const
		{
			return unaryAssert(std::isinf(var), info, AssertType::IS_INFINITE, varName);
		}

		template<typename T> bool isFinite(const T& var, const char* varName, const SrcInfo& info) const
		{
			return unaryAssert(std::isfinite(var), info, AssertType::IS_FINITE, varName);
		}

		template<typename T> bool isLessThan(const T& a, const T& b, const char* aName, const char* bName, const SrcInfo& info) const
		{
			return binaryAssert((a < b), info, AssertType::IS_LESS_THAN, aName, bName);
		}

		template<typename T> bool isLessOrEqual(const T& a, const T& b, const char* aName, const char* bName, const SrcInfo& info) const
		{
			return binaryAssert((a <= b), info, AssertType::IS_LESS_OR_EQUAL, aName, bName);
		}

		template<typename T> bool isGreaterThan(const T& a, const T& b, const char* aName, const char* bName, const SrcInfo& info) const
		{
			return binaryAssert((a > b), info, AssertType::IS_GREATER_THAN, aName, bName);
		}

		template<typename T> bool isGreaterOrEqual(const T& a, const T& b, const char* aName, const char* bName, const SrcInfo& info) const
		{
			return binaryAssert((a >= b), info, AssertType::IS_GREATER_OR_EQUAL, aName, bName);
		}

		template<typename T> bool areEqual(const T& a, const T& b, const char* aName, const char* bName, const SrcInfo& info) const
		{
			return binaryAssert((a == b), info, AssertType::ARE_EQUAL, aName, bName);
		}

		template<typename T> bool areDifferent(const T& a, const T& b, const char* aName, const char* bName, const SrcInfo& info) const
		{
			return binaryAssert((a != b), info, AssertType::ARE_DIFFERENT, aName, bName);
		}

		template<typename T> bool areStringEqual(const T* a, const T* b, const char* aName, const char* bName, const SrcInfo& info) const
		{
			return binaryAssert((a && b && strEqual(a, b)), info, AssertType::ARE_STRING_EQUAL, aName, bName);
		}

		template<typename T> bool areStringDifferent(const T* a, const T* b, const char* aName, const char* bName, const SrcInfo& info) const
		{
			return binaryAssert((a && b && !strEqual(a, b)), info, AssertType::ARE_STRING_DIFFERENT, aName, bName);
		}

		template<typename T> bool areAlmostEqual(const T& a, const T& b, const T& precision, const char* aName, const char* bName, const SrcInfo& info) const
		{
			return binaryAssert((std::abs(b - a) <= std::abs(precision)), info, AssertType::ARE_ALMOST_EQUAL, aName, bName);
		}

		template<typename T> bool areVeryDifferent(const T& a, const T& b, const T& precision, const char* aName, const char* bName, const SrcInfo& info) const
		{
			return binaryAssert((std::abs(b - a) > std::abs(precision)), info, AssertType::ARE_VERY_DIFFERENT, aName, bName);
		}

		template<typename A, typename B> bool areBitwiseEqual(const A& a, const B& b, const char* aName, const char* bName, const SrcInfo& info) const
		{
			size_t sa = sizeof(A);
			return binaryAssert(((sizeof(B) == sa) && !std::memcmp(&a, &b, sa)), info, AssertType::ARE_BITWISE_EQUAL, aName, bName);
		}

		template<typename A, typename B> bool areBitwiseDifferent(const A& a, const B& b, const char* aName, const char* bName, const SrcInfo& info) const
		{
			size_t sa = sizeof(A);
			return binaryAssert(((sizeof(B) != sa) || std::memcmp(&a, &b, sa)), info, AssertType::ARE_BITWISE_DIFFERENT, aName, bName);
		}

		bool haveSameData(const void* a, const void* b, size_t sizeInBytes, const char* aName, const char* bName, const SrcInfo& info) const
		{
			return binaryAssert((a && b && sizeInBytes && !std::memcmp(a, b, sizeInBytes)), info, AssertType::HAVE_SAME_DATA, aName, bName);
		}

		bool haveDifferentData(const void* a, const void* b, size_t sizeInBytes, const char* aName, const char* bName, const SrcInfo& info) const
		{
			return binaryAssert((a && b && sizeInBytes && std::memcmp(a, b, sizeInBytes)), info, AssertType::HAVE_DIFFERENT_DATA, aName, bName);
		}

	private:
		TestSuite(const TestSuite&) = delete;
		TestSuite& operator=(const TestSuite&) = delete;

		template<typename T> static bool strEqual(const T* strA, const T* strB)
		{
			while (*strA == *strB++)
			{
				if (*strA++ == 0)
					return true;
			}

			return false;
		}

		bool unaryAssert(bool condition, const SrcInfo& info, AssertType type, const char* varName) const;
		bool binaryAssert(bool condition, const SrcInfo& info, AssertType type, const char* varAName, const char* varBName) const;

		ITestListener* m_pTestListener = nullptr;
		unsigned int m_workerThreadIdx = 0;
		template<class C> friend class TestSuiteReg;
	};

	template<class C> class TestSuiteReg;

	template<class C> class SpecTestSuite : public TestSuite
	{
	public:
		virtual size_t getTestCaseCount() const override final
		{
			return getTestRefList().getRegistrarCount();
		}

		virtual const TestCaseRegistrar** getTestCaseList() const override final
		{
			return getTestRefList().getRegistrarList();
		}

	private:
		//We use a static function and not a simple static instance in order to
		//prevent any "static initialization order fiasco" as a simple instance
		//may be initialized AFTER TestCaseReg constructor.
		static RegistrarRefStorage<TestCaseRegistrar>& getTestRefList()
		{
			static RegistrarRefStorage<TestCaseRegistrar> s_testRefList;
			return s_testRefList;
		}

		friend TestCaseReg<C>::TestCaseReg(const char*, typename TestCaseReg<C>::TestCaseFunc);
		friend size_t TestSuiteReg<C>::getTestCaseCount() const;
	};
}

#endif //_TESTSUITE_H_
