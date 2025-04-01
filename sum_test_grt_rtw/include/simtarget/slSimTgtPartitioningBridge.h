/* Copyright 2018-2021 The MathWorks, Inc. */

#ifdef SUPPORTS_PRAGMA_ONCE
#pragma once
#endif

#ifndef SL_SIMTARGET_INSTRUMENTATION_SLSIMTGTPARTITIONINGBRIDGE_H
#define SL_SIMTARGET_INSTRUMENTATION_SLSIMTGTPARTITIONINGBRIDGE_H

#include "sl_simtarget_instrumentation_spec.h"

SL_SIMTARGET_INSTRUMENTATION_EXPORT_EXTERN_C void simTgtRaiseEvent(SimStruct* S,
                                                                   uint_T runtimeEventIndex);

SL_SIMTARGET_INSTRUMENTATION_EXPORT_EXTERN_C void simTgtRaiseEventWithEnqueue(
    SimStruct* S,
    uint_T runtimeEventIndex);

SL_SIMTARGET_INSTRUMENTATION_EXPORT_EXTERN_C void simTgtRaiseWhenTimerExpiresAfter(
    SimStruct* S,
    uint_T timerIndex,
    boolean_T isRecurring,
    double dur,
    uint_T runtimeEventIndex);

SL_SIMTARGET_INSTRUMENTATION_EXPORT_EXTERN_C void
simTgtRaiseWhenTimerExpiresAt(SimStruct* S, uint_T timerIndex, double at, uint_T runtimeEventIndex);

SL_SIMTARGET_INSTRUMENTATION_EXPORT_EXTERN_C void simTgtCancelTimerToRaiseEvent(SimStruct* S,
                                                                                uint_T timerIndex);

SL_SIMTARGET_INSTRUMENTATION_EXPORT_EXTERN_C void simTgtDisableTaskUsingRateIndex(SimStruct* S,
                                                                                  int_T sti);

SL_SIMTARGET_INSTRUMENTATION_EXPORT_EXTERN_C void simTgtEnableTaskUsingRateIndex(SimStruct* S,
                                                                                 int_T sti);

#endif
