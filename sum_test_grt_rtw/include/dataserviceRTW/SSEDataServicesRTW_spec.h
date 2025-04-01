/* Copyright 2008-2022 The MathWorks, Inc. */
#ifndef SSEDATASERVICES_RTW_SPEC_HPP
#define SSEDATASERVICES_RTW_SPEC_HPP

#ifdef SUPPORTS_PRAGMA_ONCE
#pragma once
#endif

#ifdef BUILDING_DATASERVICES_RTW

/* This header is being included by files inside this module */
#include "package.h"
#define SSE_DATASERVICES_RTW_EXPORT_CLASS DLL_EXPORT_SYM
#define SSE_DATASERVICES_RTW_EXPORT_EXTERN_C extern "C" DLL_EXPORT_SYM

#elif defined(DLL_IMPORT_SYM)

/* This file is being include by other TMW internal modules */
#ifdef __cplusplus
#define SSE_DATASERVICES_RTW_EXPORT_CLASS DLL_IMPORT_SYM
#define SSE_DATASERVICES_RTW_EXPORT_EXTERN_C extern "C" DLL_IMPORT_SYM
#else
#define SSE_DATASERVICES_RTW_EXPORT_EXTERN_C extern DLL_IMPORT_SYM
#endif

#else

/* This file is being include by other TMW internal modules */
#if defined(__cplusplus)
#define SSE_DATASERVICES_RTW_EXPORT_EXTERN_C extern "C"
#else
#define SSE_DATASERVICES_RTW_EXPORT_EXTERN_C extern
#endif

#endif

#endif // SSE_DATASERVICES_RTW_SPEC_HPP
