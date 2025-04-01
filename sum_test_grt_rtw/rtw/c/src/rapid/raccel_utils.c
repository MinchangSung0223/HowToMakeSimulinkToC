/******************************************************************
 *
 *  File: raccel_utils.c
 *
 *
 *  Abstract:
 *      - provide utility functions for rapid accelerator 
 *
 * Copyright 2007-2023 The MathWorks, Inc.
 ******************************************************************/

/* INCLUDES */
#include  <stdio.h>
#include  <stdlib.h>

#include  <string.h>
#include  <math.h>
#include  <float.h>
#include  <ctype.h>
#include <setjmp.h>

/*
 * We want access to the real mx* routines in this file and not their RTW
 * variants in rt_matrx.h, the defines below prior to including simstruc.h
 * accomplish this.
 */
#include  "mat.h"
#define TYPEDEF_MX_ARRAY
#define rt_matrx_h
#include "simstruc.h"
#undef rt_matrx_h
#undef TYPEDEF_MX_ARRAY

#include "dt_info.h"
#include "rtw_capi.h"
#include "rtw_modelmap.h"
#include "mwstringutil.h"
#include "mwslpointerutil.h"

#include "slsa_sim_common_utils.h"
#include "common_mat_utils.h"
#include "raccel_utils.h"
#include "sigstream_rtw.h"
#include "slsv_diagnostic_codegen_c_api.h"
#include "slsa_sim_engine.h"

/* external variables */
extern ssExecutionInfo gblExecutionInfo;
extern int_T         gblParamCellIndex;
/* for tuning struct params using the C-API */
extern rtwCAPI_ModelMappingInfo* rt_modelMapInfoPtr;

/* global variables */
void* gblLoggingInterval = NULL;
dl_logger_sid_t gblDiagnosticLogger = NULL;
dl_logger_sid_t gblBlockPathDB = NULL;
BlockPathMemMgr* gblBlockPathMemMgrHead = NULL;
BlockPathMemMgr* gblBlockPathMemMgrTail = NULL;

/* External Functions */
extern const char* getParamFilename(void);


/*==================*
 * NON-Visible routines *
 *==================*/

/* Function: rt_FreeParamStructs ===========================================
 * Abstract:
 *      Free and NULL the fields of all 'PrmStructData' structures.
 *      PrmStructData contains an array of ParamInfo structures, with one such
 *      structure for each non-struct data type, and one such structure for 
 *      each leaf of every struct parameter. 
 */
void rt_FreeParamStructs()
{
    PrmStructData *paramStructure = &gblExecutionInfo.gblObjects_.paramStruct;
    if (paramStructure != NULL)
    {

        size_t nNonStructDataTypes =
            paramStructure->nNonStructDataTypes;
        size_t nStructLeaves =
            paramStructure->nStructLeaves;
        ParamInfo *paramInfo =
            paramStructure->paramInfo;

        {
            size_t i;
            if (paramInfo != NULL)
            {
                for (i=0; i < nNonStructDataTypes+nStructLeaves; i++)
                {
                    /* Free copy of string data */
                    if (paramInfo[i].isString)
                    {
                        mxDestroyArray(paramInfo[i].strVals);
                    }

                    /*
                     * Must free "stolen" parts of matrices with
                     * mxFree (they are allocated with mxCalloc).
                     */
#if defined(MX_HAS_INTERLEAVED_COMPLEX)
                    mxFree(paramInfo[i].vals);
#else
                    mxFree(paramInfo[i].rVals);
                    mxFree(paramInfo[i].iVals);
#endif
                }
                free(paramInfo);
            }
        }

        paramStructure->nTrans = 0;
        paramStructure->nNonStructDataTypes = 0;
        paramStructure->nStructLeaves = 0;
        paramStructure->paramInfo = NULL;
    }
} /* end rt_FreeParamStructs */



/* Function: rt_GetNumStructLeaves ============================
 * Abstract:
 *  Recursive function to traverse a parameter structure and return the
 *  number of leaves needing a 'paramInfo'.  Initially called by
 *  rt_GetNumStructLeavesAndNumNonStructDataTypes() whenever a struct 
 *  parameter is encountered in the rtp MATFile, then recursively called 
 *  while processing the structure to find all of the leaf elements.
 */
size_t rt_GetNumStructLeaves(
    uint16_T dTypeMapIdx, 
    uint16_T dimMapIdx,
    uint16_T fixPtIdx)
{
    size_t locNParamStructs = 0;
    size_t  numArrayElements = 1;

    const rtwCAPI_ModelMappingInfo *mmi =
        rt_modelMapInfoPtr;
    
    const rtwCAPI_DataTypeMap *dTypeMap =
        rtwCAPI_GetDataTypeMap(mmi);
    
    uint16_T dTypeMapNumElems =
        rtwCAPI_GetDataTypeNumElements(
            dTypeMap,
            dTypeMapIdx);

    /* found either a non-struct leaf (dTypeMapNumElems == 0) or a struct leaf that is a
     * fixed-point value */
    if (dTypeMapNumElems==0 || fixPtIdx > 0) 
        return(1);

    /* found a non-fixed-point struct field; recurse */
    {
        int loopIdx;
        uint16_T elDTypeMapIdx;
        uint16_T elDimMapIdx;
        uint16_T elFixPtIdx;
        
        const rtwCAPI_ElementMap *elemMap =
            rtwCAPI_GetElementMap(mmi);
        
        uint16_T dTypeMapElemMapIdx =
            rtwCAPI_GetDataTypeElemMapIndex(
                dTypeMap,
                dTypeMapIdx);

        for (loopIdx=0;
             loopIdx < dTypeMapNumElems;
             loopIdx++)
        {
            elDTypeMapIdx =
                rtwCAPI_GetElementDataTypeIdx(
                    elemMap,
                    dTypeMapElemMapIdx+loopIdx);
            
            elDimMapIdx =
                rtwCAPI_GetElementDimensionIdx(
                    elemMap,
                    dTypeMapElemMapIdx+loopIdx);
            
            elFixPtIdx =
                rtwCAPI_GetElementFixPtIdx(
                    elemMap,
                    dTypeMapElemMapIdx+loopIdx);
            
            locNParamStructs +=
                rt_GetNumStructLeaves(
                    elDTypeMapIdx,
                    elDimMapIdx,
                    elFixPtIdx);
        }
    }

    /* at this point, locNParamStructs will contain the number of leaves in a single element of 
     * the current struct array. multiplying locNParamStructs by the number of elements in the
     * current struct array will give the total number of leaves. */
    {
        const rtwCAPI_DimensionMap *dimMap =
            rtwCAPI_GetDimensionMap(mmi);
        
        const uint_T *dimArray =
            rtwCAPI_GetDimensionArray(mmi);
        
        int numDims =
            rtwCAPI_GetNumDims(
                dimMap,
                dimMapIdx);
        
        uint_T dimArrayIdx =
            rtwCAPI_GetDimArrayIndex(
                dimMap,
                dimMapIdx);

        int loopIdx;        
        for (loopIdx=0;
             loopIdx < numDims;
             ++loopIdx)
        {
            numArrayElements *= dimArray[dimArrayIdx + loopIdx];
        }
    }
    
    return(locNParamStructs * numArrayElements);
} /* end rt_GetNumStructLeaves */




