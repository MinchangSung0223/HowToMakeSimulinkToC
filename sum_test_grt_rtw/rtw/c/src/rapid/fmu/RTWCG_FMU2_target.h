/* Copyright 2017-2022 The MathWorks, Inc. */
/*
 * @file: RTWCG_FMU2_target.h
 *  
 * @brief fmustruct 
 *
 * Abstract:
 *      FMI 2.0 data types, function signatures and FMU2_CS/ME_RTWCG(struct)
 *
 *      FMU2_CS/ME_RTWCG is a data struct to store FMU info, handle all 
 *      dynamic function calls to FMU lib 
 *      
 * =============================================================================
 */
#ifndef RTWCG_FMU2_GUARD
#define RTWCG_FMU2_GUARD

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
#include "fmi2Functions.h"

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

#ifndef getgblSetParamPktReceived_GUARD
#define getgblSetParamPktReceived_GUARD
#define getgblSetParamPktReceived(ptr)       {  \
    extern boolean_T   gblSetParamPktReceived;  \
    *ptr = gblSetParamPktReceived;              \
    }
#endif

#ifndef isSimTargetSimulating_GUARD
#define isSimTargetSimulating_GUARD
#define isSimTargetSimulating(S)                \
    ssGetSimMode((SimStruct*)(S))==SS_SIMMODE_NORMAL || ssRTWGenIsAccelerator((SimStruct*)(S))
#endif

#ifndef ssGetMdlSfuncS_GUARD
#define ssGetMdlSfuncS_GUARD
#define ssGetMdlSfuncS(S)                \
    (S)->_mdlRefSfcnS
#endif

typedef fmi2Status (*_fmi2_default_status_fcn_type) (fmi2Component, ...);
typedef const char* (*_fmi2_default_string_fcn_type) (void);
typedef void* (*_fmi2_default_instance_fcn_type) (const char*, ...);
typedef void (*_fmi2_default_void_fcn_type) (void*);
    
struct FMU2_CSME_RTWCG {
    /*common functions*/
    fmi2GetTypesPlatformTYPE* getTypesPlatform;
    fmi2GetVersionTYPE* getVersion;
    fmi2SetDebugLoggingTYPE* setDebugLogging;
    fmi2InstantiateTYPE* instantiate;
    fmi2FreeInstanceTYPE* freeInstance;
    fmi2SetupExperimentTYPE* setupExperiment;
    fmi2EnterInitializationModeTYPE* enterInitializationMode;
    fmi2ExitInitializationModeTYPE* exitInitializationMode;
    fmi2TerminateTYPE* terminate;
    fmi2ResetTYPE* reset;
    fmi2GetRealTYPE* getReal;
    fmi2GetIntegerTYPE* getInteger;
    fmi2GetBooleanTYPE* getBoolean;
    fmi2GetStringTYPE* getString;
    fmi2SetRealTYPE* setReal;
    fmi2SetIntegerTYPE* setInteger;
    fmi2SetBooleanTYPE* setBoolean;
    fmi2SetStringTYPE* setString;
    fmi2GetFMUstateTYPE* getFMUstate;
    fmi2SetFMUstateTYPE* setFMUstate;
    fmi2FreeFMUstateTYPE* freeFMUstate;
    fmi2SerializedFMUstateSizeTYPE* serializedFMUstateSize;
    fmi2SerializeFMUstateTYPE* serializeFMUstate;
    fmi2DeSerializeFMUstateTYPE* deSerializeFMUstate;
    fmi2GetDirectionalDerivativeTYPE* getDirectionalDerivative;

    /*fmi cs functions*/
    fmi2SetRealInputDerivativesTYPE* setRealInputDerivatives;
    fmi2GetRealOutputDerivativesTYPE* getRealOutputDerivatives;
    fmi2DoStepTYPE* doStep;
    fmi2CancelStepTYPE* cancelStep;
    fmi2GetStatusTYPE* getStatus;
    fmi2GetRealStatusTYPE* getRealStatus;
    fmi2GetIntegerStatusTYPE* getIntegerStatus;
    fmi2GetBooleanStatusTYPE* getBooleanStatus;
    fmi2GetStringStatusTYPE* getStringStatus;

