/*
 * Copyright 2007-2020 The MathWorks, Inc.
 *
 * File: raccel.h
 *
 *
 * Abstract:
 *	Data structures used in rapid accelerator, for To and From File 
 *      blocks, and for parameter handling.
 *
 * Requires include files
 *	tmwtypes.h
 *	simstruc_type.h
 * Note including simstruc.h before rsim.h is sufficient because simstruc.h
 * includes both tmwtypes.h and simstruc_types.h.
 */

#ifndef __RACCEL_UTILS_H__
#define __RACCEL_UTILS_H__

    typedef struct {
        void* path;
        void* next;
    } BlockPathMemMgr;

    extern void rt_Interpolate_Datatype(void   *x1, void   *x2, void   *yout,
                                        real_T t,   real_T t1,  real_T t2,
                                        int    outputDType);

    extern int_T rt_getTimeIdx(real_T *timePtr, real_T t, int_T numTimePoints, 
                               int_T preTimeIdx, boolean_T interp, boolean_T timeHitOnly);

    extern void rt_RapidFreeGbls(int);

    extern const char *rt_RapidCheckRemappings(void);

    extern void rt_RapidReadMatFileAndUpdateParams(const SimStruct *S);

    extern void* rt_GetISigstreamManager(SimStruct* S);

    extern const char* rt_RAccelReadInportsMatFile(SimStruct* S,
                                                   const char* inportFileName,
                                                   int* matFileFormat);

    extern double rt_NextAperiodicPartitionHitTime(SimStruct* S, double time);
    
    extern void rt_ssGetBlockPath(SimStruct* S, int_T sysIdx, int_T blkIdx, char_T **path);
    extern void rt_ssSet_slErrMsg(void*, void *);
    extern void rt_ssReportDiagnosticAsWarning(void*, void *);
    extern void rt_ssReportDiagnosticAsInfo(void*, void *);

    extern void rt_RapidInitDiagLoggerDB(const char* dbhome, size_t);
    extern void rt_RapidReleaseDiagLoggerDB();

    extern const char* rt_CreateFullPathToTop(const char* toppath, const char* subpath);
    extern void rt_FreeParamStructs();


#endif /* __RACCEL_UTILS_H__ */

/* LocalWords:  raccel RSIM rsim Blockath Vals RTP
 */
