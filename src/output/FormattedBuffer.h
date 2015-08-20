/**
 * EasyTest++
 *
 * Copyright (c) 2015, Loïc Le Page
 *
 * This project is released under the MIT license.
 * See: http://opensource.org/licenses/MIT
 */

#ifndef _FORMATTEDBUFFER_H_
#define _FORMATTEDBUFFER_H_

#include <cstdio>
#include <cstdarg>

namespace easyTest
{
	namespace output
	{
		class FormattedBuffer final
		{
		public:
			FormattedBuffer() = default;
			~FormattedBuffer();

			FormattedBuffer& append(char c);
			FormattedBuffer& append(const char* str);

			FormattedBuffer& appendFormat(const char* format, ...);
			FormattedBuffer& appendFormat(const char* format, std::va_list& varArgs);

			FormattedBuffer& appendJSString(const char* str);
			FormattedBuffer& appendFormatJSString(const char* format, ...);
			FormattedBuffer& appendFormatJSString(const char* format, std::va_list& varArgs);

			//units must be an array of 5 strings representing time units in
			//this order: {min, sec, ms, us, ns}.
			//t must be in ns.
			FormattedBuffer& formatTime(long long t, const char* const units[5], bool bInsert);

			void setInsertMark()
			{
				m_insertMark = m_strLen;
			}

			FormattedBuffer& insert(char c);
			FormattedBuffer& insert(const char* str);

			FormattedBuffer& insertFormat(const char* format, ...);
			FormattedBuffer& insertFormat(const char* format, std::va_list& varArgs);

			//Flush into pOut file is guaranteed to be atomic: two different
			//threads cannot flush to pOut at the same time.
			void flushAndClear(std::FILE* pOut);

		private:
			FormattedBuffer(const FormattedBuffer&) = delete;
			FormattedBuffer& operator=(const FormattedBuffer&) = delete;

			bool ensureCapacity(size_t nbExtraChars);

			size_t m_strCapacity = 0;
			size_t m_strLen = 0;
			size_t m_insertMark = 0;
			char* m_pBuffer = nullptr;
		};
	}
}

#endif //_FORMATTEDBUFFER_H_
