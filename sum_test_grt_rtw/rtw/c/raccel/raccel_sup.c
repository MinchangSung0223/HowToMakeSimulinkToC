/*
 * Copyright 1994-2024 The MathWorks, Inc.
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "tmwtypes.h"
#include "simstruc.h"

#include "raccel.h"
#include "raccel_sup.h"
#include "rt_nonfinite.h"
#include "rtw_capi.h"
#include "rtw_modelmap.h"
#include "slsa_sim_engine.h"

#include "ext_work.h"
#include "ext_svr.h"

#if defined (UNIX)
# include <unistd.h> /* getpid() */
#else 
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
/* win32 and win64 for all compilers */
# include <process.h>	/* _getpid */
# define getpid _getpid
# include <io.h> /* _access */
# define access _access
# define F_OK   0 /* exists     */
#endif

#if defined(TGTCONN)
    extern const char *TgtConnInit(int_T argc, char_T *argv[]);
    extern void TgtConnPreStep(int_T tid);
    extern void TgtConnPostStep(int_T tid);
#else
    #define TgtConnInit(argc, argv) NULL /* do nothing */
#endif

/*=============*
 * Global data *
 *=============*/
/* Version information */
extern const char_T *gbl_raccel_Version;

extern ssExecutionInfo gblExecutionInfo;

/*
 * File name mapping pairs (old=new) for To File, From File blocks.
 */

static ssBridgeExtModeCallbacks_T gblExtModeCallbacks = {
    rtExtModeCheckInit,
    rtExtModeWaitForStartPkt, 
    rtExtModeOneStep,
    rtExtModeUploadCheckTrigger,
    rtExtModeUpload,
    rtExtModeCheckEndTrigger,
    rtExtModePauseIfNeeded,
    rtExtModeShutdown,
    {NULL, NULL} /* assume */
};

/* Memory allocation error is used by blocks in generated code */
const char* RT_MEMORY_ALLOCATION_ERROR = "memory allocation error";
int_T gblParamCellIndex = -1;

extern SimStruct *const rtS;

/*=================================*
 * External data setup by rsim.tlc *
 *=================================*/

void raccel_set_checksum();

/* for accessing rootIOs, params, etc... using the C-API */
extern rtwCAPI_ModelMappingInfo* rt_modelMapInfoPtr;


/*===========*
 * Constants *
 *===========*/

static const char_T UsageMsgPart1[] =
" Captive Simulation Target usage: modelname [switches]\n"
"  switches:\n"
"    -c compare checksum to see if it matches"
"    -f <originalFromFile.mat=newFromFile.mat>\n"
"            Name of original input MAT-file and replacement input MAT-file\n"
"            containing TU matrix for \"From File\" block.\n"
"    -i <InportsFile.mat>\n"
"            Name of the MAT-file containing structure format for \"Root Inport\" block.\n";

static const char_T UsageMsgPart2[] =
"    -o <results.mat>\n"
"            Name of output MAT-file for MAT-file logging of simulation data\n"
"            (time,states,root outports).\n"
"    -p <parameters.mat>\n"
"            Name of new \"Parameter\" MAT-file containing new block parameter\n"
"            vector \"rtP\".\n"
"    -s,-tf <stopTime>\n"
"            Final time value to end simulation.\n"
"    -S <solver_options.mat>\n"
"            Load new solver options (e.g., Solver, RelTol, AbsTol, etc)\n";

static const char_T UsageMsgPart3[] =
"    -L timeLimit\n"
"            Exit if run time (in seconds) of the program exceeds timeLimit\n"
"    -t <originalToFile.mat=newToFile.mat>\n"
"            Name of original destination file and new destination file\n"
"            for results from a \"To File\" block.\n"
"    -w      Waits for Simulink to start model in External Mode.\n"
"    -port <TCPport>\n"
"            Overrides 17725 default port for External Mode,\n"
"            valid range 256 to 65535.\n";

static char_T UsageMsg[sizeof(UsageMsgPart1) + sizeof(UsageMsgPart2) + sizeof(UsageMsgPart3)];

static const char ObsoleteMsg[] =
"The -s switch is now obsolete and will be removed from a future version.\n"
"Please use the -tf switch in place of -s.\n";


/* Return TRUE if file 'fileName' exists */
bool FileExists(const char *fileName)
{
#if defined (UNIX)
    if (access(fileName,F_OK) == 0) 
        return true;
    else
        return false;
#else
    DWORD       fileAttr;
    fileAttr = GetFileAttributes(fileName);
    if (0xFFFFFFFF == fileAttr)
        return false;
    else
        return true;
#endif
}

