EasyTest++
==========

[![License](https://img.shields.io/badge/License-MIT-blue.svg?style=flat)](/LICENSE)
[![Linux Status](https://travis-ci.org/neodesys/EasyTest.svg?branch=master)](https://travis-ci.org/neodesys/EasyTest)
[![Windows Status](https://ci.appveyor.com/api/projects/status/rxfdax4xs28ws6xi/branch/master?svg=true)](https://ci.appveyor.com/project/neodesys/easytest/branch/master)
[![Open Bugs](https://badge.waffle.io/neodesys/easytest.svg?label=bug&title=Open%20Bugs)](https://waffle.io/neodesys/easytest)

EasyTest++ is a lightweight and easy to use C++ unit testing framework.

**Contents**

1. [Key features](#key-features)
2. [Quick start](#quick-start)
3. [Usage](#usage)
4. [How to contribute](#how-to-contribute)
5. [License](#license)
6. [Known issues](#known-issues)
7. [Changelog](#changelog)

--------------------------------------------------------------------------------

Key Features
------------

- lightweight C++11 unit testing framework
- xUnit architecture with assertions and fixtures
- single static library, no external dependencies (except `pthread` under Linux)
- auto-registration of test suites and test cases
- measures execution times of all test cases and test suites
- integrated test runner with parallel execution of test suites
- multiple internationalized output formats: log, TAP, JavaScript and JSON
- easily integrated with custom test harnesses or results formatters in HTML
- neither relies on C++ exceptions nor RTTI
- intercepts uncaught C++ exceptions and runtime errors (floating-point
  exceptions, illegal instructions, divisions by zero, memory access violation,
  etc...)
- available on Linux and Windows for 32 and 64 bits architectures

--------------------------------------------------------------------------------

Quick start
-----------

Download the
[latest release](https://github.com/neodesys/easytest/releases/latest) or clone
this repository.

```
$ git clone https://github.com/neodesys/easytest.git EasyTest++
$ cd EasyTest++
```

### Build the static library

```
$ make PLATFORM=linux ARCH=64 BUILD=release
$ make PLATFORM=linux ARCH=32 BUILD=release
$ make PLATFORM=mingw ARCH=64 BUILD=release
$ make PLATFORM=mingw ARCH=32 BUILD=release
```

### Build and run tests

```
$ make PLATFORM=linux ARCH=64 BUILD=release test && bin/Test_EasyTest++_linux64_release
$ make PLATFORM=linux ARCH=32 BUILD=release test && bin/Test_EasyTest++_linux32_release
$ make PLATFORM=mingw ARCH=64 BUILD=release test && bin/Test_EasyTest++_mingw64_release.exe
$ make PLATFORM=mingw ARCH=32 BUILD=release test && bin/Test_EasyTest++_mingw32_release.exe
```

### Generate source code documentation using [Doxygen](http://www.doxygen.org/)

```
$ make doc && xdg-open doc/generated/html/index.html
```

For detailed information about requirements, platforms and cross-building
please read the [*doc/Build.md*](/doc/Build.md) documentation.

--------------------------------------------------------------------------------

Usage
-----

**Note**: in all following commands, working directory is the parent directory
of EasyTest++ downloaded (and extracted) release package or, equally, the
parent directory of the source code repository with Linux 64 bits static
library built in release mode.

Host system is considered to be Linux 64 bits, please read
[*doc/Usage.md*](/doc/Usage.md) for other systems/architectures.

### Write a test suite

MyTestSuite.cpp

```cpp
#include <easyTest.h>

TEST_SUITE(MyTestSuite)
{
    TEST_CASE(addition);
    TEST_CASE(division);
};

TEST_IMPL(MyTestSuite, addition)
{
    ASSERT_EQUAL(1 + 1, 2);
    return true;
}

TEST_IMPL(MyTestSuite, division)
{
    ASSERT_ALMOST_EQUAL(1.0 / 3.0, 0.3333, 0.0001);
    TRACE("no issue with doubles");

    ASSERT_ALMOST_EQUAL(1.f / 3.f, 0.333f, 0.001f);
    TRACE("no issue with floats");

    return true;
}
```

### Build your test suite

If you have downloaded the release package:

```
$ g++ -std=c++11 -I./EasyTest++/include -o Test_MyTestSuite MyTestSuite.cpp -pthread -L./EasyTest++/lib -lEasyTest++_linux64
```

or if you have built EasyTest++ from sources:

```
$ g++ -std=c++11 -I./EasyTest++/include -o Test_MyTestSuite MyTestSuite.cpp -pthread -L./EasyTest++/bin -lEasyTest++_linux64_release
```

### Run your test suite

```
$ ./Test_MyTestSuite -sv
```

For detailed information about EasyTest++ advanced usage, test suites
documentation and runner options please read the [*doc/Usage.md*](/doc/Usage.md)
documentation.

--------------------------------------------------------------------------------

How to contribute
-----------------

You are welcome to contribute to EasyTest++.

If you find a bug, have issues with the documentation, great ideas or any kind
of question, don't hesitate to
[create an issue](https://guides.github.com/features/issues/).

If you can fix a bug or want to add a feature yourself, please
[fork](https://guides.github.com/activities/forking/) the repository and post a
*Pull Request*.

You can find detailed information about how to contribute in
[GitHub Guides](https://guides.github.com/activities/contributing-to-open-source/).

--------------------------------------------------------------------------------

License
-------

EasyTest++ is released under the [MIT License](/LICENSE).

```
The MIT License (MIT)

Copyright (c) 2015, Loïc Le Page

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```

--------------------------------------------------------------------------------

Known issues
------------

- Under Linux, if your glibc version is older than 2.17 you will have to link
  test runner executables with the `rt` library. If you encounter this error
  when linking `undefined reference to 'clock_gettime'`, it means that you have
  to add `-lrt` at the end of your link command.
  
  If you are building EasyTest++ test runner, replace `make` commands by
  `LDLIBS=-lrt make` or just `export LDLIBS=-lrt` once for all.
  
  If you are building your own test runner using EasyTest++ static library,
  just add `-lrt` at the end of your link command.

- Under Windows, process and thread timers have a really bad resolution
  (superior to tens of ms), so their values are not really meaningful for
  normal test cases which execute in a few nano or micro-seconds.

- If a test runner is running through a debugger while a runtime
  exception/signal occurs and is intercepted, the debugger will catch the
  exception first and the test runner will stop.
  
  Under Linux, you can bypass this behavior specifying `handle all nostop
  noprint pass` to gdb.
  
  Under Windows, you cannot bypass this behavior and the debugger will always
  stop on an uncaught runtime exception. This should not be a big issue as,
  normally, test runners are meant to be run outside of any debugger.

--------------------------------------------------------------------------------

Changelog
---------

### Version 1.0

- Continuous integration scripts for Travis-CI and AppVeyor
- Documentation
- Custom test harness
- i18n and default English translation
- JavaScript/JSON output writer
- TAP output writer
- Log output writer
- Runtime errors protection
- Parallelized test runner
- Code execution timers
- Test suite assertions
- Test suites and test cases auto-registration system
- Makefile, Netbeans and Visual Studio projects

--------------------------------------------------------------------------------
