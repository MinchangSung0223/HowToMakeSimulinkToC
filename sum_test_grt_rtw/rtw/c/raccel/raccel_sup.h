/*
 * Copyright 2007-2021 The MathWorks, Inc.
 *
 */

#ifndef __RACCEL_SUP_H__
#define __RACCEL_SUP_H__

#ifdef __cplusplus
extern "C" {
#endif

/* data */
extern const char* gblMatSigstreamLoggingFilename;
extern const char* gblMatSigLogSelectorFilename;
extern const char* gblSimDataRepoIndex;
    
/* functions */

extern int InstallRunTimeLimitHandler(void);
extern const char *ParseArgs(int_T argc, char_T *argv[], ssExecutionInfo* exInfo_in);
extern int getNumRootInputs(void);
extern int getNumRootOutputs(void);
extern int getRootInputSize(int idx);
extern int getRootOutputSize(int idx);
extern void* getRootInput(int idx);
extern void* getRootOutput(int idx);
extern int getRootInputDatatypeSSId(int idx);
extern uint_T const* getRootInputDimArray(int idx);
extern int getRootInputNumDims(int idx);
extern int getRootOutputDatatypeSSId(int idx);
extern uint_T const* getRootOutputDimArray(int idx);
extern int getRootOutputNumDims(int idx);
extern int getRootOutputDataIsComplex(int idx);
extern int getRootInputDataIsComplex(int idx);
extern int getRootInputPortNumber(int idx);
extern int getRootOutputPortNumber(int idx);
extern const char *allocMemForToFromFileBlocks(ssExecutionInfo* exInfo_in);

#ifdef __cplusplus
}
#endif

#if defined (UNIX)
# include <unistd.h>
# define GetMyPID  getpid
#else
# include <process.h>
# define GetMyPID  GetCurrentProcessId
#endif

/* Return TRUE if file 'fileName' exists */
extern bool FileExists(const char *fileName);


#endif /* __RACCEL_SUP_H__ */
