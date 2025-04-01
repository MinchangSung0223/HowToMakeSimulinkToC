/* Copyright 2019 The MathWorks, Inc.*/

#ifndef _MFL_INTERP_AKIMACOEFFICIENTS_float_H_
#define _MFL_INTERP_AKIMACOEFFICIENTS_float_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "mfl_interp_util.h" /* MFL_INTERP_UINT */

/**
 * \file
 * Compute N-D Akima cubic polynomial coefficients.
 */

/**
 * Compute N-D Akima cubic polynomial coefficients.
 *
 * <b>NOTE: The Akima coefficients only need to be computed once. You can then reuse them to
 *          evaluate the Akima cubic polynomial at different query points.</b>
 *
 * \param[in]  gridVectors  Vectors of grid coordinates: <tt> x1, x2, ..., xN</tt>.
 *                          In MATLAB notation, the underlying N-D grid is given by:
 *                          <tt>[XX1,...,XXN] = ndgrid(x1,...,xN)</tt>.
 * \param[in]  gridValues   Values at each grid node. In MATLAB notation,
 *                          <tt> gridValues(i, j, ...) = value at (x1(i), x2(j), ...) </tt>.
 *                          The size of this N-D array equals <tt>size(ndgrid(x1, ..., xN)) </tt>.
 * \param[in]  gridSize     Size of the underlying N-D grid. In MATLAB notation:
 *                          <tt>[gridSize(1), ..., gridSize(N)] = size(ndgrid(x1, ..., xN))</tt>,
 *                          where <tt>x1, ..., xN </tt> are the \p N vectors defining the N-D grid.
 * \param[in]  N            Number of dimensions of underlying N-D grid, i.e., \p N.
 *
 * \param[in]  workspaceCoefficients  \b Pre-allocated workspace for finite diffs and derivatives.
 * \param[in]  workspaceIndices       \b Pre-allocated workspace for indices and sizes.
 *
 * \param[out] coefficients  Akima cubic polynomial coefficients.
 */
void akimaCoefficients_float
(
    /* INPUTS:  */
    const float** gridVectors,
    const float*  gridValues,
    const MFL_INTERP_UINT*        gridSize,
    const MFL_INTERP_UINT         N,
    float*        workspaceCoefficients,
    MFL_INTERP_UINT*              workspaceIndices,
    /* OUTPUTS: */
    float* coefficients
);

/**
 * Compute 1-D Akima cubic polynomial coefficients.
 *
 * <b>NOTE: This is a faster 1-D alternative to the general N-D code <tt>(N = 1,2,3,...)</tt> in
 *          <tt>akimaCoefficients_float</tt>.</b>
 *
 * <b>NOTE: The Akima coefficients only need to be computed once. You can then reuse them to
 *          evaluate the Akima cubic polynomial at different query points.</b>
 *
 * \param[in]  x             Vector of 1-D grid coordinates.
 * \param[in]  v             Vector of 1-D grid values. <tt>v(i)</tt> is the data value at grid
 *                           coordinate <tt>x(i)</tt>.
 * \param[in]  nx            Number of 1-D grid coordinates.
 *
 * \param[in]  workspace  \b Pre-allocated workspace for finite differences.
 *
 * \param[out] coefficients  1-D Akima cubic polynomial coefficients.
 */
void akimaCoefficients_1D_float
(
    /* INPUTS:  */
    const float* x,
    const float* v,
    const MFL_INTERP_UINT        nx,
    float*       workspace,
    /* OUTPUTS: */
    float*       coefficients
);

#ifdef __cplusplus
}
#endif

#endif /* _MFL_INTERP_AKIMACOEFFICIENTS_float_H_ */
