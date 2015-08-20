/**
 * EasyTest++
 *
 * Copyright (c) 2015, Loïc Le Page
 *
 * This project is released under the MIT license.
 * See: http://opensource.org/licenses/MIT
 */

#include "TestWriter.h"

#include <new>

#include "../../include/TestSuite.h"
#include "FormattedBuffer.h"
#include "../i18n/i18n.h"

namespace easyTest
{
	namespace output
	{
		TestWriter::~TestWriter()
		{
			m_nbBuffers = 0;

			if (m_bufferList)
			{
				delete[] m_bufferList;
				m_bufferList = nullptr;
			}
		}

		void TestWriter::onEvent(const TestEvent& event)
		{
			switch (event.type)
			{
			case TestEventType::TESTRUNNER_START:
				//We are in the runner control thread BEFORE worker-threads
				//creation
				if (m_bufferList)
					delete[] m_bufferList;

				//Last buffer is for the runner control thread (main)
				m_nbBuffers = event.testRunnerStart.nbMaxWorkerThreads + 1;
				m_bufferList = new(std::nothrow) FormattedBuffer[m_nbBuffers];
				if (!m_bufferList)
				{
					m_nbBuffers = 0;
					std::fprintf(stderr, "%s", i18n::getString(i18n::OUT_OF_MEMORY));
				}
				break;

			case TestEventType::TESTRUNNER_FINISH:
				//We are in the runner control thread AFTER all worker-threads
				//termination
				m_nbBuffers = 0;

				if (m_bufferList)
				{
					delete[] m_bufferList;
					m_bufferList = nullptr;
				}
				break;

			default:
				break;
			}
		}

		FormattedBuffer* TestWriter::getControlThreadBuffer() const
		{
			if (m_bufferList)
				return m_bufferList + m_nbBuffers - 1;
			else
				return nullptr;
		}

		FormattedBuffer* TestWriter::getWorkerThreadBuffer(unsigned int workerThreadIdx) const
		{
			if (m_bufferList && (workerThreadIdx < m_nbBuffers))
				return m_bufferList + workerThreadIdx;
			else
				return nullptr;
		}

		const char* TestWriter::getUnaryAssertFailText(AssertType type) const
		{
			switch (type)
			{
			case AssertType::IS_TRUE:
				return i18n::getString(i18n::ASSERT_FAIL_TRUE);

			case AssertType::IS_FALSE:
				return i18n::getString(i18n::ASSERT_FAIL_FALSE);

			case AssertType::IS_NAN:
				return i18n::getString(i18n::ASSERT_FAIL_NAN);

			case AssertType::IS_INFINITE:
				return i18n::getString(i18n::ASSERT_FAIL_INFINITE);

			case AssertType::IS_FINITE:
				return i18n::getString(i18n::ASSERT_FAIL_FINITE);

			default:
				return i18n::getString(i18n::ASSERT_FAIL_UNARY);
			}
		}

		const char* TestWriter::getBinaryAssertFailText(AssertType type) const
		{
			switch (type)
			{
			case AssertType::IS_LESS_THAN:
				return i18n::getString(i18n::ASSERT_FAIL_LT);

			case AssertType::IS_LESS_OR_EQUAL:
				return i18n::getString(i18n::ASSERT_FAIL_LE);

			case AssertType::IS_GREATER_THAN:
				return i18n::getString(i18n::ASSERT_FAIL_GT);

			case AssertType::IS_GREATER_OR_EQUAL:
				return i18n::getString(i18n::ASSERT_FAIL_GE);

			case AssertType::ARE_EQUAL:
			case AssertType::ARE_STRING_EQUAL:
				return i18n::getString(i18n::ASSERT_FAIL_EQUAL);

			case AssertType::ARE_DIFFERENT:
			case AssertType::ARE_STRING_DIFFERENT:
				return i18n::getString(i18n::ASSERT_FAIL_DIFF);

			case AssertType::ARE_ALMOST_EQUAL:
				return i18n::getString(i18n::ASSERT_FAIL_ALMOST_EQ);

			case AssertType::ARE_VERY_DIFFERENT:
				return i18n::getString(i18n::ASSERT_FAIL_VERY_DIFF);

			case AssertType::ARE_BITWISE_EQUAL:
				return i18n::getString(i18n::ASSERT_FAIL_BIT_EQUAL);

			case AssertType::ARE_BITWISE_DIFFERENT:
				return i18n::getString(i18n::ASSERT_FAIL_BIT_DIFF);

			case AssertType::HAVE_SAME_DATA:
				return i18n::getString(i18n::ASSERT_FAIL_SAME_DATA);

			case AssertType::HAVE_DIFFERENT_DATA:
				return i18n::getString(i18n::ASSERT_FAIL_DIFF_DATA);

			default:
				return i18n::getString(i18n::ASSERT_FAIL_BINARY);
			}
		}