/* Function: ParseArgs ========================================================
 * Abstract:
 *	Parse the command-line arguments: valid args:
 *       -c     = compare checksum to see if it matches
 *       -f     = Fromfile block name pair for reading "from file" data
 *       -i     = Inport block name pair for reading "Inport" data
 *       -o     = Output file for matfile logging
 *       -m     = Output file for simulation metadata
 *       -p     = Param file for new rtP vector
 *       -s,-tf = Stop time for ending simulation
 *       -t     = Tofile name pair so saving "to file" data
 *       -d     = File containing s-function info
 *       -E     = Channel name for communication
 *
 *  Example:
 *       f14 -p mydata -o myoutfile -s 10.1 -f originfile.mat=newinfile.mat
 *           -t origtofile=newtofile.mat -i inport.mat
 *
 *  This results as follows:
 *       - sets data structures to run model f14
 *       - "-p" option loads new rtP param vector from "mydata.mat"
 *       - "-o" saves stand-alone simulation results to "mydata.mat"
 *              instead of saving to "f14.mat"
 *       - "-s" results in simulation stopping at t=10.1 seconds.
 *       - "-f" replaces the input matfile name for all instances of  fromfile
 *              blocks that had the original name: "originfile.mat" with the
 *              replacement name: "newinfile.mat".
 *       - "-i" read in the inport.mat file that drive the inport blocks
 *       - "-t" similar to -f option except swapping names of to file blocks.
 *
 *  Returns:
 *	NULL     - success
 *	non-NULL - error message
 */
const char *ParseArgs(int_T argc, char_T *argv[], ssExecutionInfo* exInfo_in)
{
    int_T      tvar;
    const char *result          = NULL; /* assume success */

    (void) strcpy(UsageMsg, UsageMsgPart1);
    (void) strcpy(&UsageMsg[strlen(UsageMsgPart1)],UsageMsgPart2);
    (void) strcpy(&UsageMsg[strlen(UsageMsgPart1) + strlen(UsageMsgPart2)],UsageMsgPart3);

     for (tvar = 1; tvar < argc; tvar++) { /* check list of input args */

         if (argv[tvar] == NULL)
             continue;

         if (strcmp(argv[tvar], "-server_info_file") == 0) {
             /* Handled by C++ function slsaParseArguments() */
             /* Argument should not be cleared in argv */
             tvar = tvar + 1;
             continue;
         }

         if ((strcmp(argv[tvar], "--config") == 0) ||
             (strcmp(argv[tvar], "-sim_service_pid") == 0) ||
             (strcmp(argv[tvar], "-error_file") == 0) ||
             (strcmp(argv[tvar], "-reval") == 0) ||
             (strcmp(argv[tvar], "-i") == 0) ||
             (strcmp(argv[tvar], "-d") == 0) ||
             (strcmp(argv[tvar], "-S") == 0) ||
             (strcmp(argv[tvar], "-L") == 0) ||
             (strcmp(argv[tvar], "-E") == 0) ||
             (strcmp(argv[tvar], "-m") == 0) ||
             (strcmp(argv[tvar], "-o") == 0) ||
             (strcmp(argv[tvar], "-l") == 0) ||
             (strcmp(argv[tvar], "-e") == 0) ||
             (strcmp(argv[tvar], "-R") == 0) ||
             (strcmp(argv[tvar], "-T") == 0) ||
             (strcmp(argv[tvar], "-p") == 0)) {
             /* Handled by C++ function slsaParseArguments() */
             argv[tvar++] = NULL;
             argv[tvar] = NULL;
             continue;
         }

         if ((strcmp(argv[tvar], "-verbose") == 0)) {
             /* Obsolete Arguments */
             argv[tvar++] = NULL;
             argv[tvar] = NULL;
             continue;
         }
        
         if ((strcmp(argv[tvar], "-live_stream") == 0)||
             (strcmp(argv[tvar], "-D") == 0) ||
             (strcmp(argv[tvar], "-P") == 0)) {
             /* Handled by C++ function slsaParseArguments() */
             argv[tvar] = NULL;
             continue;
         }

         // DMR Index used for lightweight workers
         if (strcmp(argv[tvar], "-dmr_idx") == 0) {
             if ((tvar + 1) == argc || argv[tvar + 1][0] == '-') {
                 result = UsageMsg;
                 goto EXIT_POINT;
             }
             if (gblExecutionInfo.gblObjects_.simDataRepoIndex != NULL) {
                 result = "only one -dmr_idx switch is allowed\n";
                 goto EXIT_POINT;
             }
             gblExecutionInfo.gblObjects_.simDataRepoIndex = argv[tvar + 1];

             argv[tvar++] = NULL;
             argv[tvar] = NULL;
             continue;
         }

         if(strcmp(argv[tvar],"-ignore-arg") == 0) {
             /* This is currently used for sbruntests
                SBRUNTESTS_SESSION_ID support, for preventing rogue
                processes.
                Do NOT NULL out the arguments.  We want this argument
                to stick around for later.  External mode may see them
                as well but "-ignore-arg" is also handled by 
                external mode. */
             tvar = tvar+1; /* move to the next arg */
             continue;
         }
     } /* end parse loop */
     
     /* Commadline sim args: 'model.exe --config configFile' */ 
     if (argc == 3) {
         return result;
     }
     
     /*
      * Check for external mode arguments.
      *
      * Menu sim args : 'model.exe -config configFile -server_info_file svrInfoFile
      * -tgtconn_server_info_file tgtSvrInfo -tgtconn_port 0'
      */
     rtExtModeParseArgs(argc, (const char_T **) argv, NULL);
     boolean_T extModeEnabled = ExtWaitForStartPkt();
     
     /* Target Connectivity */
     #if defined(TGTCONN)
     gblExtModeCallbacks.targetConnectivityCallbacks_.TgtConnPreStep = TgtConnPreStep;
     gblExtModeCallbacks.targetConnectivityCallbacks_.TgtConnPostStep = TgtConnPostStep;
     #endif
     
     exInfo_in->runtimeCallbacks_.externalModeCallbacks_ = extModeEnabled ? &gblExtModeCallbacks : NULL; 
     
     if (extModeEnabled)
     {
        /* Target connectivity initialization */
        result = TgtConnInit(argc, argv);
        if (result != NULL) return(result);
     }
     /*
      * Check for unprocessed ("unhandled") args.
      */
     {
         int i;
         for (i=1; i<argc; i++) {
             if (argv[i] != NULL) {
                 result = UsageMsg;
                 goto EXIT_POINT;
             }
         }
     }
 EXIT_POINT:
     return(result);

} /* end ParseArgs */

