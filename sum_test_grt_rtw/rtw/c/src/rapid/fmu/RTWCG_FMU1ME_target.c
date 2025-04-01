/* Copyright 2017-2022 The MathWorks, Inc. */

/******************************************************************************
 *                                                                            *
 * File    : RTWCG_FMU1_target.c                                              *
 * Abstract:                                                                  *
 *      Wrapper functions to dynamic call libraries of FMU 1.0                *
 *      This file defines functions called by code generation                 *
 *      Also handle errors, and logger                                        *
 *                                                                            *
 ******************************************************************************/

#include "RTWCG_FMU1ME_target.h"
#define FMU1_MESSAGE_SIZE 1024
#define FMU1_STRUCTMAP_SIZE 1024

#if FMU_CG_TARGET == FMUCG_SLRT
#include "SLRTLoggerWrapper.hpp"
#endif
/*
  Whenever a default function is called, it means a functions is called without successful load,
  return a fmiError;
*/
static fmiStatus defaultfcnStatus(fmiComponent c, ...) {
    (void)c;
    return fmiError;
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

static fmiString formatString(fmiString format, ...) {
    static char translatedMsg[FMU1_MESSAGE_SIZE];
    va_list args;
    va_start(args, format);
    vsnprintf(translatedMsg, FMU1_MESSAGE_SIZE - 1, format, args);
    va_end(args);
    return translatedMsg;
}

static const char* getLoadLibraryErrorMessage(void)
{
    static char message[FMU1_MESSAGE_SIZE];
#ifdef _WIN32
    unsigned long int errorCode = GetLastError();
    snprintf(message, FMU1_MESSAGE_SIZE, "Load Library Error: '%d' ", errorCode); 
#else
    const char* errorMessage = dlerror();
    snprintf(message, FMU1_MESSAGE_SIZE, "Load Library Error: '%s' ", errorMessage); 
#endif
    return message;
}


/* todo: translate value reference to names */
static void* fmu1SetGetFmuStruct(void* component, int isSet, void* fmuStructIn) {
    static void *componentList[FMU1_STRUCTMAP_SIZE] = {0}, *fmuStructList[FMU1_STRUCTMAP_SIZE] = {0};
    static int newIndex = 0;

    if (!component) return NULL;
    if (isSet == 1) {
        /* add component-fmuStruct pair to list; overwrite oldest if overflows */
        componentList[newIndex] = component;
        fmuStructList[newIndex] = fmuStructIn;
        newIndex = (newIndex+1) % FMU1_STRUCTMAP_SIZE;
    } else {
        /* find fmuStruct from list; return NULL if not found */
        int i = 0;
        for (i = 0; i < FMU1_STRUCTMAP_SIZE; i++) {
            if (componentList[i] == component) return fmuStructList[i];
        }
    }
    return NULL;    
}

static void fmu1Logger(fmiComponent c,
                       fmiString instanceName,
                       fmiStatus status,
                       fmiString category,
                       fmiString message, ...) {
    static const char* strStatus[] = {
        "fmiOK", "fmiWarning", "fmiDiscard", "fmiError", "fmiFatal" };
    
    struct FMU1_ME_RTWCG * fmustruct = (struct FMU1_ME_RTWCG *)fmu1SetGetFmuStruct((void*)c, 0, NULL);
    if (!fmustruct || !fmustruct->isLoggingOn) return;

    char translatedMsg[FMU1_MESSAGE_SIZE];
    char temp[FMU1_MESSAGE_SIZE];
    int prefixLength = snprintf(translatedMsg, FMU1_MESSAGE_SIZE, "Log from FMU: [category:%s, status:%s] ",
                            strStatus[status], category); 
    va_list args;
    va_start (args, message);
    vsnprintf(temp, FMU1_MESSAGE_SIZE, message, args);
    va_end(args);
    strncat(translatedMsg, temp, FMU1_MESSAGE_SIZE-prefixLength - 1);
    (void)instanceName;

#if FMU_CG_TARGET < FMUCG_STANDALONE_TARGETS        
    SimStruct* ssPtr = (SimStruct*)fmustruct->ssPtr;    
    void *diagnostic = CreateDiagnosticAsVoidPtr("SL_SERVICES:utils:PRINTFWRAPPER", 1,
                                                 CODEGEN_SUPPORT_ARG_STRING_TYPE, translatedMsg);
    rt_ssReportDiagnosticAsInfo(ssPtr, diagnostic);
#elif FMU_CG_TARGET == FMUCG_SLRT
    /* Skip printing info logs which are shown by fmiOK status*/
    if(status != fmiOK) {
        SLRTLog(kWarning,translatedMsg);
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
    static x f(struct FMU1_ME_RTWCG* fmustruct,                         \
               const char * fcnName,                                    \
               int isOptional) {                                        \
    x fcn = NULL;                                                       \
    fmiString message = NULL;                                           \
                                                                        \
    static char fullFcnName[FULL_FCN_NAME_MAX_LEN];                     \
    memset(fullFcnName, 0, FULL_FCN_NAME_MAX_LEN);                      \
    strncpy(fullFcnName, fmustruct->modelIdentifier, FULL_FCN_NAME_MAX_LEN - 1); \
    strncat(fullFcnName, "_", FULL_FCN_NAME_MAX_LEN - strlen(fullFcnName) - 1); \
    strncat(fullFcnName, fcnName, FULL_FCN_NAME_MAX_LEN - strlen(fullFcnName) - 1);

#if FMU_CG_TARGET < FMUCG_STANDALONE_TARGETS
#define LoadFMUFcnDecl2                         \
    (void) message;
#else
#define LoadFMUFcnDecl2
#endif

#ifdef _WIN32

#define LoadFMUFcnDecl3(x)                                              \
    fcn = (x)LOAD_FUNCTION(fmustruct->Handle, fullFcnName);

#else

#define LoadFMUFcnDecl3(x)                                              \
    *((void **)(&fcn)) = LOAD_FUNCTION(fmustruct->Handle, fullFcnName);

#endif

#define LoadFMUFcnDecl4                         \
    if (!isOptional && fcn == NULL) {           \
    fmustruct->FMUErrorStatus = fmiWarning;

#if FMU_CG_TARGET < FMUCG_STANDALONE_TARGETS

#define LoadFMUFcnDecl5                         \
    if (fmustruct->isLoggingOn) {                                       \
    void * diagnostic = CreateDiagnosticAsVoidPtr("FMUBlock:FMU:FMULoadLibFunctionError", 2, \
                                                  CODEGEN_SUPPORT_ARG_STRING_TYPE, fcnName, \
                                                  CODEGEN_SUPPORT_ARG_STRING_TYPE, fmustruct->fmuname); \
    /*A loading failure will cause a warning, ANY CALL to defualt Fcn will result in an Error and Stop*/ \
    rt_ssReportDiagnosticAsWarning(fmustruct->ssPtr, diagnostic);       \
    }

#else

#define LoadFMUFcnDecl5                         \
    message = formatString("Unable to load function '%s' for FMU '%s'.", fcnName, fmustruct->fmuname); \
    fmustruct->callbacks.logger(fmustruct->mFMIComp, fmustruct->fmuname, fmustruct->FMUErrorStatus, "LoadFMUFcn", message);

#endif

#define LoadFMUFcnDecl6(x, d)                                   \
    fcn = (x) d;                                                \
    }                                                           \
    return fcn;                                                 \
    }

#define LoadFMUFcnDecl(f, x, d)                                         \
    LoadFMUFcnDecl1(f, x) LoadFMUFcnDecl2 LoadFMUFcnDecl3(x) LoadFMUFcnDecl4 LoadFMUFcnDecl5 LoadFMUFcnDecl6(x, d) 

LoadFMUFcnDecl(LoadFMUFcnStatus, _fmi_default_status_fcn_type, defaultfcnStatus)
LoadFMUFcnDecl(LoadFMUFcnString, _fmi_default_string_fcn_type, defaultfcnString)
LoadFMUFcnDecl(LoadFMUFcnInstance, _fmi_default_instance_fcn_type, defaultfcnInstance)
LoadFMUFcnDecl(LoadFMUFcnVoid, _fmi_default_void_fcn_type, defaultfcnVoid)

/*Load FMU 1 ME Functions*/
static void LoadFMU1MEFunctions(struct FMU1_ME_RTWCG * fmustruct,
                                int loadGetRealOutputDerivativesFcn,
                                int loadSetRealInputDerivativesFcn) {
    (void)loadGetRealOutputDerivativesFcn;
    (void)loadSetRealInputDerivativesFcn;
    
    fmustruct->getModelTypesPlatform      = (_fmiGetModelTypesPlatform) LoadFMUFcnString(fmustruct, "fmiGetModelTypesPlatform", 0);
    fmustruct->getVersion                 = (_fmiGetVersion)         LoadFMUFcnString(fmustruct, "fmiGetVersion", 0);
    fmustruct->setDebugLogging            = (_fmiSetDebugLogging)    LoadFMUFcnStatus(fmustruct, "fmiSetDebugLogging", 0);   
    fmustruct->instantiateModel           = (_fmiInstantiateModel)   LoadFMUFcnInstance(fmustruct, "fmiInstantiateModel", 0);
    fmustruct->freeModelInstance          = (_fmiFreeModelInstance)  LoadFMUFcnVoid(fmustruct, "fmiFreeModelInstance", 0);
    fmustruct->setTime                    = (_fmiSetTime)            LoadFMUFcnStatus(fmustruct, "fmiSetTime", 0);    
    fmustruct->setContinuousStates        = (_fmiSetContinuousStates)      LoadFMUFcnStatus(fmustruct, "fmiSetContinuousStates", 0);
    fmustruct->completedIntegratorStep    = (_fmiCompletedIntegratorStep)  LoadFMUFcnStatus(fmustruct, "fmiCompletedIntegratorStep", 0);
    fmustruct->initialize                 = (_fmiInitialize)         LoadFMUFcnStatus(fmustruct, "fmiInitialize", 0);
    fmustruct->getDerivatives             = (_fmiGetDerivatives)     LoadFMUFcnStatus(fmustruct, "fmiGetDerivatives", 0);
    fmustruct->getEventIndicators         = (_fmiGetEventIndicators) LoadFMUFcnStatus(fmustruct, "fmiGetEventIndicators", 0);
    fmustruct->eventUpdate                = (_fmiEventUpdate)        LoadFMUFcnStatus(fmustruct, "fmiEventUpdate", 0);
    fmustruct->getContinuousStates        = (_fmiGetContinuousStates)         LoadFMUFcnStatus(fmustruct, "fmiGetContinuousStates", 0);
    fmustruct->getNominalContinuousStates = (_fmiGetNominalContinuousStates)  LoadFMUFcnStatus(fmustruct, "fmiGetNominalContinuousStates", 0);
    fmustruct->getStateValueReferences    = (_fmiGetStateValueReferences)     LoadFMUFcnStatus(fmustruct, "fmiGetStateValueReferences", 0);
    fmustruct->terminate                  = (_fmiTerminate)                   LoadFMUFcnStatus(fmustruct, "fmiTerminate", 0);
    fmustruct->setReal                    = (_fmiSetReal)                   LoadFMUFcnStatus(fmustruct, "fmiSetReal", 0);
    fmustruct->setInteger                 = (_fmiSetInteger)                LoadFMUFcnStatus(fmustruct, "fmiSetInteger", 0);
    fmustruct->setBoolean                 = (_fmiSetBoolean)                LoadFMUFcnStatus(fmustruct, "fmiSetBoolean", 0);
    fmustruct->setString                  = (_fmiSetString)                 LoadFMUFcnStatus(fmustruct, "fmiSetString", 0);
    fmustruct->getReal                    = (_fmiGetReal)                   LoadFMUFcnStatus(fmustruct, "fmiGetReal", 0);
    fmustruct->getInteger                 = (_fmiGetInteger)                LoadFMUFcnStatus(fmustruct, "fmiGetInteger", 0);
    fmustruct->getBoolean                 = (_fmiGetBoolean)                LoadFMUFcnStatus(fmustruct, "fmiGetBoolean", 0);
    fmustruct->getString                  = (_fmiGetString)                 LoadFMUFcnStatus(fmustruct, "fmiGetString", 0);
}

static fmiBoolean CheckStatus(struct FMU1_ME_RTWCG* fmustruct,
                              fmiStatus status,
                              fmiString fcnName) {
#if FMU_CG_TARGET < FMUCG_STANDALONE_TARGETS
    SimStruct* ss = fmustruct->ssPtr;
    
    if(status == fmiError || status == fmiFatal){
        if (fmustruct->isLoggingOn) {
            void * diagnostic = CreateDiagnosticAsVoidPtr("FMUBlock:FMU:FMUSimFunctionErrorDebugToDisplayOn", 2,
                                                          CODEGEN_SUPPORT_ARG_STRING_TYPE, fcnName,
                                                          CODEGEN_SUPPORT_ARG_STRING_TYPE, fmustruct->fmuname);
            rt_ssSet_slErrMsg(ss, diagnostic);
        }
        fmustruct->FMUErrorStatus = status;        
        ssSetStopRequested(ss, 1);
    }
#else
    fmiString message = NULL;
    if(status == fmiError || status == fmiFatal){
        fmustruct->FMUErrorStatus = status;
        message = formatString("Error in supplied FMU: An error occurred in function '%s' for block '%s' during simulation.", fcnName, fmustruct->fmuname);
        fmustruct->callbacks.logger(fmustruct->mFMIComp, fmustruct->fmuname, fmustruct->FMUErrorStatus, "CheckStatus", message);
    }
#endif
    
    if (status == fmiOK)
        return fmiTrue;
    else
        return fmiFalse;
}

/* fmuPrefix is modelIdentifier */
static void* FMU1_fmuInitialize(const char* lib,
                                fmiString instanceName,
                                fmiString modelIdentifier,
                                fmiString fmuGUID,
                                fmiString fmuLocation,
                                void* ssPtr,
                                int loadGetRealOutputDerivativesFcn,
                                int loadSetRealInputDerivativesFcn,
                                const void* fmuCallBacks,
                                fmiBoolean fmuVisible,
                                fmiBoolean fmuLoggingOn) {
    fmiString message = NULL;

    fmiCallbackAllocateMemory allocateMemory = (fmuCallBacks == NULL) ? calloc : ((const fmiCallbackFunctions*)(fmuCallBacks))->allocateMemory;
    struct FMU1_ME_RTWCG * fmustruct = (struct FMU1_ME_RTWCG *)allocateMemory(1, sizeof(struct FMU1_ME_RTWCG));
    fmustruct->callbacks.logger = (fmuCallBacks == NULL) ? fmu1Logger : ((const fmiCallbackFunctions*)(fmuCallBacks))->logger;
    fmustruct->callbacks.allocateMemory = allocateMemory;
    fmustruct->callbacks.freeMemory = (fmuCallBacks == NULL) ? free : ((const fmiCallbackFunctions*)(fmuCallBacks))->freeMemory;
    fmustruct->callbacks.stepFinished = (fmuCallBacks == NULL) ? NULL  : ((const fmiCallbackFunctions*)(fmuCallBacks))->stepFinished;
    
    (void) fmuVisible;
    (void) fmuLocation;
    fmiBoolean isLoggingOn = (fmustruct->callbacks.logger == NULL) ? fmiFalse : fmuLoggingOn;
       
#if FMU_CG_TARGET < FMUCG_STANDALONE_TARGETS
    (void) message;
    fmustruct->ssPtr = (SimStruct*) ssPtr;
#else
    (void) ssPtr;
#endif
    fmustruct->fmuname = (char *)instanceName;
    fmustruct->modelIdentifier = (char*)modelIdentifier;
    fmustruct->dllfile = (char *)lib;
    fmustruct->FMUErrorStatus = fmiOK;
    fmustruct->modelInitialized = false;
    fmustruct->isLoggingOn = (bool)isLoggingOn;

    /* add fake fmustruct pointer to list. used by error handling
       until real fmuInstance is instantiated.*/
    fmustruct->mFMIComp = (void*)(-1);
    fmu1SetGetFmuStruct(fmustruct->mFMIComp, 1, fmustruct);
    
    if (strlen(instanceName)+ FCN_NAME_MAX_LEN + 1 >= FULL_FCN_NAME_MAX_LEN) {
        /*FMU name is longer than 200+, rarely happens*/
        fmustruct->FMUErrorStatus = fmiFatal;
#if FMU_CG_TARGET < FMUCG_STANDALONE_TARGETS
        if (fmustruct->isLoggingOn) {
            void * diagnostic = CreateDiagnosticAsVoidPtr("SL_SERVICES:utils:PRINTFWRAPPER", 1,
                                                          CODEGEN_SUPPORT_ARG_STRING_TYPE, "FMU Name is too long.");
            rt_ssReportDiagnosticAsWarning(fmustruct->ssPtr, diagnostic);
        }
        ssSetStopRequested(fmustruct->ssPtr, 1);
#else
        message = formatString("FMU Name '%s' is too long.", instanceName);
        fmustruct->callbacks.logger(fmustruct->mFMIComp, fmustruct->fmuname, fmustruct->FMUErrorStatus, "FMU1_fmuInitialize", message);
#endif
        return NULL;
    }

    fmustruct->Handle = LOAD_LIBRARY(fmustruct->dllfile);
    if (NULL == fmustruct->Handle) {
        fmustruct->FMUErrorStatus = fmiFatal;
#if FMU_CG_TARGET < FMUCG_STANDALONE_TARGETS
        if (fmustruct->isLoggingOn) {
            void * diagnostic = CreateDiagnosticAsVoidPtr("FMUBlock:FMU:FMULoadLibraryError", 2,
                                                          CODEGEN_SUPPORT_ARG_STRING_TYPE, fmustruct->dllfile,
                                                          CODEGEN_SUPPORT_ARG_STRING_TYPE, fmustruct->fmuname);
            /*loading lib failure will cause an Fatal and Stop*/
            rt_ssReportDiagnosticAsWarning(fmustruct->ssPtr, diagnostic);
        }
        ssSetStopRequested(fmustruct->ssPtr, 1);
#else
        const char* errorCodeMessage = getLoadLibraryErrorMessage();
        message = formatString("Unable to load dynamic library: '%s'.", fmustruct->dllfile, errorCodeMessage);
        fmustruct->callbacks.logger(fmustruct->mFMIComp, fmustruct->fmuname, fmustruct->FMUErrorStatus, "FMU1_fmuInitialize", message);
#endif
        return NULL;
    }

    /* load fmi functions */
    LoadFMU1MEFunctions(fmustruct, loadGetRealOutputDerivativesFcn, loadSetRealInputDerivativesFcn);

    /* instantiate fmu */
    fmustruct->mFMIComp = fmustruct->instantiateModel(instanceName,
                                                      fmuGUID,
                                                      fmustruct->callbacks,
                                                      isLoggingOn);
    if (NULL == fmustruct->mFMIComp ){
        CheckStatus(fmustruct, fmiError, "fmiInstantiateModel");
        CLOSE_LIBRARY(fmustruct->Handle);
        fmustruct->callbacks.freeMemory(fmustruct);
        return NULL;
    }
    
    /* add fmustruct pointer to list */
    fmu1SetGetFmuStruct(fmustruct->mFMIComp, 1, fmustruct);
    
    return (void *) fmustruct;
}

void* FMU1_fmuInitializeME(const char * lib,
                           fmiString instanceName,
                           fmiString modelIdentifier,
                           fmiString fmuGUID,
                           fmiString fmuLocation,
                           void* ssPtr,
                           int loadGetRealOutputDerivativesFcn,
                           int loadSetRealInputDerivativesFcn,
                           const void* fmuCallBacks,
                           fmiBoolean fmuVisible,
                           fmiBoolean fmuLoggingOn) {
    return FMU1_fmuInitialize(lib,
                              instanceName,
                              modelIdentifier,
                              fmuGUID,
                              fmuLocation,
                              ssPtr,
                              loadGetRealOutputDerivativesFcn,
                              loadSetRealInputDerivativesFcn,
                              fmuCallBacks,
                              fmuVisible,
                              fmuLoggingOn);
}

fmiBoolean FMU1_terminateModel(void **fmuv) {
    struct FMU1_ME_RTWCG * fmustruct = (struct FMU1_ME_RTWCG *)(*fmuv);
    if (fmustruct == NULL) {
        return fmiTrue;
    }
    if(fmustruct->FMUErrorStatus != fmiFatal) {
       if(fmustruct->FMUErrorStatus != fmiError &&
          fmustruct->modelInitialized == true) {
            fmiStatus fmiFlag = fmustruct->terminate(fmustruct->mFMIComp);
            CheckStatus(fmustruct, fmiFlag, "fmiTerminate");
          }
        fmustruct->freeModelInstance(fmustruct->mFMIComp);
    }
    CLOSE_LIBRARY(fmustruct->Handle);
    fmiCallbackFreeMemory freeMemory = fmustruct->callbacks.freeMemory;
    freeMemory(fmustruct);
    return fmiTrue;
}

void FMU1_checkSimTerminate(void **fmuv,
                            const char* blkPath,
                            fmiReal time) {
    struct FMU1_ME_RTWCG * fmustruct = (struct FMU1_ME_RTWCG *)(*fmuv);
    fmiString message = NULL;
#if FMU_CG_TARGET < FMUCG_STANDALONE_TARGETS
    (void) message;
    if(fmustruct->eventInfo.terminateSimulation == fmiTrue){
        /* terminate the simulation (successfully) */
        if (fmustruct->isLoggingOn) {
            void * diagnostic = CreateDiagnosticAsVoidPtr("FMUBlock:FMU:FMUSimEventUpdateTerminated", 2,
                                                          CODEGEN_SUPPORT_ARG_STRING_TYPE, blkPath,
                                                          CODEGEN_SUPPORT_ARG_REAL_TYPE, time);
            rt_ssReportDiagnosticAsInfo(fmustruct->ssPtr, diagnostic);
        }
        ssSetStopRequested(fmustruct->ssPtr, 1);
    }
#else
    message = formatString("'TerminateSimulation' flag was set to true by FMU '%s' during 'fmiEventUpdate'."
                                     " Simulation will stop after the current time step at %f.", blkPath, time);
    fmustruct->callbacks.logger(fmustruct->mFMIComp, fmustruct->fmuname, fmustruct->FMUErrorStatus, "FMU1_checkSimTerminate", message);
#endif
}

fmiBoolean FMU1_initialize(void **fmuv,
                          unsigned char isToleranceUsed,
                          fmiReal toleranceValue) {
    struct FMU1_ME_RTWCG * fmustruct = (struct FMU1_ME_RTWCG *)(*fmuv);
    fmiStatus fmiFlag = fmustruct->initialize(fmustruct->mFMIComp,
                                              (fmiBoolean) isToleranceUsed,
                                              toleranceValue, &(fmustruct->eventInfo));
    fmustruct->modelInitialized = true;
    return CheckStatus(fmustruct, fmiFlag, "fmiInitialize");
}

fmiBoolean FMU1_setTime(void **fmuv,
                       fmiReal time) {
    struct FMU1_ME_RTWCG * fmustruct = (struct FMU1_ME_RTWCG *)(*fmuv);
    fmiStatus fmiFlag = fmustruct->setTime(fmustruct->mFMIComp, time);
    return CheckStatus(fmustruct, fmiFlag, "fmisetTime");
}

fmiBoolean FMU1_completedIntegratorStep(void **fmuv,
                                       int8_T* hasStepEvent) {
    struct FMU1_ME_RTWCG * fmustruct = (struct FMU1_ME_RTWCG *)(*fmuv);
    fmiStatus fmiFlag = fmustruct->completedIntegratorStep(fmustruct->mFMIComp, (fmiBoolean *)hasStepEvent);
    return CheckStatus(fmustruct, fmiFlag, "completedIntegratorStep");
}

fmiBoolean FMU1_setContinuousStates(void **fmuv,
                                   fmiReal states[],
                                   size_t nx) {
    struct FMU1_ME_RTWCG * fmustruct = (struct FMU1_ME_RTWCG *)(*fmuv);
    fmiStatus fmiFlag = fmustruct->setContinuousStates(fmustruct->mFMIComp, states, nx);
    return CheckStatus(fmustruct, fmiFlag, "setContinuousStates");
}
    
fmiBoolean FMU1_getContinuousStates(void **fmuv,
                                   fmiReal states[],
                                   size_t nx) {
    struct FMU1_ME_RTWCG * fmustruct = (struct FMU1_ME_RTWCG *)(*fmuv);
    fmiStatus fmiFlag = fmustruct->getContinuousStates(fmustruct->mFMIComp, states, nx);
    return CheckStatus(fmustruct, fmiFlag, "getContinuousStates");
}

fmiBoolean FMU1_getDerivatives(void **fmuv,
                              fmiReal derivatives[],
                              size_t nx) {
    struct FMU1_ME_RTWCG * fmustruct = (struct FMU1_ME_RTWCG *)(*fmuv);
    fmiStatus fmiFlag = fmustruct->getDerivatives(fmustruct->mFMIComp, derivatives, nx);
    return CheckStatus(fmustruct, fmiFlag, "getDerivatives");
}

fmiBoolean FMU1_getEventIndicators(void **fmuv,
                                  fmiReal eventIndicators[],
                                  size_t nx) {
    struct FMU1_ME_RTWCG * fmustruct = (struct FMU1_ME_RTWCG *)(*fmuv);
    fmiStatus fmiFlag = fmustruct->getEventIndicators(fmustruct->mFMIComp, eventIndicators, nx);
    return CheckStatus(fmustruct, fmiFlag, "getEventIndicators");
}

/* event helper functions*/
fmiBoolean FMU1_eventIteration(void **fmuv,
                              const char* blkPath,
                              fmiReal time) {
    struct FMU1_ME_RTWCG * fmustruct = (struct FMU1_ME_RTWCG *)(*fmuv);
    fmiStatus fmiFlag = fmiOK;
    fmiBoolean returnStatus = fmiTrue;
    int iterationNumber = 0;
    fmiString message = NULL;
#if FMU_CG_TARGET < FMUCG_STANDALONE_TARGETS
    (void) message;
#endif
    while(fmustruct->eventInfo.iterationConverged == fmiFalse){
        /*safe call to eventUpdate*/
        fmiFlag = fmustruct->eventUpdate(fmustruct->mFMIComp, fmiFalse, &(fmustruct->eventInfo));
        returnStatus = CheckStatus(fmustruct, fmiFlag, "eventUpdate");

        if(returnStatus == fmiTrue && fmustruct->eventInfo.terminateSimulation == fmiTrue){
#if FMU_CG_TARGET < FMUCG_STANDALONE_TARGETS
            /* terminate the simulation (successfully) */
            if (fmustruct->isLoggingOn) {
                void * diagnostic = CreateDiagnosticAsVoidPtr("FMUBlock:FMU:FMUSimEventUpdateTerminated", 2,
                                                              CODEGEN_SUPPORT_ARG_STRING_TYPE, blkPath,
                                                              CODEGEN_SUPPORT_ARG_REAL_TYPE, time);
                rt_ssReportDiagnosticAsInfo(fmustruct->ssPtr, diagnostic);
            }
            ssSetStopRequested(fmustruct->ssPtr, 1);
#else
            message = formatString("'TerminateSimulation' flag was set to true by FMU %s during 'fmiEventUpdate'."
                                             " Simulation will stop after the current time step at %f.", blkPath, time);
            fmustruct->callbacks.logger(fmustruct->mFMIComp, fmustruct->fmuname, fmustruct->FMUErrorStatus, "FMU1_checkSimTerminate", message);
#endif
        }


        if(iterationNumber >= 10000){
#if FMU_CG_TARGET < FMUCG_STANDALONE_TARGETS
            if (fmustruct->isLoggingOn) {
                void * diagnostic = CreateDiagnosticAsVoidPtr("FMUBlock:FMU:FMUSimEventUpdateNotConverge", 3,
                                                              CODEGEN_SUPPORT_ARG_REAL_TYPE, time,
                                                              CODEGEN_SUPPORT_ARG_INTEGER_TYPE, iterationNumber,
                                                              CODEGEN_SUPPORT_ARG_STRING_TYPE, blkPath);
                rt_ssReportDiagnosticAsWarning(fmustruct->ssPtr, diagnostic);
            }
#else
            message = formatString("Error in supplied FMU: 'fmiEventUpdate' at time %f iterated over %d times in block %s.", time, iterationNumber, blkPath);
            
            fmustruct->callbacks.logger(fmustruct->mFMIComp, fmustruct->fmuname, fmustruct->FMUErrorStatus, "FMU1_eventIteration", "FMUSimEventUpdateNotConverge.");
#endif
            returnStatus = fmiFalse;
            break;
            
        } else
            iterationNumber ++;
    }
    return returnStatus;
}

void FMU1_ifStateVRchanged(void **fmuv,
                           int8_T* fmustateValueOrStateValueRefChanged) {
    struct FMU1_ME_RTWCG * fmustruct = (struct FMU1_ME_RTWCG *)(*fmuv);
    *fmustateValueOrStateValueRefChanged = (int8_T) (fmustruct->eventInfo.stateValuesChanged || fmustruct->eventInfo.stateValueReferencesChanged);
}

void FMU1_setIterationConverged(void **fmuv,
                                int fmuIsInitialized,
                                fmiReal time) {
    struct FMU1_ME_RTWCG * fmustruct = (struct FMU1_ME_RTWCG *)(*fmuv);
    bool callEventUpdate;
    if(fmuIsInitialized != 0){
        fmustruct->eventInfo.iterationConverged = fmiFalse;
    } else {
        callEventUpdate = (fmustruct->eventInfo.upcomingTimeEvent == fmiTrue &&
                           fmustruct->eventInfo.nextEventTime <= time);
        fmustruct->eventInfo.iterationConverged = (callEventUpdate) ? fmiFalse : fmiTrue;
    }
}

void FMU1_getNextEventTime(void **fmuv,
                           fmiReal* nextEventTime,
                           int8_T* upcomingTimeEvent) {
    struct FMU1_ME_RTWCG * fmustruct = (struct FMU1_ME_RTWCG *)(*fmuv);
    *nextEventTime = fmustruct->eventInfo.nextEventTime;
    *upcomingTimeEvent = (int8_T) fmustruct->eventInfo.upcomingTimeEvent;
}

/*get set function*/
fmiBoolean FMU1ME_setRealVal(void **fmuv,
                            const fmiValueReference dvr,
                            size_t nvr,
                            const fmiReal dvalue) {
    struct FMU1_ME_RTWCG * fmustruct = (struct FMU1_ME_RTWCG *)(*fmuv);
    fmiValueReference vr =dvr;
    fmiReal value = dvalue;
    fmiStatus fmiFlag = fmustruct->setReal(fmustruct->mFMIComp, &vr, nvr, &value);
    return CheckStatus(fmustruct, fmiFlag, "fmiSetReal");
}

fmiBoolean FMU1ME_setReal(void **fmuv,
                         const fmiValueReference dvr,
                         size_t nvr,
                         const fmiReal value[]) {
    struct FMU1_ME_RTWCG * fmustruct = (struct FMU1_ME_RTWCG *)(*fmuv);
    fmiValueReference vr =dvr;
    fmiStatus fmiFlag = fmustruct->setReal(fmustruct->mFMIComp, &vr, nvr, value);
    return CheckStatus(fmustruct, fmiFlag, "fmiSetReal");
}

fmiBoolean FMU1ME_getReal(void **fmuv,
                         const fmiValueReference dvr,
                         size_t nvr,
                         fmiReal value[]) {
    struct FMU1_ME_RTWCG * fmustruct = (struct FMU1_ME_RTWCG *)(*fmuv);
    fmiValueReference vr =dvr;
    fmiStatus fmiFlag = fmustruct->getReal(fmustruct->mFMIComp, &vr, nvr, value);
    return CheckStatus(fmustruct, fmiFlag, "fmiGetReal");
}

fmiBoolean FMU1ME_setIntegerVal(void **fmuv,
                               const fmiValueReference dvr,
                               size_t nvr,
                               const fmiInteger dvalue) {
    struct FMU1_ME_RTWCG * fmustruct = (struct FMU1_ME_RTWCG *)(*fmuv);
    fmiValueReference vr =dvr;
    fmiInteger value = dvalue;
    fmiStatus fmiFlag = fmustruct->setInteger(fmustruct->mFMIComp, &vr, nvr, &value);
    return CheckStatus(fmustruct, fmiFlag, "fmiGetInteger");
}

fmiBoolean FMU1ME_setInteger(void **fmuv,
                            const fmiValueReference dvr,
                            size_t nvr,
                            const fmiInteger value[]) {
    struct FMU1_ME_RTWCG * fmustruct = (struct FMU1_ME_RTWCG *)(*fmuv);
    fmiValueReference vr =dvr;
    fmiStatus fmiFlag = fmustruct->setInteger(fmustruct->mFMIComp, &vr, nvr, value);
    return CheckStatus(fmustruct, fmiFlag, "fmiSetInteger");
}

fmiBoolean FMU1ME_getInteger(void **fmuv,
                            const fmiValueReference dvr,
                            size_t nvr,
                            fmiInteger value[]) {
    struct FMU1_ME_RTWCG * fmustruct = (struct FMU1_ME_RTWCG *)(*fmuv);
    fmiValueReference vr =dvr;
    fmiStatus fmiFlag = fmustruct->getInteger(fmustruct->mFMIComp, &vr, nvr, value);
    return CheckStatus(fmustruct, fmiFlag, "fmiGetInteger");
}

fmiBoolean FMU1ME_setBooleanVal(void **fmuv,
                               const fmiValueReference dvr,
                               size_t nvr,
                               const unsigned char dvalue) {
    struct FMU1_ME_RTWCG * fmustruct = (struct FMU1_ME_RTWCG *)(*fmuv);
    fmiValueReference vr =dvr;
    fmiBoolean value = (fmiBoolean) dvalue;
    fmiStatus fmiFlag = fmustruct->setBoolean(fmustruct->mFMIComp, &vr, nvr, &value);
    return CheckStatus(fmustruct, fmiFlag, "fmiSetBoolean");
}

fmiBoolean FMU1ME_setBoolean(void **fmuv,
                            const fmiValueReference dvr,
                            size_t nvr,
                            const unsigned char value[]) {
    struct FMU1_ME_RTWCG * fmustruct = (struct FMU1_ME_RTWCG *)(*fmuv);
    fmiValueReference vr =dvr;
    fmiStatus fmiFlag = fmustruct->setBoolean(fmustruct->mFMIComp, &vr, nvr, (fmiBoolean *) value);
    return CheckStatus(fmustruct, fmiFlag, "fmiSetBoolean");
}

fmiBoolean FMU1ME_getBoolean(void **fmuv,
                            const fmiValueReference dvr,
                            size_t nvr,
                            unsigned char value[]) {
    struct FMU1_ME_RTWCG * fmustruct = (struct FMU1_ME_RTWCG *)(*fmuv);
    fmiValueReference vr =dvr;
    fmiStatus fmiFlag = fmustruct->getBoolean(fmustruct->mFMIComp, &vr, nvr, (fmiBoolean *) value);
    return CheckStatus(fmustruct, fmiFlag, "fmiGetBoolean");
}

fmiBoolean FMU1ME_setStringVal(void **fmuv,
                              const fmiValueReference dvr,
                              size_t nvr,
                              const fmiString dvalue) {
    struct FMU1_ME_RTWCG * fmustruct = (struct FMU1_ME_RTWCG *)(*fmuv);
    fmiValueReference vr =dvr;
    fmiString value = dvalue;
    fmiStatus fmiFlag = fmustruct->setString(fmustruct->mFMIComp, &vr, nvr, &value);
    return CheckStatus(fmustruct, fmiFlag, "fmiSetString");
}

fmiBoolean FMU1ME_setString(void **fmuv,
                           const fmiValueReference dvr,
                           size_t nvr,
                           const fmiString value[]) {
    struct FMU1_ME_RTWCG * fmustruct = (struct FMU1_ME_RTWCG *)(*fmuv);
    fmiValueReference vr =dvr;
    fmiStatus fmiFlag = fmustruct->setString(fmustruct->mFMIComp, &vr, nvr, value);
    return CheckStatus(fmustruct, fmiFlag, "fmiSetString");
}

fmiBoolean FMU1ME_getString(void **fmuv,
                           const fmiValueReference dvr,
                           size_t nvr, fmiString value[]) {
    struct FMU1_ME_RTWCG * fmustruct = (struct FMU1_ME_RTWCG *)(*fmuv);
    fmiValueReference vr =dvr;
    fmiStatus fmiFlag = fmustruct->getString(fmustruct->mFMIComp, &vr, nvr, value);
    return CheckStatus(fmustruct, fmiFlag, "fmiGetString");
}
