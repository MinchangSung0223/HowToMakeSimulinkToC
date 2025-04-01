
/******************************************************************
 *
 *  File: raccel_mat.c
 *
 *
 *  Abstract:
 *      - provide matfile handling for reading and writing matfiles
 *        for use with rsim stand-alone, non-real-time simulation
 *      - provide functions for swapping rtP vector for parameter tuning
 *
 * Copyright 2007-2020 The MathWorks, Inc.
 ******************************************************************/

/*
 * This file is still using the old 32-bit mxArray API
 */
#define MX_COMPAT_32

/* INCLUDES */
#include  <stdio.h>
#include  <stdlib.h>
#include  <string.h>
#include  <math.h>
#include  <float.h>
#include  <ctype.h>

/*
 * We want access to the real mx* routines in this file and not their RTW
 * variants in rt_matrx.h, the defines below prior to including simstruc.h
 * accomplish this.
 */
#include "mat.h"
#define TYPEDEF_MX_ARRAY
#define rt_matrx_h
#include "simstruc.h"
#undef rt_matrx_h
#undef TYPEDEF_MX_ARRAY

#include  "raccel.h"
#include "slexec_parallel.h"

#include "sigstream_rtw.h"
#include "simtarget/slSimTgtSlioCoreRTW.h"
#include "simtarget/slSimTgtSlioClientsRTW.h"
#include "simtarget/slSimTgtSlioSdiRTW.h"

extern ssExecutionInfo gblExecutionInfo;

FileInfo *gblFromFileInfo = NULL;
FileInfo *gblToFileInfo = NULL;
ParallelExecutionOptions parallelExecutionOptions;
const char *configSetSolver; 
boolean_T decoupledContinuousIntegration;
boolean_T optimalSolverResetCausedByZc;
int_T numStatesForStiffnessChecking;
double stiffnessThreshold;
unsigned int solverStatusFlags;
unsigned int numSpecifiedTimesForRuntimeSolverSwitch = 0;
double *specifiedTimesForRuntimeSolverSwitch = NULL;

/* ParallelExecutionOptions parallelExecutionOptions =  */
/* {0, PARALLEL_EXECUTION_ON, 0, false, NULL}; */

/* External Functions */
extern const char* getToFileSuffix(void);

/* Function: rt_RAccelAddToFileSuffix ==========================================
 * Abstract:
 * This adds the ToFileSuffix
 */
void rt_RAccelAddToFileSuffix(char *fileName)
{
    /* We assume that the char* pointer passed in has sufficient length
     * to hold the suffix also */
    static char origFileName[MAXSTRLEN];
    const char* toFileSuffix = getToFileSuffix();
    const char *dot = strchr(fileName, '.');
    size_t baseFileNameLen = (size_t)(dot-(fileName));
    size_t maxLength;
    size_t limitedLength;
    /* If no suffix, no changes to filename, return.*/    
    if (toFileSuffix == NULL) return;
    maxLength = MAXSTRLEN - strlen(origFileName + baseFileNameLen) - strlen(toFileSuffix) - 1;
    limitedLength = baseFileNameLen > maxLength ? maxLength : baseFileNameLen;
    /* Copy entire suffix-less filename into origFileName. */
    (void)strncpy(origFileName, fileName, MAXSTRLEN);
    /* Copy suffix-less and extension-less char array origFileName[:baseFileNameLen] into fileName */
    (void)strncpy(fileName, origFileName, limitedLength);
    fileName[limitedLength] = '\0';
    /* add suffix to fileName */
    (void)strcat(fileName, toFileSuffix);
    /* Add extension back into fileName */
    (void)strcat(fileName, origFileName + baseFileNameLen);
} /* end rt_RAccelAddToFileSuffix */


/* Function rt_RAccelReplaceFromFilename =========================================
 * Abstract:
 *  This function replaces the Name for From File blocks
 */

void  rt_RAccelReplaceFromFilename(const char *blockpath, char *fileName)
{
   int_T fileIndex;
    for (fileIndex=0; fileIndex<gblExecutionInfo.gblObjects_.numFrFiles; fileIndex++) {
	if (strcmp(gblFromFileInfo[fileIndex].blockpath, blockpath)==0) {
            (void)strcpy(fileName,gblFromFileInfo[fileIndex].filename);
            break;
        }
    }
}

/* Function rt_RAccelReplaceToFilename =========================================
 * Abstract:
 *  This function replaces the Name for To File blocks
 */

