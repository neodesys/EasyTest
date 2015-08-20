/**
 * EasyTest++
 *
 * Copyright (c) 2015, Loïc Le Page
 *
 * This project is released under the MIT license.
 * See: http://opensource.org/licenses/MIT
 */

#include <cstdlib>
#include <csignal>
#include <climits>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#ifndef strcasecmp
#define strcasecmp _stricmp
#endif //strcasecmp
#endif //_WIN32

#include "../include/TestSuiteRegistrar.h"
#include "TestRunner.h"
#include "output/LOGWriter.h"
#include "output/JSWriter.h"
#include "output/TAPWriter.h"
#include "i18n/i18n.h"

namespace
{
	easyTest::TestRunner s_runner;
	volatile std::sig_atomic_t s_signalQuit = 0;

#ifdef __linux__
	void signalInterceptorHandler(int)
	{
		s_signalQuit = 1;
		s_runner.stopTestSuites();
	}

	void interceptProgramTermination()
	{
		struct sigaction signalHandler = {};
		signalHandler.sa_handler = &signalInterceptorHandler;

		sigaction(SIGHUP, &signalHandler, nullptr);
		sigaction(SIGINT, &signalHandler, nullptr);
		sigaction(SIGQUIT, &signalHandler, nullptr);
		sigaction(SIGTERM, &signalHandler, nullptr);
	}

#elif defined(_WIN32)
	BOOL WINAPI signalInterceptorHandler(DWORD)
	{
		s_signalQuit = 1;
		s_runner.stopTestSuites();
		return TRUE;
	}

	void interceptProgramTermination()
	{
		SetConsoleCtrlHandler(&signalInterceptorHandler, TRUE);
	}

#else
#error Unsupported platform: currently supported platforms are\
       Linux and Windows
#endif //__linux__

	class Config final
	{
	public:
		~Config()
		{
			reset();
		}

		static const Config& parseConfig(int argc, char** argv);

		bool isHelpRequested() const
		{
			return m_bHelpRequested;
		}

		bool isListRequested() const
		{
			return m_bListRequested;
		}

		const easyTest::TestSuiteRegistrar** getTestSuiteList() const
		{
			return m_testSuiteList;
		}

		size_t getNBTestSuites() const
		{
			return m_nbTestSuites;
		}

		unsigned int getNBThreads() const
		{
			return m_nbThreads;
		}

		easyTest::ITestListener* getTestListener() const
		{
			return m_pTestListener;
		}

	private:
		Config() = default;
		Config(const Config&) = delete;
		Config& operator=(const Config&) = delete;

		static Config s_config;

		void reset();

		bool m_bHelpRequested = false;
		bool m_bListRequested = false;

		const easyTest::TestSuiteRegistrar** m_testSuiteList = nullptr;
		size_t m_nbTestSuites = 0;
		bool m_bNeedRegistrarsDeletion = false;

		unsigned int m_nbThreads = easyTest::TestRunner::NB_HARDWARE_THREADS;
		easyTest::ITestListener* m_pTestListener = nullptr;

		std::FILE* m_pOutputFile = nullptr;
	};

	Config Config::s_config;

	void Config::reset()
	{
		m_bHelpRequested = false;
		m_bListRequested = false;

		if (m_testSuiteList)
		{
			if (m_bNeedRegistrarsDeletion)
				delete[] m_testSuiteList;

			m_testSuiteList = nullptr;
		}

		m_nbTestSuites = 0;
		m_bNeedRegistrarsDeletion = false;
		m_nbThreads = easyTest::TestRunner::NB_HARDWARE_THREADS;

		if (m_pTestListener)
		{
			delete m_pTestListener;
			m_pTestListener = nullptr;
		}

		if (m_pOutputFile)
		{
			std::fclose(m_pOutputFile);
			m_pOutputFile = nullptr;
		}
	}

