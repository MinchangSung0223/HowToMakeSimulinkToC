 /*
 * Abstract:
 *   Main program for the Rapid Acceleration target.
 *
 *
 * For history of changes of raccel_main.c prior to this comment:
 * see history of matlab/rtw/c/raccel/raccel_main_new.c
 *
 * Copyright 2007-2023 The MathWorks, Inc.
 */

#define IN_RACCEL_MAIN

#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <setjmp.h>
#include "tmwtypes.h"
#include "slsa_sim_engine.h"
#include "simstruc.h"
#include "rt_nonfinite.h"
#include "raccel.h"
#include "raccel_sup.h"
#include "raccel_utils.h"
#include "ext_work.h"
#include "slexec_parallel.h"

#include "sl_AsyncioQueue/AsyncioQueueCAPI.h"
#include "simlogCIntrf.h"
#include "i18n_c_api.h"
#include "rtw_capi.h"
#include "rtw_modelmap_simtarget.h"
#include "sigstream_rtw.h"

#if defined (UNIX)
#include <unistd.h>
#endif

/* for accessing root IOs, params, etc... using the C-API */
extern rtwCAPI_ModelMappingInfo* rt_modelMapInfoPtr;

#ifdef NOT_USING_NONFINITE_LITERALS
extern void rt_InitInfAndNaN(size_t realSize);
#endif

/*=========*
 * Defines *
 *=========*/

#ifndef TRUE
#define FALSE (0)
#define TRUE  (1)
#endif

#ifndef EXIT_FAILURE
#define EXIT_FAILURE  1
#endif
#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS  0
#endif

#define ERROR_EXIT(msg, cond)                   \
    if (cond) {                                 \
        (void)fprintf(stderr, msg, cond);       \
        return(EXIT_FAILURE);                   \
    }

/*====================*
 * External functions *
 *====================*/
extern SimStruct *raccel_register_model(ssExecutionInfo* executionInfo);
extern void raccel_setup_MMIStateLog(SimStruct*);

extern void MdlInitializeSizes(void);
extern void MdlInitializeSampleTimes(void);
extern void MdlStart(void);
extern void MdlOutputs(int_T tid);
extern void MdlUpdate(int_T tid);
extern void MdlTerminate(void);
extern void MdlOutputsParameterSampleTime(int_T tid);

boolean_T  ExtWaitForStartPkt(void);

#define SL_MAX(A, B)   (((A) > (B)) ? (A) : (B))

#if defined(TGTCONN)
extern void TgtConnTerm();
#else
#define TgtConnTerm()                /* do nothing */
#endif


/* Global data */
extern SimStruct *const rtS; /* defined in model.c, used in AtExit */
extern boolean_T   gblSetParamPktReceived;

#if MODEL_HAS_DYNAMICALLY_LOADED_SFCNS
extern size_t      gblNumMexFileSFcns;
#endif

static struct {
    int matFileFormat;
} gblStatusFlags;

ssExecutionInfo gblExecutionInfo;

/* Functions */
static void fillExecutionInfo(SimStruct *S);

/* Function types for replacement of tofile and fromfile filenames */
typedef void (*raccelToFileNameReplacementFcn)(const char *blockPath, char *fileName);
typedef void (*raccelFromFileNameReplacementFcn)(const char *blockPath, char *fileName);
raccelToFileNameReplacementFcn toFileNameReplacementFunction= NULL;
raccelFromFileNameReplacementFcn fromFileNameReplacementFunction= NULL;

#ifdef __LCC__
void sdiBindObserversAndStartStreamingEngine_wrapper(const char* modelName)
{
    sdiBindObserversAndStartStreamingEngine(modelName);
}
#endif


/* This function is called at program exit */
static void AtExitFcn(void)
{
    slsaAtProgramExit(&gblExecutionInfo);  /* do not add any code after this line! */    
} /* AtExitFcn */