/* Function: rt_GetNumStructLeavesAndNumNonStructDataTypes ===================================
 * Abstract:
 *   Count the number of non-struct data types, and the number of leaves of all struct
 *   parameters.
 */
void rt_GetNumStructLeavesAndNumNonStructDataTypes(
    const mxArray *paParamStructs,
    size_t *numStructLeaves,
    size_t *numNonStructDataTypes,
    const char **result)
{
    size_t nTrans = mxGetNumberOfElements(paParamStructs);

    *numStructLeaves       = 0;
    *numNonStructDataTypes = 0;

    {
        rtwCAPI_ModelMappingInfo *mmi =
            rt_modelMapInfoPtr;
        
        const rtwCAPI_BlockParameters *blkPrms =
            rtwCAPI_GetBlockParameters(mmi);
        
        const rtwCAPI_ModelParameters *mdlPrms =
            rtwCAPI_GetModelParameters(mmi);

        int loopIdx;
        for (loopIdx=0;
             loopIdx<nTrans;
             loopIdx++)
        {
            mxArray *valueMat =
                NULL;
            mxArray *structParamInfo =
                NULL;

            valueMat = mxGetField(
                paParamStructs,
                loopIdx,
                "values");
        
            /* 
             * If the value field of a parameter in the RTP is empty, count the parameter
             * as a non-struct datatype (struct parameters must have non-empty value fields).
             * For struct / bus parameters, and only for those types of parameters, 
             * rtp.parameters.values is a cell array.
             */
            if (valueMat == NULL || !mxIsStruct(valueMat))
            {
                /* new non-struct data type found */
                (*numNonStructDataTypes)++;
                continue;
            }

            /* struct parameter found; count all leaves */
            /* structParamInfo (contains c-api index) */
            structParamInfo = mxGetField(
                paParamStructs,
                loopIdx,
                "structParamInfo");
            
            if (structParamInfo == NULL)
            {
                *(result) = "The RTP entry for struct parameters must have a nonempty structParamInfo field.";
            }

            /* each struct transition contains exactly 1 parameter */
            if (mxGetN(valueMat) != 1 && mxGetN(structParamInfo) != 1)
            {
                *(result) = "Invalid rtp: only one struct parameter per data-type transition is allowed";
            }

            {
                double *pr = NULL;
                mxArray *mat = NULL;
                bool modelParam = false;
                size_t capiIdx = UINT_MAX;
                uint16_T dTypeMapIdx = 0;
                uint16_T dimMapIdx = 0;
                uint16_T fixPtIdx = 0;

                /* parameter type (model or block) */
                mat = mxGetField(
                    structParamInfo,
                    0,
                    "ModelParam");
                
                if (mat == NULL)
                {
                    *(result) = "The structParamInfo field for struct parameters must have a nonempty modelParam field.";
                    return;
                }                
                pr  = mxGetPr(mat);
                modelParam = (bool)pr[0];

                /* c-api index */
                mat = mxGetField(
                    structParamInfo,
                    0,
                    "CAPIIdx");
                
                if (mat == NULL)
                {
                    *(result) = "The structParamInfo field for struct parameters must have a nonempty CAPIIndex field.";
                    return;
                }
                pr  = mxGetPr(mat);
                capiIdx = (size_t)pr[0];

                /* c-api datatype map index and dimension map index */
                dTypeMapIdx = (modelParam) ?
                    rtwCAPI_GetModelParameterDataTypeIdx(mdlPrms, capiIdx) :
                    rtwCAPI_GetBlockParameterDataTypeIdx(blkPrms, capiIdx);

                dimMapIdx = (modelParam) ?
                    rtwCAPI_GetModelParameterDimensionIdx(mdlPrms, capiIdx) :
                    rtwCAPI_GetBlockParameterDimensionIdx(blkPrms, capiIdx);
                
                fixPtIdx = (modelParam) ? 
                    rtwCAPI_GetModelParameterFixPtIdx(mdlPrms, capiIdx) :
                    rtwCAPI_GetBlockParameterFixPtIdx(blkPrms, capiIdx);

                (*numStructLeaves) += rt_GetNumStructLeaves(dTypeMapIdx, dimMapIdx, fixPtIdx);
            }
        }
    }
} /* end rt_GetNumStructLeavesAndNumNonStructDataTypes */

/* Function: rt_UpdateParamInfoForStringDataType ==============================
 * Abstract:
 *  Update a ParamInfo struct for a string data type
 */
void rt_UpdateParamInfoForStringDataType(
    const mxArray *mat,
    ParamInfo *paramInfo)
{
    paramInfo->isString = true;
    /* strings are pointers */
    paramInfo->elSize = 8;
    if (mxIsCell(mat)) {
        paramInfo->strVals = mxDuplicateArray(mat);
    } else {
        /* string fields in structures need to be wrapped in cell array*/
        mxArray* aCell = mxCreateCellMatrix(1, 1);
        mxSetCell(aCell, 0, mxDuplicateArray(mat));
        paramInfo->strVals = aCell;
        paramInfo->nEls = 1;
    }
}

/* Function: rt_IsStringData ===========================
 * Abstract:
 *  Check if parameter data is string (stored as cell array of character vectors)
 */
bool rt_IsStringData(const mxArray *mat)
{
    /* Handle string fields in structures */
    if (mxIsClass(mat, "string") || mxIsChar(mat)) {
        return true;
    }
    
    if (!mxIsCell(mat))
    {
        return false;
    }
    if (mxGetNumberOfElements(mat) < 1)
    {
        return false;
    }
    mxArray *el = mxGetCell(mat, 0);
    return mxIsChar(el);
}


/* Function: rt_CreateParamInfoForNonStructDataType ===========================
 * Abstract:
 *  Create a ParamInfo struct for a non-struct data type.
 */
