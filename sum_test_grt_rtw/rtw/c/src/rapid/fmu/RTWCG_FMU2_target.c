/* Copyright 2017-2023 The MathWorks, Inc. */

/******************************************************************************
 *                                                                            *
 * File    : RTWCG_FMU2_target.c                                              *
 * Abstract:                                                                  *
 *      Wrapper functions to dynamic library calls of FMU 2.0                 *
 *      This file defines functions called by code generation                 *
 *      Also handle errors, and logger                                        *
 *                                                                            *
 ******************************************************************************/

#include "RTWCG_FMU2_target.h"
#include <stdlib.h>
#define FMU2_MESSAGE_SIZE 1024

#if FMU_CG_TARGET == FMUCG_SLRT
#include "SLRTLoggerWrapper.hpp"
#endif
/*
  Whenever a default function is called, it means a functions is called without successful load,
  return a fmi2Error;
*/
static fmi2Status defaultfcnStatus(fmi2Component c, ...) {
    (void)c;
    return fmi2Error;
}
static const char* defaultfcnString(void) {
    return NULL;
}
static void* defaultfcnInstance(const char* c,...) {
    (void)c;
    return NULL;
}
static void defaultfcnVoid(void* c) {
    (void)c;
    return;
}

static fmi2String formatString(fmi2String format, ...) {
    static char translatedMsg[FMU2_MESSAGE_SIZE];
    va_list args;
    va_start(args, format);
    vsnprintf(translatedMsg, FMU2_MESSAGE_SIZE - 1, format, args);
    va_end(args);
    return translatedMsg;
}

static const char* getLoadLibraryErrorMessage(void)
{
    static char message[FMU2_MESSAGE_SIZE];
#ifdef _WIN32
    unsigned long int errorCode = GetLastError();
    snprintf(message, FMU2_MESSAGE_SIZE, "Load Library Error: '%d' ", errorCode); 
#else
    const char* errorMessage = dlerror();
    snprintf(message, FMU2_MESSAGE_SIZE, "Load Library Error: '%s' ", errorMessage); 
#endif
    return message;
}

/* todo: translate value reference to names */
static void fmu2Logger(fmi2ComponentEnvironment c,
                       fmi2String instanceName,
                       fmi2Status status,
                       fmi2String category,
                       fmi2String message, ...) {
    
    static const char* strStatus[] = {
        "fmi2OK", "fmi2Warning", "fmi2Discard", "fmi2Error", "fmi2Fatal", "fmi2Pending" };
    
    struct FMU2_CSME_RTWCG * fmustruct = (struct FMU2_CSME_RTWCG *)c;
    if (!fmustruct || !fmustruct->isLoggingOn) return;
    
    char translatedMsg[FMU2_MESSAGE_SIZE];
    char temp[FMU2_MESSAGE_SIZE];    
    int prefixLength = snprintf(translatedMsg, FMU2_MESSAGE_SIZE, "Log from FMU: [category:%s, status:%s] ",
                            strStatus[status], category); 
    va_list args;
    va_start (args, message);
    vsnprintf(temp, FMU2_MESSAGE_SIZE, message, args);
    va_end(args);
    strncat(translatedMsg, temp, FMU2_MESSAGE_SIZE-prefixLength - 1);
    (void)instanceName;

#if FMU_CG_TARGET < FMUCG_STANDALONE_TARGETS
    SimStruct* ssPtr = (SimStruct*)fmustruct->ssPtr;    
    void *diagnostic = CreateDiagnosticAsVoidPtr("SL_SERVICES:utils:PRINTFWRAPPER", 1,
                                           CODEGEN_SUPPORT_ARG_STRING_TYPE, translatedMsg);
    rt_ssReportDiagnosticAsInfo(ssPtr, diagnostic);
#elif FMU_CG_TARGET == FMUCG_SLRT
    /* Skip printing info logs which are shown by fmiOK status*/
    if(status != fmi2OK) {
           switch (status){
        case fmi2Error:
            SLRTLog(kError,translatedMsg);
            break;
        case fmi2Fatal:
            SLRTLog(kFatal,translatedMsg);
            break;
        case fmi2Warning:
        default:
            SLRTLog(kWarning,translatedMsg);
            break;
        }
    }
#else
    printf("%s\n", translatedMsg);
#endif
}