const char* getParamFilename(void) {
    return gblExecutionInfo.simulationOptions_.paramFilename_;
}

const char* getToFileSuffix(void) {
    return gblExecutionInfo.simulationOptions_.toFileSuffix_;
}

/* long jumper */
void raccelSFcnLongJumper(const char* errMsg)
{
    gblExecutionInfo.simulationOptions_.errorStatus_ = errMsg;
    slsaLogInfo("Long jumping from raccelSFcnLongJumper()");
    longjmp(gblExecutionInfo.gblObjects_.rapidAccelJmpBuf, 1);
}

int_T main(int_T argc, char_T *argv[])
{
    const char* program = argv[0];
    volatile SimStruct* S = NULL;
    boolean_T extModeStartPktReceived = false;

    /**********************************************************************************************/
    /************************* !!! DO NOT ADD ANY CODE ABOVE THIS LINE !!! ************************/

    /* register the atexit function */
    slsaLogInfo("Registering atexit function");
    if ( atexit(AtExitFcn) != 0 ) {
        gblExecutionInfo.simulationOptions_.errorStatus_ = "Unable set exit function";
        ERROR_EXIT("Fatal Error: %s\n", gblExecutionInfo.simulationOptions_.errorStatus_);
    }

    /* Enable timing measurements */
    slsaOpenDebugTimingLog(program);

    
    /* Initialize some parameters to default values */
    slsaLogInfo("Initialize gblExecutionInfo");
    gblExecutionInfo.simulationOptions_.timeLimit_ = NULL;
    gblExecutionInfo.simulationOptions_.parameterArrayIndex_ = -1;
    gblExecutionInfo.runtimeFlags_.startPacketReceived_ = false;
    gblExecutionInfo.runtimeFlags_.simulationTimedOut_ = false;
    gblExecutionInfo.deployedExecution_.runningDeployed_ = 0U;
    gblExecutionInfo.simulationOptions_.parameterFileName_ = NULL;

#ifdef NOT_USING_NONFINITE_LITERALS
    rt_InitInfAndNaN(sizeof(real_T));
#endif

    /* Initialize i18n services explicitly. This should ideally be called as early as possible */
    slsaLogInfo("i18n_init");
    i18n_init();
    SLSA_DEBUG_TIMING_LOG("i18n_init");

    /*C++ Argument Parser*/
    slsaLogInfo("Parse Arguments");
    slsaParseArguments(argc, argv, &gblExecutionInfo);
    ERROR_EXIT("Error parsing input arguments: %s\n", gblExecutionInfo.simulationOptions_.errorStatus_);


    if(gblExecutionInfo.simulationOptions_.inportFileName_)
        gblExecutionInfo.gblObjects_.inportFileName = (char*)(gblExecutionInfo.simulationOptions_.inportFileName_);
    if(gblExecutionInfo.simulationOptions_.matSigstreamLoggingFilename_)
        gblExecutionInfo.gblObjects_.matSigstreamLoggingFilename = (char*)(gblExecutionInfo.simulationOptions_.matSigstreamLoggingFilename_);
    if(gblExecutionInfo.simulationOptions_.matSigLogSelectorFilename_)
        gblExecutionInfo.gblObjects_.matSigLogSelectorFilename = (char*)(gblExecutionInfo.simulationOptions_.matSigLogSelectorFilename_);

    /* Parse arguments */
    gblExecutionInfo.simulationOptions_.errorStatus_ = ParseArgs(argc, argv, &gblExecutionInfo);
    ERROR_EXIT("Error parsing input arguments: %s\n", gblExecutionInfo.simulationOptions_.errorStatus_);
    SLSA_DEBUG_TIMING_LOG("Parse Args");

    /* Setup the execution services */
    slsaLogInfo("SetupExecutionServices");
    slsaSetupExecutionServices(&gblExecutionInfo);
    SLSA_DEBUG_TIMING_LOG("SetupExecutionServices");

    /* Create ISigstreamManager instance */
    gblExecutionInfo.runtimeObjects_.iSigstreamManager_ = rtwISigstreamManagerCreateInstance();
    boolean_T extModeEnabled = (gblExecutionInfo.runtimeCallbacks_.externalModeCallbacks_ != NULL);

    /* Initialize Jetstream engine */
    slsaLogInfo("sdiInitializeForHostBasedTarget");
    sdiInitializeForHostBasedTarget(extModeEnabled,
                                    gblExecutionInfo.simulationOptions_.simDataRepoFilename_,
                                    gblExecutionInfo.gblObjects_.matSigLogSelectorFilename,
                                    gblExecutionInfo.simulationOptions_.liveStreaming_,
                                    gblExecutionInfo.deployedExecution_.runningDeployed_,
                                    gblExecutionInfo.gblObjects_.simDataRepoIndex);
    sdiInitializeLogIntervalsForHostBasedTarget(gblExecutionInfo.simulationOptions_.parameterFileName_);
    SLSA_DEBUG_TIMING_LOG("Initialize Jetstream engine");

#ifndef __LCC__
    /*
     * Long-jump handling is required here because, for example, a dynamically loaded s-function can
     * long-jump out of its initialize-sizes method, which is called from raccel_register_model
     * before the execution engine is created
     */
    {
        int setJmpReturnValue = setjmp(gblExecutionInfo.gblObjects_.rapidAccelJmpBuf);
        if (setJmpReturnValue != 0) {
            if (gblExecutionInfo.simulationOptions_.simMetadataFilePath_ && S) {
                ssWriteSimMetadata((SimStruct*)S, gblExecutionInfo.simulationOptions_.simMetadataFilePath_);
            }
            if (extModeEnabled) {
                raccelForceExtModeShutdown(extModeStartPktReceived);
            }
            return EXIT_FAILURE;
        }
    }
#endif

#if MODEL_HAS_DYNAMICALLY_LOADED_SFCNS
    /* Dynamically loaded s-functions */
    slsaLogInfo("raccelInitializeForMexSFcns");
    raccelInitializeForMexSFcns(gblExecutionInfo.simulationOptions_.errorStatus_, gblExecutionInfo.runtimeFlags_.sFcnInfoFileName);
    ERROR_EXIT("%s", gblExecutionInfo.simulationOptions_.errorStatus_);
    SLSA_DEBUG_TIMING_LOG("raccelInitializeForMexSFcns");
#endif

    /* Initialize the model */
    /* Setup the OperatingPoint ModelData instance in the generated raccel_register_model */
    S = raccel_register_model(&gblExecutionInfo);
    ERROR_EXIT("Error during model registration: %s\n", ssGetErrorStatus(S));

    gblExecutionInfo.simulationOptions_.numRootInportBlks_ = gblExecutionInfo.gblObjects_.numRootInportBlks;

    /* allocate files*/
    gblExecutionInfo.simulationOptions_.errorStatus_ = allocMemForToFromFileBlocks(&gblExecutionInfo);
    ERROR_EXIT("Error allocating files: %s\n", gblExecutionInfo.simulationOptions_.errorStatus_);
    SLSA_DEBUG_TIMING_LOG("Allocating files.");

    fillExecutionInfo((SimStruct*) S);

    toFileNameReplacementFunction= rt_RAccelReplaceToFilename;
    fromFileNameReplacementFunction= rt_RAccelReplaceFromFilename;

    ssClearFirstInitCondCalled(S);

    slsaLogInfo("Initialize the model");
    MdlInitializeSizes();
    MdlInitializeSampleTimes();
    SLSA_DEBUG_TIMING_LOG("Initialize the model");

    /* load additional options */
    slsaLogInfo("rsimLoadOptionsFromMATFile");
    rsimLoadOptionsFromMATFile((SimStruct*) S);
    ERROR_EXIT("Error loading additional options: %s\n", ssGetErrorStatus(S));
    SLSA_DEBUG_TIMING_LOG("rsimLoadOptionsFromMATFile");

    /* Run simulation */
    /* To add callbacks into generated/C code during simulation, see notes at
     * ExecutionEngine::run() implementation in slexec simbridge */
    slsaLogInfo("ssRunSimulation");
    ssRunSimulation((SimStruct*) S);
    SLSA_DEBUG_TIMING_LOG("RunSimulation Finished");

    /* Delete OperatingPoint ModelData */
    slsaLogInfo("slsaFreeOPModelData");
    slsaFreeOPModelData((void*)S);
    SLSA_DEBUG_TIMING_LOG("slsaFreeOPModelData");

    /********************
     * Cleanup and exit *
     ********************/
    if (ssGetErrorStatus(S) != NULL) {
        (void)fprintf(stderr, "Error: %s\n", ssGetErrorStatus(S));
    }

    slsaLogInfo("rt_FreeParamStructs");
    rt_FreeParamStructs();
    SLSA_DEBUG_TIMING_LOG("rt_FreeParamStructs");

    /* Target connectivity termination */
    slsaLogInfo("TgtConnTerm");
    TgtConnTerm();
    SLSA_DEBUG_TIMING_LOG("TgtConnTerm");

    /* Destroy ISigstreamManager instance */
    slsaLogInfo("rtwISigstreamManagerDestroyInstance");
    rtwISigstreamManagerDestroyInstance(gblExecutionInfo.runtimeObjects_.iSigstreamManager_);
    SLSA_DEBUG_TIMING_LOG("rtwISigstreamManagerDestroyInstance");

    /* Destroy LoggingInterval instance */
    slsaLogInfo("rtwLoggingIntervalDestroyInstance");
    rtwLoggingIntervalDestroyInstance(rtliGetLoggingInterval(ssGetRTWLogInfo(S)));
    SLSA_DEBUG_TIMING_LOG("rtwLoggingIntervalDestroyInstance");

    slsaLogInfo("rt_RapidReleaseDiagLoggerDB");
    rt_RapidReleaseDiagLoggerDB();
    SLSA_DEBUG_TIMING_LOG("rt_RapidReleaseDiagLoggerDB");

    slsaLogInfo("rt_RapidFreeGbls");
    rt_RapidFreeGbls(gblStatusFlags.matFileFormat);
    SLSA_DEBUG_TIMING_LOG("rt_RapidFreeGbls");

    /* Destroy reval raccel manager instance */
    slsaLogInfo("rtDestroyRevalMgr");
    rtDestroyRevalMgr();
    SLSA_DEBUG_TIMING_LOG("rtDestroyRevalMgr");

    return ssGetErrorStatus(S) ? EXIT_FAILURE : EXIT_SUCCESS;

} /* end main */



