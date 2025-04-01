/* Copyright 2013-2021 The MathWorks, Inc. */

#pragma once

#include "sf_runtime/sf_runtime_spec.h"

struct CovrtStateflowInstance;
typedef struct {
    unsigned int isEMLChart;
    void* chartInstance;
    char sFunctionName[mxMAXNAM];
    unsigned int chartInitialized;
    void (*sFunctionGateway)(void* chartInstance);
    void (*extModeExec)(void* chartInstance);
    void (*restoreLastMajorStepConfiguration)(void* chartInstance);
    void (*restoreBeforeLastMajorStepConfiguration)(void* chartInstance);
    void (*storeCurrentConfiguration)(void* chartInstance);
    void (*initializeChart)(void* chartInstance);
    void (*enableChart)(void* chartInstance);
    void (*disableChart)(void* chartInstance);
    const mxArray* (*getSimState)(SimStruct* S);
    const mxArray* (*getSimStateInfo)(void);
    void (*setSimState)(SimStruct* S, const mxArray* st);
    void (*zeroCrossings)(void* chartInstance);
    void (*outputs)(void* chartInstance);
    void (*derivatives)(void* chartInstance);
    void (*mdlRTW)(SimStruct* S);
    void (*mdlSetWorkWidths)(SimStruct* S);
    void (*mdlSetupRuntimeResources)(SimStruct* S);
    void (*mdlCleanupRuntimeResources)(void* chartInstance);
    void (*mdlStart)(void* chartInstance);
    void (*mdlTerminate)(void* chartInstance);
    void (*callAtomicSubchartUserFcn)(SimStruct* S, int fcnSSID, void* args);
    void (*callAtomicSubchartAutoFcn)(SimStruct* S, int fcnEnum, void* args);
    void (*callAtomicSubchartEventFcn)(SimStruct* S, int fcnEnum, int eventSSID);
    void (*dispatchToExportedFcn)(void* chartInstance, const char* fcnName, void* args);
} ChartInfoStruct;

LIBMWSF_RUNTIME_API void sf_init_ChartRunTimeInfo(SimStruct* S,
                                                  void* instanceInfo,
                                                  bool isJITEnabled,
                                                  bool hasSeparateUpdateOutputs);
LIBMWSF_RUNTIME_API void sf_free_ChartRunTimeInfo(SimStruct* S);
LIBMWSF_RUNTIME_API void sf_reset_warnings_ChartRunTimeInfo(SimStruct* S);

LIBMWSF_RUNTIME_API ChartInfoStruct* getChartInfoStruct(SimStruct* S);
LIBMWSF_RUNTIME_API void* sfrtGetEmlrtCtxWithEmlrtContext(SimStruct* S,
                                                          void* aSfEmlrtContextGlobal);
LIBMWSF_RUNTIME_API void* sfrtGetEmlrtCtx(SimStruct* S);
LIBMWSF_RUNTIME_API CovrtStateflowInstance* sfrtGetCovrtInstance(SimStruct* S);
LIBMWSF_RUNTIME_API ChartInfoStruct* sf_get_ChartInfoStruct_CodeGen(SimStruct* S);
LIBMWSF_RUNTIME_API void* sf_get_chart_instance_ptr(SimStruct* S);
LIBMWSF_RUNTIME_API bool sf_get_hasSeparateUpdateOutputs(SimStruct* S);
LIBMWSF_RUNTIME_API void sf_create_emlctx(SimStruct* S, void* aRootTLS, void* aContext);
LIBMWSF_RUNTIME_API bool sf_runtime_get_zc_active(SimStruct* S, int idx);
LIBMWSF_RUNTIME_API double* sf_runtime_get_zc_signal_vector(SimStruct* S, int idx);
LIBMWSF_RUNTIME_API void sfSetAnimationVectors(SimStruct* S, uint8_T* states, uint8_T* transitions);
