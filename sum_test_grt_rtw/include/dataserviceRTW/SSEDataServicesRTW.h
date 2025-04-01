/* Copyright 2020-2023 The MathWorks, Inc. */
#ifdef SUPPORTS_PRAGMA_ONCE
#pragma once
#endif

#ifndef SSEDATASERVICESRTW_H
#define SSEDATASERVICESRTW_H

#include "tmwtypes.h"

#include "SSEDataServicesRTW_spec.h"

/*
 * These functions act as a bridge to call the c-api of liveio from rapid accel target.
 * The call from c to c++ takes place here, 'Codegen' postfixed files in live/core
 * convert the c parameters to c++ objects and call the library functions.
 */

SSE_DATASERVICES_RTW_EXPORT_EXTERN_C int sse_dataservice_RegisterDataReader(const char* dtName,
                                                                            const char* typeName);

SSE_DATASERVICES_RTW_EXPORT_EXTERN_C void sse_dataservice_UnregisterDataReader(int uniqueID);

SSE_DATASERVICES_RTW_EXPORT_EXTERN_C int sse_dataservice_GetAgentID(int localID);

SSE_DATASERVICES_RTW_EXPORT_EXTERN_C int sse_dataservice_RegisterDataWriter(const char* dtName,
                                                                            const char* typeName,
                                                                            const char* icValues);

SSE_DATASERVICES_RTW_EXPORT_EXTERN_C void sse_dataservice_UnregisterDataWriter(int uniqueID);

SSE_DATASERVICES_RTW_EXPORT_EXTERN_C void sse_dataservice_SendWriteQuery(int uniqueID, char* query);

SSE_DATASERVICES_RTW_EXPORT_EXTERN_C char* sse_dataservice_SendReadQuery(int uniqueID, char* query);


/*------------------------- SysObj Actor Functions --------------------------*/

SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
uint64_T sseSysObjInitRapid(const char* modelName);


SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
void sseSysObjInitGRT(uint64_T actorID, const char* modelName);


SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
void sseSysObjResetStep();


SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
void sseSysObjGetPose(uint64_T actorID, double* mat4by4);


SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
void sseSysObjSetPose(uint64_T actorID, double* mat4by4);


SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
void sseSysObjGetVelocity(uint64_T actorID, double* vec1by3);


SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
void sseSysObjSetVelocity(uint64_T actorID, double* vec1by3);


SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
void sseSysObjGetAngularVelocity(uint64_T actorID, double* vec1by3);


SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
void sseSysObjSetAngularVelocity(uint64_T actorID, double* vec1by3);


SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
void sseSysObjGetWheelPoses(uint64_T actorID, double* mat4by4by18);


SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
void sseSysObjSetWheelPoses(uint64_T actorID, double* mat4by4by18);


SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
size_t sseSysObjGetNumActions(uint64_T actorID, const char* actionName);


SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
void sseSysObjGetActorAction(uint64_T thisActorID,
                             const char* actionName,
                             size_t idx,
                             char* actionID,
                             uint64_T* actorID,
                             int_T* phaseInterval,
                             int_T* actionType);


SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
void sseSysObjGetSpeedTarget(uint64_T actorID,
                             size_t idx,
                             double* speedValue,
                             int_T* speedComparison,
                             uint64_T* refActorID,
                             int_T* refSamplingMode);


SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
void sseSysObjGetTransitionDynamics(uint64_T actorID,
                                    size_t idx,
                                    int_T* dimension,
                                    int_T* shape,
                                    double* value);


SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
void sseSysObjGetPathTarget(uint64_T actorID,
                            size_t idx,
                            double* pathArray,
                            uint64_T* numPoints,
                            boolean_T* hasTiming);


SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
void sseSysObjGetPathPointTiming(uint64_T actorID,
                                 size_t idx,
                                 size_t pointIdx,
                                 double* time,
                                 double* speed,
                                 double* waitTime);

/*---------------------- RRScenario Block Functions -------------------------*/

SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
void sseLoadActorID(char* modelName);

SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
void sseLoadSensorID(char* sensorID);

SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
void sseLoadLaneBoundaryOutputOption(char* outputOption);

SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
void sseRegisterReaderBusType(char* busType, char* query);

SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
void sseRegisterWriterBusType(char* busType);

SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
bool sseRegisterModel();

SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
void sseResetStep();

SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
uint64_T sseGetActorID();

SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
bool sseSendMessage(void* msgData, char* msgType);

SSE_DATASERVICES_RTW_EXPORT_EXTERN_C
bool sseReadMessage(char* msgType, char* query, void* msgData);

#endif