const char *allocMemForToFromFileBlocks(ssExecutionInfo* exInfo_in)
{
    int_T toFNamepairIdx = 0;
    const char* result = NULL; /* assume success */
    /*
     * If any "to file" or "from file" blocks exist, we must allocate
     * memory for "to file" and "from file" namepairs even if the
     * file names remain unchanged from the original file names.
     */
    exInfo_in->gblObjects_.frFNamepair = NULL;
    exInfo_in->gblObjects_.toFNamepair = NULL;
    if (exInfo_in->gblObjects_.numToFiles > 0) {
        exInfo_in->gblObjects_.toFNamepair =
            (FNamePair*)calloc(exInfo_in->gblObjects_.numToFiles, sizeof(FNamePair));
        if (exInfo_in->gblObjects_.toFNamepair == NULL) {
            result = "memory allocation error (exInfo_in->gblObjects_.toFNamepair)";
            return result;
        }
    }

    if (exInfo_in->gblObjects_.numFrFiles > 0) {
        exInfo_in->gblObjects_.frFNamepair =
            (FNamePair*)calloc(exInfo_in->gblObjects_.numFrFiles, sizeof(FNamePair));
        if (exInfo_in->gblObjects_.frFNamepair == NULL) {
            result = "memory allocation error (exInfo_in->gblObjects_.frFNamepair)";
            return result;
        }
    }

    /* Check that in parallel simulation with ToFile blocks the
      ConcurrencyResolvingToFileSuffix is specified */
    if (exInfo_in->simulationOptions_.runningInParallel_ &&
        (exInfo_in->gblObjects_.numToFiles > 0) &&
        (exInfo_in->simulationOptions_.toFileSuffix_ == NULL)) {
        result =
            "'ToFile' blocks when running parallel simulations can cause concurrency issues. This "
            "can be resolved by specifying the ConcurrencyResolvingToFileSuffix in the simset "
            "options";
        return result;
    }

    int i, j;
    for (i = 0; i < toFNamepairIdx; i++) {
        if (strcmp(exInfo_in->simulationOptions_.matLoggingFilename_,
                   exInfo_in->gblObjects_.toFNamepair[i].newName) == 0) {
            result = " 'To File' filename cannot be the same as output filename of the model";
            return result;
        }
    }
    for (j = i + 1; j < toFNamepairIdx; j++) {
        if (strcmp(exInfo_in->gblObjects_.toFNamepair[i].oldName,
                   exInfo_in->gblObjects_.toFNamepair[j].oldName) == 0) {
            (void)printf("'To File' filename '%s' is replaced more than once with the -t option\n",
                         exInfo_in->gblObjects_.toFNamepair[j].oldName);
            result = "Multiple replacement of 'To File' filenames is not allowed\n";
            return result;
        }

        if (strcmp(exInfo_in->gblObjects_.toFNamepair[i].newName,
                   exInfo_in->gblObjects_.toFNamepair[j].newName) == 0) {
            (void)printf(
                "'To File' filename replacement '%s' is used more than once with the -t option\n",
                exInfo_in->gblObjects_.toFNamepair[j].newName);
            result = "All 'To File' filenames must be  unique\n";
            return result;
        }
    }
    return result;
}