#ifdef _WIN32
static FMUHANDLE loadLibraryUTF8toUTF16(const char* library_loc)
{
    int convertResult = MultiByteToWideChar(CP_UTF8, 0, library_loc, (int)strlen(library_loc), NULL, 0);
    if (convertResult <= 0) {
        return NULL;
    }
    wchar_t* wLibrary_loc = (wchar_t*)calloc((convertResult + 1), sizeof(wchar_t));
    MultiByteToWideChar(CP_UTF8, 0, library_loc, (int)strlen(library_loc), wLibrary_loc, convertResult);
    FMUHANDLE handle = LoadLibraryExW(wLibrary_loc, NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
    free(wLibrary_loc);
    return handle;
}
#endif

#define LoadFMUFcnDecl1(f, x)                                           \
    static x f(struct FMU2_CSME_RTWCG* fmustruct,                       \
               const char * fcnName,                                    \
               int isOptional) {                                        \
    x fcn = NULL;                                                       \
    fmi2String message = NULL;

#if FMU_CG_TARGET < FMUCG_STANDALONE_TARGETS
#define LoadFMUFcnDecl2                         \
    (void) message;
#else
#define LoadFMUFcnDecl2
#endif

#ifdef _WIN32

#define LoadFMUFcnDecl3(x)                                              \
    fcn = (x)LOAD_FUNCTION(fmustruct->Handle, fcnName);

#else

#if FMU_CG_TARGET == FMUCG_SLRT

#define LoadFMUFcnDecl3(x)                                              \
    /* creating a char array "fmuname_functionName" as the name to fetch from .so file. */ \
    char fullFcnName[FULL_FCN_NAME_MAX_LEN];                            \
    memset(fullFcnName, 0, FULL_FCN_NAME_MAX_LEN);                      \
    strncpy(fullFcnName, fmustruct->fmuname, FULL_FCN_NAME_MAX_LEN - 1); \
    strncat(fullFcnName, "_", FULL_FCN_NAME_MAX_LEN - strlen(fullFcnName) - 1); \
    strncat(fullFcnName, fcnName, FULL_FCN_NAME_MAX_LEN - strlen(fullFcnName) - 1); \
    *((void **)(&fcn)) = LOAD_FUNCTION(fmustruct->Handle, fullFcnName);

#else

#define LoadFMUFcnDecl3(x)                                              \
    *((void **)(&fcn)) = LOAD_FUNCTION(fmustruct->Handle, fcnName);

#endif
#endif

#define LoadFMUFcnDecl4                         \
    if (!isOptional && fcn == NULL) {           \
    fmustruct->FMUErrorStatus = fmi2Warning;

#if FMU_CG_TARGET < FMUCG_STANDALONE_TARGETS

#define LoadFMUFcnDecl5                                                 \
    if (fmustruct->isLoggingOn) {                                       \
        void * diagnostic = CreateDiagnosticAsVoidPtr("FMUBlock:FMU:FMULoadLibFunctionError", 2, \
                                                      CODEGEN_SUPPORT_ARG_STRING_TYPE, fcnName, \
                                                      CODEGEN_SUPPORT_ARG_STRING_TYPE, fmustruct->fmuname); \
        /*Loading failure will cause a warning, ANY CALL to default fcn will result in fmiError and stop*/ \
        rt_ssReportDiagnosticAsWarning(fmustruct->ssPtr, diagnostic);   \
    }

#else

#define LoadFMUFcnDecl5                                                 \
    message = formatString("Unable to load function '%s' for FMU '%s'.", fcnName, fmustruct->fmuname); \
    fmustruct->callbacks.logger(fmustruct->callbacks.componentEnvironment, fmustruct->fmuname, fmustruct->FMUErrorStatus, fcnName, message);

#endif

#define LoadFMUFcnDecl6(x, d)                                   \
    fcn = (x) d;                                                \
    }                                                           \
    return fcn;                                                 \
    }

#define LoadFMUFcnDecl(f, x, d)                                         \
    LoadFMUFcnDecl1(f, x) LoadFMUFcnDecl2 LoadFMUFcnDecl3(x) LoadFMUFcnDecl4 LoadFMUFcnDecl5 LoadFMUFcnDecl6(x, d) 

LoadFMUFcnDecl(LoadFMUFcnStatus, _fmi2_default_status_fcn_type, defaultfcnStatus)
LoadFMUFcnDecl(LoadFMUFcnString, _fmi2_default_string_fcn_type, defaultfcnString)
LoadFMUFcnDecl(LoadFMUFcnInstance, _fmi2_default_instance_fcn_type, defaultfcnInstance)
LoadFMUFcnDecl(LoadFMUFcnVoid, _fmi2_default_void_fcn_type, defaultfcnVoid)

/*Load FMU 2 Common Functions*/
static void LoadFMU2CommonFunctions(struct FMU2_CSME_RTWCG* fmustruct,
                                    int loadFMUStateFcn,
                                    int loadSerializationFcn,
                                    int loadDirectionDerivativeFcn) {
    fmustruct->getTypesPlatform   = (fmi2GetTypesPlatformTYPE*)      LoadFMUFcnString(fmustruct, "fmi2GetTypesPlatform", 0);
    fmustruct->getVersion         = (fmi2GetVersionTYPE*)            LoadFMUFcnString(fmustruct, "fmi2GetVersion", 0);
    fmustruct->setDebugLogging    = (fmi2SetDebugLoggingTYPE*)       LoadFMUFcnStatus(fmustruct, "fmi2SetDebugLogging", 0);
    fmustruct->instantiate        = (fmi2InstantiateTYPE*)           LoadFMUFcnInstance(fmustruct, "fmi2Instantiate", 0);
    fmustruct->freeInstance       = (fmi2FreeInstanceTYPE*)          LoadFMUFcnVoid(fmustruct, "fmi2FreeInstance", 0);
    fmustruct->setupExperiment    = (fmi2SetupExperimentTYPE*)       LoadFMUFcnStatus(fmustruct, "fmi2SetupExperiment", 0);
    fmustruct->enterInitializationMode  = (fmi2EnterInitializationModeTYPE*)    LoadFMUFcnStatus(fmustruct, "fmi2EnterInitializationMode", 0);
    fmustruct->exitInitializationMode   = (fmi2ExitInitializationModeTYPE*)     LoadFMUFcnStatus(fmustruct, "fmi2ExitInitializationMode", 0);
    fmustruct->terminate          = (fmi2TerminateTYPE*)             LoadFMUFcnStatus(fmustruct, "fmi2Terminate", 0);
    fmustruct->reset              = (fmi2ResetTYPE*)                 LoadFMUFcnStatus(fmustruct, "fmi2Reset", 0);
    
    fmustruct->setReal            = (fmi2SetRealTYPE*)               LoadFMUFcnStatus(fmustruct, "fmi2SetReal", 0);
    fmustruct->setInteger         = (fmi2SetIntegerTYPE*)            LoadFMUFcnStatus(fmustruct, "fmi2SetInteger", 0);
    fmustruct->setBoolean         = (fmi2SetBooleanTYPE*)            LoadFMUFcnStatus(fmustruct, "fmi2SetBoolean", 0);
    fmustruct->setString          = (fmi2SetStringTYPE*)             LoadFMUFcnStatus(fmustruct, "fmi2SetString", 0);
    fmustruct->getReal            = (fmi2GetRealTYPE*)               LoadFMUFcnStatus(fmustruct, "fmi2GetReal", 0);
    fmustruct->getInteger         = (fmi2GetIntegerTYPE*)            LoadFMUFcnStatus(fmustruct, "fmi2GetInteger", 0);
    fmustruct->getBoolean         = (fmi2GetBooleanTYPE*)            LoadFMUFcnStatus(fmustruct, "fmi2GetBoolean", 0);
    fmustruct->getString          = (fmi2GetStringTYPE*)             LoadFMUFcnStatus(fmustruct, "fmi2GetString", 0);

    fmustruct->getFMUstate        = (fmi2GetFMUstateTYPE*)           LoadFMUFcnStatus(fmustruct, "fmi2GetFMUstate", !loadFMUStateFcn);
    fmustruct->setFMUstate        = (fmi2SetFMUstateTYPE*)           LoadFMUFcnStatus(fmustruct, "fmi2SetFMUstate", !loadFMUStateFcn);
    fmustruct->freeFMUstate       = (fmi2FreeFMUstateTYPE*)          LoadFMUFcnStatus(fmustruct, "fmi2FreeFMUstate", !loadFMUStateFcn);
    fmustruct->serializedFMUstateSize    = (fmi2SerializedFMUstateSizeTYPE*)    LoadFMUFcnStatus(fmustruct, "fmi2SerializedFMUstateSize", !loadSerializationFcn);
    fmustruct->serializeFMUstate         = (fmi2SerializeFMUstateTYPE*)          LoadFMUFcnStatus(fmustruct, "fmi2SerializeFMUstate", !loadSerializationFcn);
    fmustruct->deSerializeFMUstate       = (fmi2DeSerializeFMUstateTYPE*)        LoadFMUFcnStatus(fmustruct, "fmi2DeSerializeFMUstate", !loadSerializationFcn);
    fmustruct->getDirectionalDerivative  = (fmi2GetDirectionalDerivativeTYPE*)   LoadFMUFcnStatus(fmustruct, "fmi2GetDirectionalDerivative", !loadDirectionDerivativeFcn);
}

/*Load FMU 2 CS Functions*/
static void LoadFMU2CSFunctions(struct FMU2_CSME_RTWCG* fmustruct) {
    fmustruct->setRealInputDerivatives = (fmi2SetRealInputDerivativesTYPE*)   LoadFMUFcnStatus(fmustruct, "fmi2SetRealInputDerivatives", 0);
    fmustruct->getRealOutputDerivatives = (fmi2GetRealOutputDerivativesTYPE*) LoadFMUFcnStatus(fmustruct, "fmi2GetRealOutputDerivatives", 0);
    fmustruct->cancelStep          = (fmi2CancelStepTYPE*)            LoadFMUFcnStatus(fmustruct, "fmi2CancelStep", 0);
    fmustruct->doStep              = (fmi2DoStepTYPE*)                LoadFMUFcnStatus(fmustruct, "fmi2DoStep", 0);
    fmustruct->getStatus           = (fmi2GetStatusTYPE*)             LoadFMUFcnStatus(fmustruct, "fmi2GetStatus", 0);
    fmustruct->getRealStatus       = (fmi2GetRealStatusTYPE*)         LoadFMUFcnStatus(fmustruct, "fmi2GetRealStatus", 0);
    fmustruct->getIntegerStatus    = (fmi2GetIntegerStatusTYPE*)      LoadFMUFcnStatus(fmustruct, "fmi2GetIntegerStatus", 0);
    fmustruct->getBooleanStatus    = (fmi2GetBooleanStatusTYPE*)      LoadFMUFcnStatus(fmustruct, "fmi2GetBooleanStatus", 0);
    fmustruct->getStringStatus     = (fmi2GetStringStatusTYPE*)       LoadFMUFcnStatus(fmustruct, "fmi2GetStringStatus", 0);
}

/*Load FMU 2 ME Functions*/
static void LoadFMU2MEFunctions(struct FMU2_CSME_RTWCG* fmustruct) {
    fmustruct->enterEventMode    = (fmi2EnterEventModeTYPE*) LoadFMUFcnStatus(fmustruct, "fmi2EnterEventMode", 0);
    fmustruct->newDiscreteStates = (fmi2NewDiscreteStatesTYPE*) LoadFMUFcnStatus(fmustruct, "fmi2NewDiscreteStates", 0);
    fmustruct->enterContinuousTimeMode = (fmi2EnterContinuousTimeModeTYPE*) LoadFMUFcnStatus(fmustruct, "fmi2EnterContinuousTimeMode", 0);
    fmustruct->completedIntegratorStep = (fmi2CompletedIntegratorStepTYPE*) LoadFMUFcnStatus(fmustruct, "fmi2CompletedIntegratorStep", 0);
    fmustruct->setTime             = (fmi2SetTimeTYPE*) LoadFMUFcnStatus(fmustruct, "fmi2SetTime", 0);
    fmustruct->setContinuousStates = (fmi2SetContinuousStatesTYPE*) LoadFMUFcnStatus(fmustruct,"fmi2SetContinuousStates", 0);
    fmustruct->getDerivatives      = (fmi2GetDerivativesTYPE*) LoadFMUFcnStatus(fmustruct, "fmi2GetDerivatives", 0);
    fmustruct->getEventIndicators  = (fmi2GetEventIndicatorsTYPE*) LoadFMUFcnStatus(fmustruct, "fmi2GetEventIndicators", 0);
    fmustruct->getContinuousStates = (fmi2GetContinuousStatesTYPE*) LoadFMUFcnStatus(fmustruct,"fmi2GetContinuousStates", 0);
    fmustruct->getNominalsOfContinuousStates = (fmi2GetNominalsOfContinuousStatesTYPE*) LoadFMUFcnStatus(fmustruct, "fmi2GetNominalsOfContinuousStates", 0);
}

static fmi2Boolean CheckStatus(struct FMU2_CSME_RTWCG* fmustruct,
                               fmi2Status status,
                               fmi2String fcnName) {
    
#if FMU_CG_TARGET < FMUCG_STANDALONE_TARGETS   
    SimStruct* ss = fmustruct->ssPtr;
    
    if(status == fmi2Error || status == fmi2Fatal){
        if (fmustruct->isLoggingOn) {
            void * diagnostic = CreateDiagnosticAsVoidPtr("FMUBlock:FMU:FMUSimFunctionErrorDebugToDisplayOn", 2,
                                                          CODEGEN_SUPPORT_ARG_STRING_TYPE, fcnName,
                                                          CODEGEN_SUPPORT_ARG_STRING_TYPE, fmustruct->fmuname);
            rt_ssSet_slErrMsg(ss, diagnostic);
        }
        fmustruct->FMUErrorStatus = status;
        ssSetStopRequested(ss, 1);        
    }else if(status == fmi2Pending){
        if (fmustruct->isLoggingOn) {
            void * diagnostic = CreateDiagnosticAsVoidPtr("FMUBlock:FMU:FMUSimPendingNotAllowed", 2,
                                                          CODEGEN_SUPPORT_ARG_STRING_TYPE, fcnName,
                                                          CODEGEN_SUPPORT_ARG_STRING_TYPE, fmustruct->fmuname);
            rt_ssSet_slErrMsg(ss, diagnostic);
        }
        fmustruct->FMUErrorStatus = status;
        ssSetStopRequested(ss, 1);       
    }
#else
    fmi2String message = NULL;
    if(status == fmi2Error || status == fmi2Fatal){
        fmustruct->FMUErrorStatus = status;
        message = formatString("Error in supplied FMU: An error occurred in function '%s' for block '%s' during simulation.", fcnName, fmustruct->fmuname);
        fmustruct->callbacks.logger(fmustruct->callbacks.componentEnvironment, fmustruct->fmuname, fmustruct->FMUErrorStatus, fcnName, message);
    }else if(status == fmi2Pending){
        fmustruct->FMUErrorStatus = status;
        message = formatString("Function '%s' in block '%s' returned 'fmiPending' flag during simulation.", fcnName, fmustruct->fmuname);
        fmustruct->callbacks.logger(fmustruct->callbacks.componentEnvironment, fmustruct->fmuname, fmustruct->FMUErrorStatus, fcnName, message);
    }
#endif
    
    if (status == fmi2OK)
        return fmi2True;
    else
        return fmi2False;
}

/* fmuPrefix is instanceName */
static void* FMU2_fmuInitialize(const char* lib,
                                fmi2String instanceName,
                                fmi2String fmuGUID,
                                fmi2String fmuLocation,
                                fmi2Type fmuType,
                                void* ssPtr,
                                int loadFMUStateFcn,
                                int loadSerializationFcn,
                                int loadDirectionDerivativeFcn,
                                const void* fmuCallBacks,
                                fmi2Boolean fmuVisible,
                                fmi2Boolean fmuLoggingOn) {

    struct FMU2_CSME_RTWCG * fmustruct;
    
    char* fmuResLocation;
    fmi2String message = NULL;
    
    fmi2CallbackAllocateMemory allocateMemory = (fmuCallBacks == NULL) ? calloc : ((const fmi2CallbackFunctions*)(fmuCallBacks))->allocateMemory;
    fmustruct = (struct FMU2_CSME_RTWCG *)allocateMemory(1, sizeof(struct FMU2_CSME_RTWCG));
    //fmi callback functions
    fmustruct->callbacks.logger = (fmuCallBacks == NULL) ? fmu2Logger : ((const fmi2CallbackFunctions*)(fmuCallBacks))->logger;
    fmustruct->callbacks.allocateMemory = allocateMemory;
    fmustruct->callbacks.freeMemory = (fmuCallBacks == NULL) ? free : ((const fmi2CallbackFunctions*)(fmuCallBacks))->freeMemory;
    fmustruct->callbacks.stepFinished = (fmuCallBacks == NULL) ? NULL : ((const fmi2CallbackFunctions*)(fmuCallBacks))->stepFinished;
    fmustruct->callbacks.componentEnvironment = (fmuCallBacks == NULL) ? fmustruct : ((const fmi2CallbackFunctions*)(fmuCallBacks))->componentEnvironment; /* used in logger */

    //fmi parameters
    fmi2Boolean visible = fmuVisible;
    fmi2Boolean isLoggingOn = (fmustruct->callbacks.logger == NULL) ? fmi2False : fmuLoggingOn;
    
#if FMU_CG_TARGET < FMUCG_STANDALONE_TARGETS
    fmustruct->ssPtr =  (SimStruct*)ssPtr;
    (void) message;
#else
    (void) ssPtr;
#endif
    fmustruct->fmuname = (char *)instanceName;
    fmustruct->dllfile = (char *)lib;
    fmuResLocation = (char*) fmuLocation;
    fmustruct->FMUErrorStatus = fmi2OK;
    fmustruct->modelInitialized = false;
    fmustruct->isLoggingOn = isLoggingOn;
    
    if (strlen(instanceName)+ FCN_NAME_MAX_LEN + 1 >= FULL_FCN_NAME_MAX_LEN) {
        /*FMU name is longer than 200+, rarely happens*/
        fmustruct->FMUErrorStatus = fmi2Fatal;
#if FMU_CG_TARGET < FMUCG_STANDALONE_TARGETS
        if (fmustruct->isLoggingOn) {
            void *diagnostic = CreateDiagnosticAsVoidPtr("SL_SERVICES:utils:PRINTFWRAPPER", 1,
                                                         CODEGEN_SUPPORT_ARG_STRING_TYPE, "FMU Name is too long.");
            rt_ssReportDiagnosticAsWarning(fmustruct->ssPtr, diagnostic);
        }
        ssSetStopRequested(fmustruct->ssPtr, 1);
#else 
        message = formatString("FMU Name '%s' is too long.", instanceName);
        fmustruct->callbacks.logger(fmustruct->callbacks.componentEnvironment, fmustruct->fmuname, fmustruct->FMUErrorStatus, "FMU2_fmuInitialize", message);
#endif
        return NULL;
    }

    fmustruct->Handle = LOAD_LIBRARY(fmustruct->dllfile);
    if (NULL == fmustruct->Handle) {
        fmustruct->FMUErrorStatus = fmi2Fatal;
#if FMU_CG_TARGET < FMUCG_STANDALONE_TARGETS
        if (fmustruct->isLoggingOn) {
            void *diagnostic = CreateDiagnosticAsVoidPtr("FMUBlock:FMU:FMULoadLibraryError", 2,
                                                         CODEGEN_SUPPORT_ARG_STRING_TYPE, fmustruct->dllfile,
                                                         CODEGEN_SUPPORT_ARG_STRING_TYPE, fmustruct->fmuname);
            /*loading lib failure will halt simulation*/
            rt_ssReportDiagnosticAsWarning(fmustruct->ssPtr, diagnostic);
        }
        ssSetStopRequested(fmustruct->ssPtr, 1);
#else
        const char* errorCodeMessage = getLoadLibraryErrorMessage();
        message = formatString("Unable to load dynamic library: '%s'. %s", fmustruct->dllfile, errorCodeMessage);
        fmustruct->callbacks.logger(fmustruct->callbacks.componentEnvironment, fmustruct->fmuname, fmustruct->FMUErrorStatus, "FMU2_fmuInitialize", message);
#endif
        return NULL;
    }
    
    /* load fmi functions */
    LoadFMU2CommonFunctions(fmustruct, loadFMUStateFcn, loadSerializationFcn, loadDirectionDerivativeFcn);
    if (fmuType == fmi2CoSimulation){
        LoadFMU2CSFunctions(fmustruct);
    }else if(fmuType == fmi2ModelExchange){
        LoadFMU2MEFunctions(fmustruct);
    }
    
    /* instantiate fmu */
    fmustruct->mFMIComp = fmustruct->instantiate(instanceName,
                                                 fmuType,
                                                 fmuGUID,
                                                 fmuResLocation,
                                                 &fmustruct->callbacks,
                                                 visible,
                                                 isLoggingOn);      

    if (NULL == fmustruct->mFMIComp ){
        CheckStatus(fmustruct, fmi2Error, "fmi2Instantiate");
        CLOSE_LIBRARY(fmustruct->Handle);
        fmustruct->callbacks.freeMemory(fmustruct);
        return NULL;
    }
    return (void *) fmustruct;
}

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
                           fmi2Boolean fmuLoggingOn) {
    return FMU2_fmuInitialize(lib,
                              instanceName,
                              fmuGUID,
                              fmuLocation,
                              fmi2CoSimulation,
                              ssPtr,
                              loadFMUStateFcn,
                              loadSerializationFcn,
                              loadDirectionDerivativeFcn,
                              fmuCallBacks,
                              fmuVisible,
                              fmuLoggingOn);
}

