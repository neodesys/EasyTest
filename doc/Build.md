Build EasyTest++
================

**Contents**

1. [Requirements](#requirements)
  * [Windows and MSC compiler](#windows-and-msc-compiler)
  * [Windows and MinGW-w64 compiler](#windows-and-mingw-w64-compiler)
  * [Linux and GNU compiler](#linux-and-gnu-compiler)
  * [Linux and cross-compilation](#linux-and-cross-compilation)
  * [Documentation](#documentation)
2. [Build using an IDE](#build-using-an-ide)
  * [Visual Studio](#visual-studio)
  * [Netbeans](#netbeans)
3. [Build using the Makefile](#build-using-the-makefile)
  * [Targets](#targets)
  * [Configuration](#configuration)
  * [Commands](#commands)
4. [Source code organization](#source-code-organization)

--------------------------------------------------------------------------------

Requirements
------------

First of all, you need to clone the repository:

```
$ git clone https://github.com/neodesys/easytest.git EasyTest++
```

Then you need a full-featured C++11 compiler and its tool suite.

Currently, EasyTest++ supports:

- [g++](https://gcc.gnu.org/) (version 4.8 and above)
- [MinGW-w64](http://mingw-w64.org/) (version 3.0 and above)
- [Visual Studio](https://www.visualstudio.com/) (version 2015 and above)

Here are the development tools you need to install by platform and compiler.

### Windows and MSC compiler

You need to download and install
[Visual Studio](https://www.visualstudio.com/) version 2015 or above.

### Windows and MinGW-w64 compiler

You need to download and install [Cygwin](https://www.cygwin.com/) with, at
least, the following packages:

- make
- ncurses
- mingw64-i686-gcc-g++
- mingw64-x86_64-gcc-g++ (only if you have a 64 bits system)

### Linux and GNU compiler

You need to install the g++ tool suite and GNU make (and optionally gdb if you
want to debug the library).

On Debian and Ubuntu based distributions, it can be done like this:

```
$ sudo apt-get install build-essential gdb
```

### Linux and cross-compilation

On a Linux 64 bits system, you can cross-build EasyTest++ for 32 and 64 bits
architectures for Linux and Windows (using MinGW-w64).

In addition to g++, make and gdb you need to install g++-multilib and
mingw-w64.  
If you want to run and cross-debug EasyTest++ test runner for Windows under
Linux, you also need to install wine and MinGW-w64 *gdbserver.exe* targets for
Windows.

On Debian and Ubuntu based distributions, it can be done like this:

```
$ sudo dpkg --add-architecture i386
$ sudo apt-get install build-essential gdb g++-multilib mingw-w64 gdb-mingw-w64-target wine
```

### Documentation

To build the auto-generated documentation of EasyTest++ source code, you need
to install [Doxygen](http://www.doxygen.org/) and
[Graphviz](http://www.graphviz.org/).

--------------------------------------------------------------------------------

Build using an IDE
------------------

EasyTest++ provides projects for [Netbeans](https://netbeans.org/) and
[Visual Studio](https://www.visualstudio.com/) IDE in the [*ide*](/ide)
directory.

### Visual Studio

Open the EasyTest++ MSVC solution at
[*ide/msvc/EasyTest++.sln*](/ide/msvc/EasyTest++.sln).

This solution includes a project called `EasyTest++` which builds the library,
and another project called `Test_EasyTest++` which builds the test runner.  
The solution provides targets for `x86` and `x64` architectures in `Debug` and
`Release` modes.

Libraries and test runners are built into the *bin* directory under the names
`EasyTest++_msc(32|64)_(debug|release).lib` and
`Test_EasyTest++_msc(32|64)_(debug|release).exe`.

### Netbeans

You must activate the C/C++ plugin and configure associated build tools.  
In the family of `GNU` tool collections, you must add a first collection named
`Mingw32` configured with the MinGW-w64 32 bits tool suite, and a second
collection named `Mingw64` configured with the MinGW-w64 64 bits tool suite.

Open the EasyTest++ Netbeans project at [*ide/nbproject*](/ide/nbproject).

This project provides targets for Linux and MinGW compilers in 32 and 64 bits,
as well as targets to build the test runner.  
In reality, when building EasyTest++, Netbeans just calls the *Makefile* with
different configurations.

If you are under Linux and you plan to debug the test runner, you will find in
[*ide/nbproject/gdbinit*](/ide/nbproject/gdbinit) directory some gdb init files
that may be useful.  
You can add these files to the project properties under the `Debug` category
and into the `Gdb Init File` property.

- [*ide/nbproject/gdbinit/linux_test_gdbinit*](/ide/nbproject/gdbinit/linux_test_gdbinit)
  is meant to be used with the `Test_Debug_Linux_64` and `Test_Debug_Linux_32`
  configurations,
- [*ide/nbproject/gdbinit/mingw64_test_gdbinit*](/ide/nbproject/gdbinit/mingw64_test_gdbinit)
  with the `Test_Debug_Mingw_64` configuration and
- [*ide/nbproject/gdbinit/mingw32_test_gdbinit*](/ide/nbproject/gdbinit/mingw32_test_gdbinit)
  with the `Test_Debug_Mingw_32` configuration.

The MinGW configuration files allow to easily cross-debug the Windows versions
of the test runner on a Linux host.  
They both connect gdb to an external gdbserver on port 6400 for the 64 bits
version and port 3200 for the 32 bits one.

If you have correctly installed MinGW-w64 *gdbserver.exe* targets for Windows
(see [Linux and cross-compilation](#linux-and-cross-compilation)), you can
start them in multi-process mode from the parent directory of EasyTest++.  
On Debian and Ubuntu based distributions, it can be done like this:

```
$ /usr/share/win32/gdbserver.exe --multi :3200
```

or

```
$ /usr/share/win64/gdbserver.exe --multi :6400
```

**WARNING**: depending on your version of wine and system configuration,
cross-debugging using this technic may be a little bit unstable. The emulation
of gdbserver through wine may send unexpected signals to the gdb client. If you
encounter this kind of issue, you can also launch gdbserver from a Windows
virtual machine with a redirection of ports 3200 and 6400.

--------------------------------------------------------------------------------

Build using the Makefile
------------------------

Whether you are under Linux, or under Windows using Cygwin, the fastest way to
build EasyTest++ is to use the *Makefile*.

```
$ cd /path/to/EasyTest++
$ make [target]
```

### Targets

EasyTest++ *Makefile* provides the following targets:

- `all` (or no target specified) builds the EasyTest++ static library
- `clean` cleans up the EasyTest++ static library and its intermediate build
  files
- `test` builds the test runner
- `clean-test` cleans up the test runner and its intermediate build files
- `doc` generates source code documentation in *doc/generated* using Doxygen
- `clean-doc` cleans up the auto-generated source code documentation
- `package` builds release versions of EasyTest++ static libraries for Linux
  and MinGW in 32 and 64 bits, and packages them with all needed includes in a
  distributable tar.gz archive
- `clean-package` cleans up the EasyTest++ distributable tar.gz archive
- `clean-all` cleans up everything

### Configuration

Compiler suite and architecture are defined by environment variables:

- `BUILD` defines the build mode which can be `debug` (default) or `release`
- `PLATFORM` defines the compiler suite which can be `linux` (default) or
  `mingw`
- `ARCH` defines the architecture which can be `64` (default) or `32`

EasyTest++ *Makefile* accepts two other advanced configuration variables:

- `EXCEPTIONS` which can be `true` (default) or `false` (then C++ exception
  handling is fully disabled)
- `THREADS` which can be `true` (default) or `false` (then multithreading is
  fully disabled)

EasyTest++ neither uses directly nor relies on C++ exceptions. It may be safely
built with exception handling fully disabled (`EXCEPTIONS=false`) and still
intercept uncaught exceptions.  
However, it is advised to build with exception handling enabled in order to get
more information about uncaught exceptions, in particular the exception type.  
If built without exception support, any uncaught C++ exception will be just
reported as "undefined exception" without any more information.

You can also build EasyTest++ without multithreading support (`THREADS=false`)
in order to get rid of the `pthread` library dependency under Linux.  
In this case, all test suites will be executed sequentially and you will loose
the benefits of parallel execution in particular within huge projects.

As a general matter, it is advised to keep `EXCEPTIONS` and `THREADS` variables
with their default values.

### Commands

**Build the EasyTest++ static library**

- for Linux 64 bits

```
$ make PLATFORM=linux ARCH=64 BUILD=debug
$ make PLATFORM=linux ARCH=64 BUILD=release
```

- for Linux 32 bits

```
$ make PLATFORM=linux ARCH=32 BUILD=debug
$ make PLATFORM=linux ARCH=32 BUILD=release
```

- for Windows 64 bits (using MinGW)

```
$ make PLATFORM=mingw ARCH=64 BUILD=debug
$ make PLATFORM=mingw ARCH=64 BUILD=release
```

- for Windows 32 bits (using MinGW)

```
$ make PLATFORM=mingw ARCH=32 BUILD=debug
$ make PLATFORM=mingw ARCH=32 BUILD=release
```

Libraries are built into the *bin* directory under the names
`libEasyTest++_(linux|mingw)(32|64)_(debug|release).a`.

**Build the EasyTest++ test runner**

- for Linux 64 bits

```
$ make PLATFORM=linux ARCH=64 BUILD=debug test
$ make PLATFORM=linux ARCH=64 BUILD=release test
```

- for Linux 32 bits

```
$ make PLATFORM=linux ARCH=32 BUILD=debug test
$ make PLATFORM=linux ARCH=32 BUILD=release test
```

- for Windows 64 bits (using MinGW)

```
$ make PLATFORM=mingw ARCH=64 BUILD=debug test
$ make PLATFORM=mingw ARCH=64 BUILD=release test
```

- for Windows 32 bits (using MinGW)

```
$ make PLATFORM=mingw ARCH=32 BUILD=debug test
$ make PLATFORM=mingw ARCH=32 BUILD=release test
```

Test runners are built into the *bin* directory under the names
`Test_EasyTest++_(linux|mingw)(32|64)_(debug|release)(.exe)`.

**Build the source code documentation using Doxygen**

```
$ make doc
```

Auto-generated documentation is built into the *code/generated* directory.

--------------------------------------------------------------------------------

Source code organization
------------------------

EasyTest++ source code is organized under the following directories:

- [*doc*](/doc) contains the static and generated documentations
- [*extra*](/extra) contains extra source files like harness examples
- [*ide*](/ide) contains project files for Netbeans and Visual Studio IDE
- [*include*](/include) contains include files needed to use the library. These
  files **MUST** be distributed with the static library
- [*src*](/src) contains the source files needed to build the static library.
  Source files are organized by namespace:
  - [*src*](/src) itself is for the `easyTest` namespace which contains all
    files used by the test runner and its runtime protection
  - [*src/i18n*](/src/i18n) is for the `easyTest::i18n` namespace which
    contains internationalization files, in particular translation files for
    all implemented languages (currently only English)
  - [*src/output*](/src/output) is for the `easyTest::output` namespace which
    contains output writers files
  - [*src/stats*](/src/stats) is for the `easyTest::stats` namespace which
    contains files for code execution timers
- [*test*](/test) contains the unit test suites needed to test the static
  library

--------------------------------------------------------------------------------
