/**
 * EasyTest++
 *
 * Copyright (c) 2015, Loïc Le Page
 *
 * This project is released under the MIT license.
 * See: http://opensource.org/licenses/MIT
 */

#include "FormattedBuffer.h"

#include <cstdlib>
#include <cstring>
#include <cassert>

namespace easyTest
{
	namespace output
	{
		FormattedBuffer::~FormattedBuffer()
		{
			if (m_pBuffer)
			{
				std::free(m_pBuffer);
				m_pBuffer = nullptr;
			}

			m_strCapacity = 0;
			m_strLen = 0;
			m_insertMark = 0;
		}

		FormattedBuffer& FormattedBuffer::append(char c)
		{
			if (ensureCapacity(1))
			{
				assert(m_pBuffer);
				m_pBuffer[m_strLen++] = c;
				m_pBuffer[m_strLen] = '\0';
			}

			return *this;
		}

		FormattedBuffer& FormattedBuffer::append(const char* str)
		{
			if (str && (str[0] != '\0'))
			{
				size_t nbChars = std::strlen(str);
				if (ensureCapacity(nbChars))
				{
					assert(m_pBuffer);
					std::memcpy(m_pBuffer + m_strLen, str, nbChars + 1);
					m_strLen += nbChars;
				}
			}

			return *this;
		}

		FormattedBuffer& FormattedBuffer::appendFormat(const char* format, ...)
		{
			if (format && (format[0] != '\0'))
			{
				std::va_list varArgs;
				va_start(varArgs, format);
				appendFormat(format, varArgs);
				va_end(varArgs);
			}

			return *this;
		}

		FormattedBuffer& FormattedBuffer::appendFormat(const char* format, std::va_list& varArgs)
		{
			assert(format);

			if (ensureCapacity(1))
			{
				assert(m_pBuffer);

				std::va_list varCopy;
				va_copy(varCopy, varArgs);

				assert(m_strCapacity >= m_strLen);
				size_t maxBytesRemaining = m_strCapacity + 1 - m_strLen;
				int nbChars = std::vsnprintf(m_pBuffer + m_strLen, maxBytesRemaining, format, varArgs);
				if (nbChars >= static_cast<int>(maxBytesRemaining))
				{
					//Output has been truncated
					if (!ensureCapacity(nbChars))
					{
						va_end(varCopy);
						return *this;
					}

					maxBytesRemaining = m_strCapacity + 1 - m_strLen;
					nbChars = std::vsnprintf(m_pBuffer + m_strLen, maxBytesRemaining, format, varCopy);
				}

				va_end(varCopy);

				if (nbChars > 0)
					m_strLen += nbChars;
			}

			return *this;
		}

		FormattedBuffer& FormattedBuffer::appendJSString(const char* str)
		{
			if (str)
			{
				size_t nbChars = 0;
				for (const char* p = str; *p != '\0'; ++p)
				{
					switch (*p)
					{
					case '"':
					case '\\':
					case '\b':
					case '\f':
					case '\n':
					case '\r':
					case '\t':
						nbChars += 2;
						break;

					default:
						nbChars++;
						break;
					}
				}

				if (nbChars && ensureCapacity(nbChars))
				{
					assert(m_pBuffer);
					char* q = m_pBuffer + m_strLen;
					for (const char* p = str; *p != '\0'; ++p)
					{
						switch (*p)
						{
						case '"':
						case '\\':
							*q++ = '\\';
							*q++ = *p;
							break;

						case '\b':
							*q++ = '\\';
							*q++ = 'b';
							break;

						case '\f':
							*q++ = '\\';
							*q++ = 'f';
							break;

						case '\n':
							*q++ = '\\';
							*q++ = 'n';
							break;

						case '\r':
							*q++ = '\\';
							*q++ = 'r';
							break;

						case '\t':
							*q++ = '\\';
							*q++ = 't';
							break;

						default:
							*q++ = *p;
							break;
						}
					}

					m_strLen += nbChars;
					m_pBuffer[m_strLen] = '\0';
				}
			}

			return *this;
		}

