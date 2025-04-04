# Copyright 1994-2022 The MathWorks, Inc.
#
# File    : unixtools.mk   
# Abstract:
#	Setup Unix tools for GNU make

ARCH := $(shell echo "$(COMPUTER)" | tr '[A-Z]' '[a-z]')
OS:=$(shell uname)

DEFAULT_OPT_OPTS = -O0
ANSI_OPTS        =
CPP_ANSI_OPTS    = 
CPP              = c++
LD               = $(CPP)
SYSLIBS          =
LDFLAGS          =
ARCH_SPECIFIC_LDFLAGS =
SHRLIBLDFLAGS    =
AR               = ar

# Override based on platform if needed

GCC_WALL_FLAG     :=
GCC_WALL_FLAG_MAX :=
ifeq ($(COMPUTER),GLNXA64)
  MATLAB_ARCH_BIN = $(MATLAB_ROOT)/bin/glnxa64
  CC  = gcc
  CPP = g++
  DEFAULT_OPT_OPTS = -O0
  SHRLIBLDFLAGS = -shared -Wl,--no-undefined -Wl,--version-script,
  COMMON_ANSI_OPTS = -fwrapv -fPIC
  ANSI_OPTS        = $(COMMON_ANSI_OPTS)
  CPP_ANSI_OPTS    = $(COMMON_ANSI_OPTS)

  # These definitions are used by targets that have the WARN_ON_GLNX option
  GCC_WARN_OPTS     := -Wall -W -Wwrite-strings -Winline \
                       -Wpointer-arith -Wcast-align

  # Command line options valid for C/ObjC but not for C++
  ifneq ($(TARGET_LANG_EXT),cpp)
    GCC_WARN_OPTS := $(GCC_WARN_OPTS) -Wstrict-prototypes -Wnested-externs 
  endif

  GCC_WARN_OPTS_MAX := $(GCC_WARN_OPTS) -Wcast-qual -Wshadow

  ifeq ($(WARN_ON_GLNX), 1)
    GCC_WALL_FLAG     := $(GCC_WARN_OPTS)
    GCC_WALL_FLAG_MAX := $(GCC_WARN_OPTS_MAX)
  endif
endif

ifeq ($(COMPUTER),MACI64)
  MATLAB_ARCH_BIN = $(MATLAB_ROOT)/bin/maci64
  # architecture support x86_64 
  ARCHS = x86_64
endif

ifeq ($(COMPUTER),MACA64)
  MATLAB_ARCH_BIN = $(MATLAB_ROOT)/bin/maca64
  # architecture support x86_64 
  ARCHS = arm64
endif

ifneq (,$(filter $(COMPUTER),MACI64 MACA64))
  DEFAULT_OPT_OPTS := -O0
  XCODE_SDK_VER   := $(shell perl $(MATLAB_ROOT)/rtw/c/tools/macsdkver.pl)
  XCODE_SDK       := MacOSX$(XCODE_SDK_VER).sdk
  XCODE_DEVEL_DIR := $(shell xcode-select -print-path)
  MW_SDK_ROOT  := $(XCODE_DEVEL_DIR)/Platforms/MacOSX.platform/Developer/SDKs/$(XCODE_SDK) 
  XCODE_LD_VERSION := $(shell xcrun ld -v 2>&1 >/dev/null | head -n 1)
  XCODE_LD_VERSION_IS_LD64 := $(findstring ld64-,$(XCODE_LD_VERSION))
  XCODE_DYLD_NO_WARN_DUPLICATE_LDFLAG := -Wl,-no_warn_duplicate_libraries
  NO_WARN_DUPLICATE_LIBRARIES := $(if $(XCODE_LD_VERSION_IS_LD64),,$(XCODE_DYLD_NO_WARN_DUPLICATE_LDFLAG))


  CC  = xcrun clang -arch $(ARCHS) -isysroot $(MW_SDK_ROOT) $(MACOSX_VERSION_MIN_FLAG)
  CPP = xcrun clang++ -arch $(ARCHS) -isysroot $(MW_SDK_ROOT) $(MACOSX_VERSION_MIN_FLAG)
  AR  = xcrun ar
  
  ANSI_OPTS = -fno-common -fexceptions -ffp-contract=off
  CPP_ANSI_OPTS = -std=c++14 -fno-common -fexceptions -ffp-contract=off

  ARCH_SPECIFIC_LDFLAGS = -Wl,-rpath,@executable_path $(NO_WARN_DUPLICATE_LIBRARIES)

  #instead of using -bundle, use -dynamiclib flag to make the lib dlopen compatible
  SHRLIBLDFLAGS = \
    -dynamiclib -install_name @rpath/$(notdir $(PRODUCT)) \
    -Wl,$(LD_NAMESPACE) $(LD_UNDEFS) -Wl,-exported_symbols_list,
endif

