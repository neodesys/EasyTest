EasyTest++ Usage
================

**Contents**

1. [Test suite and test cases definition](#test-suite-and-test-cases-definition)
  * [Declaration](#declaration)
  * [Implementation](#implementation)
  * [Private fields](#private-fields)
  * [Fixtures](#fixtures)
2. [Tests control](#tests-control)
  * [Assertions](#assertions)
  * [Trace](#trace)
  * [Runtime errors](#runtime-errors)
3. [Example](#example)
  * [Write test suite](#write-test-suite)
  * [Build test runner](#build-test-runner)
4. [Running test runner](#running-test-runner)
  * [Test runner options](#test-runner-options)
  * [Test runner output formats](#test-runner-output-formats)
5. [Custom test harness](#custom-test-harness)

--------------------------------------------------------------------------------

Test suite and test cases definition
------------------------------------

In order to use EasyTest++ you need to include *easyTest.h* in your test files,
add the *include* folder to your includes path and link your test objects with
the EasyTest++ static library.  
You will find more information about how to do this in chapter
[Example](#example).

### Declaration

Basically, an EasyTest++ test suite is a C++ class with methods implementing
test cases, so it can be declared and implemented in separated .h and .cpp
files.

However, it is advised to declare and implement each test suite in its own and
unique .cpp file. This way, each .cpp file in your *test* folder represents a
well-defined test suite with all its test cases isolated at the same place.

A test suite is declared with the `TEST_SUITE(MyTestSuiteName)` macro just
like you would declare `class MyTestSuiteName`.  
Then each test case belonging to this test suite is declared within the test
suite declaration block using the `TEST_CASE(testCaseName)` macro.

```cpp
TEST_SUITE(MyTestSuiteName)
{
    TEST_CASE(test1Name);
    TEST_CASE(test2Name);
};
```

### Implementation

Test cases are implemented like normal methods using the
`TEST_IMPL(MyTestSuiteName, testCaseName)` macro.

A test case implementation must return `true` on success and `false` on failure.

Test cases are executed by the test runner in their implementation order.

```cpp
TEST_IMPL(MyTestSuiteName, test1Name)
{
    //Test case processing...
    return true; //Success
}

TEST_IMPL(MyTestSuiteName, test2Name)
{
    //Test case processing...
    return false; //Failure
}
```

### Private fields

As test suites are C++ classes, you can initialize per test suite private
fields using the test suite default constructor/destructor.  
A new test suite instance is always created right before tests execution and
destroyed right after.

```cpp
TEST_SUITE(MyTestSuiteName)
{
public:
    MyTestSuiteName() : m_buffer(new unsigned char[1024]()) {}
    virtual ~MyTestSuiteName() override final { delete[] m_buffer; }

    TEST_CASE(test1Name);
    TEST_CASE(test2Name);

private:
    unsigned char* const m_buffer;
};

TEST_IMPL(MyTestSuiteName, test1Name)
{
    //Test something with m_buffer
    return true;
}

TEST_IMPL(MyTestSuiteName, test2Name)
{
    //Test something else with m_buffer
    return true;
}
```

**WARNING**: per test suite fields are not reseted between test cases, if
*test1* modifies `m_buffer` content, *test2* will see the modifications.
Normally, per test suite fields should be declared `const` as their content
should not be modified by individual test cases (`const unsigned char* const
m_buffer;`).

### Fixtures

In order to initialize/reset private fields before each test case (fixture),
you can override the `setupFixture()` and `teardownFixture()` virtual methods.

```cpp
TEST_SUITE(MyTestSuiteName)
{
public:
    virtual void setupFixture() override final;
    virtual void teardownFixture() override final;

    TEST_CASE(test1Name);
    TEST_CASE(test2Name);

private:
    unsigned char* m_buffer = nullptr;
};

void MyTestSuiteName::setupFixture()
{
    m_buffer = new unsigned char[1024]();
}

void MyTestSuiteName::teardownFixture()
{
    delete[] m_buffer;
    m_buffer = nullptr;
}

TEST_IMPL(MyTestSuiteName, test1Name)
{
    //Test something with the whole new m_buffer
    return true;
}

TEST_IMPL(MyTestSuiteName, test2Name)
{
    //Test something else with the whole new m_buffer
    return true;
}
```

In this latter case, any modification of `m_buffer` by *test1* will have no
impact on *test2*.

Obviously, you can combine both technics.

```cpp
TEST_SUITE(MyTestSuiteName)
{
public:
    MyTestSuiteName() : m_buffer(new unsigned char[1024]) {}
    virtual ~MyTestSuiteName() override final { delete[] m_buffer; }

    virtual void setupFixture() override final;

    TEST_CASE(test1Name);
    TEST_CASE(test2Name);

private:
    unsigned char* const m_buffer;
};

void MyTestSuiteName::setupFixture()
{
    std::memset(m_buffer, 0, 1024);
}

TEST_IMPL(MyTestSuiteName, test1Name)
{
    //Test something with the blank m_buffer
    return true;
}

TEST_IMPL(MyTestSuiteName, test2Name)
{
    //Test something else with the blank m_buffer
    return true;
}
```

In this latter case, each test case has access to a writable `m_buffer`
initialized with 0 before each individual test case execution, but `m_buffer`
is only allocated once during test suite creation and before executing all test
cases.  
*Remark:* in this example `m_buffer` is not zero-initialized during allocation,
this operation is useless as the buffer content is set to 0 before each
individual test case.

**WARNING**: different test suites **MUST NEVER** share any global data as
their repective executions can be performed in parallel. All test cases in a
same test suite are always executed sequentially in the same thread and in the
order of their implementation (`TEST_IMPL` macros).

--------------------------------------------------------------------------------

Tests control
-------------

A successful test case must return `true`.  
If a test case method returns `false`, test has failed.  
If a test case method doesn't return because an uncaught exception has been
thrown or a runtime error occurred, test has failed and a runtime error
notification is emitted.

### Assertions

All assertions macros do nothing on success and return `false` on failure.

When an assertion fails, it also emits an assertion notification including
information about file, function and code line number of the assertion as well
as details about the assertion failure.

Types of variables can be of any type compatible with the asserted operation,
including instances of objects with overloaded operators.

- `ASSERT_TRUE(var)` asserts that `var` is `true`
- `ASSERT_FALSE(var)` asserts that `var` is `false`
- `ASSERT_NAN(var)` asserts that `var` is `NaN` (not a number)
- `ASSERT_INFINITE(var)` asserts that `var` is infinite (positive or negative)
- `ASSERT_FINITE(var)` asserts that `var` is a finite number (neither infinite
  nor `NaN`)
- `ASSERT_LESS_THAN(a, b)` asserts that `a < b`
- `ASSERT_LESS_OR_EQUAL(a, b)` asserts that `a <= b`
- `ASSERT_GREATER_THAN(a, b)` asserts that `a > b`
- `ASSERT_GREATER_OR_EQUAL(a, b)` asserts that `a >= b`
- `ASSERT_EQUAL(a, b)` asserts that `a == b`
- `ASSERT_DIFFERENT(a, b)` asserts that `a != b`
- `ASSERT_STRING_EQUAL(a, b)` asserts that `strcmp(a, b) == 0`, characters type
  can be anything (`char`, `wchar_t`, `char16_t`, `char32_t`, etc...) but
  strings must end with a `\0` character. If any of the parameters is `nullptr`,
  the assertion fails
- `ASSERT_STRING_DIFFERENT(a, b)` asserts that `strcmp(a, b) != 0`
- `ASSERT_ALMOST_EQUAL(a, b, precision)` asserts that
  `abs(b - a) <= abs(precision)`
- `ASSERT_VERY_DIFFERENT(a, b, precision)` asserts that
  `abs(b - a) > abs(precision)`
- `ASSERT_BITWISE_EQUAL(a, b)` asserts that `sizeof(a) == sizeof(b)` and that
  `a` equals `b` bit per bit in memory (`a` and `b` should have different
  types). For example, with `short a = -1` and `unsigned short b = 65535`, `a`
  and `b` are different but are bitwise-equal
- `ASSERT_BITWISE_DIFFERENT(a, b)` asserts that `a` and `b` are not bitwise-equal
- `ASSERT_SAME_DATA(a, b, sizeInBytes)` asserts that memory block pointed by
  `a` and memory block pointed by `b` have same data for at least `sizeInBytes`
  bytes. If any of the parameters is `nullptr` or `sizeInBytes` is 0, the
  assertion fails
- `ASSERT_DIFFERENT_DATA(a, b, sizeInBytes)` asserts that `a` and `b` memory
  blocks of `sizeInBytes` bytes are not equal

### Trace

If you need to display out any kind of information during a test case, you can
use TRACE macros.

Calling any of these macros will emit a notification including information
about file, function and code line number of the TRACE call as well as the text
provided.

- `TRACE(str)` will display out the `str` string
- `TRACE_FORMAT(format, ...)` will display out a formatted string just like
  `printf(format, ...)` would do

### Runtime errors

Test suites contruction and destruction, test fixtures and test cases are all
protected against runtime errors.

If any uncaught exception or other kind of runtime error occurs at those
moments, current test fails immediately and a runtime error notification is
emitted.  
Following tests will keep on executing normally.

Here are the different types of runtime errors intercepted:

- Uncaught C++ exception
- Access to a virtual memory address not mapped
- No permission to acces to a mapped memory address
- Memory bus error (non existent physical address or other hardware error)
- Access to misaligned address in memory
- Error while executing an instruction (coprocessor or internal stack error)
- Execution of an illegal instruction
- Execution of a priviledged instruction
- Integer division by zero
- Integer overflow
- Undefined floating-point error
- Floating-point division by zero
- Floating-point overflow
- Floating-point underflow
- Inexact floating-point result
- Invalid floating-point operation
- Index access outside of the bounds of an array (if hardware supports bounds
  checking)
- Bad argument passed to a system call
- Write to a pipe/socket with no reader
- File size limit has been exceeded

--------------------------------------------------------------------------------

Example
-------

All along this example, we assume that the working directory is the parent
folder of the EasyTest++ installed package.

```
working-dir
     |
     |- MyTestSuite.cpp
     |
     |- EasyTest++
            |
            |- include
            |     |
            |     |- RegistrarRefStorage.h
            |     |- TestCaseRegistrar.h
            |     |- TestSuite.h
            |     |- TestSuiteRegistrar.h
            |     |- easyTest.h
            |
            |- lib
                |
                |- EasyTest++_msc32.lib
                |- EasyTest++_msc64.lib
                |- libEasyTest++_linux32.a
                |- libEasyTest++_linux64.a
                |- libEasyTest++_mingw32.a
                |- libEasyTest++_mingw64.a
```

We also assume that we are using a Linux 64 bits with a cross-compilation
environment (see
[requirements for cross-compilation under Linux](/doc/Build.md#linux-and-cross-compilation)).

If you are under a Linux 32 bits or you have not installed MinGW-w64, commands
are the same. You just have to choose the ones which correspond to your
situation.

Under Windows with Cygwin installed (see
[requirements for compiling on Windows with MinGW-w64](/doc/Build.md#windows-and-mingw-w64-compiler)),
this is the same. You just have to execute the commands with `mingw` in a
Cygwin prompt.

Under Windows with Visual Studio (see
[requirements for compiling on Windows with MSC](/doc/Build.md#windows-and-msc-compiler)),
you can't use command lines. Instead, use Visual Studio to create a new
project targeting a console application.  
Add *MyTestSuite.cpp* to this project and, in project properties, configure
compiler and linker:

- add EasyTest++ *include* path to the compiler configuration
- add EasyTest++ *lib* path to the linker configuration
- add *EasyTest++_msc32.lib* or *EasyTest++_msc64.lib* to the list of libraries
  to link with your executable depending on your targeted architecture (`x86`
  or `x64`)

### Write test suite

In order to have access to all test suites macros and assertions, you only need
to include *easyTest.h* at the beginning of your test suite file.

Create a file named *MyTestSuite.cpp* and paste this code into it.

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

### Build test runner

**Note**: following commands are reduced to the strict minimum needed for the
purpose of this example.

**Linux versions of the test runner**

Under Linux, in addition to the EasyTest++ static library, the test runner
will only depend on the `pthread` library.  
So, you have to add the `-lpthread` or just `-pthread` flag to the link command.

```
$ g++ -std=c++11 -m64 -I./EasyTest++/include -o Test_MyTestSuite_linux64 MyTestSuite.cpp -pthread -L./EasyTest++/lib -lEasyTest++_linux64
$ g++ -std=c++11 -m32 -I./EasyTest++/include -o Test_MyTestSuite_linux32 MyTestSuite.cpp -pthread -L./EasyTest++/lib -lEasyTest++_linux32
```

**Windows (MinGW) versions of the test runner**

With MinGW, the test runner just needs the EasyTest++ static library to work.

```
$ x86_64-w64-mingw32-g++ -std=c++11 -m64 -I./EasyTest++/include -o Test_MyTestSuite_mingw64.exe MyTestSuite.cpp -static -L./EasyTest++/lib -lEasyTest++_mingw64
$ i686-w64-mingw32-g++ -std=c++11 -m32 -I./EasyTest++/include -o Test_MyTestSuite_mingw32.exe MyTestSuite.cpp -static -L./EasyTest++/lib -lEasyTest++_mingw32
```

**Note**: the `-static` flag used with MinGW linker is not compulsory but it is
an interesting option because it links the executable with the static versions
of MinGW *libstdc++* and *libgcc*. Thus our Windows executable will only depend
on Windows default system libraries without any kind of external dependencies.

--------------------------------------------------------------------------------

Running test runner
-------------------

Basic usage of the test runner is:

> runner [OPTION...] [TEST_SUITE...]

If no test suite is specified in `[TEST_SUITE...]`, all test suites included in
the test runner will be executed once.

If a test suite is specified more than once, it will be executed many times.

Unknown test suites which are not included in the test runner are just ignored.

**Note**: options and test suites names are not case sensitive.

### Test runner options

In case of doubt, just call the test runner with a `-h` or `--help` option to
display all available options.  
All test runners come with a complete help reminder.

- `-h` or `--help` displays the help reminder
- `-l` or `--list` lists all unit test suites included in the test runner
- `-n` or `--nthreads` sets the maximum number of worker-threads to use while
  executing unit test suites. The special value `max` corresponds to the
  maximum number of hardware threads available. This is the default value if
  the option is not specified
- `-o` or `--out` writes unit tests results to the specified file. If not
  specified, unit tests results are written to the default output (`stdout`)
- `-t` or `--type` specifies the format of unit tests results. Valid formats
  are: `log` (default), `js` and `tap`
- `-v` or `--verbose` writes extra information to unit tests results, including
  asserts failures and traces
- `-s` or `--stats` writes execution times for all unit test cases and test
  suites in tests results

### Test runner output formats

Test suites can be executed by different worker-threads and results output is
buffered by each of these threads independently. Those buffers are flushed out
into the destination stream at specific synchronization points which depend on
the runner output format.

Thus, the execution order of test suites and how test cases results appear will
be different according to the output format chosen and, anyway, may be
different each time the test runner is launched.

All output formats have synchronization points when test runner starts and when
it stops, so output will always start with a header displaying information
about the test runner execution (in particular the maximum number of
worker-threads used) and it will always end with a footer displaying global
information about tests results (in particular the total number of failed test
suites). The order of what happens between this header and this footer depends
on the output format chosen.

- `log` is a plain text format. It is synchronized at the end of each test
  suite, so you can't have mixed test cases from different suites following
  each other, but you won't have any information about individual test cases
  until the whole test suite has finished its execution. This is a format
  perfect for logs dedicated to human readers but not for external parsers.

- `tap` is also a plain text format respecting the
  [TAP](https://testanything.org/) protocol. It is synchronized at the end of
  each test case, so you may have mixed test cases from different test suites
  following each other but each test case is displayed out as soon as it has
  finished its execution. This output format focusses on individual test cases
  and not on test suites. It is as suitable for human readers as for external
  parsers. It can be connected to a TAP-compatible external test harness.

- `js` output format is particular: it covers in reality two different output
  formats:
  
  - **JavaScript** when verbose mode is active (option `-v` or `--verbose`).
    Then, it is synchronized at the end of each test suite (like the `log`
    format) and it produces a literal array stored in a global variable named
    `g_testResult`. This format is not suitable for human readers, its purpose
    is to be easily included in an HTML page in charge of formatting results.
    Assuming that your output file is called *results.js*, you can import it
    directly in your HTML page as a script source:
    `<script src="results.js"></script>`, then you may format tests results as
    you wish using the `g_testResult` global array. This array only contains
    JavaScript literal objects respecting the structures defined right after
    in [Custom test harness](#custom-test-harness) chapter.
  
  - **JSON** when in normal mode (verbose mode disabled). Then, it is
    synchronized after each event which means that anything from test suites or
    test cases starting or finishing, assertions, traces, runtime errors, may
    be displayed out in any order. All information is flushed out as soon as it
    occurs. The output stream is composed of JSON objects separated from each
    other by an end-of-line `\n` character. This format is not suitable for
    human readers, its purpose is to be linked to a client web page through an
    `XMLHttpRequest` or a `WebSocket` in order to display tests execution in
    real time. The structures of transmitted JSON objects, as well as an
    example of how to use this output format, are presented right after in
    [Custom test harness](#custom-test-harness) chapter.

--------------------------------------------------------------------------------

Custom test harness
-------------------

As seen in [Test runner output formats](#test-runner-output-formats) chapter,
the `js` output format can be used easily by external parsers using an
HTML/JavaScript technology.

When verbose mode is active, the output is a valid script containing a literal
array like this:

```javascript
var g_testResult = [
    { ... },
    { ... },
    { ... }
];
```

When in normal mode, the output is composed of JSON objects separated from each
other by an end-of-line `\n` character like this:

```
{ ... }\n
{ ... }\n
{ ... }\n
```

In both cases, the transmitted literal objects are the same and follow the same
structures. Each object represents an event: something that occurs during the
execution of the test runner.

All objects have a field `type` which is a string identifying the type of the
event. The rest of the object structure depends on its type.

- RunnerStart event

```javascript
{
    type: "runner_start",
    nbMaxWorkerThreads: [number],
    nbTotalSuites: [number],
    nbTotalTestCases: [number]
}
```

- SuiteError event

```javascript
{
    type: "suite_error",
    workerThreadIdx: [number],
    testSuiteName: [string],
    nbTestCases: [number]
}
```

- SuiteStart event

```javascript
{
    type: "suite_start",
    workerThreadIdx: [number],
    testSuiteName: [string],
    nbTotalCases: [number]
}
```

- CaseStart event

```javascript
{
    type: "case_start",
    workerThreadIdx: [number],
    testSuiteName: [string],
    testCaseName: [string]
}
```

- Trace event

```javascript
{
    type: "trace",
    workerThreadIdx: [number],
    file: [string],
    function: [string],
    line: [number],
    details: [string]
}
```

- Assert event

```javascript
{
    type: "assert",
    workerThreadIdx: [number],
    file: [string],
    function: [string],
    line: [number],
    details: [string]
}
```

- RuntimeError event

```javascript
{
    type: "runtime_error",
    workerThreadIdx: [number],
    details: [string]
}
```

- CaseFinish event

```javascript
{
    type: "case_finish",
    workerThreadIdx: [number],
    testSuiteName: [string],
    testCaseName: [string],
    bSuccess: [bool],
    timer: [object] //can be null if stats are not activated
}
```

- Timer object

```javascript
{
    real: [number],  //real time in ns
    proc: [number],  //process time in ns
    thread: [number] //thread time in ns
}
```

- SuiteFinish event

```javascript
{
    type: "suite_finish",
    workerThreadIdx: [number],
    testSuiteName: [string],
    nbTotalCases: [number],
    nbSuccessCases: [number],
    nbFailedCases: [number],
    timer: [object] //can be null if stats are not activated
}
```

- RunnerFinish event

```javascript
{
    type: "runner_finish",
    nbUsedWorkerThreads: [number], //may be inferior to nbMaxWorkerThreads
                                   //in RunnerStart event
    nbTotalSuites: [number],
    nbSuccessSuites: [number],
    nbFailedSuites: [number],
    timer: [object]                //can be null if stats are not activated
}
```

**Note**: for all `workerThreadIdx` fields, it is guaranteed that
`0 <= workerThreadIdx < nbMaxWorkerThreads` with `nbMaxWorkerThreads` provided
by the `RunnerStart` event which is always the first event transmitted.

**Note**: `SuiteFinish` and `RunnerFinish` events report the total number of
elements to process as well as successes and failures. This may sound
redundant, but in reality the sum of successes and failures may be inferior to
the total number of elements when the test runner execution has been
interrupted before completion (by hitting `CTRL+C` or quitting the terminal
window for example).

You will find an illustration of how to develop a custom real-time test harness
using these events in the [*extra*](/extra) directory.

The [*harnessServer*](/extra/harnessServer) shell script emulates a simple HTTP
server calling test runners (built in the *bin* directory) and the
[*harnessExample.html*](/extra/harnessExample.html) web page connects to this
server and displays test runners executions in real time.

To see this example of custom test harness in action:

- launch [*harnessServer*](/extra/harnessServer) script from the
  [*extra*](/extra) directory (path to the *bin* directory is relative)
- open [*harnessExample.html*](/extra/harnessExample.html) page in your web
  browser

--------------------------------------------------------------------------------
