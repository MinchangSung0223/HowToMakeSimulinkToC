/* Copyright 2019-2021 The MathWorks, Inc. */

#ifdef SUPPORTS_PRAGMA_ONCE
#pragma once
#endif

#ifndef SLSIMTGTLIVECORERTW_H
#define SLSIMTGTLIVECORERTW_H

#include "sl_simtarget_instrumentation_spec.h"

SL_SIMTARGET_INSTRUMENTATION_EXPORT_EXTERN_C void* cGetParticipantPublisherSharedMemory(
    const char* identifier,
    const unsigned int maxSize);

SL_SIMTARGET_INSTRUMENTATION_EXPORT_EXTERN_C void* cGetOwnerPublisherSharedMemory(
    const char* identifier,
    const unsigned int maxSize);

SL_SIMTARGET_INSTRUMENTATION_EXPORT_EXTERN_C int cWaitForConnection(
    void* publisher,
    const unsigned int milliseconds);

SL_SIMTARGET_INSTRUMENTATION_EXPORT_EXTERN_C int cPublish(void* publisher,
                                                          const void* time,
                                                          const void* data);

SL_SIMTARGET_INSTRUMENTATION_EXPORT_EXTERN_C int cPublishTimeout(
    void* publisher,
    const void* time,
    const void* data,
    const unsigned int timeoutMilliseconds);

SL_SIMTARGET_INSTRUMENTATION_EXPORT_EXTERN_C void cDestroyPublisherSharedMemory(void** publisher);

SL_SIMTARGET_INSTRUMENTATION_EXPORT_EXTERN_C void* cGetOwnerSubscriberSharedMemory(
    const char* identifier,
    const unsigned int maxSize,
    void* callbackObject);

SL_SIMTARGET_INSTRUMENTATION_EXPORT_EXTERN_C void* cGetParticipantSubscriberSharedMemory(
    const char* identifier,
    const unsigned int maxSize,
    void* callbackObject);

SL_SIMTARGET_INSTRUMENTATION_EXPORT_EXTERN_C void cDestroySubscriberSharedMemory(void** subscriber);

SL_SIMTARGET_INSTRUMENTATION_EXPORT_EXTERN_C void* cCreateMultiSubscriberQueuePair(
    const char* identifier,
    const unsigned int dataSize);

SL_SIMTARGET_INSTRUMENTATION_EXPORT_EXTERN_C void cDestroyMultiSubscriberQueuePair(
    void** subscriber);

SL_SIMTARGET_INSTRUMENTATION_EXPORT_EXTERN_C void cPopAndWriteFromQueue(void* pairWithQueue,
                                                                        void* destinationBuffer);

#endif

