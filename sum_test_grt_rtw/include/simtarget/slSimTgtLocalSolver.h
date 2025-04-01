/* Copyright 2021-2022 The MathWorks, Inc. */

#ifdef SUPPORTS_PRAGMA_ONCE
#pragma once
#endif

#ifndef SL_SIMTARGET_INSTRUMENTATION_SLSIMTGTLOCALSOLVER_H
#define SL_SIMTARGET_INSTRUMENTATION_SLSIMTGTLOCALSOLVER_H

#include "sl_simtarget_instrumentation_spec.h"
#include "simstruc.h"

SL_SIMTARGET_INSTRUMENTATION_EXPORT_EXTERN_C void ssAdvanceLocalContinuousTimeService(
    SimStruct* mdlRefSfcnS,
    SimStruct* localSolverS);

SL_SIMTARGET_INSTRUMENTATION_EXPORT_EXTERN_C void
ssEstimateLocalEquationState(SimStruct* mdlRefSfcnS, SimStruct* localSolverS, double time);

SL_SIMTARGET_INSTRUMENTATION_EXPORT_EXTERN_C void ssLocalMdlInfoInitialization(
    SimStruct* mdlRefSfcnS,
    SimStruct* localSolverS);

SL_SIMTARGET_INSTRUMENTATION_EXPORT_EXTERN_C void ssCreateLocalContinuousTimeService(
    SimStruct* mdlRefSfcnS,
    SimStruct* localSolverS,
    int_T inputCompMode,
    int_T outputCompMode,
    int_T fixedStepGlobalSti);

SL_SIMTARGET_INSTRUMENTATION_EXPORT_EXTERN_C void ssLocalSolverEnable(SimStruct* mdlRefSfcnS,
                                                                      SimStruct* localSolverS,
                                                                      double time);

SL_SIMTARGET_INSTRUMENTATION_EXPORT_EXTERN_C void ssLocalSolverOutputSetup(SimStruct* mdlRefSfcnS,
                                                                           SimStruct* localSolverS);

SL_SIMTARGET_INSTRUMENTATION_EXPORT_EXTERN_C void
ssLocalSolverOutputTeardown(SimStruct* mdlRefSfcnS, SimStruct* localSolverS, double time);

SL_SIMTARGET_INSTRUMENTATION_EXPORT_EXTERN_C void ssLocalSolverTerminate(SimStruct* mdlRefSfcnS,
                                                                         SimStruct* localSolverS);

#endif
