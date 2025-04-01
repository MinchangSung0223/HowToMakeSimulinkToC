/******************************************************************
 * File: raccel_sfcn_utils.h
 *
 * Abstract:
 *      functions for dynamically loading s-function mex files
 *
 * Copyright 2007-2021 The MathWorks, Inc.
 ******************************************************************/

#ifndef __RACCEL_SFCN_UTILS_H__
#define __RACCEL_SFCN_UTILS_H__

#ifdef __cplusplus
struct ssExecutionInfo_tag;
typedef ssExecutionInfo_tag ssExecutionInfo;
#else
struct ssExecutionInfo;
#endif

void
raccelInitializeForMexSFcns(const char* errorStatus, const char* sFcnName);

void
raccelLoadSFcnMexFile(
    const char* sFcnName,
    const char* blockSID,
    SimStruct* simstruct,
    size_t childIdx
    );

#endif /* __RACCEL_SFCN_UTILS_H__ */