void rt_CreateParamInfoForNonStructDataType(
    mxArray* mat, 
    const mxArray* paParamStructs,   
    const size_t paramStructIndex,
    PrmStructData *paramStructure,
    size_t paramInfoIndex)
{
    ParamInfo *paramInfo = NULL;
    double *pr = NULL;
    
    paramInfo = &paramStructure->paramInfo[paramInfoIndex];
    paramInfo->structLeaf = false;
    paramInfo->elSize = mxGetElementSize(mat);
    paramInfo->nEls   = mxGetNumberOfElements(mat);

    if (rt_IsStringData(mat))
    {
        rt_UpdateParamInfoForStringDataType(mat, paramInfo);
    } else {
#if defined(MX_HAS_INTERLEAVED_COMPLEX)
        paramInfo->vals  = mxGetData(mat);
        mxSetData(mat,NULL);
#else
        paramInfo->rVals  = mxGetData(mat);
        mxSetData(mat,NULL);

        if (mxIsNumeric(mat))
        {
            paramInfo->iVals  = mxGetImagData(mat);
            mxSetImagData(mat,NULL);
        }  
#endif
    }
    
    /* Grab the datatype id. */
    mat = mxGetField(
        paParamStructs,
        paramStructIndex,
        "dataTypeId");
    
    pr  = mxGetPr(mat);
    paramInfo->dataType = (int)pr[0];

    /* Grab the complexity. */
    mat = mxGetField(
        paParamStructs,
        paramStructIndex,
        "complex");
    
    pr  = mxGetPr(mat);
    paramInfo->complex = (bool)pr[0];

    /* Grab the dtTransIdx */
    mat = mxGetField(
        paParamStructs,
        paramStructIndex,
        "dtTransIdx");
    
    pr  = mxGetPr(mat);
    paramInfo->dtTransIdx = (int)pr[0];
} /* end rt_CreateParamInfoForNonStructDataType */




/* Function: rt_CopyStructFromStructArray ======================================
 * Abstract:
 *   Given an mxArray of that is an array of structs, and given an index, 
 *   create a copy of the struct in the array that's at the given index.  
 *   This function assumes that the argument structArray is an mxStruct.
 */
void rt_CopyStructFromStructArray(
    mxArray *structArray,
    mxArray **destination,
    size_t structArrayIndex)
{
    int numFields
        = mxGetNumberOfFields(structArray);
    
    const char **fieldNames =
        (const char **) malloc(sizeof(const char *) * numFields);
    
    {
        int fieldIndex;
        for (fieldIndex = 0;
             fieldIndex < numFields;
             ++fieldIndex)
        {
            fieldNames[fieldIndex] =
                mxGetFieldNameByNumber(
                    structArray,
                    fieldIndex);
        }

        *destination = mxCreateStructMatrix(
            1,
            1,
            numFields,
            fieldNames);
    
        for (fieldIndex = 0;
             fieldIndex < numFields;
             ++fieldIndex)
        {
            mxArray *field = mxGetField(
                structArray, 
                structArrayIndex, 
                fieldNames[fieldIndex]);
            
            mxSetField(
                *destination, 
                0, 
                fieldNames[fieldIndex],
                field);
        }
    }
}


/* Function: rt_GetStructLeafInfo ================================================
 * Abstract:
 *  Recursive function to traverse the fields of a single struct parameter with
 *  value valueMat, and to create a ParamInfo structure for each leaf of the
 *  parameter.
 */
void rt_GetStructLeafInfo(
    uint16_T dTypeMapIdx,
    uint16_T fixPtIdx,
    size_t capiIdx,
    void *baseAddr,
    boolean_T isPointer,
    mxArray *valueMat,
    boolean_T modelParam,
    PrmStructData *paramStructure,
    size_t *paramInfoIndex,
    const char** result)
{
    rtwCAPI_ModelMappingInfo  *mmi =
        rt_modelMapInfoPtr;
    
    rtwCAPI_DataTypeMap const *dTypeMap
        = rtwCAPI_GetDataTypeMap(mmi);
    
    uint16_T dTypeMapNumElems =
        rtwCAPI_GetDataTypeNumElements(
            dTypeMap,
            dTypeMapIdx);

    /* fixPtIdx will be > 0 if and only if the parameter currently being inspected is a fixed-point
     * parameter. */
    if (dTypeMapNumElems == 0 || fixPtIdx > 0)
    {
        /* This is a leaf of a struct. Increment paramInfoIndex. */
        ParamInfo *paramInfo = &paramStructure->paramInfo[(*paramInfoIndex)++];
        paramInfo->structLeaf = true;
        paramInfo->modelParam = modelParam;
        paramInfo->dataType = dTypeMapIdx;
        paramInfo->complex = (boolean_T) rtwCAPI_GetDataIsComplex(dTypeMap, dTypeMapIdx);
        paramInfo->prmAddr = baseAddr;
        paramInfo->isPointer = isPointer;
        paramInfo->capiIdx = capiIdx;
        
        /* Grab the data and any attributes.  We "steal" the data from the mxArray. */
        if (valueMat)
        {
            paramInfo->elSize = mxGetElementSize(valueMat);
            paramInfo->nEls   = mxGetNumberOfElements(valueMat);
            if (rt_IsStringData(valueMat))
            {
                rt_UpdateParamInfoForStringDataType(valueMat, paramInfo);
            } else {
#if defined(MX_HAS_INTERLEAVED_COMPLEX)
                paramInfo->vals  = mxGetData(valueMat);
                mxSetData(valueMat,NULL);
#else
                paramInfo->rVals  = mxGetData(valueMat);
                mxSetData(valueMat,NULL);

                if (mxIsNumeric(valueMat))
                {
                    paramInfo->iVals  = mxGetImagData(valueMat);
                    mxSetImagData(valueMat,NULL);
                }
#endif
            }
        } else {
            paramInfo->nEls   = 0;
            paramInfo->elSize = 0;
#if defined(MX_HAS_INTERLEAVED_COMPLEX)
            paramInfo->vals  = NULL;
#else
            paramInfo->rVals  = NULL;
            paramInfo->iVals  = NULL;
#endif
        }

        return;
    } else {
        /* This is a struct, possibly a struct array; recurse over its fields */     
        uint16_T dTypeMapElemMapIdx =
            rtwCAPI_GetDataTypeElemMapIndex(
                dTypeMap,
                dTypeMapIdx);
        
        rtwCAPI_ElementMap const *elemMap =
            rtwCAPI_GetElementMap(mmi);
        
        rtwCAPI_DimensionMap const *dimMap =
            rtwCAPI_GetDimensionMap(mmi);

        {
            int loopIdx;
            for (loopIdx=0;
                 loopIdx < dTypeMapNumElems;
                 loopIdx++)
            {       
                uint16_T elDTypeMapIdx =
                    rtwCAPI_GetElementDataTypeIdx(
                        elemMap,
                        dTypeMapElemMapIdx+loopIdx);
                
                uint32_T elOffset =
                    rtwCAPI_GetElementOffset(
                        elemMap,
                        dTypeMapElemMapIdx+loopIdx);
                
                uint16_T elFixPtIdx =
                    rtwCAPI_GetElementFixPtIdx(
                        elemMap,
                        dTypeMapElemMapIdx+loopIdx);
                
                uint16_T elDimMapIdx =
                    rtwCAPI_GetElementDimensionIdx(
                        elemMap,
                        dTypeMapElemMapIdx+loopIdx);
                
                boolean_T elIsPointer =
                    (boolean_T) rtwCAPI_GetDataIsPointer(
                        dTypeMap, elDTypeMapIdx);
                
                const char_T* fieldName =
                    rtwCAPI_GetElementName(
                        elemMap,
                        dTypeMapElemMapIdx+loopIdx);
                
                mxArray *subMat =
                    mxGetField(
                        valueMat,
                        0,
                        fieldName);

                uint8_T elNumDims =
                    rtwCAPI_GetNumDims(
                        dimMap,
                        elDimMapIdx);
                
                uint_T elDimArrayIdx =
                    rtwCAPI_GetDimArrayIndex(
                        dimMap,
                        elDimMapIdx);
                
                const uint_T *dimArray =
                    rtwCAPI_GetDimensionArray(mmi);
                
                uint16_T elNumElems =
                    rtwCAPI_GetDataTypeNumElements(
                        dTypeMap,
                        elDTypeMapIdx);
                
                uint_T numArrayElements = 1;
                int dimensionLoopCounter;

                for (dimensionLoopCounter=0;
                     dimensionLoopCounter < elNumDims;
                     ++dimensionLoopCounter)
                {
                    numArrayElements *= dimArray[elDimArrayIdx + dimensionLoopCounter];
                }
            
                if (numArrayElements == 1 ||
                    elFixPtIdx > 0 ||
                    elNumElems == 0)
                {
                    /* this field is either not a struct, or is a 1x1 struct array */
                    rt_GetStructLeafInfo(
                        elDTypeMapIdx, 
                        elFixPtIdx,
                        capiIdx,
                        (unsigned char *) baseAddr+elOffset,
                        elIsPointer,
                        subMat,
                        modelParam,
                        paramStructure,
                        paramInfoIndex,
                        result);
                } else {
                    /* non 1x1 struct array */
                    void* localBaseAddr =
                        baseAddr;

                    size_t arrayIdx;
                    for (arrayIdx=0; arrayIdx < numArrayElements; ++arrayIdx)
                    {
                        mxArray* subMatElement =
                            NULL;
                        
                        uint16_T elStructSize  =
                            rtwCAPI_GetDataTypeSize(
                                dTypeMap,
                                elDTypeMapIdx);
                        
                        rt_CopyStructFromStructArray(
                            subMat,
                            &subMatElement,
                            arrayIdx);
                        
                        rt_GetStructLeafInfo(
                            elDTypeMapIdx, 
                            elFixPtIdx,
                            capiIdx,
                            (unsigned char *)localBaseAddr+elOffset,
                            elIsPointer,
                            subMatElement,
                            modelParam,
                            paramStructure,
                            paramInfoIndex,
                            result);

                        localBaseAddr = (void*)((char*)localBaseAddr + elStructSize);                    
                    }
                }
            }
        }
    }
} /* end rt_GetStructLeafInfo */