/*
 * Get the number of root inports
 */
int getNumRootInputs(void) {
    return (int)(rtwCAPI_GetNumRootInputs(rt_modelMapInfoPtr));
}

/*
 * Get the number of root outputs
 */
int getNumRootOutputs(void) {
    return (int)(rtwCAPI_GetNumRootOutputs(rt_modelMapInfoPtr));
}

/*
 * Get the size of the idx_th root inport
 */
int getRootInputSize(int idx) {
    rtwCAPI_Signals const* modelInputs = NULL;

    modelInputs = rtwCAPI_GetRootInputs(rt_modelMapInfoPtr);

    // get element size
    uint16_T dTypeIdx = rtwCAPI_GetSignalDataTypeIdx(modelInputs, idx);
    rtwCAPI_DataTypeMap const *dataTypeMap =
        rtwCAPI_GetDataTypeMap(rt_modelMapInfoPtr);
    uint16_T elSize = rtwCAPI_GetDataTypeSize(dataTypeMap, dTypeIdx);

    // get dims
    rtwCAPI_DimensionMap const* dimMap = rtwCAPI_GetDimensionMap(rt_modelMapInfoPtr);
    uint_T const* dimArray = rtwCAPI_GetDimensionArray(rt_modelMapInfoPtr);

    uint_T dimMapIdx = rtwCAPI_GetSignalDimensionIdx(modelInputs, idx);
    int numDims = rtwCAPI_GetNumDims(dimMap, dimMapIdx);

    uint_T dimArrayIdx = rtwCAPI_GetDimArrayIndex(dimMap, dimMapIdx);

    int numArrayElements = 1;
    int loopIdx;
    for (loopIdx = 0; loopIdx < numDims; ++loopIdx) {
        numArrayElements *= dimArray[dimArrayIdx + loopIdx];
    }

    return numArrayElements * elSize;
}

/*
 * Get the size of the idx_th root outport
 */
int getRootOutputSize(int idx) {

    rtwCAPI_Signals const* modelOutputs = rtwCAPI_GetRootOutputs(rt_modelMapInfoPtr);

    // get element size
    uint16_T dTypeIdx = rtwCAPI_GetSignalDataTypeIdx(modelOutputs, idx);
    rtwCAPI_DataTypeMap const* dataTypeMap = rtwCAPI_GetDataTypeMap(rt_modelMapInfoPtr);
    uint16_T elSize = rtwCAPI_GetDataTypeSize(dataTypeMap, dTypeIdx);

    // get dims
    rtwCAPI_DimensionMap const* dimMap = rtwCAPI_GetDimensionMap(rt_modelMapInfoPtr);
    uint_T const* dimArray = rtwCAPI_GetDimensionArray(rt_modelMapInfoPtr);

    uint_T dimMapIdx = rtwCAPI_GetSignalDimensionIdx(modelOutputs, idx);
    int numDims = rtwCAPI_GetNumDims(dimMap, dimMapIdx);

    uint_T dimArrayIdx = rtwCAPI_GetDimArrayIndex(dimMap, dimMapIdx);

    int numArrayElements = 1;
    int loopIdx;
    for (loopIdx = 0; loopIdx < numDims; ++loopIdx) {
        numArrayElements *= dimArray[dimArrayIdx + loopIdx];
    }

    return numArrayElements * elSize;
}

