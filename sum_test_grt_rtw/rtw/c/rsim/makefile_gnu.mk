#
# file : rtw/c/rsim/makefile.unix -- GNU Makefile
#        will create C++ copies of the the C static main files to avoid hardcoding TMF
#        rules to build C files with C++ compilers
#

CPP_FILES = \
	rsim_main.cpp

PREBUILD_TARGETS = $(CPP_FILES)

#
# This target needs to be invoked on the master Build & Test
# machine at the start of each build cycle (after the regular
# header files are built)
#
prebuild: $(PREBUILD_TARGETS)

$(CPP_FILES) : $(MAKEFILE_LIST)

rsim_main.cpp : rsim_main.c
	@$(RM) $@
	cp $< $@

clean :
	$(RM) $(PREBUILD_TARGETS)