	const Config& Config::parseConfig(int argc, char** argv)
	{
		s_config.reset();

		bool bVerbose = false;
		bool bStats = false;
		const char* outputPath = nullptr;

		enum struct OutputType : unsigned char
		{
			LOG,
			JS,
			TAP
		} outputType = OutputType::LOG;

		//Fetch options
		const char* const invalidArgs = easyTest::i18n::getString(easyTest::i18n::INVALID_ARGS);
		while (--argc > 0)
		{
			//We jump over first arg which is always the executable
			//path and name
			char* arg = *++argv;
			if (arg[0] != '-')
				break;

			int i = 1;
			bool bNextArgUsed = false;
			while (arg[i] != '\0')
			{
				switch (arg[i++])
				{
				case '-':
					if ((i == 2)
							&& (!strcasecmp(arg + i, "help")
							|| !strcasecmp(arg + i, "list")
							|| !strcasecmp(arg + i, "verbose")
							|| !strcasecmp(arg + i, "stats")
#ifndef EASYTEST_NO_THREADS
							|| !strcasecmp(arg + i, "nthreads")
#endif //!EASYTEST_NO_THREADS
							|| !strcasecmp(arg + i, "out")
							|| !strcasecmp(arg + i, "type")))
						arg[3] = '\0';
					else
					{
						s_config.reset();
						std::fprintf(stderr, "%s", invalidArgs);
						return s_config;
					}
					break;

				case 'h':
				case 'H':
					s_config.reset();
					s_config.m_bHelpRequested = true;
					return s_config;

				case 'l':
				case 'L':
					s_config.reset();
					s_config.m_bListRequested = true;
					return s_config;

				case 'v':
				case 'V':
					bVerbose = true;
					break;

				case 's':
				case 'S':
					bStats = true;
					break;

#ifndef EASYTEST_NO_THREADS
				case 'n':
				case 'N':
					if (bNextArgUsed || (argc <= 1) || (**(argv + 1) == '-'))
					{
						s_config.reset();
						std::fprintf(stderr, "%s", invalidArgs);
						return s_config;
					}
					else
					{
						bNextArgUsed = true;
						argc--;
						const char* str = *++argv;

						if (!strcasecmp(str, "max"))
							s_config.m_nbThreads = easyTest::TestRunner::NB_HARDWARE_THREADS;
						else
						{
							char* pEnd = nullptr;
							long n = std::strtol(str, &pEnd, 10);
							if (!pEnd || (pEnd == str) || (*pEnd != '\0') || (n < 0) || (n == LONG_MAX))
							{
								s_config.reset();
								std::fprintf(stderr, "%s", invalidArgs);
								return s_config;
							}

							s_config.m_nbThreads = n;
						}
					}
					break;
#endif //!EASYTEST_NO_THREADS

				case 'o':
				case 'O':
					if (bNextArgUsed || (argc <= 1) || (**(argv + 1) == '-'))
					{
						s_config.reset();
						std::fprintf(stderr, "%s", invalidArgs);
						return s_config;
					}
					else
					{
						bNextArgUsed = true;
						argc--;
						outputPath = *++argv;
					}
					break;

				case 't':
				case 'T':
					if (bNextArgUsed || (argc <= 1) || (**(argv + 1) == '-'))
					{
						s_config.reset();
						std::fprintf(stderr, "%s", invalidArgs);
						return s_config;
					}
					else
					{
						bNextArgUsed = true;
						argc--;
						const char* str = *++argv;

						if (!strcasecmp(str, "log"))
							outputType = OutputType::LOG;
						else if (!strcasecmp(str, "js"))
							outputType = OutputType::JS;
						else if (!strcasecmp(str, "tap"))
							outputType = OutputType::TAP;
						else
						{
							s_config.reset();
							std::fprintf(stderr, "%s", invalidArgs);
							return s_config;
						}
					}
					break;

				default:
					s_config.reset();
					std::fprintf(stderr, "%s", invalidArgs);
					return s_config;
				}
			}
		}

		//Fetch test suites list
		if (argc > 0)
		{
			s_config.m_testSuiteList = new(std::nothrow) const easyTest::TestSuiteRegistrar*[argc]();
			s_config.m_bNeedRegistrarsDeletion = true;

			if (!s_config.m_testSuiteList)
			{
				s_config.reset();
				std::fprintf(stderr, "%s", easyTest::i18n::getString(easyTest::i18n::OUT_OF_MEMORY));
				return s_config;
			}

			const easyTest::TestSuiteRegistrar** pList = easyTest::TestSuiteRegistrar::getTestSuiteList();
			size_t nbTestSuites = easyTest::TestSuiteRegistrar::getTestSuiteCount();
			while (argc-- > 0)
			{
				const char* arg = *argv++;
				if (arg[0] == '-')
				{
					s_config.reset();
					std::fprintf(stderr, "%s", invalidArgs);
					return s_config;
				}

				for (size_t i = 0; i < nbTestSuites; ++i)
				{
					const easyTest::TestSuiteRegistrar* pReg = pList[i];
					if (!strcasecmp(pReg->getTestSuiteName(), arg))
					{
						s_config.m_testSuiteList[s_config.m_nbTestSuites++] = pReg;
						break;
					}
				}
			}

			if (!s_config.m_nbTestSuites)
			{
				s_config.reset();
				std::fprintf(stderr, "%s", easyTest::i18n::getString(easyTest::i18n::UNKNOWN_TEST_SUITE));
				return s_config;
			}
		}
		else
		{
			s_config.m_testSuiteList = easyTest::TestSuiteRegistrar::getTestSuiteList();
			s_config.m_nbTestSuites = easyTest::TestSuiteRegistrar::getTestSuiteCount();
		}

		//Initialize output
		std::FILE* pOut = stdout;
		if (outputPath)
		{
#if defined(_WIN32) && defined(_MSC_VER) //Visual C++ compiler
			pOut = nullptr;
			fopen_s(&pOut, outputPath, "wt");
#else
			pOut = std::fopen(outputPath, "wt");
#endif //_WIN32 && _MSC_VER

			if (!pOut)
			{
				s_config.reset();
				std::fprintf(stderr, "%s", easyTest::i18n::getString(easyTest::i18n::CANNOT_WRITE_FILE));
				return s_config;
			}

			s_config.m_pOutputFile = pOut;
		}

		switch (outputType)
		{
		case OutputType::LOG:
			s_config.m_pTestListener = new(std::nothrow) easyTest::output::LOGWriter(pOut, bStats, bVerbose);
			break;

		case OutputType::JS:
			s_config.m_pTestListener = new(std::nothrow) easyTest::output::JSWriter(pOut, bStats, bVerbose);
			break;

		case OutputType::TAP:
			s_config.m_pTestListener = new(std::nothrow) easyTest::output::TAPWriter(pOut, bStats, bVerbose);
			break;
		}

		if (!s_config.m_pTestListener)
		{
			s_config.reset();
			std::fprintf(stderr, "%s", easyTest::i18n::getString(easyTest::i18n::OUT_OF_MEMORY));
			return s_config;
		}

		return s_config;
	}

	void displayList()
	{
		const easyTest::TestSuiteRegistrar** pList = easyTest::TestSuiteRegistrar::getTestSuiteList();
		size_t nbTestSuites = easyTest::TestSuiteRegistrar::getTestSuiteCount();

		for (size_t i = 0; i < nbTestSuites; ++i)
			std::printf("%s\n", pList[i]->getTestSuiteName());
	}
}

int main(int argc, char* argv[])
{
	easyTest::i18n::setLangFromLocale();
	interceptProgramTermination();
	const Config& conf = Config::parseConfig(argc, argv);

	if (conf.isHelpRequested())
	{
		std::printf("%s", easyTest::i18n::getString(easyTest::i18n::RUNNER_HELP));
		return 0;
	}
	else if (conf.isListRequested())
	{
		displayList();
		return 0;
	}

	if (!s_runner.startTestSuites(conf.getTestSuiteList(), conf.getNBTestSuites(), conf.getNBThreads(), conf.getTestListener()))
	{
		std::fprintf(stderr, "%s", easyTest::i18n::getString(easyTest::i18n::CANNOT_START_RUNNER));
		return -1;
	}

	if (s_signalQuit)
		s_runner.stopTestSuites();

	return static_cast<int>(s_runner.waitTestsTermination());
}