/*
 * Get the pointer to the idx_th root inport
 */
void* getRootInput(int idx) {
    int numRootInputs = (int)(rtwCAPI_GetNumRootInputs(rt_modelMapInfoPtr));

    void** dataAddrMap = NULL;
    rtwCAPI_Signals const* modelInputs = NULL;
    uint_T addrIdx = 0;
    void* val = NULL;
    if (idx < numRootInputs) {
        dataAddrMap = rtwCAPI_GetDataAddressMap(rt_modelMapInfoPtr);
        modelInputs = rtwCAPI_GetRootInputs(rt_modelMapInfoPtr);
        addrIdx = rtwCAPI_GetSignalAddrIdx(modelInputs, idx);
        val = dataAddrMap[addrIdx];
    }
    return val;
}

/*
 * Get the pointer to the idx_th rooth outport
 */
void* getRootOutput(int idx) {
    int numRootOutputs = (int)(rtwCAPI_GetNumRootOutputs(rt_modelMapInfoPtr));
    void** dataAddrMap = NULL;
    rtwCAPI_Signals const* modelOutputs = NULL;
    uint_T addrIdx = 0;
    void* val = NULL;
    if (idx < numRootOutputs) {
        dataAddrMap = rtwCAPI_GetDataAddressMap(rt_modelMapInfoPtr);
        modelOutputs = rtwCAPI_GetRootOutputs(rt_modelMapInfoPtr);
        addrIdx = rtwCAPI_GetSignalAddrIdx(modelOutputs, idx);
        val = dataAddrMap[addrIdx];
    }
    return val;
}

/*
 * Get the datatype for the idx_th root input
 */
int getRootInputDatatypeSSId(int idx) {

    // get signal data ptr
    rtwCAPI_Signals const* modelInputs = NULL;
    modelInputs = rtwCAPI_GetRootInputs(rt_modelMapInfoPtr);

    // get index in datatypemap
    uint16_T dTypeIdx = rtwCAPI_GetSignalDataTypeIdx(modelInputs, idx);

    // get datatypemap
    rtwCAPI_DataTypeMap const* dataTypeMap = rtwCAPI_GetDataTypeMap(rt_modelMapInfoPtr);

    return rtwCAPI_GetDataTypeSLId(dataTypeMap, dTypeIdx);
}


/*
 * Get if the idx_th root input is Complex
 */
int getRootInputDataIsComplex(int idx) {

    // get signal data ptr
    rtwCAPI_Signals const* modelInputs = NULL;
    modelInputs = rtwCAPI_GetRootInputs(rt_modelMapInfoPtr);

    // get index in datatypemap
    uint16_T dTypeIdx = rtwCAPI_GetSignalDataTypeIdx(modelInputs, idx);

    // get datatypemap
    rtwCAPI_DataTypeMap const* dataTypeMap = rtwCAPI_GetDataTypeMap(rt_modelMapInfoPtr);

    return rtwCAPI_GetDataIsComplex(dataTypeMap, dTypeIdx);
}



/*
 * Get the datatype for the idx_th root output
 */
int getRootOutputDatatypeSSId(int idx) {

    // get signal data ptr
    rtwCAPI_Signals const* modelOutputs = NULL;
    modelOutputs = rtwCAPI_GetRootOutputs(rt_modelMapInfoPtr);

    // get index in datatypemap
    uint16_T dTypeIdx = rtwCAPI_GetSignalDataTypeIdx(modelOutputs, idx);

    // get datatypemap
    rtwCAPI_DataTypeMap const* dataTypeMap = rtwCAPI_GetDataTypeMap(rt_modelMapInfoPtr);

    return rtwCAPI_GetDataTypeSLId(dataTypeMap, dTypeIdx);
}

/*
 * Get the datatype for the idx_th root output
 */
int getRootOutputDataIsComplex(int idx) {

    // get signal data ptr
    rtwCAPI_Signals const* modelOutputs = NULL;
    modelOutputs = rtwCAPI_GetRootOutputs(rt_modelMapInfoPtr);

    // get index in datatypemap
    uint16_T dTypeIdx = rtwCAPI_GetSignalDataTypeIdx(modelOutputs, idx);

    // get datatypemap
    rtwCAPI_DataTypeMap const* dataTypeMap = rtwCAPI_GetDataTypeMap(rt_modelMapInfoPtr);

    return rtwCAPI_GetDataIsComplex(dataTypeMap, dTypeIdx);
}


