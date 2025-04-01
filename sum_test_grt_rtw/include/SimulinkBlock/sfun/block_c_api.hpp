// Copyright 2007-2020 The MathWorks, Inc.

#pragma once

#include "../sl_core_block_spec.hpp"
#include "cgir_construct_from_SimulinkBlock.hpp"
#include "fwd.hpp"

#if defined(BUILDING_SIMULINKBLOCK) || defined(DLL_IMPORT_SYM)

#include "simstruct/simstruc.h"
#else

#include "simstruc.h"
#endif

class SLBlock;

namespace SLCG {
enum SystemFcnEnum : int;
}

namespace RTW {

extern "C" {

SL_CORE_BLOCK_EXPORT_CLASS SFun_Block_Impl* SFun_create_Block_Impl(
    SimStruct* S,
    void* rtwBlk,
    SFun_BlockFunctionTable* pFcnTable,
    int apiRev,
    int binaryRev);

SL_CORE_BLOCK_EXPORT_CLASS int SFun_Block_setOuter_B(SFun_CodeCtor_Impl* pThis,
                                                     CodeConstructor* pOuter);

SL_CORE_BLOCK_EXPORT_CLASS int SFun_Block_constructCode(SFun_CodeCtor_Impl* pThis);

SL_CORE_BLOCK_EXPORT_CLASS SimStruct* SFun_Block_getSimStruct(SFun_CodeCtor_Impl* pThis);

SL_CORE_BLOCK_EXPORT_CLASS SLBlock* SFun_Block_getSlBlock(SFun_CodeCtor_Impl* pThis);

SL_CORE_BLOCK_EXPORT_CLASS SFun_Value_Impl* SFun_Block_param_i(SFun_CodeCtor_Impl* pThis,
                                                               int index);

SL_CORE_BLOCK_EXPORT_CLASS SFun_Reference_Impl* SFun_Block_dWork_i(SFun_CodeCtor_Impl* pThis,
                                                                   int index);
}
}

