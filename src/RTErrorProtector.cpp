/**
 * EasyTest++
 *
 * Copyright (c) 2015, Loïc Le Page
 *
 * This project is released under the MIT license.
 * See: http://opensource.org/licenses/MIT
 */

#include "RTErrorProtector.h"

#include <cassert>
#include <csetjmp>

#ifdef __GNUC__ //GNU-compatible compilers: g++, mingw and clang
#ifdef __EXCEPTIONS
#define HAS_EXCEPTIONS 1
#endif
#elif defined(_MSC_VER) //Visual C++ compiler
#ifdef _CPPUNWIND
#define HAS_EXCEPTIONS 1
#endif
#else
#error Unsupported compiler: currently supported compilers are\
       gnu-compatible ones (gcc, mingw and clang) and Visual C++
#endif

#ifdef HAS_EXCEPTIONS
#include <exception>
#endif //HAS_EXCEPTIONS

#ifdef __linux__
#include <cstdlib>
#include <csignal>

#elif defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define sigjmp_buf std::jmp_buf
#define sigsetjmp(env, savesigs) setjmp(env)
#define siglongjmp(env, val) std::longjmp(env, val)
#else
#error Unsupported platform: currently supported platforms are\
       Linux and Windows
#endif //__linux__

#include "../include/TestSuiteRegistrar.h"
#include "ITestListener.h"

namespace
{
	thread_local sigjmp_buf s_threadLocaljumpBuffer = {};

	thread_local easyTest::RTErrorType s_threadLocalRuntimeErrorType = easyTest::RTErrorType::UNDEFINED_RTERROR;

	std::terminate_handler s_defaultTerminateHandler = nullptr;

	void terminateHandler()
	{
		s_threadLocalRuntimeErrorType = easyTest::RTErrorType::UNDEFINED_CPP_EXCEPTION;
		siglongjmp(s_threadLocaljumpBuffer, 1);
	}

#ifdef __linux__
	class SignalInterceptor final
	{
	public:
		SignalInterceptor(int signal) : m_signal(signal) {}
		~SignalInterceptor();

		bool initInterception();
		void shutInterception();

	private:
		SignalInterceptor(const SignalInterceptor&) = delete;
		SignalInterceptor& operator=(const SignalInterceptor&) = delete;

		const int m_signal;
		bool m_bIntercepted = false;
		struct sigaction m_defaultSignalHandler = {};
		stack_t m_defaultSignalStack = {};

		static void signalInterceptorHandler(int signal, siginfo_t* pSignalInfo, void* pContext);
	};

	SignalInterceptor::~SignalInterceptor()
	{
		shutInterception();
	}

	bool SignalInterceptor::initInterception()
	{
		if (m_bIntercepted)
			return true;

		struct sigaction signalHandler = {};
		signalHandler.sa_sigaction = &signalInterceptorHandler;
		signalHandler.sa_flags = SA_SIGINFO;

		if (m_signal == SIGSEGV)
		{
			//SIGSEGV signal may be emitted in response to a stack overflow
			//issue, so we need to handle it on an alternate stack as the
			//program default stack may be broken when receiveing this signal.
			signalHandler.sa_flags |= SA_ONSTACK;

			stack_t signalStack = {};
			signalStack.ss_size = SIGSTKSZ;
			signalStack.ss_sp = std::malloc(SIGSTKSZ);
			if (!signalStack.ss_sp)
				return false;

			if (sigaltstack(&signalStack, &m_defaultSignalStack))
			{
				std::free(signalStack.ss_sp);
				return false;
			}
		}

		if (sigaction(m_signal, &signalHandler, &m_defaultSignalHandler))
		{
			if (m_signal == SIGSEGV)
			{
				//Restablish the default signal stack.
				stack_t signalStack = {};
				sigaltstack(&m_defaultSignalStack, &signalStack);
				if (signalStack.ss_sp)
					std::free(signalStack.ss_sp);
			}

			return false;
		}

		m_bIntercepted = true;
		return true;
	}

	void SignalInterceptor::shutInterception()
	{
		if (m_bIntercepted)
		{
			sigaction(m_signal, &m_defaultSignalHandler, nullptr);

			if (m_signal == SIGSEGV)
			{
				//Restablish the default signal stack.
				stack_t signalStack = {};
				sigaltstack(&m_defaultSignalStack, &signalStack);
				if (signalStack.ss_sp)
					std::free(signalStack.ss_sp);
			}

			m_bIntercepted = false;
		}
	}

