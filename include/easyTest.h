/**
 * EasyTest++
 *
 * Copyright (c) 2015, Loïc Le Page
 *
 * This project is released under the MIT license.
 * See: http://opensource.org/licenses/MIT
 */

#ifndef _EASYTEST_H_
#define _EASYTEST_H_

#ifdef __GNUC__ //GNU-compatible compilers: g++, mingw and clang
#if __cplusplus < 201103L
#error You must use a C++11 compatible compiler
#endif //__cplusplus < 201103L
#elif defined(_MSC_VER) //Visual C++ compiler
#if _MSC_VER < 1900
#error You must use a C++11 compatible compiler
#endif //_MSC_VER < 1900
#else
#error Unsupported compiler: currently supported compilers are\
       gnu-compatible ones (gcc, mingw and clang) and Visual C++
#endif

#include "TestSuiteRegistrar.h"

#define TEST_SUITE(name)\
	class name;\
	static const easyTest::TestSuiteReg<name> s_##name##_Reg(#name);\
	class name final : public easyTest::SpecTestSuite<name>

#define TEST_CASE(name) public: bool name()

#define TEST_IMPL(suiteName, testName)\
	static const easyTest::TestCaseReg<suiteName> s_##suiteName##_##testName##_Reg(#testName, &suiteName::testName);\
	bool suiteName::testName()

#define SRC_INFO() easyTest::SrcInfo(__FILE__, __func__, __LINE__)

#define TRACE(str) trace(SRC_INFO(), str)
#define TRACE_FORMAT(format, ...) trace(SRC_INFO(), format, __VA_ARGS__)

#define ASSERT_TRUE(var) if (!isTrue(var, #var, SRC_INFO())) return false
#define ASSERT_FALSE(var) if (!isFalse(var, #var, SRC_INFO())) return false

#define ASSERT_NAN(var) if (!isNaN(var, #var, SRC_INFO())) return false
#define ASSERT_INFINITE(var) if (!isInfinite(var, #var, SRC_INFO())) return false
#define ASSERT_FINITE(var) if (!isFinite(var, #var, SRC_INFO())) return false

#define ASSERT_LESS_THAN(a, b) if (!isLessThan(a, b, #a, #b, SRC_INFO())) return false
#define ASSERT_LESS_OR_EQUAL(a, b) if (!isLessOrEqual(a, b, #a, #b, SRC_INFO())) return false
#define ASSERT_GREATER_THAN(a, b) if (!isGreaterThan(a, b, #a, #b, SRC_INFO())) return false
#define ASSERT_GREATER_OR_EQUAL(a, b) if (!isGreaterOrEqual(a, b, #a, #b, SRC_INFO())) return false

#define ASSERT_EQUAL(a, b) if (!areEqual(a, b, #a, #b, SRC_INFO())) return false
#define ASSERT_DIFFERENT(a, b) if (!areDifferent(a, b, #a, #b, SRC_INFO())) return false

//All string asserts return false if any of the parameters is nullptr
#define ASSERT_STRING_EQUAL(a, b) if (!areStringEqual(a, b, #a, #b, SRC_INFO())) return false
#define ASSERT_STRING_DIFFERENT(a, b) if (!areStringDifferent(a, b, #a, #b, SRC_INFO())) return false

#define ASSERT_ALMOST_EQUAL(a, b, precision) if (!areAlmostEqual(a, b, precision, #a, #b, SRC_INFO())) return false
#define ASSERT_VERY_DIFFERENT(a, b, precision) if (!areVeryDifferent(a, b, precision, #a, #b, SRC_INFO())) return false

#define ASSERT_BITWISE_EQUAL(a, b) if (!areBitwiseEqual(a, b, #a, #b, SRC_INFO())) return false
#define ASSERT_BITWISE_DIFFERENT(a, b) if (!areBitwiseDifferent(a, b, #a, #b, SRC_INFO())) return false

//All data asserts return false if any of the parameters is nullptr or
//sizeInBytes is 0
#define ASSERT_SAME_DATA(a, b, sizeInBytes) if (!haveSameData(a, b, sizeInBytes, #a, #b, SRC_INFO())) return false
#define ASSERT_DIFFERENT_DATA(a, b, sizeInBytes) if (!haveDifferentData(a, b, sizeInBytes, #a, #b, SRC_INFO())) return false