/* Function: rt_GetInfoFromOneStructParam ==================================
 * Abstract:
 *   Create ParamInfo structures for each leaf of a struct parameter. 
 */
void rt_GetInfoFromOneStructParam(
    mxArray *valueMat,
    PrmStructData *paramStructure,
    size_t *paramInfoIndex,
    boolean_T modelParam,
    size_t capiIdx,
    const char** result)
{
    if (valueMat == NULL)
    {
        *(result) = "the value field of an RTP entry for some struct param is empty (rt_GetInfoFromOneStructParam)";
        return;
    }

    if (!mxIsStruct(valueMat))
    {
        /* This function should deal only with struct params */
        *(result) = "non-struct param found where only struct params were expected (rt_GetInfoFromOneStructParam)";
        return;
    } else {
        uint16_T structSizeInBytes;
        uint16_T fixPtIdx;
        uint16_T dTypeMapIdx;
        uint16_T dimMapIdx;
        
        int addrIdx = -1;
        void *baseAddr = NULL;
        
        rtwCAPI_ModelMappingInfo *mmi =
            rt_modelMapInfoPtr;
        
        rtwCAPI_BlockParameters const *blkPrms =
            rtwCAPI_GetBlockParameters(mmi);
        
        rtwCAPI_ModelParameters const *mdlPrms =
            rtwCAPI_GetModelParameters(mmi);
        
        void ** addrMap =
            rtwCAPI_GetDataAddressMap(mmi);
        
        const rtwCAPI_DataTypeMap *dTypeMap =
            rtwCAPI_GetDataTypeMap(mmi);

        addrIdx  = (modelParam) ?
            rtwCAPI_GetModelParameterAddrIdx(mdlPrms, capiIdx) :
            rtwCAPI_GetBlockParameterAddrIdx(blkPrms, capiIdx);
        
        baseAddr = rtwCAPI_GetDataAddress(
            addrMap,
            addrIdx);

        dTypeMapIdx = (modelParam) ?
            rtwCAPI_GetModelParameterDataTypeIdx(mdlPrms, capiIdx) :
            rtwCAPI_GetBlockParameterDataTypeIdx(blkPrms, capiIdx);

        dimMapIdx = (modelParam) ?
            rtwCAPI_GetModelParameterDimensionIdx(mdlPrms, capiIdx) :
            rtwCAPI_GetBlockParameterDimensionIdx(blkPrms, capiIdx);

        structSizeInBytes = rtwCAPI_GetDataTypeSize(
            dTypeMap,
            dTypeMapIdx);

        fixPtIdx = modelParam ?
            rtwCAPI_GetModelParameterFixPtIdx(mdlPrms, capiIdx) : 
            rtwCAPI_GetBlockParameterFixPtIdx(blkPrms, capiIdx);
        
        if (fixPtIdx != 0)
        {
            *(result) = "Fixed-point index of struct params should be 0. (rt_GetInfoFromOneStructParam)";
            return;
        }

        /* Recursively traverse the struct param, creating a new ParamInfo structure for each leaf of the parameter. */
        {
            size_t numArrayElements =
                mxGetNumberOfElements(valueMat);
            
            if (numArrayElements == 1)
            {
                rt_GetStructLeafInfo(
                    dTypeMapIdx, 
                    fixPtIdx,
                    capiIdx,
                    baseAddr,
                    false, 
                    valueMat,
                    modelParam,
                    paramStructure,
                    paramInfoIndex,
                    result);
            } else {
                void* localBaseAddr =
                    baseAddr;

                int loopIdx;
                for (loopIdx = 0;
                     loopIdx < numArrayElements;
                     loopIdx++)
                {
                    mxArray* valueMatElement = NULL;
                
                    rt_CopyStructFromStructArray(
                        valueMat,
                        &valueMatElement, 
                        loopIdx);

                    rt_GetStructLeafInfo(
                        dTypeMapIdx, 
                        fixPtIdx,
                        capiIdx,
                        localBaseAddr,
                        false,
                        valueMatElement,
                        modelParam,
                        paramStructure,
                        paramInfoIndex,
                        result);

                    localBaseAddr = (void*)((char*)localBaseAddr+ structSizeInBytes);
                }
            }
        }
    }
}  /* end rt_GetInfoFromOneStructParam */