		FormattedBuffer& FormattedBuffer::appendFormatJSString(const char* format, ...)
		{
			if (format && (format[0] != '\0'))
			{
				std::va_list varArgs;
				va_start(varArgs, format);
				appendFormatJSString(format, varArgs);
				va_end(varArgs);
			}

			return *this;
		}

		FormattedBuffer& FormattedBuffer::appendFormatJSString(const char* format, std::va_list& varArgs)
		{
			const size_t strLen = m_strLen;
			appendFormat(format, varArgs);
			if (m_strLen > strLen)
			{
				assert(m_pBuffer);
				char* const pStart = m_pBuffer + strLen;

				size_t nbExtraChars = 0;
				for (const char* p = pStart; *p != '\0'; ++p)
				{
					switch (*p)
					{
					case '"':
					case '\\':
					case '\b':
					case '\f':
					case '\n':
					case '\r':
					case '\t':
						nbExtraChars++;
						break;

					default:
						break;
					}
				}

				if (!nbExtraChars)
					return *this;

				if (!ensureCapacity(nbExtraChars))
				{
					*pStart = '\0';
					m_strLen = strLen;
					return *this;
				}

				char* q = m_pBuffer + m_strLen + nbExtraChars;
				*q-- = '\0';
				for (const char* p = m_pBuffer + m_strLen - 1; p >= pStart; --p)
				{
					switch (*p)
					{
					case '"':
					case '\\':
						*q-- = *p;
						*q-- = '\\';
						break;

					case '\b':
						*q-- = 'b';
						*q-- = '\\';
						break;

					case '\f':
						*q-- = 'f';
						*q-- = '\\';
						break;

					case '\n':
						*q-- = 'n';
						*q-- = '\\';
						break;

					case '\r':
						*q-- = 'r';
						*q-- = '\\';
						break;

					case '\t':
						*q-- = 't';
						*q-- = '\\';
						break;

					default:
						*q-- = *p;
						break;
					}
				}

				assert(q + 1 == pStart);
				m_strLen += nbExtraChars;
			}

			return *this;
		}

		FormattedBuffer& FormattedBuffer::formatTime(long long t, const char* const units[5], bool bInsert)
		{
			if (units)
			{
				if (t >= 60000000000)
				{
					//at least 1 min
					long min = static_cast<long>(t / 60000000000);
					t -= min * 60000000000;
					double sec = static_cast<double>(t) / 1000000000.0;
					if (bInsert)
						insertFormat("%ld%s%0.3f%s", min, sec, units[0], units[1]);
					else
						appendFormat("%ld%s%0.3f%s", min, sec, units[0], units[1]);
				}
				else if (t >= 1000000000)
				{
					//at least 1 sec
					double sec = static_cast<double>(t) / 1000000000.0;
					if (bInsert)
						insertFormat("%0.3f%s", sec, units[1]);
					else
						appendFormat("%0.3f%s", sec, units[1]);
				}
				else if (t >= 1000000)
				{
					//at least 1 ms
					double ms = static_cast<double>(t) / 1000000.0;
					if (bInsert)
						insertFormat("%0.3f%s", ms, units[2]);
					else
						appendFormat("%0.3f%s", ms, units[2]);
				}
				else if (t >= 1000)
				{
					//at least 1 us
					double us = static_cast<double>(t) / 1000.0;
					if (bInsert)
						insertFormat("%0.3f%s", us, units[3]);
					else
						appendFormat("%0.3f%s", us, units[3]);
				}
				else
				{
					//only ns
					if (bInsert)
						insertFormat("%lld%s", t, units[4]);
					else
						appendFormat("%lld%s", t, units[4]);
				}
			}

			return *this;
		}

