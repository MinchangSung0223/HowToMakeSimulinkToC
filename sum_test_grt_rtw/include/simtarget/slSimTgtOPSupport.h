/* Copyright 2021 The MathWorks, Inc. */

#ifdef SUPPORTS_PRAGMA_ONCE
#pragma once
#endif

#ifndef SL_SIMTARGET_INSTRUMENTATION_SLSIMTGTOPSUPPORT_H
#define SL_SIMTARGET_INSTRUMENTATION_SLSIMTGTOPSUPPORT_H

#include "sl_simtarget_instrumentation_spec.h"
#include "simstruc.h"

SL_SIMTARGET_INSTRUMENTATION_EXPORT_EXTERN_C void simTgtAllocOPModelData(SimStruct* ss);

SL_SIMTARGET_INSTRUMENTATION_EXPORT_EXTERN_C void simTgtFreeOPModelData(SimStruct* ss);

SL_SIMTARGET_INSTRUMENTATION_EXPORT_EXTERN_C void simTgtCacheDWorkPointerForSimTargetOP(void* ss,
                                                                                        void** ptr);

SL_SIMTARGET_INSTRUMENTATION_EXPORT_EXTERN_C void
simTgtCacheDWorkDataForSimTargetOP(void* ss, void* ptr, unsigned int sizeInBytes);

SL_SIMTARGET_INSTRUMENTATION_EXPORT_EXTERN_C void simTgtSaveRawMemoryForSimTargetOP(
    void* ss,
    const char* key,
    void** ptr,
    unsigned int sizeInBytes,
    void* (*customOPSaveFcn)(void* dworkPtr, unsigned int* sizeInBytes),
    void (*customOPRestoreFcn)(void* dworkPtr, const void* data, unsigned int sizeInBytes));

SL_SIMTARGET_INSTRUMENTATION_EXPORT_EXTERN_C bool simTgtGlobalInitialStatesAvailable(SimStruct* S);

#endif