void* FMU2_fmuInitializeME(const char* lib,
                           fmi2String instanceName,
                           fmi2String fmuGUID,
                           fmi2String fmuLocation,
                           void* ssPtr,
                           int loadFMUStateFcn,
                           int loadSerializationFcn,
                           int loadDirectionDerivativeFcn,
                           const void* fmuCallBacks,
                           fmi2Boolean fmuVisible,
                           fmi2Boolean fmuLoggingOn) {
    return FMU2_fmuInitialize(lib,
                              instanceName,
                              fmuGUID,
                              fmuLocation,
                              fmi2ModelExchange,
                              ssPtr,
                              loadFMUStateFcn,
                              loadSerializationFcn,
                              loadDirectionDerivativeFcn,
                              fmuCallBacks,
                              fmuVisible,
                              fmuLoggingOn);
}

fmi2Boolean FMU2_setupExperiment(void** fmuv,
                                fmi2Boolean isToleranceUsed,
                                fmi2Real toleranceValue,
                                fmi2Real currentTime,
                                fmi2Boolean isTFinalUsed,
                                fmi2Real TFinal) {
    struct FMU2_CSME_RTWCG * fmustruct = (struct FMU2_CSME_RTWCG *)(*fmuv);
    fmi2Status fmi2Flag = fmustruct->setupExperiment(fmustruct->mFMIComp,
                                                     isToleranceUsed,
                                                     toleranceValue,
                                                     currentTime,
                                                     isTFinalUsed,
                                                     TFinal);
    return CheckStatus(fmustruct, fmi2Flag, "fmi2SetupExperiment");
}

