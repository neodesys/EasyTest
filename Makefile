################################################################################
# EasyTest++ Makefile
#
# Copyright (c) 2015, Loïc Le Page
#
# This project is released under the MIT license.
# See: http://opensource.org/licenses/MIT
################################################################################

#Build configuration
#
#Available options are:
#  BUILD=[debug] or release
#  PLATFORM=[linux] or mingw
#  ARCH=[64] or 32
#
#  EXCEPTIONS=[true] or false
#  THREADS=[true] or false
#
BUILD?=debug
PLATFORM?=linux
ARCH?=64
EXCEPTIONS?=true
THREADS?=true

ifneq ($(BUILD),debug)
    ifneq ($(BUILD),release)
        $(error Invalid BUILD type, please select "debug" or "release")
    endif
endif

ifneq ($(PLATFORM),linux)
    ifneq ($(PLATFORM),mingw)
        $(error Invalid PLATFORM type, please select "linux" or "mingw")
    endif
endif

ifneq ($(ARCH),64)
    ifneq ($(ARCH),32)
        $(error Invalid ARCH type, please select "64" or "32")
    endif
endif

ifneq ($(EXCEPTIONS),true)
    ifneq ($(EXCEPTIONS),false)
        $(error Invalid EXCEPTIONS flag, please set to "true" or "false")
    endif
endif

ifneq ($(THREADS),true)
    ifneq ($(THREADS),false)
        $(error Invalid THREADS flag, please set to "true" or "false")
    endif
endif

#Project definitions
PROJECT_NAME=EasyTest++

SRC_DIR=src
INCLUDE_DIR=include

BUILD_DIR=build
BIN_DIR=bin
TEST_DIR=test

DOC_DIR=doc/generated
DOXYFILE=doc/Doxyfile

PACKAGE_EXTRA_FILES=LICENSE README.md doc/Build.md doc/Usage.md

#Build paths and tools
ifeq ($(PLATFORM),linux)
    AR=ar
    CXX=g++
else
    ifeq ($(ARCH),64)
        AR=x86_64-w64-mingw32-ar
        CXX=x86_64-w64-mingw32-g++
    else
        AR=i686-w64-mingw32-ar
        CXX=i686-w64-mingw32-g++
    endif
endif

LD=$(CXX)
MKDIR=mkdir -p
RMDIR=rmdir --ignore-fail-on-non-empty
RM=rm -rf
SED=sed
DOCGEN=doxygen
TAR=tar

#Compiler/Linker default options
CXXFLAGS+=-Wall -Werror -std=c++11 -pedantic -fno-rtti
ARFLAGS=rcs

ifeq ($(EXCEPTIONS),false)
    CXXFLAGS+=-fno-exceptions
endif

ifeq ($(ARCH),64)
    CXXFLAGS+=-m64
    LDFLAGS+=-m64
else
    CXXFLAGS+=-m32
    LDFLAGS+=-m32
endif

ifeq ($(THREADS),true)
    ifeq ($(PLATFORM),linux)
        CXXFLAGS+=-pthread
        LDFLAGS+=-pthread
    endif
else
    CXXFLAGS:=-DEASYTEST_NO_THREADS $(CXXFLAGS)
endif

ifeq ($(BUILD),debug)
    CXXFLAGS:=-g $(CXXFLAGS)
else
    CXXFLAGS:=-O3 -s -DNDEBUG $(CXXFLAGS)
    LDFLAGS:=-s $(LDFLAGS)
endif

ifeq ($(PLATFORM),mingw)
    LDFLAGS+=-static
endif

#Build variables
STATIC_LIB_TARGET=$(BIN_DIR)/lib$(PROJECT_NAME)_$(PLATFORM)$(ARCH)_$(BUILD)
STATIC_OBJ_DIR=$(BUILD_DIR)/static_$(PLATFORM)$(ARCH)_$(BUILD)

TEST_TARGET=$(BIN_DIR)/Test_$(PROJECT_NAME)_$(PLATFORM)$(ARCH)_$(BUILD)
TEST_OBJ_DIR=$(BUILD_DIR)/test_$(PLATFORM)$(ARCH)_$(BUILD)

ifeq ($(EXCEPTIONS),false)
    STATIC_LIB_TARGET:=$(STATIC_LIB_TARGET)_noexcept
    STATIC_OBJ_DIR:=$(STATIC_OBJ_DIR)_noexcept
    TEST_TARGET:=$(TEST_TARGET)_noexcept
    TEST_OBJ_DIR:=$(TEST_OBJ_DIR)_noexcept
endif

ifeq ($(THREADS),false)
    STATIC_LIB_TARGET:=$(STATIC_LIB_TARGET)_nothreads
    STATIC_OBJ_DIR:=$(STATIC_OBJ_DIR)_nothreads
    TEST_TARGET:=$(TEST_TARGET)_nothreads
    TEST_OBJ_DIR:=$(TEST_OBJ_DIR)_nothreads
endif

STATIC_LIB_TARGET:=$(STATIC_LIB_TARGET).a

ifeq ($(PLATFORM),mingw)
    TEST_TARGET:=$(TEST_TARGET).exe
endif

