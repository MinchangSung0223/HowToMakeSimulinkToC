/* Copyright 2019 The MathWorks, Inc.*/

#include "akimaStrides.h"

/**
 * \file
 * Interpolation indexing utilities: prod, cumprod, stride.
 */

/**
 * Compute the product of the first \p N elements in \p A:  <tt>B = prod(A(1:N))</tt> (in MATLAB).
 *
 */
MFL_INTERP_UINT akimaProd(const MFL_INTERP_UINT* A, const MFL_INTERP_UINT N)
{
    MFL_INTERP_UINT ii = 0;
    MFL_INTERP_UINT B = 1; /* Return 1 for N = 0: */
    for (ii = 0; ii < N; ++ii) {
        B *= A[ii];
    }
    return B;
}

/**
 * Compute the following cumprod:  <tt>A = [1 cumprod(A(1:N-1))]</tt> (in MATLAB).
 *
 */
void akimaCumprod(MFL_INTERP_UINT* A, const MFL_INTERP_UINT N)
{
    MFL_INTERP_UINT p, t, ii;
    p = 1;
    for (ii = 0; ii < N; ++ii) {
        t = A[ii];
        A[ii] = p;
        p *= t;
    }
}

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
)
{
    *stride = akimaProd(sizeA,dim); /* Set to 1 for dim == 0 */
    *numelVector = sizeA[dim];
    *numelPage = (*stride) * (*numelVector);
}