/*
 * Get the dims pointer for the idx_th root input
 */
uint_T const* getRootInputDimArray(int idx) {

    // get signal info ptr
    rtwCAPI_Signals const* modelInputs = rtwCAPI_GetRootInputs(rt_modelMapInfoPtr);

    // get dimensions map
    rtwCAPI_DimensionMap const* dimMap = rtwCAPI_GetDimensionMap(rt_modelMapInfoPtr);

    // get dimensions array
    uint_T const* dimArray = rtwCAPI_GetDimensionArray(rt_modelMapInfoPtr);

    // get the index in dimensions map
    uint_T dimMapIdx = rtwCAPI_GetSignalDimensionIdx(modelInputs, idx);

    // get the start index within dimensions array
    uint_T dimArrayIdx = rtwCAPI_GetDimArrayIndex(dimMap, dimMapIdx);

    return &(dimArray[dimArrayIdx]);
}

/*
 * Get the number of dimensions for dimensions array for the idx_th root input
 */
int getRootInputNumDims(int idx) {
    // get signal info ptr
    rtwCAPI_Signals const* modelInputs = rtwCAPI_GetRootInputs(rt_modelMapInfoPtr);

    // get dimensions map
    rtwCAPI_DimensionMap const* dimMap = rtwCAPI_GetDimensionMap(rt_modelMapInfoPtr);

    // get index in dimensions map
    uint_T dimMapIdx = rtwCAPI_GetSignalDimensionIdx(modelInputs, idx);

    // return number of dimensions
    return rtwCAPI_GetNumDims(dimMap, dimMapIdx);
}


/*
 * Get the dims pointer for the idx_th root output
 */
uint_T const* getRootOutputDimArray(int idx) {

    // get signal info ptr
    rtwCAPI_Signals const* modelOutputs = rtwCAPI_GetRootOutputs(rt_modelMapInfoPtr);

    // get dimensions map
    rtwCAPI_DimensionMap const* dimMap = rtwCAPI_GetDimensionMap(rt_modelMapInfoPtr);

    // get dimensions array
    uint_T const* dimArray = rtwCAPI_GetDimensionArray(rt_modelMapInfoPtr);

    // get the index in dimensions map
    uint_T dimMapIdx = rtwCAPI_GetSignalDimensionIdx(modelOutputs, idx);

    // get the start index within dimensions array
    uint_T dimArrayIdx = rtwCAPI_GetDimArrayIndex(dimMap, dimMapIdx);

    return &(dimArray[dimArrayIdx]);
}

/*
 * Get the number of dimensions for dimensions array for the idx_th root output
 */
int getRootOutputNumDims(int idx) {
    // get signal info ptr
    rtwCAPI_Signals const* modelOutputs = rtwCAPI_GetRootOutputs(rt_modelMapInfoPtr);

    // get dimensions map
    rtwCAPI_DimensionMap const* dimMap = rtwCAPI_GetDimensionMap(rt_modelMapInfoPtr);

    // get index in dimensions map
    uint_T dimMapIdx = rtwCAPI_GetSignalDimensionIdx(modelOutputs, idx);

    // return number of dimensions
    return rtwCAPI_GetNumDims(dimMap, dimMapIdx);
}

/*
 * Get the port nubmer for the idx_th root inport
 */
int getRootInputPortNumber(int idx) {
    // get signal info ptr
    rtwCAPI_Signals const* modelInputs = rtwCAPI_GetRootInputs(rt_modelMapInfoPtr);

    // get the portNumber
    return (int)rtwCAPI_GetSignalPortNumber(modelInputs, idx);
}

/*
 * Get the port nubmer for the idx_th root outport
 */
int getRootOutputPortNumber(int idx) {
    // get signal info ptr
    rtwCAPI_Signals const* modelOutputs = rtwCAPI_GetRootOutputs(rt_modelMapInfoPtr);

    // get the portNumber
    return (int)rtwCAPI_GetSignalPortNumber(modelOutputs, idx);
}



/* LocalWords:  getpid TUtable gbl TUtables rsim modelname Pport Namepair RSim
 * LocalWords:  origname FName Fromfile matfile Tofile mydata myoutfile fh
 * LocalWords:  originfile newinfile origtofile newtofile fromfile namepairs
 * LocalWords:  FNamepair pid tvar oldfile newfile oldname inportfile Logsout
 * LocalWords:  logsout paramfile lf
 */