fmi2Boolean FMU2_enterInitializationMode(void** fmuv) {
    struct FMU2_CSME_RTWCG * fmustruct = (struct FMU2_CSME_RTWCG *)(*fmuv);
    fmi2Status fmi2Flag = fmustruct->enterInitializationMode(fmustruct->mFMIComp);
    return CheckStatus(fmustruct, fmi2Flag, "fmi2EnterInitializationMode");
}
fmi2Boolean FMU2_exitInitializationMode(void** fmuv) {
    struct FMU2_CSME_RTWCG * fmustruct = (struct FMU2_CSME_RTWCG *)(*fmuv);
    fmi2Status fmi2Flag = fmustruct->exitInitializationMode(fmustruct->mFMIComp);
    fmustruct->modelInitialized = true;
    return CheckStatus(fmustruct, fmi2Flag, "fmi2ExitInitializationMode");
}

fmi2Boolean FMU2_terminate(void **fmuv) {
    struct FMU2_CSME_RTWCG * fmustruct = (struct FMU2_CSME_RTWCG *)(*fmuv);
    fmi2Boolean returnStatus = fmi2True;
    if (fmustruct == NULL) {
        return returnStatus;
    }
    if(fmustruct->FMUErrorStatus != fmi2Fatal){
       if(fmustruct->FMUErrorStatus != fmi2Error &&
          fmustruct->modelInitialized == true) {
            fmi2Status fmi2Flag = fmustruct->terminate(fmustruct->mFMIComp);
            returnStatus = CheckStatus(fmustruct, fmi2Flag, "fmi2TerminateSlave");
        }
        fmustruct->freeInstance(fmustruct->mFMIComp);
    }
    if (fmustruct->Handle != NULL) {
        CLOSE_LIBRARY(fmustruct->Handle);
    }
    fmi2CallbackFreeMemory freeMemory = fmustruct->callbacks.freeMemory;
    freeMemory(fmustruct->paramIdxToOffset);
    freeMemory(fmustruct->enumValueList);
    freeMemory(fmustruct);
    return returnStatus;
}

