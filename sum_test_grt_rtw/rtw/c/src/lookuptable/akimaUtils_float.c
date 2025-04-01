/* Copyright 2020 The MathWorks, Inc.*/

#include <math.h> /* fabs */

#include "akimaUtils_float.h"

/**
 * \file
 * Interpolation utilities: <tt>abs</tt>, bin search.
 */

/**
 * Abs of a floating-point scalar: |a|
 *
 * \param[in]  a  Scalar double or single.
 * \return  <tt>abs(a)</tt>
 */
float akimaAbs_float(const float a)
{
    /*
     * C89 does not guarantee support for fabsf.
     * Convert float input to double, and then convert double output to float again.
     */
    return (float)fabs((double)a);
}

/**
 * Find the bins (grid intervals) containing a given 1-D query point.
 *
 * \param[in]  x   Vector of 1-D grid coordinates.
 * \param[in]  n   <tt>numel(x)</tt>.
 * \param[in]  xq  Query point.
 *
 * \return  Bin (interval) number of x which contains xq. Bins are 0-based.
 */
MFL_INTERP_UINT akimaFindGridInterval1D_float
(
    const float* x,
    const MFL_INTERP_UINT        n,
    const float  xq
)
{
    /* Linear search on strictly increasing x with minimal number of comparisons */    
    MFL_INTERP_UINT b;
    if (x[n-2] <= xq) {
        b = n-2;
    }
    else {
        b = 1;
        while (x[b] < xq) {
            ++b;
        }
        --b;
    }
    /* NOTE: NaN belongs in bin 0 */
    return b;
}

/**
 * Check that 1-D grid coordinates are strictly increasing and finite.
 *
 * \param[in]  x        Vector of 1-D grid coordinates.
 * \param[in]  nx       Number of 1-D grid coordinates.
 *
 * \return     isValid  1 for valid grid coordinates.
 */
MFL_INTERP_UINT akimaGridVectorsAreValid1D_float
(
    const float* x,
    const MFL_INTERP_UINT        nx
)
{
    MFL_INTERP_UINT isValid, jj;

    isValid = 1;
    /* Grid must have at least two nodes in each direction */
    if (nx < 2) {
        isValid = 0;
    }
    else {
        /* Grid coordinates must be strictly increasing and not NaN */
        for (jj = 1; jj < nx; ++jj) {
            if ( !(x[jj-1] < x[jj]) ) {
                isValid = 0;
                break;
            }
        }
        /* Grid coordinates must be finite */
        for (jj = 0; jj < nx; ++jj) {
            /* The difference must not be NaN */
            if ((x[jj] - x[jj]) != (x[jj] - x[jj])) {
                isValid = 0;
                break;
            }
        }
    }
    return isValid;
}

/**
 * Check that the grid coordinates are strictly increasing and finite.
 *
 * \param[in]  gridVectors  Vectors of grid coordinates: <tt> x1, x2, ..., xN</tt>.
 *                          In MATLAB notation, the underlying N-D grid is given by:
 *                          <tt>[XX1,...,XXN] = ndgrid(x1,...,xN)</tt>.
 * \param[in]  gridSize     Size of the underlying N-D grid. In MATLAB notation:
 *                          <tt>[gridSize(1), ..., gridSize(N)] = size(ndgrid(x1, ..., xN))</tt>,
 *                          where <tt>x1, ..., xN </tt> are the \p N vectors defining the N-D grid.
 * \param[in]  N            Number of dimensions of underlying N-D grid, i.e., \p N.
 *
 * \return     isValid      1 for valid grid coordinates.
 */
MFL_INTERP_UINT akimaGridVectorsAreValid_float
(
    const float** gridVectors,
    const MFL_INTERP_UINT*        gridSize,
    const MFL_INTERP_UINT         N
)
{
    MFL_INTERP_UINT isValid, ii;

    isValid = 1;
    for (ii = 0; ii < N; ++ii) {
        isValid = akimaGridVectorsAreValid1D_float(
                                                gridVectors[ii],
                                                gridSize[ii]);
        if (isValid != 1) {
            break;
        }
    }
    return isValid;
}
