/**
 * EasyTest++
 *
 * Copyright (c) 2015, Loïc Le Page
 *
 * This project is released under the MIT license.
 * See: http://opensource.org/licenses/MIT
 */

#include "i18n.h"

#include <cstring>

#include <locale.h>

#ifdef _WIN32
#define LC_MESSAGES LC_ALL
#endif //_WIN32

namespace
{
	struct Lang
	{
		const char* code;
		const char* strList[easyTest::i18n::NB_STRINGS];
	};

	const Lang s_langList[] =
	{
		{"EN", {
#include "EN.lang"
		}}
	};

	const size_t s_nbLangs = sizeof(s_langList) / sizeof(s_langList[0]);

	const Lang* s_pSelectedLang = s_langList;
}

namespace easyTest
{
	namespace i18n
	{
		const char* getString(StringId id)
		{
			const char* ret = "";
			if (id < NB_STRINGS)
			{
				ret = s_pSelectedLang->strList[id];
				if (!ret)
				{
					ret = s_langList->strList[id];
					if (!ret)
						return "";
				}
			}

			return ret;
		}

		const char* const* getSequence(StringId id, unsigned short nbStrings)
		{
			if (id + nbStrings <= NB_STRINGS)
				return s_pSelectedLang->strList + id;
			else
				return nullptr;
		}

		const char* getLang()
		{
			return s_pSelectedLang->code;
		}

		bool setLang(const char* lang)
		{
			if (lang && (lang[0] != '\0') && (lang[1] != '\0'))
			{
				char buffer[3] = {};
				for (int i = 0; i < 2; ++i)
				{
					char c = lang[i];
					if ((c >= 'A') && (c <= 'Z'))
						buffer[i] = c;
					else if ((c >= 'a') && (c <= 'z'))
						buffer[i] = 'A' + c - 'a';
					else
						return false;
				}

				for (size_t i = 0; i < s_nbLangs; ++i)
				{
					if (!std::strcmp(s_langList[i].code, buffer))
					{
						s_pSelectedLang = s_langList + i;
						return true;
					}
				}
			}

			return false;
		}

		bool setLangFromLocale()
		{
			const char* locale = setlocale(LC_MESSAGES, "");
			return setLang(locale);
		}

		void resetLang()
		{
			s_pSelectedLang = s_langList;
		}
	}
}