/* Function: rt_PrepareToCreateParamInfosForStructParam ===========================
 * Abstract:
 *  Create a ParamInfo struct for a non-struct data type.
 */
const char * rt_PrepareToCreateParamInfosForStructParam(
    mxArray* valueMat, 
    const mxArray* paParamStructs,
    size_t paramStructIndex,
    PrmStructData *paramStructure,
    size_t *paramInfoIndex)
{
    const mxArray *structParamInfo = NULL;
    const mxArray *temp = NULL;                
    size_t capiIndex = UINT_MAX;
    int structParamInfoIndex = 0;
    double *pr = NULL;
    boolean_T modelParam = false;
    const char *result = NULL;

    if (valueMat == NULL)
    {
        return result;
    } else if (!mxIsStruct(valueMat)) {
        result = "A non-struct value was found in the rtp where a struct value was expected.";
        return result;
    }

    structParamInfo = mxGetField(
        paParamStructs,
        paramStructIndex,
        "structParamInfo");

    if (structParamInfo == NULL)
    {
        result = "Some struct parameter has an empty structParamInfo field.";
        return result;
    }

    if (mxGetN(valueMat) != 1 && mxGetN(structParamInfo) != 1)
    {
        result = "Invalid rtp: only one struct parameter per data-type transition is allowed";
        return result;
    }
                
    temp = mxGetField(structParamInfo, structParamInfoIndex, "ModelParam");
    if (temp)
    {
        pr = mxGetPr(temp);
        modelParam = (boolean_T)pr[0];
    } else {
        modelParam = 0;
    }

    temp = mxGetField(structParamInfo, structParamInfoIndex, "CAPIIdx");
    if (temp)
    {
        pr = mxGetPr(temp);
        capiIndex = (int) pr[0];
    } else {
        capiIndex = UINT_MAX;
    }             

    rt_GetInfoFromOneStructParam(
        valueMat,
        paramStructure,
        paramInfoIndex,
        modelParam,
        capiIndex,
        &result);

    if (result != NULL) 
        return result;

    return result;
} /* end rt_PrepareToCreateParamInfosForStructParam */



/* Function: rt_ReadParamStructMatFile=======================================
 * Abstract:
 *  Reads a matfile containing a new parameter structure.  It also reads the
 *  model checksum and compares this with the RTW generated code's checksum
 *  before inserting the new parameter structure.
 *
 * Returns:
 *	NULL    : success
 *	non-NULL: error string
 */
