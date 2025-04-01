/* Copyright 2015-2022 The MathWorks, Inc. */

#ifdef SUPPORTS_PRAGMA_ONCE
#pragma once
#endif

#ifndef SLSIMTGTINSTRUMENTATIONSLMESSAGEBRIDGE_H
#define SLSIMTGTINSTRUMENTATIONSLMESSAGEBRIDGE_H

#include "sl_simtarget_instrumentation_spec.h"

SL_SIMTARGET_INSTRUMENTATION_EXPORT_EXTERN_C
void* slmsg_ssGetMsgMemMgr(SimStruct* S);

SL_SIMTARGET_INSTRUMENTATION_EXPORT_EXTERN_C
int slmsg_ssGetMsgQueueId(SimStruct* S, int qId);

SL_SIMTARGET_INSTRUMENTATION_EXPORT_EXTERN_C
int slmsg_ssGetExternalQueueForOutput(SimStruct* S, int rootOutportIdx);

SL_SIMTARGET_INSTRUMENTATION_EXPORT_EXTERN_C
ssExecServiceInfo ssGetInputServiceInfo(SimStruct* S, int portIdx, const char* svcName);

SL_SIMTARGET_INSTRUMENTATION_EXPORT_EXTERN_C
ssExecServiceInfo ssGetOutputServiceInfo(SimStruct* S, int portIdx, const char* svcName);

SL_SIMTARGET_INSTRUMENTATION_EXPORT_EXTERN_C
void ssSetInputServiceInfo(SimStruct* S,
                           int portIdx,
                           const char* svcName,
                           const ssExecServiceInfo* info);

SL_SIMTARGET_INSTRUMENTATION_EXPORT_EXTERN_C
void ssSetOutputServiceInfo(SimStruct* S,
                            int portIdx,
                            const char* svcName,
                            const ssExecServiceInfo* info);

SL_SIMTARGET_INSTRUMENTATION_EXPORT_EXTERN_C
void slmsg_invokeSendData(SimStruct* S,
                          void* hostPtr,
                          void* fcnPtr,
                          const void* data_to_send,
                          int* status);

SL_SIMTARGET_INSTRUMENTATION_EXPORT_EXTERN_C
void slmsg_invokeRecvData(SimStruct* S,
                          void* hostPtr,
                          void* fcnPtr,
                          void* received_data,
                          int* status);

SL_SIMTARGET_INSTRUMENTATION_EXPORT_EXTERN_C
void ssUsingGRTCodeForSimulation(SimStruct* S, void* host, void* fcn, int* usingGRT);

#endif
