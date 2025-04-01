/* Copyright 1997-2022 The MathWorks, Inc. */

#ifndef _debugger_codegen_interface_h
#define _debugger_codegen_interface_h

#ifdef SUPPORTS_PRAGMA_ONCE
#pragma once
#endif

#if defined(S_FUNCTION_NAME)
#include "mwmathutil.h"
#endif

#include "sf_runtime/sf_runtime_spec.h"
#ifndef _SIMSTRUCT
#define _SIMSTRUCT
typedef struct SimStruct_tag SimStruct;

#endif

#include "matrix.h"

LIBMWSF_RUNTIME_API void* sfListenerCacheSimStruct(SimStruct* S);
LIBMWSF_RUNTIME_API void* sfListenerInitializeUsingSimStruct(SimStruct* S);

LIBMWSF_RUNTIME_API void sfListenerTerminate(void* rt);
LIBMWSF_RUNTIME_API void sfListenerLightTerminate(void* rt);
LIBMWSF_RUNTIME_API void sfListenerInitializeRuntimeVars(void* rt,
                                                         int* aIsDebuggerActivePtr,
                                                         int* aIsSeqViewerPresentPtr,
                                                         bool isJitCompliantDebuggerOn,
                                                         bool isSequenceViewerPresent,
                                                         uint32_T* lineNumberPtr,
                                                         int* aIsHeatMapPresentPtr,
                                                         bool isHeatMapPresent);
LIBMWSF_RUNTIME_API void sfListenerInitializeIsStmtByStmtModeVar(void* rt, int* isStmtByStmtMode);
LIBMWSF_RUNTIME_API void sfListenerInitializeRuntimeFcnVars(void* rt,
                                                            unsigned int* numFcnVars,
                                                            const char* dataNames[],
                                                            unsigned int dataSsIds[],
                                                            void* dataPtrs[],
                                                            const char* marshallingOutFcnNames[],
                                                            const char* marshallingInFcnNames[],
                                                            unsigned int statuses[]);
LIBMWSF_RUNTIME_API void sfListenerInitializeRuntimeFcnVarsJitOff(void* rt,
                                                                  unsigned int* numFcnVars,
                                                                  const char* dataNames[],
                                                                  unsigned int dataSsIds[],
                                                                  void* dataPtrs[],
                                                                  void* marshallingOutFcns[],
                                                                  void* marshallingInFcns[],
                                                                  unsigned int statuses[]);

LIBMWSF_RUNTIME_API void sfListenerReportScriptName(void* rt,
                                                    char const* aScriptPath,
                                                    char const* className,
                                                    char const* methodName);

LIBMWSF_RUNTIME_API void sfListenerReportStartingSection(void* rt,
                                                         unsigned int aSSIDNumber,
                                                         int aSectionId);
LIBMWSF_RUNTIME_API void sfListenerReportEndingSection(void* rt,
                                                       unsigned int aSSIDNumber,
                                                       int aSectionId);

LIBMWSF_RUNTIME_API void sfListenerPushScopeForSingleVar(void* rt,
                                                         unsigned int aSSIDNumber,
                                                         const char* aVarNames,
                                                         void* aDataPtr,
                                                         const char* aMarchallingOutFunctionName,
                                                         const char* aMarshallingInFunctionName,
                                                         unsigned int aStatus);
LIBMWSF_RUNTIME_API void sfListenerPushScopeForSingleVarJitOff(void* rt,
                                                               unsigned int aSSIDNumber,
                                                               const char* aVarName,
                                                               void* aDataPtr,
                                                               void* aMarshallingOutFunction,
                                                               void* aMarshallingInFunction,
                                                               unsigned int aStatus);

LIBMWSF_RUNTIME_API void sfListenerPushScopeForDynamicMatrices(
    void* rt,
    unsigned int aSSIDNumber,
    const char* aVarName,
    void* aDataPtrOrig,
    const char* aMarshallingOutFunctionName,
    const char* aMarshallingInFunctionName,
    unsigned int aStatus);
LIBMWSF_RUNTIME_API void sfListenerPushScopeForDynamicMatricesJitOff(void* rt,
                                                                     unsigned int aSSIDNumber,
                                                                     const char* aVarName,
                                                                     void* aDataPtrOrig,
                                                                     void* aMarshallingOutFunction,
                                                                     void* aMarshallingInFunction,
                                                                     unsigned int aStatus);

