/**
 * EasyTest++
 *
 * Copyright (c) 2015, Loïc Le Page
 *
 * This project is released under the MIT license.
 * See: http://opensource.org/licenses/MIT
 */

#ifndef _ITESTLISTENER_H_
#define	_ITESTLISTENER_H_

#include <cstddef>
#include <cstdarg>

namespace easyTest
{
	namespace stats
	{
		class CodeTimer;
	}
	class SrcInfo;
	enum struct AssertType : unsigned char;

	enum struct TestEventType : unsigned char
	{
		TESTRUNNER_START,	//Always sent by the TestRunner control thread BEFORE worker-threads creation
		TESTRUNNER_FINISH,	//Always sent by the TestRunner control thread AFTER all worker-threads termination
		TESTSUITE_ERROR,	//Always sent by a TestSuite execution worker-thread
		TESTSUITE_START,	//Always sent by a TestSuite execution worker-thread
		TESTSUITE_FINISH,	//Always sent by a TestSuite execution worker-thread
		TESTCASE_START,		//Always sent by a TestSuite execution worker-thread
		TESTCASE_FINISH		//Always sent by a TestSuite execution worker-thread
	};

	struct TestRunnerStartEvent
	{
		TestEventType type;
		unsigned int nbMaxWorkerThreads;
		size_t nbTotalSuites;
		size_t nbTotalTestCases;
	};

	struct TestRunnerFinishEvent
	{
		//nbTotalSuites can be greater than nbSuccessSuites + nbFailedSuites if
		//execution has been stopped before completion.

		TestEventType type;
		unsigned int nbUsedWorkerThreads;
		size_t nbTotalSuites;
		size_t nbSuccessSuites;
		size_t nbFailedSuites;
		const stats::CodeTimer* pTimer;
	};

	struct TestSuiteErrorEvent
	{
		TestEventType type;
		unsigned int workerThreadIdx;
		const char* testSuiteName;
		size_t nbTestCases;
	};

	struct TestSuiteStartEvent
	{
		TestEventType type;
		unsigned int workerThreadIdx;
		const char* testSuiteName;
		size_t nbTotalCases;
	};

	struct TestSuiteFinishEvent
	{
		//nbTotalCases can be greater than nbSuccessCases + nbFailedCases if
		//execution has been stopped before completion.

		TestEventType type;
		unsigned int workerThreadIdx;
		const char* testSuiteName;
		size_t nbTotalCases;
		size_t nbSuccessCases;
		size_t nbFailedCases;
		const stats::CodeTimer* pTimer;
	};

	struct TestCaseStartEvent
	{
		TestEventType type;
		unsigned int workerThreadIdx;
		const char* testSuiteName;
		const char* testCaseName;
	};

	struct TestCaseFinishEvent
	{
		TestEventType type;
		unsigned int workerThreadIdx;
		const char* testSuiteName;
		const char* testCaseName;
		bool bSuccess;
		const stats::CodeTimer* pTimer;
	};

	union TestEvent
	{
		TestEventType type;
		TestRunnerStartEvent testRunnerStart;
		TestRunnerFinishEvent testRunnerFinish;
		TestSuiteErrorEvent testSuiteError;
		TestSuiteStartEvent testSuiteStart;
		TestSuiteFinishEvent testSuiteFinish;
		TestCaseStartEvent testCaseStart;
		TestCaseFinishEvent testCaseFinish;
	};

	enum struct RTErrorType : unsigned char
	{
		UNDEFINED_RTERROR,			//Undefined runtime error
		UNDEFINED_CPP_EXCEPTION,	//Undefined C++ exception (uncaught)
		CPP_EXCEPTION,				//Normal C++ exception (uncaught)
		MEM_NOT_MAPPED,				//Access to a virtual memory address not mapped
		MEM_ACCESS_VIOLATION,		//No permission to acces to a mapped memory address
		MEM_BUS_ERROR,				//Memory bus error (non existent physical address or other hardware error)
		MEM_MISALIGNMENT,			//Access to misaligned address in memory
		INSTR_ERROR,				//Error while executing an instruction (coprocessor or internal stack error)
		INSTR_ILLEGAL,				//Execution of an illegal instruction
		INSTR_PRIVILEDGED,			//Execution of a priviledged instruction
		INT_DIV_BY_ZERO,			//Integer division by zero
		INT_OVERFLOW,				//Integer overflow
		FLOAT_ERROR,				//Undefined floating-point error
		FLOAT_DIV_BY_ZERO,			//Floating-point division by zero
		FLOAT_OVERFLOW,				//Floating-point overflow
		FLOAT_UNDERFLOW,			//Floating-point underflow
		FLOAT_BAD_RESULT,			//Inexact floating-point result
		FLOAT_INVALID_OP,			//Invalid floating-point operation
		ARRAY_OUT_OF_BOUNDS,		//Index access outside of the bounds of an array (if hardware supports bounds checking)
		SYSTEM_ERROR,				//Bad argument passed to a system call
		BROKEN_PIPE,				//Write to a pipe/socket with no reader
		FILE_SIZE_OVER_LIMIT		//File size limit has been exceeded
	};

	class ITestListener
	{
	public:
		virtual ~ITestListener() = default;

		//Can be called from the TestRunner control thread (main) or from a
		//TestSuite exectution worker-thread depending on the event type
		//(see TestEventType enum for more information)
		virtual void onEvent(const TestEvent& event) = 0;

		//Always called from a TestSuite exectution worker-thread
		virtual void onTrace(unsigned int workerThreadIdx, const SrcInfo& info, const char* format, std::va_list& varArgs) = 0;

		//Always called from a TestSuite exectution worker-thread
		virtual void onUnaryAssertFailure(unsigned int workerThreadIdx, const SrcInfo& info, AssertType type, const char* varName) = 0;

		//Always called from a TestSuite exectution worker-thread
		virtual void onBinaryAssertFailure(unsigned int workerThreadIdx, const SrcInfo& info, AssertType type, const char* varAName, const char* varBName) = 0;

		//The parameter exceptionDetails is only specified with the
		//CPP_EXCEPTION type, it is nullptr for all other error types.
		//Always called from a TestSuite exectution worker-thread
		virtual void onRuntimeError(unsigned int workerThreadIdx, RTErrorType type, const char* exceptionDetails) = 0;
	};
}

#endif //_ITESTLISTENER_H_