    /*fmi me functions*/
    fmi2EnterEventModeTYPE* enterEventMode;
    fmi2NewDiscreteStatesTYPE* newDiscreteStates;
    fmi2EnterContinuousTimeModeTYPE* enterContinuousTimeMode;
    fmi2CompletedIntegratorStepTYPE* completedIntegratorStep;
    fmi2SetTimeTYPE* setTime;
    fmi2SetContinuousStatesTYPE* setContinuousStates;
    fmi2GetDerivativesTYPE* getDerivatives;
    fmi2GetEventIndicatorsTYPE* getEventIndicators;
    fmi2GetContinuousStatesTYPE* getContinuousStates;
    fmi2GetNominalsOfContinuousStatesTYPE* getNominalsOfContinuousStates;

    char* fmuname;
    char* dllfile;
    FMUHANDLE Handle;
    fmi2Component mFMIComp;
#if FMU_CG_TARGET < FMUCG_STANDALONE_TARGETS
    SimStruct *ssPtr;
#endif
    fmi2Status FMUErrorStatus;
    fmi2EventInfo eventInfo;
    fmi2CallbackFunctions callbacks;
    bool modelInitialized;
    bool isLoggingOn;
    
    /*two int arrays for maping enum param original value to actual value*/
    int* paramIdxToOffset;
    int* enumValueList;
};

/* RTWCG entry points for FMU2 */
void* FMU2_fmuInitializeCS(const char* lib,
                           fmi2String instanceName,
                           fmi2String fmuGUID,
                           fmi2String fmuLocation,
                           void* ssPtr,
                           int loadFMUStateFcn,
                           int loadSerializationFcn,
                           int loadDirectionDerivativeFcn,
                           const void* fmuCallBacks,
                           fmi2Boolean fmuVisible,
                           fmi2Boolean fmuLoggingOn);

fmi2Boolean FMU2_setupExperiment(void** fmuv,
                                fmi2Boolean isToleranceUsed,
                                fmi2Real toleranceValue,
                                fmi2Real currentTime,
                                fmi2Boolean isTFinalUsed,
                                fmi2Real TFinal);

fmi2Boolean FMU2_doStep(void **fmuv,
                        double currentCommunicationPoint,
                        double communicationStepSize,
                        fmi2Boolean noSetFMUStatePriorToCurrentPoint);

fmi2Boolean FMU2_terminate(void **fmuv);

fmi2Boolean FMU2_enterInitializationMode(void **fmuv);

fmi2Boolean FMU2_exitInitializationMode(void **fmuv);

fmi2Boolean FMU2_setRealVal(void **fmuv,
                           const fmi2ValueReference dvr,
                           size_t nvr,
                           const fmi2Real dvalue);
////////////////////////////////////////////////////////////////////down
fmi2Boolean FMU2_setRealPtr(void **fmuv,
                            const fmi2ValueReference dvr,
                            size_t nvr,
                            const fmi2Real value[]);
fmi2Boolean FMU2_setIntegerPtr(void **fmuv,
                           const fmi2ValueReference dvr,
                           size_t nvr,
                            const fmi2Integer value[]);
fmi2Boolean FMU2_setBooleanPtr(void **fmuv,
                           const fmi2ValueReference dvr,
                           size_t nvr,
                           const fmi2Boolean value[]);
fmi2Boolean FMU2_setStringPtr(void **fmuv,
                          const fmi2ValueReference dvr,
                          size_t nvr,
                              const fmi2String value[]);
    fmi2Boolean FMU2_getRealPtr(void **fmuv,
                            const fmi2ValueReference dvr,
                            size_t nvr,
                                fmi2Real value[]);
    fmi2Boolean FMU2_getIntegerPtr(void **fmuv,
                               const fmi2ValueReference dvr,
                               size_t nvr,
                                   fmi2Integer value[]);
    fmi2Boolean FMU2_getBooleanPtr(void **fmuv,
                               const fmi2ValueReference dvr,
                               size_t nvr,
                                   fmi2Boolean value[]);
    fmi2Boolean FMU2_getStringPtr(void **fmuv,
                              const fmi2ValueReference dvr,
                              size_t nvr,
                                  fmi2String value[]);
////////////////////////////////////////////////////////////////////up
    
fmi2Boolean FMU2_setReal(void **fmuv,
                        const fmi2ValueReference vr[],
                        size_t nvr,
                        const fmi2Real value[]);

fmi2Boolean FMU2_getReal(void **fmuv,
                        const fmi2ValueReference vr[],
                        size_t nvr,
                        fmi2Real value[]);

fmi2Boolean FMU2_setIntegerVal(void **fmuv,
                              const fmi2ValueReference dvr,
                              size_t nvr,
                              const fmi2Integer dvalue);