const char * rt_ReadParamStructMatFile(
    PrmStructData **prmStructOut,
    const SimStruct * S,
    int cellParamIndex)
{
    size_t nTrans = 0;
    MATFile *pmat = NULL;
    mxArray *mxParameters = NULL;
    mxArray *mxChecksum = NULL;
    PrmStructData *paramStructure = NULL;
    const char *result = NULL; /* assume success */

    paramStructure = &gblExecutionInfo.gblObjects_.paramStruct;

    /**************************************************************************
     * Open parameter MAT-file, read checksum, swap rtP data for type Double *
     **************************************************************************/

    if ((pmat=matOpen(getParamFilename(),"r")) == NULL)
    {
        result = "could not find MAT-file containing new parameter data";
        goto EXIT_POINT;
    }

    /*
     * Read the param variable. The variable name must be passed in
     * from the generated code.
     */
    const char* parameterField = "parameters";
    const char* checksumField = "modelChecksum";
    if((mxChecksum = matGetVariable(pmat, checksumField)) == NULL)
    {
        result = "Parameter MAT file must contain a 'modelChecksum' field (matGetVariable)";
        goto EXIT_POINT;

    }

    if ((mxParameters = matGetVariable(pmat, parameterField)) == NULL)
    {
        result = "Parameter MAT file must contain a 'parameters' field (matGetVariable)";
        goto EXIT_POINT;

    }

    /* look for modelChecksum field */
    {
        const double  *newChecksum;

        if (!mxIsDouble(mxChecksum) ||
            mxIsComplex(mxChecksum) ||
            mxGetNumberOfDimensions(mxChecksum) > 2 ||
            mxGetM(mxChecksum) < 1 ||
            mxGetN(mxChecksum) !=4 )
        {
            result = "invalid modelChecksum in parameter MAT-file";
            goto EXIT_POINT;
        }

        newChecksum = mxGetPr(mxChecksum);

        paramStructure->checksum[0] = newChecksum[0];
        paramStructure->checksum[1] = newChecksum[1];
        paramStructure->checksum[2] = newChecksum[2];
        paramStructure->checksum[3] = newChecksum[3];
    }

    /* be sure checksums all match */
    if (paramStructure->checksum[0] != ssGetChecksum0(S) ||
        paramStructure->checksum[1] != ssGetChecksum1(S) ||
        paramStructure->checksum[2] != ssGetChecksum2(S) ||
        paramStructure->checksum[3] != ssGetChecksum3(S) )
    {
        result = "model checksum mismatch - incorrect parameter data "
            "specified";
        goto EXIT_POINT;
    }

    /*
     * If the parameters field is a cell array then pick out the cell
     * array pointed to by the cellParamIndex
     */
    if ( mxIsCell(mxParameters) )
    {
        /* check that cellParamIndex is in range */
        size_t size =
            mxGetM(mxParameters) * mxGetN(mxParameters);
        
        if (cellParamIndex > 0 &&
            cellParamIndex <= (int) size)
        {
            mxParameters = mxGetCell(
                mxParameters,
                cellParamIndex-1);
        } else{
            result = "Invalid index into parameter cell array";
            goto EXIT_POINT;
        }
        
        if (mxParameters == NULL)
        {
            result = "Invalid parameter field in parameter structure";
            goto EXIT_POINT;
        }
    }

    /* the number of data-types in the RTP */
    nTrans = mxGetNumberOfElements(mxParameters);
    if (nTrans == 0) goto EXIT_POINT;

    /*
     * Validate the array fields - only check the first element of the
     * array since all elements of a structure array have the same
     * fields.
     *
     * It is assumed that if the proper data fields exists, that the
     * data is correct.
     */
    {
        mxArray *dum;

        if ((dum = mxGetField(mxParameters, 0, "dataTypeName")) == NULL)
        {
            result = "parameters struct must contain a dataTypeName field";
            goto EXIT_POINT;
        }

        if ((dum = mxGetField(mxParameters, 0, "dataTypeId")) == NULL)
        {
            result = "parameters struct must contain a dataTypeId field";
            goto EXIT_POINT;
        }

        if ((dum = mxGetField(mxParameters, 0, "complex")) == NULL)
        {
            result = "parameters struct must contain a complex field";
            goto EXIT_POINT;
        }

        if ((dum = mxGetField(mxParameters, 0, "dtTransIdx")) == NULL)
        {
            result = "parameters struct must contain a dtTransIdx field";
            goto EXIT_POINT;
        }
    }

    /* 
     * Calculate the total number of ParamInfo structures needed. Each non-struct
     * data type is given one ParamInfo, and each leaf of every struct 
     * parameter is given one ParamInfo.
     */ 
    {
        size_t nStructLeaves = 0;
        size_t nNonStructDataTypes = 0;
        size_t nParamInfos = 0;
        
        rt_GetNumStructLeavesAndNumNonStructDataTypes(
            mxParameters, 
            &nStructLeaves,
            &nNonStructDataTypes,
            &result);

        if (result != NULL) goto EXIT_POINT;

        paramStructure->nTrans =
            nTrans;
        paramStructure->nStructLeaves =
            nStructLeaves;
        paramStructure->nNonStructDataTypes =
            nNonStructDataTypes;

        /*
         * Allocate the ParamInfo's.
         * The total number of ParamInfos needed is nStructLeaves + nNonStructDataTypes,
         */
        nParamInfos =
            nStructLeaves + nNonStructDataTypes;
        
        paramStructure->paramInfo =
            (ParamInfo *) calloc(nParamInfos, sizeof(ParamInfo));
    }
    
    if (paramStructure->paramInfo == NULL)
    {
        result = "Memory allocation error";
        goto EXIT_POINT;
    }

    /* Get the new parameter data for each data type. */
    {
        size_t paramStructIndex = 0;
        size_t paramInfoIndex   = 0;

        for (paramStructIndex=0;
             paramStructIndex < nTrans;
             paramStructIndex++) 
        {
            mxArray *mat = NULL;
            
            mat = mxGetField(
                mxParameters,
                paramStructIndex,
                "values");

            if (mat == NULL)
            {
                ParamInfo *paramInfo =
                    &paramStructure->paramInfo[paramInfoIndex];
                paramInfo->nEls = 0;
                paramInfo->elSize = 0;
#if defined(MX_HAS_INTERLEAVED_COMPLEX)
                paramInfo->vals = NULL;
#else
                paramInfo->rVals = NULL;
                paramInfo->iVals = NULL;
#endif
                paramInfoIndex++;
                continue;
            } 
      
            if (!mxIsStruct(mat))
            {
                /* Non-struct data type */
                rt_CreateParamInfoForNonStructDataType(
                    mat,
                    mxParameters,
                    paramStructIndex,
                    paramStructure, 
                    paramInfoIndex
                    );
                
                paramInfoIndex++;
            } else {
                /* This is a struct param; grab information from rtp.parameters(i).structParamInfo 
                 * paramInfoIndex is incremented inside rt_CreateParamInfosForStructOrBusType
                 */
                result = rt_PrepareToCreateParamInfosForStructParam(
                    mat, 
                    mxParameters,
                    paramStructIndex,
                    paramStructure,
                    &paramInfoIndex);
            }
        }
    } 

EXIT_POINT:
    mxDestroyArray(mxParameters);
    mxDestroyArray(mxChecksum);

    if (pmat != NULL)
    {
        matClose(pmat); pmat = NULL;
    }

    if (result != NULL)
    {
        rt_FreeParamStructs();
        paramStructure = NULL;
    }
    
    *prmStructOut = paramStructure;
    return(result);
} /* end rt_ReadParamStructMatFile */

/* Function: ReplaceRtP ========================================================
 * Abstract
 *  Initialize the rtP structure using the parameters from the specified
 *  'paramStructure'.  The 'paramStructure' contains parameter info that was
 *  read from a mat file (see raccel_mat.c/rt_ReadParamStructMatFile).
 */
