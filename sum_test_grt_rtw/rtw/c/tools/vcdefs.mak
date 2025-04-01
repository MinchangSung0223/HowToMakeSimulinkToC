
# cdebug ----------------------------------------------------------------------
!IFDEF NODEBUG
cdebug = -Ox -DNDEBUG 
!ELSE
cdebug = -Zi -Od -DDEBUG
!ENDIF

# cflags ----------------------------------------------------------------------
cflags = $(cflags) -c -nologo -GS -W4

# Compatibility with legacy code
cflags = $(cflags) -DWIN32

# cvars cvarsmt cvarsdll ------------------------------------------------------
# for Windows applications that use the C Run-Time libraries
!IFDEF NODEBUG
cvarsmt    = -D_MT -MT
cvars      = $(cvarsmt)
cvarsdll   = -D_MT -D_DLL -MD
!ELSE
cvarsmt    = -D_MT -MTd
cvars      = $(cvarsmt)
cvarsdll   = -D_MT -D_DLL -MDd
!ENDIF

# ldebug ----------------------------------------------------------------------
!IFDEF NODEBUG
ldebug = /RELEASE
!ELSE
ldebug = /DEBUG /DEBUGTYPE:cv
!ENDIF


# conflags-------------------------------
lflags  = $(lflags) /INCREMENTAL:NO /NOLOGO
conflags  = $(lflags)

# conlibs
winsocklibs = ws2_32.lib mswsock.lib
conlibs     = kernel32.lib $(winsocklibs) advapi32.lib
conlibsmt   = $(conlibs)
conlibsdll  = $(conlibs)
