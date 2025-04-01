/* Copyright 2020-2021 The MathWorks, Inc. */

/**
 * @file: slImageSFcnAPI.h
 *
 * @brief SimStruct API for Simulink Image operations
 *
 */

#ifndef __SIMSTRUC__IMAGEAPI
#define __SIMSTRUC__IMAGEAPI

#include "sl_simtarget_instrumentation_spec.h"
#include "sl_simtarget_instrumentation_spec.h"
#ifdef MDL_REF_SIM_TGT
#include "ImageDefs.hpp"
#else
#include "images/datatypes/ImageDefs.hpp"
#endif

SL_SIMTARGET_INSTRUMENTATION_EXPORT_EXTERN_C DTypeId
ssRegisterImageDataType(SimStruct* S,
                        size_t numChannels,
                        double numRows,
                        double numCols,
                        images::datatypes::ColorFormat colorFormat,
                        images::datatypes::Layout dataLayout,
                        images::datatypes::UnderlyingType baseType);

SL_SIMTARGET_INSTRUMENTATION_EXPORT_EXTERN_C bool ssIsImageDataType(SimStruct* S,
                                                                    DTypeId dataTypeId);

SL_SIMTARGET_INSTRUMENTATION_EXPORT_EXTERN_C void ssGetInputPortImageSize(SimStruct* S,
                                                                          int portIdx,
                                                                          size_t* numRows,
                                                                          size_t* numColumns,
                                                                          size_t* numChannels);
SL_SIMTARGET_INSTRUMENTATION_EXPORT_EXTERN_C void ssGetOutputPortImageSize(SimStruct* S,
                                                                           int portIdx,
                                                                           size_t* numRows,
                                                                           size_t* numColumns,
                                                                           size_t* numChannels);

SL_SIMTARGET_INSTRUMENTATION_EXPORT_EXTERN_C void ssSetOutputPortImageSize(SimStruct* S,
                                                                           int portIdx,
                                                                           size_t numRows,
                                                                           size_t numColumns);

SL_SIMTARGET_INSTRUMENTATION_EXPORT_EXTERN_C const void* ssGetInputPortImageData(SimStruct* S,
                                                                                 int portIdx);
SL_SIMTARGET_INSTRUMENTATION_EXPORT_EXTERN_C void* ssGetOutputPortImageData(SimStruct* S,
                                                                            int portIdx);
SL_SIMTARGET_INSTRUMENTATION_EXPORT_EXTERN_C const void*
ssGetNonScalarInputPortImageData(SimStruct* S, int portIdx, int elemIdx);
SL_SIMTARGET_INSTRUMENTATION_EXPORT_EXTERN_C void* ssGetNonScalarOutputPortImageData(SimStruct* S,
                                                                                     int portIdx,
                                                                                     int elemIdx);

#endif