/**
 * Usage:
 * You define a test suite and its associated test cases in the same .cpp file
 * as follows:
 *
 * TEST_SUITE(MyTestSuiteName)
 * {
 *     TEST_CASE(test1Name);
 *     TEST_CASE(test2Name);
 * };
 *
 * TEST_IMPL(MyTestSuiteName, test1Name)
 * {
 *     ...test1 content...
 *     ...must return true on success and false on failure...
 * }
 *
 * TEST_IMPL(MyTestSuiteName, test2Name)
 * {
 *     ...test2 content...
 *     ...must return true on success and false on failure...
 * }
 *
 * Normally you should use one .cpp file for one test suite but you can also
 * implement more than one test suite in the same .cpp file as long as the
 * whole test suite is implemented in the same file.
 *
 *
 * You can initialize per test suite private fields using the test suite
 * default constructor/destructor as follows. A new test suite instance is
 * always created right before tests execution and destroyed right after.
 *
 * TEST_SUITE(MyTestSuiteName)
 * {
 * public:
 *     MyTestSuiteName() : m_buffer(new unsigned char[1024]()) {}
 *     virtual ~MyTestSuiteName() override final { delete[] m_buffer; }
 *
 *     TEST_CASE(test1Name);
 *     TEST_CASE(test2Name);
 *
 * private:
 *     unsigned char* const m_buffer;
 * };
 *
 * TEST_IMPL(MyTestSuiteName, test1Name)
 * {
 *     ...
 * }
 *
 * TEST_IMPL(MyTestSuiteName, test2Name)
 * {
 *     ...
 * }
 *
 * WARNING: per test suite fields are not reseted between test cases, if test1
 * modifies m_buffer content, test2 will see the modifications. Normally, per
 * test suite fields should be declared const as their content should not be
 * modified by individual test cases (const unsigned char* const m_buffer;).
 *
 *
 * In order to initialize/reset private fields before each test case (fixture),
 * you can override the setupFixture() and teardownFixture() virtual methods:
 *
 * TEST_SUITE(MyTestSuiteName)
 * {
 * public:
 *     virtual void setupFixture() override final;
 *     virtual void teardownFixture() override final;
 *
 *     TEST_CASE(test1Name);
 *     TEST_CASE(test2Name);
 *
 * private:
 *     unsigned char* m_buffer = nullptr;
 * };
 *
 * void MyTestSuiteName::setupFixture()
 * {
 *     m_buffer = new unsigned char[1024]();
 * }
 *
 * void MyTestSuiteName::teardownFixture()
 * {
 *     delete[] m_buffer;
 *     m_buffer = nullptr;
 * }
 *
 * TEST_IMPL(MyTestSuiteName, test1Name)
 * {
 *     ...
 * }
 *
 * TEST_IMPL(MyTestSuiteName, test2Name)
 * {
 *     ...
 * }
 *
 * Any modification of m_buffer by test1 will have no impact on test2.
 *
 *
 * Obviously, you can combine both technics:
 *
 * TEST_SUITE(MyTestSuiteName)
 * {
 * public:
 *     MyTestSuiteName() : m_buffer(new unsigned char[1024]) {}
 *     virtual ~MyTestSuiteName() override final { delete[] m_buffer; }
 *
 *     virtual void setupFixture() override final;
 *
 *     TEST_CASE(test1Name);
 *     TEST_CASE(test2Name);
 *
 * private:
 *     unsigned char* const m_buffer;
 * };
 *
 * void MyTestSuiteName::setupFixture()
 * {
 *     std::memset(m_buffer, 0, 1024);
 * }
 *
 * TEST_IMPL(MyTestSuiteName, test1Name)
 * {
 *     ...
 * }
 *
 * TEST_IMPL(MyTestSuiteName, test2Name)
 * {
 *     ...
 * }
 *
 * In this case, each test case has access to a writable m_buffer initialized
 * with 0 before each individual test case execution, but m_buffer is only
 * allocated once during test suite creation and before executing all test
 * cases. Remark: in this example m_buffer is not zero-initialized during
 * allocation, this operation is useless as the buffer content is set to 0
 * before each individual test case.
 *
 *
 * Different test suites MUST NEVER share any global data as their repective
 * executions can be performed in parallel.
 * All test cases in a same test suite are always executed sequentially in the
 * order of their implementation (TEST_IMPL macros).
 *
 * The EasyTest++ system neither uses directly nor relies on C++ exceptions. It
 * may be safely compiled with exception handling fully disabled
 * (-fno-exceptions flag with g++). However, it is advised to compile it with
 * exception handling enabled in order to get more information about uncaught
 * exceptions, in particular the exception type. If built without exception
 * support, any uncaught C++ exception will be just reported as
 * "undefined exception" without any more information.
 *
 * In all cases, the EasyTest++ system intercepts runtime exceptions/signals
 * such as floating-point exceptions, unaligned memory access, etc...
 * If the test executable is running through a debugger while a runtime
 * exception/signal occurs, the debugger will catch the exception first and the
 * test runner will stop. Under linux, you can bypass this behavior specifying
 * "handle all nostop noprint pass" to gdb. Under Windows, you cannot bypass
 * this behavior and the debugger will always stop on an uncaught runtime
 * exception. This should not be an issue as, normally, test runners are meant
 * to be run outside of any debugger.
 */

#endif //_EASYTEST_H_
