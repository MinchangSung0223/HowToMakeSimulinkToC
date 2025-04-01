/* Copyright 2020-2021 The MathWorks, Inc. */

#ifdef SUPPORTS_PRAGMA_ONCE
#pragma once
#endif

#ifndef SL_SIMTARGET_INSTRUMENTATION_SLSIMTGTSLEXECSIMBRIDGE_H
#define SL_SIMTARGET_INSTRUMENTATION_SLSIMTGTSLEXECSIMBRIDGE_H

#include "sl_simtarget_instrumentation_spec.h"
#include "simstruc.h"

SL_SIMTARGET_INSTRUMENTATION_EXPORT_EXTERN_C
uint32_T simTgtGetLiveOutputIndex(SimStruct* S, const char* identifier, const char* path);

SL_SIMTARGET_INSTRUMENTATION_EXPORT_EXTERN_C
void simTgtRecordLiveOutputData(SimStruct* S, uint32_T index, double time, void const* data);

SL_SIMTARGET_INSTRUMENTATION_EXPORT_EXTERN_C
void simTgtPushModelBlockPath(SimStruct* S, const char* path);

SL_SIMTARGET_INSTRUMENTATION_EXPORT_EXTERN_C
void simTgtPopModelBlockPath(SimStruct* S, const char* path);

#endif