RECFIND_CPP=$(strip $(wildcard $1/*.cpp) $(foreach d, $(wildcard $1/*), $(call RECFIND_CPP, $d)))
SRC_FILES=$(call RECFIND_CPP, $(SRC_DIR))
TEST_FILES=$(call RECFIND_CPP, $(TEST_DIR)) $(SRC_FILES)

STATIC_OBJ_FILES=$(patsubst %.cpp, $(STATIC_OBJ_DIR)/%.o, $(SRC_FILES))
STATIC_DEP_FILES=$(patsubst %.cpp, $(STATIC_OBJ_DIR)/%.d, $(SRC_FILES))

TEST_OBJ_FILES=$(patsubst %.cpp, $(TEST_OBJ_DIR)/%.o, $(TEST_FILES))
TEST_DEP_FILES=$(patsubst %.cpp, $(TEST_OBJ_DIR)/%.d, $(TEST_FILES))

#Package variables
PACKAGE_NAME=$(BIN_DIR)/$(PROJECT_NAME)
PACKAGE_LIBS=$(BIN_DIR)/lib$(PROJECT_NAME)_???????_release
PACKAGE_MSC_LIBS=$(BIN_DIR)/$(PROJECT_NAME)_msc??_release

ifeq ($(EXCEPTIONS),false)
    PACKAGE_NAME:=$(PACKAGE_NAME)_noexcept
    PACKAGE_LIBS:=$(PACKAGE_LIBS)_noexcept
    PACKAGE_MSC_LIBS:=$(PACKAGE_MSC_LIBS)_noexcept
endif

ifeq ($(THREADS),false)
    PACKAGE_NAME:=$(PACKAGE_NAME)_nothreads
    PACKAGE_LIBS:=$(PACKAGE_LIBS)_nothreads
    PACKAGE_MSC_LIBS:=$(PACKAGE_MSC_LIBS)_nothreads
endif

PACKAGE_NAME:=$(PACKAGE_NAME).tgz
PACKAGE_LIBS:=$(PACKAGE_LIBS).a $(PACKAGE_MSC_LIBS).lib

#Build targets and rules
.PHONY: all clean test clean-test doc clean-doc package clean-package clean-all

all: $(STATIC_LIB_TARGET)

clean:
	$(info Cleaning $(BUILD) build on $(PLATFORM) $(ARCH) bits...)
	@$(RM) $(STATIC_OBJ_DIR) $(STATIC_LIB_TARGET)
	@-$(RMDIR) $(BIN_DIR) $(BUILD_DIR) 2>/dev/null

test: $(TEST_TARGET)

clean-test:
	$(info Cleaning $(BUILD) tests on $(PLATFORM) $(ARCH) bits...)
	@$(RM) $(TEST_OBJ_DIR) $(TEST_TARGET)
	@-$(RMDIR) $(BIN_DIR) $(BUILD_DIR) 2>/dev/null

doc:
	$(info Building documentation...)
	@$(DOCGEN) $(DOXYFILE)

clean-doc:
	$(info Cleaning documentation...)
	@$(RM) $(DOC_DIR)

package:
	$(info Packaging $(PACKAGE_NAME)...)
	@$(MAKE) --no-print-directory BUILD=release PLATFORM=linux ARCH=64
	@$(MAKE) --no-print-directory BUILD=release PLATFORM=linux ARCH=32
	@$(MAKE) --no-print-directory BUILD=release PLATFORM=mingw ARCH=64
	@$(MAKE) --no-print-directory BUILD=release PLATFORM=mingw ARCH=32
	@$(TAR) -czf $(PACKAGE_NAME) --no-acls --transform "s;^$(BIN_DIR)/;lib/;" --transform "s;_release;;" `ls -x $(PACKAGE_LIBS) 2>/dev/null` $(INCLUDE_DIR) $(PACKAGE_EXTRA_FILES)

clean-package:
	$(info Cleaning package $(PACKAGE_NAME)...)
	@$(RM) $(PACKAGE_NAME)
	@-$(RMDIR) $(BIN_DIR) 2>/dev/null

clean-all:
	$(info Cleaning all builds, tests, packages and documentation on all platforms...)
	@$(RM) $(BIN_DIR) $(BUILD_DIR) $(DOC_DIR)

$(STATIC_LIB_TARGET): $(STATIC_OBJ_FILES)
	$(info Creating $(BUILD) static library on $(PLATFORM) $(ARCH) bits...)
	@$(MKDIR) $(@D)
	$(AR) $(ARFLAGS) $@ $^

$(TEST_TARGET): $(TEST_OBJ_FILES)
	$(info Linking $(BUILD) tests binary on $(PLATFORM) $(ARCH) bits...)
	@$(MKDIR) $(@D)
	$(LD) $(LDFLAGS) -o $@ $^ $(LDLIBS)

$(STATIC_OBJ_DIR)/%.d: %.cpp
	@$(MKDIR) $(@D)
	@$(CXX) -MM $(CXXFLAGS) $< 2>/dev/null | $(SED) "s#.*: *#$(STATIC_OBJ_DIR)/$*.o $@: #g" > $@

$(TEST_OBJ_DIR)/%.d: %.cpp
	@$(MKDIR) $(@D)
	@$(CXX) -MM $(CXXFLAGS) $< 2>/dev/null | $(SED) "s#.*: *#$(TEST_OBJ_DIR)/$*.o $@: #g" > $@

ifeq ($(MAKECMDGOALS),test)
    -include $(TEST_DEP_FILES)
else ifneq ($(MAKECMDGOALS),doc)
    ifneq ($(MAKECMDGOALS),package)
        ifeq ($(findstring clean,$(MAKECMDGOALS)),)
            -include $(STATIC_DEP_FILES)
        endif
    endif
endif

$(STATIC_OBJ_DIR)/%.o: %.cpp
	@$(MKDIR) $(@D)
	$(CXX) $(CXXFLAGS) -o $@ -c $<

$(TEST_OBJ_DIR)/%.o: %.cpp
	@$(MKDIR) $(@D)
	$(CXX) $(CXXFLAGS) -o $@ -c $<