fmi2Boolean FMU2_setInteger(void **fmuv,
                           const fmi2ValueReference vr[],
                           size_t nvr,
                           const fmi2Integer value[]);

fmi2Boolean FMU2_getInteger(void **fmuv,
                           const fmi2ValueReference vr[],
                           size_t nvr,
                           fmi2Integer value[]);

fmi2Boolean FMU2_setBooleanVal(void **fmuv,                              
                              const fmi2ValueReference dvr,
                              size_t nvr,
                              const fmi2Boolean dvalue);

fmi2Boolean FMU2_setBoolean(void **fmuv,
                           const fmi2ValueReference vr[],
                           size_t nvr,
                           const fmi2Boolean value[]);

fmi2Boolean FMU2_getBoolean(void **fmuv,
                           const fmi2ValueReference vr[],
                           size_t nvr,
                           fmi2Boolean value[]);

fmi2Boolean FMU2_setString(void **fmuv,
                          const fmi2ValueReference vr[],
                          size_t nvr,
                          void* value[]);

fmi2Boolean FMU2_setStringVal(void **fmuv,
                             const fmi2ValueReference dvr,
                             size_t nvr,
                             const fmi2String);

fmi2Boolean FMU2_getString(void **fmuv,
                          const fmi2ValueReference vr[],
                          size_t nvr,
                          void* value[]);
    
void FMU2_strncpy(char* dest, void* src, int size);

char* FMU2_strndup(const char* src, int size);
    
void FMU2_simTerminate(void **fmuv,
                       const char* blkPath,
                       fmi2Real time);

void FMU2_terminateModel(void **fmuv);

void FMU2_getNextEventTime(void **fmuv,
                           fmi2Real* nextEventTime,
                           int32_T* upcomingTimeEvent);

void * FMU2_fmuInitializeME(const char * lib,
                            fmi2String instanceName,
                            fmi2String fmuGUID,
                            fmi2String fmuLocation,
                            void* ssPtr,
                            int loadFMUStateFcn,
                            int loadSerializationFcn,
                            int loadDirectionDerivativeFcn,
                            const void* fmuCallBacks,
                            fmi2Boolean fmuVisible,
                            fmi2Boolean fmuLoggingOn);

fmi2Boolean FMU2_eventIteration(void **fmuv,
                                const char* blkPath,
                                fmi2Real time);

void FMU2_valuesOfContinuousStatesChanged(void **fmuv,
                                          int* stateChanged);

fmi2Boolean FMU2_enterEventMode(void** fmuv);

fmi2Boolean FMU2_enterContinuousTimeMode(void** fmuv);

fmi2Boolean FMU2_completedIntegratorStep(void** fmuv,
                                        fmi2Boolean,
                                        fmi2Boolean*,
                                        fmi2Boolean*);

fmi2Boolean FMU2_setTime(void** fmuv,
                        fmi2Real);

fmi2Boolean FMU2_setContinuousStates(void** fmuv,
                                    const fmi2Real[],
                                    size_t);

fmi2Boolean FMU2_getDerivatives(void** fmuv,                               
                               fmi2Real[],
                               size_t);

fmi2Boolean FMU2_getEventIndicators(void** fmuv,
                                   fmi2Real[],
                                   size_t);

fmi2Boolean FMU2_getContinuousStates(void** fmuv,
                                    fmi2Real[],
                                    size_t);

fmi2Boolean FMU2_getNominalsOfContinuousStates(void** fmuv,
                                              fmi2Real[],
                                              size_t);

void FMU2_setFMUState(void** fmuv, void* fmustate);
void FMU2_getFMUState(void** fmuv, void** fmustate);
void FMU2_freeFMUState(void** fmuv, void** fmustate);
void FMU2_serializedFMUstateSize(void** fmuv, void* fmustate, size_t* size);
void FMU2_serializeFMUstate(void** fmuv, void* fmustate, char* byteArray);
void FMU2_deserializeFMUstate(void** fmuv, char* byteArray, void** fmustate);

/*helper to preprocess Enum type*/
void createParamIdxToOffset(void** fmuv,
                            int array_size);

void createEnumValueList(void** fmuv,
                         int array_size);

void setParamIdxToOffsetByIdx(void** fmuv,
                              int idx,
                              int value);

void setEnumValueListByIdx(void** fmuv,
                           int idx,
                           int value);

void getParamIdxToOffsetByIdx(void** fmuv,
                              int idx,
                              int* offset);

void getEnumValueByIdx(void** fmuv,
                       int idx,
                       int* val);

#ifdef __cplusplus
}
#endif

#endif
