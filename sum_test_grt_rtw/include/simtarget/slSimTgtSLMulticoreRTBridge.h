/* Copyright 2019-2023 The MathWorks, Inc. */

#ifdef SUPPORTS_PRAGMA_ONCE
#pragma once
#endif

#ifndef SLSIMTGTSLMULTICORERTBRIDGE_H
#define SLSIMTGTSLMULTICORERTBRIDGE_H

#include "sl_simtarget_instrumentation_spec.h"

SL_SIMTARGET_INSTRUMENTATION_EXPORT_EXTERN_C void slmcrtCreateSectionProfiles(
    void** opaqueSectionProfiles,
    int sectionCount,
    const char* checksum,
    const char* cachePath);
SL_SIMTARGET_INSTRUMENTATION_EXPORT_EXTERN_C void slmcrtStartProfiling(void* opaqueSectionProfiles,
                                                                       int sectionNumber);
SL_SIMTARGET_INSTRUMENTATION_EXPORT_EXTERN_C void slmcrtStopProfiling(void* opaqueSectionProfiles,
                                                                      int sectionNumber);
SL_SIMTARGET_INSTRUMENTATION_EXPORT_EXTERN_C void slmcrtWriteProfileDataToMat(
    void* opaqueSectionProfiles);
SL_SIMTARGET_INSTRUMENTATION_EXPORT_EXTERN_C void slmcrtDestroySectionProfiles(
    void** opaqueSectionProfiles);

SL_SIMTARGET_INSTRUMENTATION_EXPORT_EXTERN_C void* slmcrtCreateTaskGroup();
SL_SIMTARGET_INSTRUMENTATION_EXPORT_EXTERN_C void slmcrtInsertTask(void* group, void (*f)());
SL_SIMTARGET_INSTRUMENTATION_EXPORT_EXTERN_C void slmcrtWaitTaskGroup(void* group);
SL_SIMTARGET_INSTRUMENTATION_EXPORT_EXTERN_C void slmcrtDestroyTaskGroup(void* group);
#endif