void FMU2_setFMUState(void** fmuv, void* fmustate){
    struct FMU2_CSME_RTWCG * fmustruct = (struct FMU2_CSME_RTWCG *)(*fmuv);
    fmustruct->setFMUstate(fmustruct->mFMIComp, fmustate);
}
void FMU2_getFMUState(void** fmuv, void** fmustate){
    struct FMU2_CSME_RTWCG * fmustruct = (struct FMU2_CSME_RTWCG *)(*fmuv);
    fmustruct->getFMUstate(fmustruct->mFMIComp, fmustate);
}

void FMU2_freeFMUState(void** fmuv, void** fmustate){
    struct FMU2_CSME_RTWCG * fmustruct = (struct FMU2_CSME_RTWCG *)(*fmuv);
    fmustruct->freeFMUstate(fmustruct->mFMIComp, fmustate);
}

void FMU2_serializedFMUstateSize(void** fmuv, void* fmustate, size_t* size){
    struct FMU2_CSME_RTWCG * fmustruct = (struct FMU2_CSME_RTWCG *)(*fmuv);
    fmustruct->serializedFMUstateSize(fmustruct->mFMIComp, fmustate, size);
}

void FMU2_serializeFMUstate(void** fmuv, void* fmustate, char* byteArray){
    struct FMU2_CSME_RTWCG * fmustruct = (struct FMU2_CSME_RTWCG *)(*fmuv);
    size_t size = 0;
    FMU2_serializedFMUstateSize(fmuv, fmustate, &size);
    fmustruct->serializeFMUstate(fmustruct->mFMIComp, fmustate, byteArray, size);
}

void FMU2_deserializeFMUstate(void** fmuv, char* byteArray, void** fmustate){
    struct FMU2_CSME_RTWCG * fmustruct = (struct FMU2_CSME_RTWCG *)(*fmuv);
    size_t size = 0;
    FMU2_serializedFMUstateSize(fmuv, fmustate, &size);
    fmustruct->deSerializeFMUstate(fmustruct->mFMIComp, byteArray, size, fmustate);
}

fmi2Boolean FMU2_doStep(void **fmuv,
                       fmi2Real currentCommunicationPoint,
                       fmi2Real communicationStepSize,
                       fmi2Boolean noSetFMUStatePriorToCurrentPoint) {
    fmi2String message = NULL;
#if FMU_CG_TARGET < FMUCG_STANDALONE_TARGETS
    (void) message;
#endif
    struct FMU2_CSME_RTWCG * fmustruct = (struct FMU2_CSME_RTWCG *)(*fmuv);
    fmi2Status fmi2Flag = fmustruct->doStep(fmustruct->mFMIComp, currentCommunicationPoint,communicationStepSize, noSetFMUStatePriorToCurrentPoint);
    if(fmi2Flag == fmi2Discard){
         fmi2Boolean boolVal;
         fmustruct->getBooleanStatus(fmustruct->mFMIComp, fmi2Terminated, &boolVal);
         if(boolVal == fmi2True) {
             char time[20];             
             memset(time, 0, 20);
             snprintf(time, 20, "%f", currentCommunicationPoint);
#if FMU_CG_TARGET < FMUCG_STANDALONE_TARGETS
             if (fmustruct->isLoggingOn) {
                 void *diagnostic = CreateDiagnosticAsVoidPtr("FMUBlock:FMU2:FMU2SimDoStepTerminated", 2,
                                                              CODEGEN_SUPPORT_ARG_STRING_TYPE, fmustruct->fmuname,
                                                              CODEGEN_SUPPORT_ARG_STRING_TYPE, time);
                 rt_ssReportDiagnosticAsInfo(fmustruct->ssPtr, diagnostic);
             }
             ssSetStopRequested(fmustruct->ssPtr, 1);
#else 
             message = formatString("'fmi2Terminated' flag was set to true for FMU '%s' after 'fmi2DoStep' function returns 'fmi2Discard' at time %f.", fmustruct->fmuname, time);
             fmustruct->callbacks.logger(fmustruct->callbacks.componentEnvironment, fmustruct->fmuname, fmustruct->FMUErrorStatus, "FMU2_doStep", message);
#endif
         }
    }
    return CheckStatus(fmustruct, fmi2Flag, "fmi2DoStep");
}

fmi2Boolean FMU2_setRealVal(void **fmuv,
                           const fmi2ValueReference dvr,
                           size_t nvr,
                           const fmi2Real dvalue) {
    struct FMU2_CSME_RTWCG * fmustruct = (struct FMU2_CSME_RTWCG *)(*fmuv);
    fmi2ValueReference vr =dvr;
    fmi2Real value = dvalue;
    fmi2Status fmi2Flag = fmustruct->setReal(fmustruct->mFMIComp, &vr, nvr, &value);
    return CheckStatus(fmustruct, fmi2Flag, "fmi2SetReal");
}