static void fillExecutionInfo(SimStruct* S)
{
    ssExecutionInfo *exInfo = ssGetExecutionInfo(S);

    /* Simulation Options */
    exInfo->simulationOptions_.enableSLExecSSBridgeFeatureValue_ = ENABLE_SLEXEC_SSBRIDGE;
    exInfo->simulationOptions_.matFileFormat_ = &(gblStatusFlags.matFileFormat);

    /* model methods */
    exInfo->modelMethods_.start = &MdlStart;
    exInfo->modelMethods_.outputsParameterSampleTime = &MdlOutputsParameterSampleTime;
    exInfo->modelMethods_.terminate = &MdlTerminate;

    /* Runtime callbacks */
    exInfo->runtimeCallbacks_.setupMMIStateLog = &raccel_setup_MMIStateLog;
    exInfo->runtimeCallbacks_.rapidReadInportsAndAperiodicHitTimes =
        &raccelLoadInputsAndAperiodicHitTimes;
    exInfo->runtimeCallbacks_.rapidCheckRemappings = &rt_RapidCheckRemappings;
#ifdef __LCC__
    exInfo->runtimeCallbacks_.sdiBindObserversAndStartStreamingEngine =
        &sdiBindObserversAndStartStreamingEngine_wrapper;
#else
    exInfo->runtimeCallbacks_.sdiBindObserversAndStartStreamingEngine =
        &sdiBindObserversAndStartStreamingEngine;
#endif
    exInfo->runtimeCallbacks_.nextAperiodicPartitionHitTime = &rt_NextAperiodicPartitionHitTime;
    exInfo->runtimeCallbacks_.updateParamsFcn = &rt_RapidReadMatFileAndUpdateParams;

    exInfo->rootIODataMethods_.getRootInput = &getRootInput;
    exInfo->rootIODataMethods_.getRootOutput = &getRootOutput;
    exInfo->rootIODataMethods_.getNumRootInputs = &getNumRootInputs;
    exInfo->rootIODataMethods_.getNumRootOutputs = &getNumRootOutputs;
    exInfo->rootIODataMethods_.getRootInputSize = &getRootInputSize;
    exInfo->rootIODataMethods_.getRootOutputSize = &getRootOutputSize;
    exInfo->rootIODataMethods_.getRootInputDatatypeSSId = &getRootInputDatatypeSSId;
    exInfo->rootIODataMethods_.getRootInputDimArray = &getRootInputDimArray;
    exInfo->rootIODataMethods_.getRootInputNumDims = &getRootInputNumDims;
    exInfo->rootIODataMethods_.getRootOutputDatatypeSSId = &getRootOutputDatatypeSSId;
    exInfo->rootIODataMethods_.getRootOutputDimArray = &getRootOutputDimArray;
    exInfo->rootIODataMethods_.getRootOutputNumDims = &getRootOutputNumDims;
    exInfo->rootIODataMethods_.getRootOutputDataIsComplex = &getRootOutputDataIsComplex;
    exInfo->rootIODataMethods_.getRootInputDataIsComplex = &getRootInputDataIsComplex;
    exInfo->rootIODataMethods_.getRootInputPortNumber = &getRootInputPortNumber;
    exInfo->rootIODataMethods_.getRootOutputPortNumber = &getRootOutputPortNumber;
    /* Runtime flags */
    exInfo->runtimeFlags_.parameterPacketReceived_ = &gblSetParamPktReceived;

    /* Runtime objects */
#ifdef __LCC__
    exInfo->runtimeObjects_.longJumpBuffer_ = NULL;
#else
    exInfo->runtimeObjects_.longJumpBuffer_ = &gblExecutionInfo.gblObjects_.rapidAccelJmpBuf;
#endif

    /* Parallel options */
#ifdef RACCEL_ENABLE_PARALLEL_EXECUTION
     exInfo->parallelExecution_.enabled_ = TRUE;
     exInfo->parallelExecution_.simulatorType_ = RACCEL_PARALLEL_SIMULATOR_TYPE;
     exInfo->parallelExecution_.options_.numberOfNodes = RACCEL_NUM_PARALLEL_NODES;
     exInfo->parallelExecution_.options_.numberOfThreads = RACCEL_PARALLEL_EXECUTION_NUM_THREADS;
     exInfo->parallelExecution_.options_.enableTiming = false;
     exInfo->parallelExecution_.options_.numberOfStepsToAnalyze = 0;
     exInfo->parallelExecution_.options_.timingOutputFilename = NULL;
     exInfo->parallelExecution_.options_.nodeExecutionModesFilename = NULL;
     exInfo->parallelExecution_.options_.parallelExecutionMode = PARALLEL_EXECUTION_OFF;
#else
     exInfo->parallelExecution_.enabled_ = FALSE;
#endif
     exInfo->gblObjects_.mmi = (const rtwCAPI_ModelMappingInfo *)rt_modelMapInfoPtr;

    return;
}



/* EOF */

/* LocalWords:  tofile fromfile RSIM rsim ISigstream slexec simbridge
 */