	void SignalInterceptor::signalInterceptorHandler(int signal, siginfo_t* pSignalInfo, void*)
	{
		//We only intercept synchronous signals, so signalInterceptorHandler()
		//is called in the same thread as the one which emitted the signal.
		//Therefore, the thread-local s_threadLocaljumpBuffer is available
		//and initialized in this context.

		switch (signal)
		{
		case SIGSEGV:
			s_threadLocalRuntimeErrorType = easyTest::RTErrorType::MEM_NOT_MAPPED;
			if (pSignalInfo && (pSignalInfo->si_code == SEGV_ACCERR))
				s_threadLocalRuntimeErrorType = easyTest::RTErrorType::MEM_ACCESS_VIOLATION;
			break;

		case SIGBUS:
			s_threadLocalRuntimeErrorType = easyTest::RTErrorType::MEM_BUS_ERROR;
			if (pSignalInfo && (pSignalInfo->si_code == BUS_ADRALN))
				s_threadLocalRuntimeErrorType = easyTest::RTErrorType::MEM_MISALIGNMENT;
			break;

		case SIGILL:
			s_threadLocalRuntimeErrorType = easyTest::RTErrorType::INSTR_ERROR;
			if (pSignalInfo)
			{
				switch (pSignalInfo->si_code)
				{
				case ILL_ILLOPC:
				case ILL_ILLOPN:
				case ILL_ILLADR:
				case ILL_ILLTRP:
					s_threadLocalRuntimeErrorType = easyTest::RTErrorType::INSTR_ILLEGAL;
					break;

				case ILL_PRVOPC:
				case ILL_PRVREG:
					s_threadLocalRuntimeErrorType = easyTest::RTErrorType::INSTR_PRIVILEDGED;
					break;
				}
			}
			break;

		case SIGFPE:
			s_threadLocalRuntimeErrorType = easyTest::RTErrorType::FLOAT_ERROR;
			if (pSignalInfo)
			{
				switch (pSignalInfo->si_code)
				{
				case FPE_INTDIV:
					s_threadLocalRuntimeErrorType = easyTest::RTErrorType::INT_DIV_BY_ZERO;
					break;

				case FPE_INTOVF:
					s_threadLocalRuntimeErrorType = easyTest::RTErrorType::INT_OVERFLOW;
					break;

				case FPE_FLTDIV:
					s_threadLocalRuntimeErrorType = easyTest::RTErrorType::FLOAT_DIV_BY_ZERO;
					break;

				case FPE_FLTOVF:
					s_threadLocalRuntimeErrorType = easyTest::RTErrorType::FLOAT_OVERFLOW;
					break;

				case FPE_FLTUND:
					s_threadLocalRuntimeErrorType = easyTest::RTErrorType::FLOAT_UNDERFLOW;
					break;

				case FPE_FLTRES:
					s_threadLocalRuntimeErrorType = easyTest::RTErrorType::FLOAT_BAD_RESULT;
					break;

				case FPE_FLTINV:
					s_threadLocalRuntimeErrorType = easyTest::RTErrorType::FLOAT_INVALID_OP;
					break;

				case FPE_FLTSUB:
					s_threadLocalRuntimeErrorType = easyTest::RTErrorType::ARRAY_OUT_OF_BOUNDS;
					break;
				}
			}
			break;

		case SIGSYS:
			s_threadLocalRuntimeErrorType = easyTest::RTErrorType::SYSTEM_ERROR;
			break;

		case SIGPIPE:
			s_threadLocalRuntimeErrorType = easyTest::RTErrorType::BROKEN_PIPE;
			break;

		case SIGXFSZ:
			s_threadLocalRuntimeErrorType = easyTest::RTErrorType::FILE_SIZE_OVER_LIMIT;
			break;

		default:
			s_threadLocalRuntimeErrorType = easyTest::RTErrorType::UNDEFINED_RTERROR;
			break;
		}

		siglongjmp(s_threadLocaljumpBuffer, 1);
	}