///////////////////////////////////////////////////////////////////////////////////down//
// temporary - all setRealScalar will be removed
fmi2Boolean FMU2_setRealPtr(void **fmuv,
                           const fmi2ValueReference dvr,
                           size_t nvr,
                           const fmi2Real value[]) {
    struct FMU2_CSME_RTWCG * fmustruct = (struct FMU2_CSME_RTWCG *)(*fmuv);
    fmi2ValueReference vr =dvr;
    fmi2Status fmi2Flag = fmustruct->setReal(fmustruct->mFMIComp, &vr, nvr, value);
    return CheckStatus(fmustruct, fmi2Flag, "fmi2SetReal");
}
fmi2Boolean FMU2_setIntegerPtr(void **fmuv,
                           const fmi2ValueReference dvr,
                           size_t nvr,
                           const fmi2Integer value[]) {
    struct FMU2_CSME_RTWCG * fmustruct = (struct FMU2_CSME_RTWCG *)(*fmuv);
    fmi2ValueReference vr =dvr;
    fmi2Status fmi2Flag = fmustruct->setInteger(fmustruct->mFMIComp, &vr, nvr, value);
    return CheckStatus(fmustruct, fmi2Flag, "fmi2SetInteger");
}
fmi2Boolean FMU2_setBooleanPtr(void **fmuv,
                           const fmi2ValueReference dvr,
                           size_t nvr,
                           const fmi2Boolean value[]) {
    struct FMU2_CSME_RTWCG * fmustruct = (struct FMU2_CSME_RTWCG *)(*fmuv);
    fmi2ValueReference vr =dvr;
    fmi2Status fmi2Flag = fmustruct->setBoolean(fmustruct->mFMIComp, &vr, nvr, value);
    return CheckStatus(fmustruct, fmi2Flag, "fmi2SetBoolean");
}
fmi2Boolean FMU2_setStringPtr(void **fmuv,
                          const fmi2ValueReference dvr,
                          size_t nvr,
                          const fmi2String value[]) {
    struct FMU2_CSME_RTWCG * fmustruct = (struct FMU2_CSME_RTWCG *)(*fmuv);
    fmi2ValueReference vr =dvr;
    fmi2Status fmi2Flag = fmustruct->setString(fmustruct->mFMIComp, &vr, nvr, value);
    return CheckStatus(fmustruct, fmi2Flag, "fmi2SetString");
}
fmi2Boolean FMU2_getRealPtr(void **fmuv,
                            const fmi2ValueReference dvr,
                            size_t nvr,
                            fmi2Real value[]) {
    struct FMU2_CSME_RTWCG * fmustruct = (struct FMU2_CSME_RTWCG *)(*fmuv);
    fmi2ValueReference vr =dvr;
    fmi2Status fmi2Flag = fmustruct->getReal(fmustruct->mFMIComp, &vr, nvr, value);
    return CheckStatus(fmustruct, fmi2Flag, "fmi2GetReal");
}
fmi2Boolean FMU2_getIntegerPtr(void **fmuv,
                               const fmi2ValueReference dvr,
                               size_t nvr,
                               fmi2Integer value[]) {
    struct FMU2_CSME_RTWCG * fmustruct = (struct FMU2_CSME_RTWCG *)(*fmuv);
    fmi2ValueReference vr =dvr;
    fmi2Status fmi2Flag = fmustruct->getInteger(fmustruct->mFMIComp, &vr, nvr, value);
    return CheckStatus(fmustruct, fmi2Flag, "fmi2GetInteger");
}
fmi2Boolean FMU2_getBooleanPtr(void **fmuv,
                               const fmi2ValueReference dvr,
                               size_t nvr,
                               fmi2Boolean value[]) {
    struct FMU2_CSME_RTWCG * fmustruct = (struct FMU2_CSME_RTWCG *)(*fmuv);
    fmi2ValueReference vr =dvr;
    fmi2Status fmi2Flag = fmustruct->getBoolean(fmustruct->mFMIComp, &vr, nvr, value);
    return CheckStatus(fmustruct, fmi2Flag, "fmi2GetBoolean");
}
fmi2Boolean FMU2_getStringPtr(void **fmuv,
                              const fmi2ValueReference dvr,
                              size_t nvr,
                              fmi2String value[]) {
    struct FMU2_CSME_RTWCG * fmustruct = (struct FMU2_CSME_RTWCG *)(*fmuv);
    fmi2ValueReference vr =dvr;
    fmi2Status fmi2Flag = fmustruct->getString(fmustruct->mFMIComp, &vr, nvr, value);
    return CheckStatus(fmustruct, fmi2Flag, "fmi2GetString");
}


//////////////////////////////////////////////////////////////////////////////////up////


fmi2Boolean FMU2_setReal(void **fmuv,
                        const fmi2ValueReference vr[],
                        size_t nvr,
                        const fmi2Real value[]) {
    struct FMU2_CSME_RTWCG * fmustruct = (struct FMU2_CSME_RTWCG *)(*fmuv);
    fmi2Status fmi2Flag = fmustruct->setReal(fmustruct->mFMIComp, vr, nvr, value);
    return CheckStatus(fmustruct, fmi2Flag, "fmi2SetReal");
}

fmi2Boolean FMU2_getReal(void **fmuv,
                        const fmi2ValueReference vr[],
                        size_t nvr,
                        fmi2Real value[]) {
    struct FMU2_CSME_RTWCG * fmustruct = (struct FMU2_CSME_RTWCG *)(*fmuv);
    fmi2Status fmi2Flag = fmustruct->getReal(fmustruct->mFMIComp, vr, nvr, value);
    return CheckStatus(fmustruct, fmi2Flag, "fmi2GetReal");
}

fmi2Boolean FMU2_setIntegerVal(void **fmuv,
                              const fmi2ValueReference dvr,
                              size_t nvr,
                              const fmi2Integer dvalue) {
    struct FMU2_CSME_RTWCG * fmustruct = (struct FMU2_CSME_RTWCG *)(*fmuv);
    fmi2ValueReference vr =dvr;
    fmi2Integer value = dvalue;
    fmi2Status fmi2Flag = fmustruct->setInteger(fmustruct->mFMIComp, &vr, nvr, &value);
    return CheckStatus(fmustruct, fmi2Flag, "fmi2SetInteger");
}

fmi2Boolean FMU2_setInteger(void **fmuv,
                           const fmi2ValueReference vr[],
                           size_t nvr,
                           const fmi2Integer value[]) {
    struct FMU2_CSME_RTWCG * fmustruct = (struct FMU2_CSME_RTWCG *)(*fmuv);
    fmi2Status fmi2Flag = fmustruct->setInteger(fmustruct->mFMIComp, vr, nvr, value);
    return CheckStatus(fmustruct, fmi2Flag, "fmi2SetInteger");
}

fmi2Boolean FMU2_getInteger(void **fmuv,
                           const fmi2ValueReference vr[],
                           size_t nvr,
                           fmi2Integer value[]) {
    struct FMU2_CSME_RTWCG * fmustruct = (struct FMU2_CSME_RTWCG *)(*fmuv);
    fmi2Status fmi2Flag = fmustruct->getInteger(fmustruct->mFMIComp, vr, nvr, value);
    return CheckStatus(fmustruct, fmi2Flag, "fmi2GetInteger");
}

fmi2Boolean FMU2_setBooleanVal(void **fmuv,
                              const fmi2ValueReference dvr,
                              size_t nvr,
                              const fmi2Boolean dvalue) {
    struct FMU2_CSME_RTWCG * fmustruct = (struct FMU2_CSME_RTWCG *)(*fmuv);
    fmi2ValueReference vr =dvr;
    fmi2Boolean value = dvalue;
    fmi2Status fmi2Flag = fmustruct->setBoolean(fmustruct->mFMIComp, &vr, nvr, &value);
    return CheckStatus(fmustruct, fmi2Flag, "fmi2SetBoolean");
}