static const char * ReplaceRtP(
    const SimStruct *S,
    const PrmStructData *paramStructure)
{
    const char *errStr = NULL;
    const ParamInfo *paramInfo =
        paramStructure->paramInfo;
    size_t nStructLeaves =
        paramStructure->nStructLeaves;
    size_t nNonStructDataTypes =
        paramStructure->nNonStructDataTypes;
    const DataTypeTransInfo *dtInfo =
        (const DataTypeTransInfo *)ssGetModelMappingInfo(S);
    DataTypeTransitionTable *dtTable =
        dtGetParamDataTypeTrans(dtInfo);
    uint_T *dataTypeSizes =
        dtGetDataTypeSizes(dtInfo);
    rtwCAPI_ModelMappingInfo *mmi =
        rt_modelMapInfoPtr;
    rtwCAPI_DataTypeMap const *dTypeMap =
        rtwCAPI_GetDataTypeMap(mmi);

    {
        size_t loopIdx;
        for (loopIdx=0;
             loopIdx < nStructLeaves+nNonStructDataTypes;
             loopIdx++)
        {
            bool structLeaf =
                paramInfo[loopIdx].structLeaf;
            bool complex =
                (bool)paramInfo[loopIdx].complex;
            bool isString = paramInfo[loopIdx].isString;
            bool isPointer =  paramInfo[loopIdx].isPointer;
            int dtTransIdx =
                paramInfo[loopIdx].dtTransIdx;
            int dataType =
                paramInfo[loopIdx].dataType;
            int dtSize = 0;
            size_t nEls = 0;
            size_t elSize = 0;
            size_t nParams = 0;
            char *address = NULL;

#if !defined(MX_HAS_INTERLEAVED_COMPLEX)
            char *dst = NULL;
#endif
            if (isString)
            {
                const mxArray *mxStrings = paramInfo[loopIdx].strVals;
                nParams = paramInfo[loopIdx].nEls;
                elSize = paramInfo[loopIdx].elSize;
                if (structLeaf) {
                    address = paramInfo[loopIdx].prmAddr;
                } else {
                    address = dtTransGetAddress(
                        dtTable,
                        dtTransIdx);
                }
                for (size_t idx = 0; idx < nParams; ++idx)
                {
                    const mxArray *mxStr = mxGetCell(mxStrings, idx);
                    suInitializeStringFromMx(address + idx * elSize, mxStr);
                }

                continue;
            }

            dtSize = structLeaf ?
                rtwCAPI_GetDataTypeSize(dTypeMap, dataType) :
                (int)dataTypeSizes[dataType];

            /*
             * The datatype-size table (dataTypeSizes) contains only real data types 
             * whereas the c-api includes both real and complex datatypes (cint32_T, for 
             * example). If dtSize was obtained from the c-api, then it must be divided
             * by two for nParams to be correct.
             */
            dtSize = (complex && structLeaf) ?
                (dtSize / 2) :
                dtSize;

#if defined(MX_HAS_INTERLEAVED_COMPLEX)
            if(complex) { dtSize*= 2; } /* Added for complex-interleaved conversion. */
#endif

            nEls = paramInfo[loopIdx].nEls;
            elSize = paramInfo[loopIdx].elSize;
            nParams = (elSize*nEls)/dtSize;

            if (!nEls)
                continue;

            if (!structLeaf)
            {
                address = dtTransGetAddress(
                    dtTable,
                    dtTransIdx);
                /*
                 * Check for consistent element size.  paramInfo->elSize is the size
                 * as stored in the parameter mat-file.  This should match the size
                 * used by the generated code (i.e., stored in the SimStruct).
                 */
                if ((dataType <= 13 && elSize != dtSize) ||
                    (dataType > 13 && (dtSize % elSize != 0)))
                {
                    errStr = "Parameter data type sizes in MAT-file not same "
                        "as data type sizes in RTW generated code";
                    goto EXIT_POINT;
                }
            } else{
                address = paramInfo[loopIdx].prmAddr;
            }

#if defined(MX_HAS_INTERLEAVED_COMPLEX)
            if(isPointer) {
                storeArrayInPointerPool(address, paramInfo[loopIdx].vals, (int32_T) (nEls*elSize));
            } else {

            (void)memcpy(
                address,
                paramInfo[loopIdx].vals,
                nParams*dtSize);
            }
#else
            if (!complex)
            {
                if(isPointer) {
                    storeArrayInPointerPool(address, paramInfo[loopIdx].vals, (int32_T) (nEls*elSize));
                } else {

                (void)memcpy(
                    address,
                    paramInfo[loopIdx].rVals,
                    nParams*dtSize);
                }
            } else {
                FL_DIAG_ASSERT(!isPointer);
                /*
                 * Must interleave the real and imaginary parts.  Simulink style.
                 */
                size_t elIdx;
                const char *realSrc =
                    (const char *)paramInfo[loopIdx].rVals;
                const char *imagSrc =
                    (const char *)paramInfo[loopIdx].iVals;
                dst = address;

                for (elIdx=0;
                     structLeaf ?
                         elIdx<nEls :
                         elIdx<nParams;
                     elIdx++)
                {
                    /* Copy real part. */
                    (void)memcpy(dst,realSrc,dtSize);
                    dst += dtSize;
                    realSrc += dtSize;

                    /* Copy imag part. */
                    (void)memcpy(dst,imagSrc,dtSize);
                    dst += dtSize;
                    imagSrc += dtSize;
                }
            }
#endif
        }        
    }        

  EXIT_POINT:
    return(errStr);
} /* end ReplaceRtP */


/*==================*
 * Visible routines *
 *==================*/

/* Function: rt_RapidReadMatFileAndUpdateParams ========================================
 *
 */
void rt_RapidReadMatFileAndUpdateParams(const SimStruct *S)
{
    const char* result = NULL;
    PrmStructData* paramStructure = NULL;

    if (getParamFilename() == NULL)
        goto EXIT_POINT;

    /* checksum comparison is performed in rt_ReadParamStructMatFile */
    result = rt_ReadParamStructMatFile(
        &paramStructure,
        S,
        gblParamCellIndex);
    
    if (result != NULL)
        goto EXIT_POINT;

    /* Replace the rtP structure */
    result = ReplaceRtP(
        S,
        paramStructure);
    
    if (result != NULL)
        goto EXIT_POINT;

EXIT_POINT:

    if (result) {
        ssSetErrorStatus(S, result);
    }
    
    return;

} /* rt_RapidReadMatFileAndUpdateParams */


void rt_ssGetBlockPath(SimStruct* S, int_T sysIdx, int_T blkIdx, char_T **path) {
    (void)(S);
    if (!gblBlockPathDB)
    {
        gblBlockPathDB = dl_init_objpath(1000);
    }
    *path = dl_get_object_path(gblBlockPathDB, sysIdx, blkIdx);
}
void rt_ssSet_slErrMsg(void* S, void* diag) {
    if (gblDiagnosticLogger)
    {
        dl_report_from_diagnostic(gblDiagnosticLogger, CODEGEN_SUPPORT_DIAGNOSTIC_ERROR, diag);
        rt_RapidReleaseDiagLoggerDB();
    }
    else 
    {
        _ssSet_slErrMsg((SimStruct*)S, diag);
    }
    slsaLogInfo("Long jumping from rt_ssSet_slErrMsg()");
    longjmp(gblExecutionInfo.gblObjects_.rapidAccelJmpBuf, 1);
}
void rt_ssReportDiagnosticAsWarning(void* S, void* diag) {
    if (gblDiagnosticLogger)
    {
        dl_report_from_diagnostic(gblDiagnosticLogger, CODEGEN_SUPPORT_DIAGNOSTIC_WARNING, diag);
    }
    else 
    {
        _ssReportDiagnosticAsWarning((SimStruct*)S, diag);
    }
}

void rt_ssReportDiagnosticAsInfo(void* S, void* diag) {
    if (gblDiagnosticLogger)
    {
        dl_report_from_diagnostic(gblDiagnosticLogger, CODEGEN_SUPPORT_DIAGNOSTIC_INFO, diag);
    }
    else
    {
        _ssReportDiagnosticAsInfo((SimStruct*)S, diag);
    }
}


void rt_RapidInitDiagLoggerDB(const char* dbhome, size_t sid)
{
    dl_set_home(dbhome);
    gblDiagnosticLogger = dl_init(sid);
    gblBlockPathMemMgrHead = NULL;
    gblBlockPathMemMgrTail = NULL;
}