	//Intercepted signals MUST ALL be synchronous signals exclusively because
	//we need the signal to be handled in the same thread as the one which
	//emitted it.
	SignalInterceptor s_signalInterceptorList[] = {{SIGSEGV}, {SIGBUS}, {SIGILL}, {SIGFPE}, {SIGSYS}, {SIGPIPE}, {SIGXFSZ}};
	const size_t s_nbSignalInterceptors = sizeof(s_signalInterceptorList) / sizeof(s_signalInterceptorList[0]);

#elif defined(_WIN32)
	LPTOP_LEVEL_EXCEPTION_FILTER s_defaultExceptionHandler = nullptr;

	LONG WINAPI exceptionHandler(EXCEPTION_POINTERS* pExceptionInfo)
	{
		if (!pExceptionInfo || !pExceptionInfo->ExceptionRecord)
			return EXCEPTION_CONTINUE_SEARCH;

		//exceptionHandler() is called in the same thread as the one which
		//caused the fault. Therefore, the thread-local s_threadLocaljumpBuffer
		//is available and initialized in this context.

		s_threadLocalRuntimeErrorType = easyTest::RTErrorType::UNDEFINED_RTERROR;
		switch (pExceptionInfo->ExceptionRecord->ExceptionCode)
		{
		case EXCEPTION_IN_PAGE_ERROR: //SIGSEGV -> SEGV_MAPERR
		case EXCEPTION_STACK_OVERFLOW:
			s_threadLocalRuntimeErrorType = easyTest::RTErrorType::MEM_NOT_MAPPED;
			break;

		case EXCEPTION_ACCESS_VIOLATION: //SIGSEGV -> SEGV_ACCERR
			s_threadLocalRuntimeErrorType = easyTest::RTErrorType::MEM_ACCESS_VIOLATION;
			break;

		case EXCEPTION_DATATYPE_MISALIGNMENT: //SIGBUS -> BUS_ADRALN
			s_threadLocalRuntimeErrorType = easyTest::RTErrorType::MEM_MISALIGNMENT;
			break;

		case EXCEPTION_ILLEGAL_INSTRUCTION: //SIGILL -> ILL_ILLOPC, ILL_ILLOPN, ILL_ILLADR, ILL_ILLTRP
			s_threadLocalRuntimeErrorType = easyTest::RTErrorType::INSTR_ILLEGAL;
			break;

		case EXCEPTION_PRIV_INSTRUCTION: //SIGILL -> ILL_PRVOPC, ILL_PRVREG
			s_threadLocalRuntimeErrorType = easyTest::RTErrorType::INSTR_PRIVILEDGED;
			break;

		case EXCEPTION_INT_DIVIDE_BY_ZERO: //SIGFPE -> FPE_INTDIV
			s_threadLocalRuntimeErrorType = easyTest::RTErrorType::INT_DIV_BY_ZERO;
			break;

		case EXCEPTION_INT_OVERFLOW: //SIGFPE -> FPE_INTOVF
			s_threadLocalRuntimeErrorType = easyTest::RTErrorType::INT_OVERFLOW;
			break;

		case EXCEPTION_FLT_DENORMAL_OPERAND: //SIGFPE -> undefined
		case EXCEPTION_FLT_STACK_CHECK:
			s_threadLocalRuntimeErrorType = easyTest::RTErrorType::FLOAT_ERROR;
			break;

		case EXCEPTION_FLT_DIVIDE_BY_ZERO: //SIGFPE -> FPE_FLTDIV
			s_threadLocalRuntimeErrorType = easyTest::RTErrorType::FLOAT_DIV_BY_ZERO;
			break;

		case EXCEPTION_FLT_OVERFLOW: //SIGFPE -> FPE_FLTOVF
			s_threadLocalRuntimeErrorType = easyTest::RTErrorType::FLOAT_OVERFLOW;
			break;

		case EXCEPTION_FLT_UNDERFLOW: //SIGFPE -> FPE_UND
			s_threadLocalRuntimeErrorType = easyTest::RTErrorType::FLOAT_UNDERFLOW;
			break;

		case EXCEPTION_FLT_INEXACT_RESULT: //SIGFPE -> FPE_FLTRES
			s_threadLocalRuntimeErrorType = easyTest::RTErrorType::FLOAT_BAD_RESULT;
			break;

		case EXCEPTION_FLT_INVALID_OPERATION: //SIGFPE -> FPE_FLTINV
			s_threadLocalRuntimeErrorType = easyTest::RTErrorType::FLOAT_INVALID_OP;
			break;

		case EXCEPTION_ARRAY_BOUNDS_EXCEEDED: //SIGFPE -> FPE_FLTSUB
			s_threadLocalRuntimeErrorType = easyTest::RTErrorType::ARRAY_OUT_OF_BOUNDS;
			break;

		default: //Breakpoints, trace and unrecoverable errors are not intercepted
			return EXCEPTION_CONTINUE_SEARCH;
		}

		siglongjmp(s_threadLocaljumpBuffer, 1);
		return EXCEPTION_CONTINUE_EXECUTION;
	}
#endif //__linux__
}