		FormattedBuffer& FormattedBuffer::insert(char c)
		{
			assert(m_insertMark <= m_strLen);

			if (ensureCapacity(1))
			{
				assert(m_pBuffer);
				char* pInsertPoint = m_pBuffer + m_insertMark;

				std::memmove(pInsertPoint + 1, pInsertPoint, m_strLen + 1 - m_insertMark);
				*pInsertPoint = c;

				m_strLen++;
				m_insertMark++;
			}

			return *this;
		}

		FormattedBuffer& FormattedBuffer::insert(const char* str)
		{
			assert(m_insertMark <= m_strLen);

			if (str && (str[0] != '\0'))
			{
				size_t nbChars = std::strlen(str);
				if (ensureCapacity(nbChars))
				{
					assert(m_pBuffer);
					char* pInsertPoint = m_pBuffer + m_insertMark;

					std::memmove(pInsertPoint + nbChars, pInsertPoint, m_strLen + 1 - m_insertMark);
					std::memcpy(pInsertPoint, str, nbChars);

					m_strLen += nbChars;
					m_insertMark += nbChars;
				}
			}

			return *this;
		}

		FormattedBuffer& FormattedBuffer::insertFormat(const char* format, ...)
		{
			if (format && (format[0] != '\0'))
			{
				std::va_list varArgs;
				va_start(varArgs, format);
				insertFormat(format, varArgs);
				va_end(varArgs);
			}

			return *this;
		}

		FormattedBuffer& FormattedBuffer::insertFormat(const char* format, std::va_list& varArgs)
		{
			assert(m_insertMark <= m_strLen);
			assert(format);

			std::va_list varCopy;
			va_copy(varCopy, varArgs);

			int nbChars = std::vsnprintf(nullptr, 0, format, varArgs);
			if ((nbChars > 0) && ensureCapacity(nbChars))
			{
				assert(m_pBuffer);
				char* pInsertPoint = m_pBuffer + m_insertMark;
				const size_t nbBytesToMove = m_strLen + 1 - m_insertMark; //includes ending null-character

				std::memmove(pInsertPoint + nbChars, pInsertPoint, nbBytesToMove);
				char c = pInsertPoint[nbChars];

				if (std::vsnprintf(pInsertPoint, nbChars + 1, format, varCopy) != nbChars)
				{
					pInsertPoint[nbChars] = c;
					std::memmove(pInsertPoint, pInsertPoint + nbChars, nbBytesToMove);
					va_end(varCopy);
					return *this;
				}

				pInsertPoint[nbChars] = c;
				m_strLen += nbChars;
				m_insertMark += nbChars;
			}

			va_end(varCopy);
			return *this;
		}

		void FormattedBuffer::flushAndClear(std::FILE* pOut)
		{
			if (m_strLen && pOut)
			{
				assert(m_pBuffer);

				//fwrite is always thread-safe as it always locks the current
				//thread during writing.
				std::fwrite(m_pBuffer, 1, m_strLen, pOut);
				std::fflush(pOut);
			}

			m_strLen = 0;
			m_insertMark = 0;
		}

		bool FormattedBuffer::ensureCapacity(size_t nbExtraChars)
		{
			assert(m_strCapacity >= m_strLen);
			size_t maxBytesRemaining = m_strCapacity + 1 - m_strLen;
			if (nbExtraChars < maxBytesRemaining)
				return true;

			size_t strCaps = m_strCapacity;
			if (!strCaps)
			{
				//Minimum buffer size will be 64 characters
				maxBytesRemaining += 32;
				strCaps = 32;
			}

			do
			{
				maxBytesRemaining += strCaps;
				strCaps <<= 1;
			}
			while (maxBytesRemaining <= nbExtraChars);

			char* pBuff = static_cast<char*>(std::realloc(m_pBuffer, strCaps + 1));
			if (!pBuff)
				return false;

			m_strCapacity = strCaps;
			m_pBuffer = pBuff;
			m_pBuffer[m_strLen] = '\0'; //Ensure an empty string on first allocation

			return true;
		}
	}
}
