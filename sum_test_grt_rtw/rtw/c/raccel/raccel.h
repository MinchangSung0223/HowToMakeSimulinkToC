/*
 * Copyright 2007-2020 The MathWorks, Inc.
 *
 * File: raccel.h
 *
 *
 * Abstract:
 *	Data structures used with the RSIM from file and from workspace block
 *      handling.
 *
 * Requires include files
 *	tmwtypes.h
 *	simstruc_type.h
 * Note including simstruc.h before rsim.h is sufficient because simstruc.h
 * includes both tmwtypes.h and simstruc_types.h.
 */

#ifndef __RACCEL_H__
#define __RACCEL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "slsa_sim_common_utils.h"
#include "raccel_utils.h"

#if MODEL_HAS_DYNAMICALLY_LOADED_SFCNS    
#include "raccel_sfcn_utils.h"
#endif

extern void rsimLoadOptionsFromMATFile(SimStruct* S);
extern void rt_RAccelReplaceFromFilename(const char *blockpath, char *fileNam);
extern void rt_RAccelReplaceToFilename(const char *blockpath, char *fileNam);
extern void rt_RAccelAddToFileSuffix(char *fileName);
extern const char* raccelLoadInputsAndAperiodicHitTimes(SimStruct* S,
                                                        const char* inportFileName,
                                                        int* matFileFormat);

void raccelForceExtModeShutdown(boolean_T extModeStartPktReceived);
    
#ifdef __cplusplus
}
#endif

#endif /* __RACCEL_H__ */