namespace easyTest
{
	bool RTErrorProtector::s_hasProtectors = false;

	bool RTErrorProtector::initProtectors()
	{
		if (s_hasProtectors)
			return true;

#ifdef __linux__
		for (size_t i = 0; i < s_nbSignalInterceptors; ++i)
		{
			if (!s_signalInterceptorList[i].initInterception())
			{
				for (size_t j = 0; j < i; ++j)
					s_signalInterceptorList[j].shutInterception();

				return false;
			}
		}

#elif defined(_WIN32)
		s_defaultExceptionHandler = SetUnhandledExceptionFilter(exceptionHandler);
#endif //__linux__

		s_defaultTerminateHandler = std::set_terminate(&terminateHandler);

		s_hasProtectors = true;
		return true;
	}

	void RTErrorProtector::shutProtectors()
	{
		if (s_hasProtectors)
		{
			std::set_terminate(s_defaultTerminateHandler);
			s_defaultTerminateHandler = nullptr;

#ifdef __linux__
			for (size_t i = 0; i < s_nbSignalInterceptors; ++i)
				s_signalInterceptorList[i].shutInterception();

#elif defined(_WIN32)
			SetUnhandledExceptionFilter(s_defaultExceptionHandler);
			s_defaultExceptionHandler = nullptr;
#endif //__linux__

			s_hasProtectors = false;
		}
	}

	RTErrorProtector::~RTErrorProtector()
	{
		deleteProtectedTestSuite();
	}

	TestSuite* RTErrorProtector::createProtectedTestSuite(const TestSuiteRegistrar* pRegistrar)
	{
		//There is no race access to s_hasProtectors flag because
		//initProtectors() is always called before worker-threads creation and
		//shutProtectors() is always called after all worker-threads have
		//terminated.
		if (!s_hasProtectors || !pRegistrar || m_pTestSuite)
		{
			if (m_pTestListener)
			{
				TestEvent event;
				event.type = TestEventType::TESTSUITE_ERROR;
				event.testSuiteError.workerThreadIdx = m_workerThreadIdx;

				if (pRegistrar)
				{
					event.testSuiteError.testSuiteName = pRegistrar->getTestSuiteName();
					event.testSuiteError.nbTestCases = pRegistrar->getTestCaseCount();
				}
				else
				{
					event.testSuiteError.testSuiteName = "";
					event.testSuiteError.nbTestCases = 0;
				}

				m_pTestListener->onEvent(event);
			}

			return nullptr;
		}

		if (sigsetjmp(s_threadLocaljumpBuffer, 1))
		{
			fireRuntimeError(s_threadLocalRuntimeErrorType, nullptr);
			assert(!m_pTestSuite);
		}
		else
		{
#ifdef HAS_EXCEPTIONS
			try
			{
				m_pTestSuite = pRegistrar->createTestSuite(m_pTestListener, m_workerThreadIdx);
			}
			catch (const std::exception& e)
			{
				fireRuntimeError(RTErrorType::CPP_EXCEPTION, e.what());
			}
			catch (...)
			{
				fireRuntimeError(RTErrorType::UNDEFINED_CPP_EXCEPTION, nullptr);
			}
#else
			m_pTestSuite = pRegistrar->createTestSuite(m_pTestListener, m_workerThreadIdx);
#endif //HAS_EXCEPTIONS
		}

		if (!m_pTestSuite)
		{
			if (m_pTestListener)
			{
				TestEvent event;
				event.type = TestEventType::TESTSUITE_ERROR;
				event.testSuiteError.workerThreadIdx = m_workerThreadIdx;
				event.testSuiteError.testSuiteName = pRegistrar->getTestSuiteName();
				event.testSuiteError.nbTestCases = pRegistrar->getTestCaseCount();
				m_pTestListener->onEvent(event);
			}

			return nullptr;
		}

		m_pRegistrar = pRegistrar;
		return m_pTestSuite;
	}

