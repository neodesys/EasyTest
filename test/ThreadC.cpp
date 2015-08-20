/**
 * EasyTest++
 *
 * Copyright (c) 2015, Loïc Le Page
 *
 * This project is released under the MIT license.
 * See: http://opensource.org/licenses/MIT
 */

#include "../include/easyTest.h"

#include <cstdlib>
#include <ctime>

#ifdef __linux__
#include <unistd.h>
#elif defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#error Unsupported platform: currently supported platforms are\
       Linux and Windows
#endif //__linux__

TEST_SUITE(ThreadC)
{
public:
	ThreadC()
	{
		std::srand(static_cast<unsigned int>(std::time(nullptr)));
	}

	TEST_CASE(mediumTest);
	TEST_CASE(longTest);
	TEST_CASE(fastTest);

private:
	unsigned int waitAtMax(unsigned int ms) const;
};

TEST_IMPL(ThreadC, mediumTest)
{
	TRACE_FORMAT("Waiting %u ms...", waitAtMax(2000));
	return true;
}

TEST_IMPL(ThreadC, longTest)
{
	TRACE_FORMAT("Waiting %u ms...", waitAtMax(2500));
	return true;
}

TEST_IMPL(ThreadC, fastTest)
{
	TRACE_FORMAT("Waiting %u ms...", waitAtMax(1500));
	return true;
}

unsigned int ThreadC::waitAtMax(unsigned int ms) const
{
	unsigned int u = ms >> 1;
	u += static_cast<unsigned int>(u * static_cast<float>(std::rand()) / RAND_MAX);

#ifdef __linux__
	usleep(1000 * u);
#else
	Sleep(u);
#endif //__linux__

	return u;
}
