/* Copyright 2019 The MathWorks, Inc.*/

#ifndef _MFL_INTERP_MFL_INTERP_UTIL_H_
#define _MFL_INTERP_MFL_INTERP_UTIL_H_



#include "rtwtypes.h"
/**
 * \file
 * Definitions used throughout \b mfl_interp:
 * MFL_INTERP_API.
 */

/**
 * Defines the unsigned integer MFL_INTERP_UINT used by \b mfl_interp for all array indexing
 * and pointer arithmetic. We need size_t because large N-D problems require large indices.
 */
#ifndef MFL_INTERP_UINT
#include <stddef.h>
#define MFL_INTERP_UINT uint32_T
#endif


#endif  /* _MFL_INTERP_MFL_INTERP_UTIL_H_ */