	bool RTErrorProtector::executeProtectedTestCase(const TestCaseRegistrar* pCaseReg)
	{
		bool bRet = false;
		if (m_pTestSuite && pCaseReg)
		{
			//Setup fixture
			if (sigsetjmp(s_threadLocaljumpBuffer, 1))
			{
				fireRuntimeError(s_threadLocalRuntimeErrorType, nullptr);
				return false;
			}
			else
			{
#ifdef HAS_EXCEPTIONS
				try
				{
					m_pTestSuite->setupFixture();
				}
				catch (const std::exception& e)
				{
					fireRuntimeError(RTErrorType::CPP_EXCEPTION, e.what());
					return false;
				}
				catch (...)
				{
					fireRuntimeError(RTErrorType::UNDEFINED_CPP_EXCEPTION, nullptr);
					return false;
				}
#else
				m_pTestSuite->setupFixture();
#endif //HAS_EXCEPTIONS
			}

			//Execute test case
			if (sigsetjmp(s_threadLocaljumpBuffer, 1))
			{
				fireRuntimeError(s_threadLocalRuntimeErrorType, nullptr);
				assert(!bRet);
			}
			else
			{
#ifdef HAS_EXCEPTIONS
				try
				{
					if (pCaseReg->executeTest(m_pTestSuite))
						bRet = true;
				}
				catch (const std::exception& e)
				{
					fireRuntimeError(RTErrorType::CPP_EXCEPTION, e.what());
				}
				catch (...)
				{
					fireRuntimeError(RTErrorType::UNDEFINED_CPP_EXCEPTION, nullptr);
				}
#else
				if (pCaseReg->executeTest(m_pTestSuite))
					bRet = true;
#endif //HAS_EXCEPTIONS
			}

			//Teardown fixture
			if (sigsetjmp(s_threadLocaljumpBuffer, 1))
			{
				fireRuntimeError(s_threadLocalRuntimeErrorType, nullptr);
				return false;
			}
			else
			{
#ifdef HAS_EXCEPTIONS
				try
				{
					m_pTestSuite->teardownFixture();
				}
				catch (const std::exception& e)
				{
					fireRuntimeError(RTErrorType::CPP_EXCEPTION, e.what());
					return false;
				}
				catch (...)
				{
					fireRuntimeError(RTErrorType::UNDEFINED_CPP_EXCEPTION, nullptr);
					return false;
				}
#else
				m_pTestSuite->teardownFixture();
#endif //HAS_EXCEPTIONS
			}
		}

		return bRet;
	}

	void RTErrorProtector::deleteProtectedTestSuite()
	{
		if (m_pTestSuite)
		{
			assert(m_pRegistrar);

			if (sigsetjmp(s_threadLocaljumpBuffer, 1))
				fireRuntimeError(s_threadLocalRuntimeErrorType, nullptr);
			else
			{
#ifdef HAS_EXCEPTIONS
				try
				{
					m_pRegistrar->deleteTestSuite(m_pTestSuite);
				}
				catch (const std::exception& e)
				{
					fireRuntimeError(RTErrorType::CPP_EXCEPTION, e.what());
				}
				catch (...)
				{
					fireRuntimeError(RTErrorType::UNDEFINED_CPP_EXCEPTION, nullptr);
				}
#else
				m_pRegistrar->deleteTestSuite(m_pTestSuite);
#endif //HAS_EXCEPTIONS
			}

			m_pTestSuite = nullptr;
			m_pRegistrar = nullptr;
		}
	}

	void RTErrorProtector::fireRuntimeError(RTErrorType type, const char* exceptionDetails) const
	{
		if (m_pTestListener)
		{
			if ((type == RTErrorType::CPP_EXCEPTION) && !exceptionDetails)
				exceptionDetails = "";

			m_pTestListener->onRuntimeError(m_workerThreadIdx, type, exceptionDetails);
		}
	}
}