		const char* TestWriter::getRuntimeErrorText(RTErrorType type) const
		{
			switch (type)
			{
			case RTErrorType::UNDEFINED_RTERROR:
				return i18n::getString(i18n::RTERROR_UNDEFINED);

			case RTErrorType::UNDEFINED_CPP_EXCEPTION:
				return i18n::getString(i18n::RTERROR_UNDEF_CPP_EXCEPT);

			case RTErrorType::CPP_EXCEPTION:
				return i18n::getString(i18n::RTERROR_CPP_EXCEPT);

			case RTErrorType::MEM_NOT_MAPPED:
				return i18n::getString(i18n::RTERROR_MEM_NOT_MAPPED);

			case RTErrorType::MEM_ACCESS_VIOLATION:
				return i18n::getString(i18n::RTERROR_MEM_ACCESS_VIOL);

			case RTErrorType::MEM_BUS_ERROR:
				return i18n::getString(i18n::RTERROR_MEM_BUS_ERROR);

			case RTErrorType::MEM_MISALIGNMENT:
				return i18n::getString(i18n::RTERROR_MEM_MISALIGNMENT);

			case RTErrorType::INSTR_ERROR:
				return i18n::getString(i18n::RTERROR_INSTR_ERROR);

			case RTErrorType::INSTR_ILLEGAL:
				return i18n::getString(i18n::RTERROR_INSTR_ILLEGAL);

			case RTErrorType::INSTR_PRIVILEDGED:
				return i18n::getString(i18n::RTERROR_INSTR_PRIV);

			case RTErrorType::INT_DIV_BY_ZERO:
				return i18n::getString(i18n::RTERROR_INT_DIV_BY_ZERO);

			case RTErrorType::INT_OVERFLOW:
				return i18n::getString(i18n::RTERROR_INT_OVERFLOW);

			case RTErrorType::FLOAT_ERROR:
				return i18n::getString(i18n::RTERROR_FLOAT_ERROR);

			case RTErrorType::FLOAT_DIV_BY_ZERO:
				return i18n::getString(i18n::RTERROR_FLOAT_DIV_BY_ZERO);

			case RTErrorType::FLOAT_OVERFLOW:
				return i18n::getString(i18n::RTERROR_FLOAT_OVERFLOW);

			case RTErrorType::FLOAT_UNDERFLOW:
				return i18n::getString(i18n::RTERROR_FLOAT_UNDERFLOW);

			case RTErrorType::FLOAT_BAD_RESULT:
				return i18n::getString(i18n::RTERROR_FLOAT_BAD_RESULT);

			case RTErrorType::FLOAT_INVALID_OP:
				return i18n::getString(i18n::RTERROR_FLOAT_INVALID_OP);

			case RTErrorType::ARRAY_OUT_OF_BOUNDS:
				return i18n::getString(i18n::RTERROR_ARRAY_OOB);

			case RTErrorType::SYSTEM_ERROR:
				return i18n::getString(i18n::RTERROR_SYSTEM_ERROR);

			case RTErrorType::BROKEN_PIPE:
				return i18n::getString(i18n::RTERROR_BROKEN_PIPE);

			case RTErrorType::FILE_SIZE_OVER_LIMIT:
				return i18n::getString(i18n::RTERROR_FILE_OVER_LIMIT);

			default:
				return i18n::getString(i18n::RTERROR_UNDEFINED);
			}
		}
	}
}