extern void rt_RapidReleaseDiagLoggerDB()
{
    BlockPathMemMgr* memMgrCur = NULL;
    while (gblBlockPathMemMgrHead)
    {
        memMgrCur = gblBlockPathMemMgrHead;
        free(memMgrCur->path);
        gblBlockPathMemMgrHead = memMgrCur->next;
        free(memMgrCur);
    }
    if (gblDiagnosticLogger)
    {
        dl_reset(gblDiagnosticLogger);
        gblDiagnosticLogger = NULL;
    }
}

void rt_AddPathToMemMgr(char* path)
{
    if (gblBlockPathMemMgrTail)
    {
        gblBlockPathMemMgrTail->next = (BlockPathMemMgr*)malloc(sizeof(BlockPathMemMgr));
        gblBlockPathMemMgrTail = gblBlockPathMemMgrTail->next;
        gblBlockPathMemMgrTail->path = path;
        gblBlockPathMemMgrTail->next = NULL;
    }
    else
    {
        gblBlockPathMemMgrTail = (BlockPathMemMgr*)malloc(sizeof(BlockPathMemMgr));
        gblBlockPathMemMgrTail->path = path;
        gblBlockPathMemMgrTail->next = NULL;
        gblBlockPathMemMgrHead = gblBlockPathMemMgrTail;
    }
}

extern const char* rt_CreateFullPathToTop(const char* toppath, const char* subpath)
{
	char* fullpath;
	if(!toppath) {
		return "Block_in_protected_model";
	}
     /* fullpath size = toppath + '|' + subpath + '0' */
    fullpath = (char*)malloc((strlen(toppath) + strlen(subpath) + 2) * sizeof(char));
    rt_AddPathToMemMgr(fullpath);
    strcpy(fullpath, toppath);
    strcat(fullpath, "|");
    strcat(fullpath, subpath);
    return fullpath;
}

/* Function: rt_GetISigstreamManager ============================================
 *
 * Abstract:
 *      This function gets the pointer to the single global ISigstreamManager
 *      instance.
 *      
 *      
 *      
 * Returns:
 *	void * pointer to the ISigstreamManager instance.
 */
void* rt_GetISigstreamManager(SimStruct* S) {
    return ssGetExecutionInfo(S)->runtimeObjects_.iSigstreamManager_;
}

/* Function: rt_RAccelReadInportsMatFile ============================================
 *
 * Abstract:
 *      This function opens a "Inport matfile" and read it to InportTUTable    
 *      
 *      
 *      
 * Returns:
 *	NULL    : success
 *      non-NULL: error message
 */
const char* rt_RAccelReadInportsMatFile(SimStruct* S,
                                        const char* inportFileName,
                                        int* matFileFormat) {
    static char  errmsg[1024];
    const  char* result;
    const int maxErrorBufferSize = 1024;
    int errorXMLMode = 1;

    int_T fileDataFormat = -1; 
    int_T  inportIdx;
    
    mxLogical    *periodicFunctionCallInports = NULL;
    bool          externalInputIsInDatasetFormat = false;
   
    errmsg[0] = '\0'; /* assume success */

    int_T numRootInportBlks = ssGetExecutionInfo(S)->gblObjects_.numRootInportBlks;
    
    /* no root inports return */
    if (numRootInportBlks == 0){
        goto EXIT_POINT;
    } else {
        if (inportFileName == NULL) {
            printf("*** Warning: -i command is not used and there are "
                   "inports block in the model. All inports will be "
                   "set to zero. ***\n ");
            goto EXIT_POINT;
        }
    }
    
    {
        void *pISigstreamManager = rt_GetISigstreamManager(S);
        rtwISigstreamManagerGetDatasetInputFromMatFile(
	    pISigstreamManager, inportFileName, errmsg, maxErrorBufferSize, errorXMLMode);
        rtwISigstreamManagerGetInputIsInDatasetFormat(
            pISigstreamManager, 
            &externalInputIsInDatasetFormat
            );
        if (externalInputIsInDatasetFormat) {            
            goto EXIT_POINT;
        }
    }

    periodicFunctionCallInports = malloc(sizeof(mxLogical)*gblExecutionInfo.gblObjects_.numRootInportBlks);
    if (periodicFunctionCallInports == NULL) {
        (void)sprintf(errmsg,"Memory allocation error"); 
        goto EXIT_POINT;
    }

    *matFileFormat = fileDataFormat; 
    result = rt_VerifyInportsMatFile(inportFileName,
                                     matFileFormat,
                                     true,
                                     periodicFunctionCallInports);
                                         
    if (result != NULL){
        (void)strcpy(errmsg, result);
        goto EXIT_POINT;
    }
    
    /* reach here, data file is valid, steal data to TU table*/
    result  = rt_ConvertInportsMatDatatoTUtable(*matFileFormat,
                                                inportFileName,
                                                true,
                                                periodicFunctionCallInports);
    if (result != NULL){
        (void)strcpy(errmsg, result);
        goto EXIT_POINT;
    }

    // Cache the 'currTimeIdx' for use in save/restore OperatingPoint
    for (inportIdx = 0; inportIdx < numRootInportBlks; ++inportIdx) {
        char currTimeIdxKey[256];
        sprintf(currTimeIdxKey, "gblInportTUtables_currTimeIdx_%d", inportIdx);
        slsaSaveRawMemoryForSimTargetOP(
            S, currTimeIdxKey,
            (void**)(&(gblExecutionInfo.gblObjects_.inportTUtables[inportIdx].currTimeIdx)),
            sizeof(gblExecutionInfo.gblObjects_.inportTUtables[inportIdx].currTimeIdx), NULL, NULL);
    }

    /* Reach here, data is successfully loaded */
    printf(" *** %s is successfully loaded! ***\n", inportFileName);

EXIT_POINT:

    if (errmsg[0] != '\0') {
        gblExecutionInfo.gblObjects_.inportFileName = NULL;
        return errmsg;
    } else {
        return NULL;
    }
} /* end rt_RAccelReadInportsMatFile */

/* Function: rt_NextAperiodicPartitionHitTime =======================================
 *
 * Abstract:
 *      This function returns the next aperiodic partition hit time 
 *      
 *      
 *      
 * Returns:
 *	Next aperiodic partition hit time.
 */
double rt_NextAperiodicPartitionHitTime(SimStruct* S, double time) {
    return rtwISigstreamManagerNextTimeForAperiodicPartitions(
        ssGetExecutionInfo(S)->runtimeObjects_.iSigstreamManager_, time);
}


/* EOF raccel_utils.c */

/* LocalWords:  RSim matrx smaple matfile rb scaler Tx gbl tu Datato TUtable
 * LocalWords:  UTtable FName FFname raccel el fromfile npts npoints nchannels
 * LocalWords:  curr tfinal timestep Gbls Remappings DType rtp CAPI rsim
 */
