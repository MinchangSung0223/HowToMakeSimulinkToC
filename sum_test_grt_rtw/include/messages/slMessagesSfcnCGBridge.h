/* Copyright 2008-2022 The MathWorks, Inc. */

#ifdef SUPPORTS_PRAGMA_ONCE
#pragma once
#endif

#ifndef __slMessagesSfcnCGBridge_hpp__
#define __slMessagesSfcnCGBridge_hpp__

#include "simstruct/simstruc.h"

#include "simulink_spec.h"

namespace RTWCG {
class CGIR_Block_IF;
}
namespace CG {
class CoreConstructionFacade;
class Var;
class Expr;
class Region;
}

namespace slmsgcg {

SIMULINK_EXPORT_EXTERN_C
CG::Region* cgCreateAndSendMessage(SimStruct* S,
                                   CG::CoreConstructionFacade* f,
                                   CG::Var* u,
                                   int queueType,
                                   int queueId,
                                   CG::Var* localQId);

SIMULINK_EXPORT_EXTERN_C
CG::Expr* cgCreateMessage(SimStruct* S,
                          CG::CoreConstructionFacade* f,
                          CG::Var* u,
                          int queueType,
                          int id,
                          CG::Var* localQId);

SIMULINK_EXPORT_EXTERN_C
CG::Region* cgSendMessage(SimStruct* S,
                          CG::CoreConstructionFacade* f,
                          CG::Var* msgVar,
                          int queueType,
                          int prtId,
                          CG::Var* localQId);

SIMULINK_EXPORT_EXTERN_C
CG::Region* cgReceiveMessage(SimStruct* S,
                             CG::CoreConstructionFacade* f,
                             CG::Var* msgVar,
                             int queueType,
                             int portId,
                             CG::Var* localQId,
                             CG::Var* statusVar);

SIMULINK_EXPORT_EXTERN_C
CG::Expr* cgGetMessageDataPtr(SimStruct* S,
                              CG::CoreConstructionFacade* f,
                              int queueType,
                              int portId,
                              CG::Var* msgVar);

SIMULINK_EXPORT_EXTERN_C
CG::Expr* cgGetNumMessages(SimStruct* S,
                           CG::CoreConstructionFacade* f,
                           int queueType,
                           int portId,
                           CG::Var* localQId);

SIMULINK_EXPORT_EXTERN_C
CG::Expr* cgDestroyMessage(SimStruct* S,
                           CG::CoreConstructionFacade* f,
                           int queueType,
                           int portId,
                           CG::Var* msgVar,
                           bool isFromInput = true);

SIMULINK_EXPORT_EXTERN_C
CG::Region* cgTargetQueueSend(CG::CoreConstructionFacade& ccf,
                              SimStruct* s,
                              CG::Var* msgDataVar,
                              int queueType,
                              int portId,
                              CG::Var* localQVar,
                              CG::Var* msgQueueOverflowVar);

SIMULINK_EXPORT_EXTERN_C
CG::Region* cgTargetQueueReceive(CG::CoreConstructionFacade& ccf,
                                 CG::Var* successVar,
                                 SimStruct* s,
                                 CG::Var* msgHandleVar,
                                 CG::Var* msgDataVar,
                                 CG::Var* payloadVar,
                                 int queueType,
                                 int portId,
                                 CG::Var* localQVar,
                                 CG::Var* msgQueueOverflowVar);

SIMULINK_EXPORT_EXTERN_C
CG::Region* cgTargetQueueDiscard(CG::CoreConstructionFacade& ccf,
                                 SimStruct* s,
                                 CG::Var* msgHandleVar,
                                 int queueType,
                                 int portId);

SIMULINK_EXPORT_EXTERN_C
bool cgIsTargetInputQueueExternal(SimStruct* S, int queueType, int portId);

SIMULINK_EXPORT_EXTERN_C
bool cgIsTargetOutputQueueExternal(SimStruct* S, int queueType, int portId);

SIMULINK_EXPORT_EXTERN_C
CG::Region* createSfMsgSendDWorkExpr(CG::CoreConstructionFacade& ccf,
                                     SimStruct* s,
                                     CG::Var* msgDataVar,
                                     CG::Var* msgQueueOverflowVar);

SIMULINK_EXPORT_EXTERN_C
CG::Region* createSfMsgRecvDWorkExpr(CG::CoreConstructionFacade& ccf,
                                     SimStruct* s,
                                     CG::Var* payloadVar,
                                     CG::Var* msgDataPtrVar,
                                     CG::Var* msgQueueOverflowVar);

}

#endif