LIBMWSF_RUNTIME_API void sfListenerPushScopeForChart(void* rt,
                                                     unsigned int aSSIDNumber,
                                                     unsigned int N,
                                                     const char* aVarNames[],
                                                     unsigned int aDataSsIds[],
                                                     void* aDataPtr[],
                                                     const char* aMarshallingOutFunctionNames[],
                                                     const char* aMarshallingInFunctionNames[],
                                                     unsigned int aStatus[]);
LIBMWSF_RUNTIME_API void sfListenerPushScopeForChartJitOff(void* rt,
                                                           unsigned int aSSIDNumber,
                                                           unsigned int N,
                                                           const char* aVarNames[],
                                                           unsigned int aDataSsIds[],
                                                           void* aDataPtr[],
                                                           void* aMarshallingOutFunctions[],
                                                           void* aMarshallingInFunctions[],
                                                           unsigned int aStatus[]);

LIBMWSF_RUNTIME_API void sfListenerPushMessagesForChart(void* aRt,
                                                        unsigned int msgSSIDs[],
                                                        unsigned int N,
                                                        const char* msgNames[],
                                                        void* msgInterfaces[],
                                                        void* msgPayloads[]);

LIBMWSF_RUNTIME_API void sfListenerPushActiveStateSelfVars(void* aRt,
                                                           unsigned int N,
                                                           unsigned int ssIds[],
                                                           void* dataPtrs[]);

LIBMWSF_RUNTIME_API void sfListenerPushActiveStateChildVars(void* aRt,
                                                            unsigned int N,
                                                            unsigned int ssIds[],
                                                            void* dataPtrs[]);

LIBMWSF_RUNTIME_API void sfListenerPushActiveStateLeafVars(void* aRt,
                                                           unsigned int N,
                                                           unsigned int ssIds[],
                                                           void* dataPtrs[]);

LIBMWSF_RUNTIME_API void sfListenerPushScopeForDynamicMatricesForLegacyVars(
    void* rt,
    unsigned int aSSIDNumber,
    const char* aVarName,
    void* aDataPtr,
    void* aSize,
    const char* aMarshallingOutFunctionName,
    const char* aMarshallingInFunctionName,
    unsigned int aStatus);
LIBMWSF_RUNTIME_API void sfListenerPushScopeForDynamicMatricesForLegacyVarsJitOff(
    void* rt,
    unsigned int aSSIDNumber,
    const char* aVarName,
    void* aDataPtr,
    void* aSize,
    void* aMarshallingOutFunction,
    void* aMarshallingInFunction,
    unsigned int aStatus);

LIBMWSF_RUNTIME_API void sfListenerReportStartingEventBroadcast(void* rt,
                                                                unsigned int aEventSSIDNumber,
                                                                int aSectionId);
LIBMWSF_RUNTIME_API void sfListenerReportEndingEventBroadcast(void* rt,
                                                              unsigned int aEventSSIDNumber,
                                                              int aSectionId);
LIBMWSF_RUNTIME_API void sfListenerReportLineNumber(void* rt,
                                                    unsigned int aSSIDNumber,
                                                    int aLineNumber);
LIBMWSF_RUNTIME_API void sfListenerReportOffsetLength(void* rt,
                                                      unsigned int aSSIDNumber,
                                                      int aOffset,
                                                      int aLength);

LIBMWSF_RUNTIME_API void sfListenerRegisterHover(void* aRt, void* aFcnPtr);

LIBMWSF_RUNTIME_API void sfListenerRemoveChartMCOS(void* runtimeVar, const char* machineName);
LIBMWSF_RUNTIME_API mxArray* sfListenerGetChartMCOS(void* runtimeVar);
LIBMWSF_RUNTIME_API void sfListenerCreateChartMCOS(void* runtimeVar,
                                                   const char* machineName,
                                                   const char* chartName,
                                                   unsigned int filteredObjects[],
                                                   unsigned int filteredObjectSize);

LIBMWSF_RUNTIME_API void sfAppendHoverData(mxArray** target,
                                           mxArray* rawDataToAppend,
                                           unsigned int ssid,
                                           const char* key,
                                           const char* varName,
                                           unsigned int srcSSIDNumber,
                                           int labelStringStartPosition,
                                           int labelStringEndPosition,
                                           int labelStringArgStartPosition,
                                           int labelStringArgEndPosition);

LIBMWSF_RUNTIME_API void* sfMarshallInSLString(const mxArray* mxArrayInput);
LIBMWSF_RUNTIME_API void* sfMarshallOutSLString(const void* slString);
LIBMWSF_RUNTIME_API bool sfDebugCheckFixedPointLicense(void);
LIBMWSF_RUNTIME_API void sfListenerReportChartEnableDisable(void* aRt,
                                                            real_T* aTimeVar,
                                                            int aSectionId);

#endif
