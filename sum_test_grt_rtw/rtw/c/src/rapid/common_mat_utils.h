/*
 * Copyright 2020 The MathWorks, Inc.
 *
 * File: common_mat_utils.h
 *
 *
 * Abstract:
 *	Common functions used in rsim and rapid accelerator, for To and From File blocks.
 *
 * Requires include files
 *	tmwtypes.h
 *	simstruc_type.h
 * Note including simstruc.h before rsim.h is sufficient because simstruc.h
 * includes both tmwtypes.h and simstruc_types.h.
 */

#ifndef __COMMON_MAT_UTILS_H__
#define __COMMON_MAT_UTILS_H__

const char* rt_VerifyInportsMatFile(const char* inportFileName,
                                    int* matFileDataFormat,
                                    int isRaccel,
                                    mxLogical* periodicFunctionCallInports);

const char* rt_ConvertInportsMatDatatoTUtable(int_T matFileDataFormat,
                                              const char* inportFileName,
                                              int isRaccel,
                                              const mxLogical* periodicFunctionCallInports);
    
#endif /* __COMMON_MAT_UTILS_H__ */