fmi2Boolean FMU2_setBoolean(void **fmuv,
                           const fmi2ValueReference vr[],
                           size_t nvr,
                           const fmi2Boolean value[]) {
    struct FMU2_CSME_RTWCG * fmustruct = (struct FMU2_CSME_RTWCG *)(*fmuv);
    fmi2Status fmi2Flag = fmustruct->setBoolean(fmustruct->mFMIComp, vr, nvr, value);
    return CheckStatus(fmustruct, fmi2Flag, "fmi2SetBoolean");
}

fmi2Boolean FMU2_getBoolean(void **fmuv,
                           const fmi2ValueReference vr[],
                           size_t nvr,
                           fmi2Boolean value[]) {
    struct FMU2_CSME_RTWCG * fmustruct = (struct FMU2_CSME_RTWCG *)(*fmuv);
    fmi2Status fmi2Flag = fmustruct->getBoolean(fmustruct->mFMIComp, vr, nvr, value);
    return CheckStatus(fmustruct, fmi2Flag, "fmi2GetBoolean");
}

fmi2Boolean FMU2_setStringVal(void **fmuv,
                             const fmi2ValueReference dvr,
                             size_t nvr,
                             const fmi2String dvalue) {
    struct FMU2_CSME_RTWCG * fmustruct = (struct FMU2_CSME_RTWCG *)(*fmuv);
    fmi2ValueReference vr =dvr;
    fmi2String value = dvalue;
    fmi2Status fmi2Flag = fmustruct->setString(fmustruct->mFMIComp, &vr, nvr, &value);
    return CheckStatus(fmustruct, fmi2Flag, "fmiSetString");
}

fmi2Boolean FMU2_setString(void **fmuv,
                          const fmi2ValueReference vr[],
                          size_t nvr,
                          void* value[]) {
    struct FMU2_CSME_RTWCG * fmustruct = (struct FMU2_CSME_RTWCG *)(*fmuv);
    fmi2Status fmi2Flag = fmustruct->setString(fmustruct->mFMIComp, vr, nvr, (const fmi2String*)value);
    return CheckStatus(fmustruct, fmi2Flag, "fmi2SetString");
}

fmi2Boolean FMU2_getString(void **fmuv,
                          const fmi2ValueReference vr[],
                          size_t nvr,
                          void* value[]) {
    struct FMU2_CSME_RTWCG * fmustruct = (struct FMU2_CSME_RTWCG *)(*fmuv);
    fmi2Status fmi2Flag = fmustruct->getString(fmustruct->mFMIComp, vr, nvr, (fmi2String*)value);
    return CheckStatus(fmustruct, fmi2Flag, "fmi2GetString");
}

void FMU2_strncpy(char* dest, void* src, int size) {
    strncpy(dest, (char*)src, size);
    dest[size] = '\x00';
}

char* FMU2_strndup(const char* src, int size) {
    int srclen = strlen(src);
    int copylen = srclen < size? srclen : size;
    char *dest= (char*)malloc(copylen+1);
    memcpy(dest, src, copylen);
    dest[copylen] = '\x00';
    return dest;
}

/*me standard functions wrapper*/
fmi2Boolean FMU2_enterEventMode(void** fmuv) {
    struct FMU2_CSME_RTWCG * fmustruct = (struct FMU2_CSME_RTWCG *)(*fmuv);
    fmi2Status fmi2Flag = fmustruct->enterEventMode(fmustruct->mFMIComp);
    return CheckStatus(fmustruct, fmi2Flag, "fmi2EnterEventMode");
}

/* this function was directly called in eventIteration, no wrapper needed*/
/* fmi2Status FMU2_newDiscreteStates(void** fmuv, fmi2EventInfo* eventInfo); */

fmi2Boolean FMU2_enterContinuousTimeMode(void** fmuv) {
    struct FMU2_CSME_RTWCG * fmustruct = (struct FMU2_CSME_RTWCG *)(*fmuv);
    fmi2Status fmi2Flag = fmustruct->enterContinuousTimeMode(fmustruct->mFMIComp);
    return CheckStatus(fmustruct, fmi2Flag, "fmi2EnterContinuousTimeMode");
}

fmi2Boolean FMU2_completedIntegratorStep(void** fmuv,
                                        fmi2Boolean noSetFMUStatePriorToCurrentPoint,
                                        fmi2Boolean* enterEventMode,
                                        fmi2Boolean* terminateSimulation) {
    struct FMU2_CSME_RTWCG * fmustruct = (struct FMU2_CSME_RTWCG *)(*fmuv);
    fmi2Status fmi2Flag = fmustruct->completedIntegratorStep(fmustruct->mFMIComp, noSetFMUStatePriorToCurrentPoint, enterEventMode, terminateSimulation);
    return CheckStatus(fmustruct, fmi2Flag, "fmi2CompletedIntegratorStep");

}

fmi2Boolean FMU2_setTime(void** fmuv,
                        fmi2Real time) {
    struct FMU2_CSME_RTWCG * fmustruct = (struct FMU2_CSME_RTWCG *)(*fmuv);
    fmi2Status fmi2Flag = fmustruct->setTime(fmustruct->mFMIComp, time);
    return CheckStatus(fmustruct, fmi2Flag, "fmi2SetTime");
}

fmi2Boolean FMU2_setContinuousStates(void** fmuv,
                                    const fmi2Real states[],
                                    size_t nx)  {
    struct FMU2_CSME_RTWCG * fmustruct = (struct FMU2_CSME_RTWCG *)(*fmuv);
    fmi2Status fmi2Flag = fmustruct->setContinuousStates(fmustruct->mFMIComp, states, nx);
    return CheckStatus(fmustruct, fmi2Flag, "fmi2SetContinuousStates");
}

fmi2Boolean FMU2_getDerivatives(void** fmuv,
                               fmi2Real derivatives[],
                               size_t nx) {
    struct FMU2_CSME_RTWCG * fmustruct = (struct FMU2_CSME_RTWCG *)(*fmuv);
    fmi2Status fmi2Flag = fmustruct->getDerivatives(fmustruct->mFMIComp, derivatives, nx);
    return CheckStatus(fmustruct, fmi2Flag, "fmi2GetDerivatives");
}

fmi2Boolean FMU2_getEventIndicators(void** fmuv,
                                   fmi2Real eventIndicators[],
                                   size_t nx) {
    struct FMU2_CSME_RTWCG * fmustruct = (struct FMU2_CSME_RTWCG *)(*fmuv);
    fmi2Status fmi2Flag = fmustruct->getEventIndicators(fmustruct->mFMIComp, eventIndicators, nx);
    return CheckStatus(fmustruct, fmi2Flag, "fmi2GetEventIndicators");
}

fmi2Boolean FMU2_getContinuousStates(void** fmuv,
                                    fmi2Real states[],
                                    size_t nx)  {
    struct FMU2_CSME_RTWCG * fmustruct = (struct FMU2_CSME_RTWCG *)(*fmuv);
    fmi2Status fmi2Flag = fmustruct->getContinuousStates(fmustruct->mFMIComp, states, nx);
    return CheckStatus(fmustruct, fmi2Flag, "fmi2GetContinuousStates");
}

