# Copyright 1994-2018 The MathWorks, Inc.
#
# File    : vctools.mak   
# Abstract:
#	Setup Visual tools for nmake.

!ifndef DEBUG_BUILD
DEBUG_BUILD = 0
!endif

!if "$(DEBUG_BUILD)" == "0"
NODEBUG = 1
!endif

!include $(MATLAB_ROOT)\rtw\c\tools\vcdefs.mak

CC     = cl
LD     = link
LIBCMD = lib

#
# Default optimization options.
#
DEFAULT_OPT_OPTS = -Od
