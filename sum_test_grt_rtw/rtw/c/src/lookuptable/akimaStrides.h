/* Copyright 2019 The MathWorks, Inc.*/

#ifndef _MFL_INTERP_AKIMASTRIDES_H_
#define _MFL_INTERP_AKIMASTRIDES_H_

#include "mfl_interp_util.h" /* MFL_INTERP_UINT */

/**
 * \file
 * Interpolation indexing utilities: prod, cumprod, stride.
 */

/**
 * Compute the product of the first \p N elements in \p A:  <tt>B = prod(A(1:N))</tt> (in MATLAB).
 *
 */
MFL_INTERP_UINT akimaProd(const MFL_INTERP_UINT* A, const MFL_INTERP_UINT N);

/**
 * Compute the following cumprod:  <tt>A = [1 cumprod(A(1:N-1))]</tt> (in MATLAB).
 *
 */
void akimaCumprod(MFL_INTERP_UINT* A, const MFL_INTERP_UINT N);

/**
 * Compute stride information for an N-D array.
 *
 * \param[in]  sizeA  N-D array size: <tt>sizeA = size(A)</tt>
 * \param[in]  dim    Dimension to stride along (0-based).
 *
 * \param[out]  stride       <tt>stride = prod(sizeA(1:(dim-1)))</tt> (in MATLAB).
 * \param[out]  numelPage    Number of elements per page.
 * \param[out]  numelVector  Number of elements per strided vector.
 */
void akimaStrideData
(
    /* INPUTS:  */
    const MFL_INTERP_UINT* sizeA,
    const MFL_INTERP_UINT  dim,
    /* OUTPUTS: */
    MFL_INTERP_UINT*       stride,
    MFL_INTERP_UINT*       numelPage,
    MFL_INTERP_UINT*       numelVector
);

#endif /* _MFL_INTERP_AKIMASTRIDES_H_ */
