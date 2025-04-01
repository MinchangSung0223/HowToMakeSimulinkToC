/* Copyright 2017-2022 The MathWorks, Inc. */
/*
 * @file: RTWCG_FMU1_target.h
 *  
 * @brief fmustruct 
 *
 * Abstract:
 *      FMI 1.0 data types, function signatures and FMU1_ME_RTWCG(struct)
 *
 *      FMU1_ME_RTWCG is a data struct to store FMU info, handle all 
 *      dynamic function calls to FMU lib 
 *      
 * =============================================================================
 */
#ifndef RTWCG_FMU1ME_GUARD
#define RTWCG_FMU1ME_GUARD

#ifdef __cplusplus
extern "C"{
#endif
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdarg.h>
#ifndef __bool_true_false_are_defined
#undef true /* may be defined in rtwtypes.h */
#undef false /* may be defined in rtwtypes.h */
#include <stdbool.h>
#endif
#include "fmiFunctions.h"

#include "FMUCG_Target.h"
#if FMU_CG_TARGET < FMUCG_STANDALONE_TARGETS
#include <simstruc.h>
#include <slsv_diagnostic_codegen_c_api.h>
/*raccel_utils.h is needed by win plateform to mute warning warning C4013*/
#include "rapid/raccel_utils.h"
#if FMU_CG_TARGET == FMUCG_PROTECTED_MODEL
#include "RTWCG_FMU_util.h"
#elif FMU_CG_TARGET == FMUCG_MCC
#include "RTWCG_FMU_util.h"
#endif
#else
#include "rtwtypes.h"
#include "RTWCG_FMU_util.h"
#endif

#ifndef __FMU1ME_COMMON
#define __FMU1ME_COMMON

#ifdef _WIN32
#include "windows.h"
#define FMUHANDLE                      HMODULE
#define LOAD_LIBRARY(LIBRARY_LOC)      loadLibraryUTF8toUTF16(LIBRARY_LOC)
#define LOAD_FUNCTION                  GetProcAddress
#define CLOSE_LIBRARY                  FreeLibrary

#else
#include <dlfcn.h>
#define FMUHANDLE                      void*
#ifdef RTLD_DEEPBIND
#define LOAD_LIBRARY(LIBRARY_LOC)      dlopen(LIBRARY_LOC, RTLD_NOW | RTLD_LOCAL | RTLD_DEEPBIND)
#else
#define LOAD_LIBRARY(LIBRARY_LOC)      dlopen(LIBRARY_LOC, RTLD_NOW | RTLD_LOCAL)
#endif
#define LOAD_FUNCTION                  dlsym
#define CLOSE_LIBRARY                  dlclose
#endif

#define FULL_FCN_NAME_MAX_LEN 256
#define FCN_NAME_MAX_LEN      28

#ifndef isSimTargetSimulating_GUARD
#define isSimTargetSimulating_GUARD
#define isSimTargetSimulating(S)                                      \
    ssGetSimMode((SimStruct*)(S))==SS_SIMMODE_NORMAL || ssRTWGenIsAccelerator((SimStruct*)(S))
#endif

#ifndef ssGetMdlSfuncS_GUARD
#define ssGetMdlSfuncS_GUARD
#define ssGetMdlSfuncS(S)                \
    (S)->_mdlRefSfcnS
#endif

/*typedef default function type*/    
typedef fmiStatus (*_fmi_default_status_fcn_type) (fmiComponent, ...);
typedef const char* (*_fmi_default_string_fcn_type) (void);
typedef void* (*_fmi_default_instance_fcn_type) (const char*, ...);
typedef void (*_fmi_default_void_fcn_type) (void*);
    
/***************************************************
Types for FMI 1.0 Common Functions
****************************************************/
typedef const char* (*_fmiGetVersion)(void);
typedef fmiStatus (*_fmiSetDebugLogging)    (fmiComponent c, fmiBoolean loggingOn);
typedef fmiStatus (*_fmiSetReal)   (fmiComponent c, const fmiValueReference vr[], size_t nvr, const fmiReal    value[]);
typedef fmiStatus (*_fmiSetInteger)(fmiComponent c, const fmiValueReference vr[], size_t nvr, const fmiInteger value[]);
typedef fmiStatus (*_fmiSetBoolean)(fmiComponent c, const fmiValueReference vr[], size_t nvr, const fmiBoolean value[]);
typedef fmiStatus (*_fmiSetString) (fmiComponent c, const fmiValueReference vr[], size_t nvr, const fmiString  value[]);
typedef fmiStatus (*_fmiGetReal)   (fmiComponent c, const fmiValueReference vr[], size_t nvr, fmiReal    value[]);
typedef fmiStatus (*_fmiGetInteger)(fmiComponent c, const fmiValueReference vr[], size_t nvr, fmiInteger value[]);
typedef fmiStatus (*_fmiGetBoolean)(fmiComponent c, const fmiValueReference vr[], size_t nvr, fmiBoolean value[]);
typedef fmiStatus (*_fmiGetString) (fmiComponent c, const fmiValueReference vr[], size_t nvr, fmiString  value[]);
#endif

/*Model Exchange*/
typedef const char* (*_fmiGetModelTypesPlatform)(void);
typedef fmiComponent (*_fmiInstantiateModel) (fmiString            instanceName,
                                           fmiString            GUID,
                                           fmiCallbackFunctions functions,
                                           fmiBoolean           loggingOn);
typedef void      (*_fmiFreeModelInstance) (fmiComponent c);
typedef fmiStatus (*_fmiSetTime)                (fmiComponent c, fmiReal time);
typedef fmiStatus (*_fmiSetContinuousStates)    (fmiComponent c, const fmiReal x[], size_t nx);
typedef fmiStatus (*_fmiCompletedIntegratorStep) (fmiComponent c, fmiBoolean* callEventUpdate);
typedef fmiStatus (*_fmiInitialize) (fmiComponent c, fmiBoolean toleranceControlled,
                                 fmiReal relativeTolerance, fmiEventInfo* eventInfo);
typedef fmiStatus (*_fmiGetDerivatives)    (fmiComponent c, fmiReal derivatives[]    , size_t nx);
typedef fmiStatus (*_fmiGetEventIndicators)(fmiComponent c, fmiReal eventIndicators[], size_t ni);
typedef fmiStatus (*_fmiEventUpdate)               (fmiComponent c, fmiBoolean intermediateResults, fmiEventInfo* eventInfo);
typedef fmiStatus (*_fmiGetContinuousStates)       (fmiComponent c, fmiReal states[], size_t nx);
typedef fmiStatus (*_fmiGetNominalContinuousStates)(fmiComponent c, fmiReal x_nominal[], size_t nx);
typedef fmiStatus (*_fmiGetStateValueReferences)   (fmiComponent c, fmiValueReference vrx[], size_t nx);
typedef fmiStatus (*_fmiTerminate) (fmiComponent c);

struct FMU1_ME_RTWCG {
    _fmiGetModelTypesPlatform getModelTypesPlatform;
    _fmiGetVersion getVersion;    
    _fmiSetDebugLogging setDebugLogging;
    _fmiSetReal setReal;
    _fmiSetInteger setInteger;
    _fmiSetBoolean setBoolean;
    _fmiSetString setString;
    _fmiGetReal getReal;
    _fmiGetInteger getInteger;
    _fmiGetBoolean getBoolean;
    _fmiGetString getString;
    _fmiInstantiateModel  instantiateModel;
    _fmiFreeModelInstance freeModelInstance;
    _fmiSetTime setTime;
    _fmiSetContinuousStates setContinuousStates;
    _fmiCompletedIntegratorStep completedIntegratorStep;
    _fmiInitialize initialize;
    _fmiGetDerivatives getDerivatives;
    _fmiGetEventIndicators getEventIndicators;
    _fmiEventUpdate eventUpdate;
    _fmiGetContinuousStates getContinuousStates;
    _fmiGetNominalContinuousStates getNominalContinuousStates;
    _fmiGetStateValueReferences getStateValueReferences;
    _fmiTerminate terminate;
    
    char * fmuname;
    char* modelIdentifier;
    char* dllfile;
    FMUHANDLE Handle;
    fmiComponent mFMIComp;
#if FMU_CG_TARGET < FMUCG_STANDALONE_TARGETS
    SimStruct* ssPtr;
#endif
    fmiStatus FMUErrorStatus;
    fmiEventInfo eventInfo;
    fmiCallbackFunctions callbacks;
    bool modelInitialized;
    bool isLoggingOn;
};

void* FMU1_fmuInitializeME(const char* lib,
                           fmiString instanceName,
                           fmiString modelIdentifier,
                           fmiString fmuLocation,
                           fmiString fmuGUID,
                           void* ssPtr,
                           int loadGetRealOutputDerivativesFcn,
                           int loadSetRealInputDerivativesFcn,
                           const void* fmuCallBacks,
                           fmiBoolean fmuVisible,
                           fmiBoolean fmuLoggingOn);

fmiBoolean FMU1_terminateModel(void **fmuv);

fmiBoolean FMU1_initialize(void **fmuv,
                          unsigned char isToleranceUsed,
                          fmiReal toleranceValue);

fmiBoolean FMU1_setTime(void **fmuv,
                       fmiReal time);

fmiBoolean FMU1_completedIntegratorStep(void **fmuv,
                                       int8_T* hasStepEvent);

fmiBoolean FMU1_setContinuousStates(void **fmuv,
                                   fmiReal states[],
                                   size_t nx);

fmiBoolean FMU1_getContinuousStates(void **fmuv,
                                   fmiReal states[],
                                   size_t nx);

fmiBoolean FMU1_getDerivatives(void **fmuv,
                              fmiReal derivatives[],
                              size_t nx);

fmiBoolean FMU1_getEventIndicators(void **fmuv,
                                  fmiReal eventIndicators[],
                                  size_t nx);

fmiBoolean FMU1_eventIteration(void **fmuv,
                              const char* blkPath,
                              fmiReal time);

void FMU1_ifStateVRchanged(void **fmuv,
                           int8_T* fmustateValueOrStateValueRefChanged);

void FMU1_setIterationConverged(void **fmuv,
                                int fmuIsInitialized,
                                fmiReal time);

void FMU1_getNextEventTime(void **fmuv,
                           fmiReal* nextEventTime,
                           int8_T* upcomingTimeEvent);

fmiBoolean FMU1ME_setRealVal(void **fmuv,
                            const fmiValueReference dvr,
                            size_t nvr,
                            const fmiReal dvalue);

fmiBoolean FMU1ME_setReal(void **fmuv,
                         const fmiValueReference dvr,
                         size_t nvr,
                         const fmiReal value[]);

fmiBoolean FMU1ME_getReal(void **fmuv,
                         const fmiValueReference dvr,
                         size_t nvr,
                         fmiReal value[]);

fmiBoolean FMU1ME_setIntegerVal(void **fmuv,
                               const fmiValueReference dvr,
                               size_t nvr,
                               const fmiInteger dvalue);

fmiBoolean FMU1ME_setInteger(void **fmuv,
                            const fmiValueReference dvr,
                            size_t nvr,
                            const fmiInteger value[]);

fmiBoolean FMU1ME_getInteger(void **fmuv,
                            const fmiValueReference vr,
                            size_t nvr,
                            fmiInteger value[]);

fmiBoolean FMU1ME_setBooleanVal(void **fmuv,
                               const fmiValueReference dvr,
                               size_t nvr,
                               const unsigned char dvalue);

fmiBoolean FMU1ME_setBoolean(void **fmuv,
                            const fmiValueReference dvr,
                            size_t nvr,
                            const unsigned char value[]);

fmiBoolean FMU1ME_getBoolean(void **fmuv,
                            const fmiValueReference dvr,
                            size_t nvr, unsigned char value[]);

fmiBoolean FMU1ME_setString(void **fmuv,
                           const fmiValueReference dvr,
                           size_t nvr,
                           const fmiString value[]);

fmiBoolean FMU1ME_setStringVal(void **fmuv,
                              const fmiValueReference dvr,
                              size_t nvr,
                              const fmiString);

fmiBoolean FMU1ME_getString(void **fmuv,
                           const fmiValueReference vr,
                           size_t nvr,
                           fmiString value[]);

void FMU1_checkSimTerminate(void **fmuv,
                            const char* blkPath,
                            fmiReal time);

#ifdef __cplusplus
}
#endif

#endif