fmi2Boolean FMU2_getNominalsOfContinuousStates(void** fmuv,
                                              fmi2Real states[],
                                              size_t nx) {
    struct FMU2_CSME_RTWCG * fmustruct = (struct FMU2_CSME_RTWCG *)(*fmuv);
    fmi2Status fmi2Flag = fmustruct->getNominalsOfContinuousStates(fmustruct->mFMIComp, states, nx);
    return CheckStatus(fmustruct, fmi2Flag, "fmi2GetNominalsOfContinuousStates");
}

/* me helper functions*/
void FMU2_getNextEventTime(void **fmuv,
                           fmi2Real* nextEventTime,
                           int32_T* upcomingTimeEvent) {
    struct FMU2_CSME_RTWCG * fmustruct = (struct FMU2_CSME_RTWCG *)(*fmuv);
    *nextEventTime = fmustruct->eventInfo.nextEventTime;
    *upcomingTimeEvent = (int32_T) fmustruct->eventInfo.nextEventTimeDefined;
}

void FMU2_simTerminate(void **fmuv,
                       const char* blkPath,
                       fmi2Real time){
    struct FMU2_CSME_RTWCG* fmustruct = (struct FMU2_CSME_RTWCG *)(*fmuv);
#if FMU_CG_TARGET < FMUCG_STANDALONE_TARGETS
    /* terminate the simulation (successfully) */
    if (fmustruct->isLoggingOn) {
        void * diagnostic = CreateDiagnosticAsVoidPtr("FMUBlock:FMU2:FMU2SimEventUpdateTerminated", 2,
                                                      CODEGEN_SUPPORT_ARG_STRING_TYPE, blkPath,
                                                      CODEGEN_SUPPORT_ARG_REAL_TYPE, time);
        rt_ssReportDiagnosticAsInfo(fmustruct->ssPtr, diagnostic);
    }
    ssSetStopRequested(fmustruct->ssPtr, 1);
#else
    fmi2String message = formatString("'TerminateSimulation' flag was set to true by FMU '%s' during event iteration. Simulation will stop after the current time step at '%f'.", blkPath, time);
    fmustruct->callbacks.logger(fmustruct->callbacks.componentEnvironment, fmustruct->fmuname, fmustruct->FMUErrorStatus, "FMU2_simTerminate", message);
#endif
}

fmi2Boolean FMU2_eventIteration(void **fmuv,
                         const char* blkPath,
                         fmi2Real time) {
    struct FMU2_CSME_RTWCG* fmustruct = (struct FMU2_CSME_RTWCG *)(*fmuv);
    fmi2Status fmi2Flag = fmi2OK;
    fmi2Boolean returnStatus = fmi2True;
    int iterationNumber = 0;
    fmi2String message = NULL;
    
#if FMU_CG_TARGET < FMUCG_STANDALONE_TARGETS
    (void) message;
#endif
    fmustruct->eventInfo.newDiscreteStatesNeeded = fmi2True;
    while(fmustruct->eventInfo.newDiscreteStatesNeeded == fmi2True){

        /*initialize all members of fmi2EventInfo before calling new discrete state*/
        fmustruct->eventInfo.newDiscreteStatesNeeded = false;
        fmustruct->eventInfo.terminateSimulation = false;
        fmustruct->eventInfo.nominalsOfContinuousStatesChanged = false;
        fmustruct->eventInfo.valuesOfContinuousStatesChanged = false;
        fmustruct->eventInfo.nextEventTimeDefined = false;
        fmustruct->eventInfo.nextEventTime = 0.0;

        /*safe call to newDiscreteStates*/
        fmi2Flag = fmustruct->newDiscreteStates(fmustruct->mFMIComp, &(fmustruct->eventInfo));
        returnStatus = CheckStatus(fmustruct, fmi2Flag, "fmi2NewDiscreteStates");

        if(returnStatus == fmi2True && fmustruct->eventInfo.terminateSimulation == fmi2True){
            /* terminate the simulation (successfully) */
            FMU2_simTerminate(fmuv, blkPath, time);
        }

        if(iterationNumber >= 10000){
#if FMU_CG_TARGET < FMUCG_STANDALONE_TARGETS
            if (fmustruct->isLoggingOn) {
                void * diagnostic = CreateDiagnosticAsVoidPtr("FMUBlock:FMU:FMUSimEventUpdateTerminated", 2,
                                                              CODEGEN_SUPPORT_ARG_REAL_TYPE, time,
                                                              CODEGEN_SUPPORT_ARG_INTEGER_TYPE, iterationNumber);
                rt_ssReportDiagnosticAsInfo(fmustruct->ssPtr, diagnostic);
            }
#else
            message = formatString("'TerminateSimulation' flag was set to true by FMU '%s' during event iteration. Simulation will stop after the current time step at '%f'.", blkPath, time);
            fmustruct->callbacks.logger(fmustruct->callbacks.componentEnvironment, fmustruct->fmuname, fmustruct->FMUErrorStatus, "FMU2_eventIteration", message);
#endif
            returnStatus = fmi2False;
            break;
            
        } else
            iterationNumber ++;
    }
    return returnStatus;
}

void FMU2_valuesOfContinuousStatesChanged(void **fmuv,
                                          int* stateChanged) {
    struct FMU2_CSME_RTWCG* fmustruct = (struct FMU2_CSME_RTWCG *)(*fmuv);
    *stateChanged = fmustruct->eventInfo.valuesOfContinuousStatesChanged;
}

/*helper to preprocess Enum type*/
void createParamIdxToOffset(void** fmuv,
                            int array_size) {
    struct FMU2_CSME_RTWCG * fmustruct = (struct FMU2_CSME_RTWCG *)(*fmuv);
    fmi2CallbackAllocateMemory allocateMemory = fmustruct->callbacks.allocateMemory;
    fmustruct->paramIdxToOffset = allocateMemory(array_size, sizeof(int));
}

void createEnumValueList(void** fmuv,
                         int array_size) {
    struct FMU2_CSME_RTWCG * fmustruct = (struct FMU2_CSME_RTWCG *)(*fmuv);
    fmi2CallbackAllocateMemory allocateMemory = fmustruct->callbacks.allocateMemory;
    fmustruct->enumValueList = allocateMemory(array_size, sizeof(int));
}

void setParamIdxToOffsetByIdx(void** fmuv,
                              int idx,
                              int value) {
    struct FMU2_CSME_RTWCG * fmustruct = (struct FMU2_CSME_RTWCG *)(*fmuv);
    *(fmustruct->paramIdxToOffset + idx) = value;
}

void setEnumValueListByIdx(void** fmuv,
                           int idx,
                           int value) {
    struct FMU2_CSME_RTWCG * fmustruct = (struct FMU2_CSME_RTWCG *)(*fmuv);
    *(fmustruct->enumValueList + idx) = value;
}

void getParamIdxToOffsetByIdx(void** fmuv,
                              int idx,
                              int* offset) {
    struct FMU2_CSME_RTWCG * fmustruct = (struct FMU2_CSME_RTWCG *)(*fmuv);
    *offset = *(fmustruct->paramIdxToOffset + idx);
}

void getEnumValueByIdx(void** fmuv,
                       int idx,
                       int* val) {
    struct FMU2_CSME_RTWCG * fmustruct = (struct FMU2_CSME_RTWCG *)(*fmuv);
    *val = *(fmustruct->enumValueList + idx);
}