void  rt_RAccelReplaceToFilename(const char *blockpath, char *fileName)
{
   int_T fileIndex;
    for (fileIndex=0; fileIndex<gblExecutionInfo.gblObjects_.numToFiles; fileIndex++) {
	if (strcmp(gblToFileInfo[fileIndex].blockpath, blockpath)==0) {
            (void)strcpy(fileName,gblToFileInfo[fileIndex].filename);
            break;
        }
    }
    rt_RAccelAddToFileSuffix(fileName);
}

// Routine to load additional data from mat file
void rsimLoadOptionsFromMATFile(SimStruct* S) {
    MATFile*    matf      = NULL;
    mxArray*    pa        = NULL;
    mxArray*    pFromFile = NULL;
    mxArray*    pToFile   = NULL;
    mxArray*    sa        = NULL;
    int_T       idx       = 0;
    int_T       fileIndex;
    const char* result = NULL;
    RTWLogInfo *li = ssGetRTWLogInfo(S);
    bool logStateDataForArrayLogging= false;
    bool logStateDataForStructLogging= false;
    int_T saveFormatOpt = 0;
    ssExecutionInfo* exInfo = ssGetExecutionInfo(S);

    if (exInfo->simulationOptions_.parameterFileName_ == NULL) return;

    if ((matf=matOpen(exInfo->simulationOptions_.parameterFileName_,"r")) == NULL) {
        result = "could not find MAT-file containing new parameter data";
        goto EXIT_POINT;
    }


    /* Load From File blocks information*/
    {
        if ((pFromFile=matGetVariable(matf,"fromFile")) == NULL){
            result = "error reading From File blocks options from MAT-file";
            goto EXIT_POINT;
        }

        if ( gblExecutionInfo.gblObjects_.numFrFiles ) {
            gblFromFileInfo = (FileInfo*)calloc(gblExecutionInfo.gblObjects_.numFrFiles,sizeof(FileInfo));
            if (gblFromFileInfo==NULL) {
                result = "memory allocation error (gblFromFileInfo)";
                goto EXIT_POINT;
            }
     
            for (fileIndex=0; fileIndex < gblExecutionInfo.gblObjects_.numFrFiles; fileIndex++){
                char storedBlockpath[MAXSTRLEN] = "\0";
                char storedFilename[MAXSTRLEN] = "\0";
                mxGetString(mxGetField(pFromFile,fileIndex,"blockPath"),storedBlockpath,MAXSTRLEN);
                (void)strcpy(gblFromFileInfo[fileIndex].blockpath,storedBlockpath);
                mxGetString(mxGetField(pFromFile,fileIndex,"filename"),storedFilename,MAXSTRLEN);
                (void)strcpy(gblFromFileInfo[fileIndex].filename,storedFilename);
            }
        }
    }

    /* Load To File blocks information*/
    {
        if ((pToFile=matGetVariable(matf,"toFile")) == NULL){
            result = "error reading To File blocks options from MAT-file";
            goto EXIT_POINT;
        }

        if (gblExecutionInfo.gblObjects_.numToFiles >0 ) {
            gblToFileInfo = (FileInfo*)calloc(gblExecutionInfo.gblObjects_.numToFiles,sizeof(FileInfo));
            if (gblToFileInfo==NULL) {
                result = "memory allocation error (gblFromFileInfo)";
                goto EXIT_POINT;
            }
        

            for (fileIndex=0; fileIndex < gblExecutionInfo.gblObjects_.numToFiles; fileIndex++){
                char storedBlockpath[MAXSTRLEN] = "\0";
                char storedFilename[MAXSTRLEN] = "\0";
                mxGetString(mxGetField(pToFile,fileIndex,"blockPath"),storedBlockpath,MAXSTRLEN);
                (void)strcpy(gblToFileInfo[fileIndex].blockpath,storedBlockpath);
                mxGetString(mxGetField(pToFile,fileIndex,"filename"),storedFilename,MAXSTRLEN);
                (void)strcpy(gblToFileInfo[fileIndex].filename,storedFilename);
            }
        }
    }

     /* Load the structure of solver options */
    if ((pa=matGetVariable(matf,"slvrOpts")) == NULL ) {
        result = "error reading new solver options from MAT-file";
        goto EXIT_POINT;
    }
        /* Should be structure */
    if ( !mxIsStruct(pa) || (mxGetN(pa) > 1 && mxGetM(pa) > 1) ) {
        result = "solver options should be a vector of structures";
        goto EXIT_POINT;
    }

    /* LoggingIntervals */
    {
        const char* opt = "LoggingIntervals";

        /* Create LoggingInterval instance */
        rtwLoggingIntervalCreateInstance(&rtliGetLoggingInterval(ssGetRTWLogInfo(S)));

        sa = mxGetField(pa, idx, opt);
        if (!rtwLoggingIntervalConstructIntervalTree(
                rtliGetLoggingInterval(ssGetRTWLogInfo(S)), sa))
        {
            result = "Error reading solver option LoggingIntervals";
            goto EXIT_POINT;
        }
    }

    /* Diagnostic Logger DB */
    {
        const char* opt_dir = "diaglogdb_dir";
        const char* opt_sid = "diaglogdb_sid";
        static char dbhome[1024] = "\0";

        /* Create diag log db */

        mxArray* sa_sid = mxGetField(pa, idx, opt_sid);
        if (!mxIsDouble(sa_sid) || mxGetNumberOfElements(sa_sid) != 1) {
            result = "error reading diaglogdb_sid";
            goto EXIT_POINT;
        }
        
        if ((sa = mxGetField(pa, idx, opt_dir)) != NULL) {
            if (mxGetString(sa, dbhome, 1024) != 0) {
                result = "error reading diaglogdb_dir";
                goto EXIT_POINT;
            }
        }

        rt_RapidInitDiagLoggerDB(dbhome, (size_t)mxGetPr(sa_sid)[0]);
    }

    /* LogStateDataForArrayLogging */
    {
        const char* opt = "LogStateDataForArrayLogging";
        if ( (sa=mxGetField(pa,idx,opt)) != NULL ) {
            if ( !mxIsLogicalScalar(sa) ) {
                result = "error reading logging option LogStateDataForArrayLogging";
                goto EXIT_POINT;
            }
            logStateDataForArrayLogging= (int_T)mxIsLogicalScalarTrue(sa);
        }
    }

    /* LogStateDataForStructLogging */
    {
        const char* opt = "LogStateDataForStructLogging";
        if ( (sa=mxGetField(pa,idx,opt)) != NULL ) {
            if ( !mxIsLogicalScalar(sa) ) {
                result = "error reading logging option LogStateDataForStructLogging";
                goto EXIT_POINT;
            }
            logStateDataForStructLogging= (int_T)mxIsLogicalScalarTrue(sa);
        }
    }

    /* SaveFormat */
    {
        const char* opt = "SaveFormat";
        static char saveFormat[256] = "\0";
        if ( (sa=mxGetField(pa,idx,opt)) != NULL ) {
            if (mxGetString(sa, saveFormat, 256) != 0) {
                result = "error reading logging option SaveFormat";
                goto EXIT_POINT;
            }
            if (strcmp(saveFormat,"Dataset")==0) {
                saveFormatOpt = 3;
            } else if (strcmp(saveFormat,"StructureWithTime")==0) {
                saveFormatOpt = 2;
            } else if (strcmp(saveFormat,"Structure")==0) {
                saveFormatOpt = 1;
            } else if (strcmp(saveFormat,"Matrix")==0 ||
                       strcmp(saveFormat,"Array")==0) {
                saveFormatOpt = 0;
            }
            rtliSetLogFormat(ssGetRTWLogInfo(S), saveFormatOpt);
        }
    }

    /* StateTimeName */
    {
        const char* opt = "TimeSaveName";
        static char timeSaveName[mxMAXNAM] = "\0";
        if ( (sa=mxGetField(pa,idx,opt)) != NULL ) {
            if (mxGetString(sa, timeSaveName, mxMAXNAM) != 0) {
                result = "error reading logging option TimeSaveName";
                goto EXIT_POINT;
            }
            rtliSetLogT(ssGetRTWLogInfo(S), timeSaveName);
        }
    }

    /* OutputSaveName */
    if ( rtliGetLogY(li)[0] != '\0' )
    {
        const char* opt2 = "OutputSaveName";
        static char outputSaveName[mxMAXNAM] = "\0";

        if ( (sa=mxGetField(pa,idx,opt2)) != NULL ) {
            if (mxGetString(sa, outputSaveName, mxMAXNAM) != 0) {
                result = "error reading logging option OutputSaveName";
                goto EXIT_POINT;
            }
            rtliSetLogY(ssGetRTWLogInfo(S), outputSaveName);
        }
    }

    /* FinalStateName 
     * This is affected by other options computed from the main MATLAB
     * process to determine whether or not to log state data.  This is because
     * we switched to always using structwithtime logging, so there are cases
     * where state information needs to not be logged which are not covered
     * without this additional logic.
     */
    if (rtliGetLogXFinal(li)[0] != '\0' && 
        ((saveFormatOpt == 0 && logStateDataForArrayLogging == 1) ||
         (saveFormatOpt != 0 && logStateDataForStructLogging == 1)))
    {
        const char* opt2 = "FinalStateName";
        static char finalStateName[mxMAXNAM] = "\0";
        
        if ( (sa=mxGetField(pa,idx,opt2)) != NULL ) {
            if (mxGetString(sa, finalStateName, mxMAXNAM) != 0) {
                result = "error reading logging option FinalStateName";
                goto EXIT_POINT;
            }
            rtliSetLogXFinal(ssGetRTWLogInfo(S), finalStateName);
        }
        else {
            rtliSetLogXFinal(ssGetRTWLogInfo(S), "");
        }
    }
    else {
        rtliSetLogXFinal(ssGetRTWLogInfo(S), "");
    }

    /* StateSaveName
     * This is affected by other options computed from the main MATLAB
     * process to determine whether or not to log state data.  This is because
     * we switched to always using structwithtime logging, so there are cases
     * where state information needs to not be logged which are not covered
     * without this additional logic.
    */
    {
        if (rtliGetLogX(li)[0] != '\0' && 
            ((saveFormatOpt == 0 && logStateDataForArrayLogging == 1) ||
             (saveFormatOpt != 0 && logStateDataForStructLogging == 1)))
        {
            const char* opt2 = "StateSaveName";
            static char stateSaveName[mxMAXNAM] = "\0";
        
            if ( (sa=mxGetField(pa,idx,opt2)) != NULL ) {
                if (mxGetString(sa, stateSaveName, mxMAXNAM) != 0) {
                    result = "error reading logging option StateSaveName";
                    goto EXIT_POINT;
                }
                rtliSetLogX(ssGetRTWLogInfo(S), stateSaveName);
            }
            else {
                rtliSetLogX(ssGetRTWLogInfo(S), "");
            }
        }
        else {
            rtliSetLogX(ssGetRTWLogInfo(S), "");
        }
    }

    /* Decimation */
    {
        const char* opt = "Decimation";
        if ( (sa=mxGetField(pa,idx,opt)) != NULL ) {
            /* consult Foundation Libraries before using mxIsIntVectorWrapper G978320 */
            if ( !mxIsIntVectorWrapper(sa) || mxGetNumberOfElements(sa) != 1 ) {
                result = "error reading logging option Decimation";
                goto EXIT_POINT;
            }
            rtliSetLogDecimation(ssGetRTWLogInfo(S), (int_T)mxGetPr(sa)[0]);
        }
    }

    /* MaxDataPoints */
    {
        const char* opt = "MaxDataPoints";
        if ( (sa=mxGetField(pa,idx,opt)) != NULL ) {
            /* consult Foundation Libraries before using mxIsIntVectorWrapper G978320 */
            if ( !mxIsIntVectorWrapper(sa) || mxGetNumberOfElements(sa) != 1 ) {
                result = "error reading logging option MaxDataPoints";
                goto EXIT_POINT;
            }
            rtliSetLogMaxRows(ssGetRTWLogInfo(S), (int_T)mxGetPr(sa)[0]);
        }
    }

    /* AperiodicPartitionHitTimes */
    {
        const char* opt = "AperiodicPartitionHitTimes";
        if ( (sa=mxGetField(pa,idx,opt)) != NULL ) {
            exInfo->executionOptions_.aperiodicPartitionHitTimes_ = mxDuplicateArray(sa);
        }
    }

  EXIT_POINT:
    if(pa!=NULL) { 
        mxDestroyArray(pa); 
        pa = NULL; 
    } 
         
    if(pToFile !=NULL){
        mxDestroyArray(pToFile); 
        pToFile = NULL; 
    }
    
    if(pFromFile !=NULL){
        mxDestroyArray(pFromFile); 
        pFromFile = NULL;
    }
    
    if (matf != NULL) {
        matClose(matf); matf = NULL;
    }  

    ssSetErrorStatus(S, result);
    return;

} /* rsimLoadOptionsFromMATFile */

/* EOF raccel_mat.c */

/* LocalWords:  matfile matfiles rsim RAccel gbl slvr variablestepdiscrete
 * LocalWords:  fixedstepdiscrete ZCs MAXNAM Zc structwithtime slexec
 */
