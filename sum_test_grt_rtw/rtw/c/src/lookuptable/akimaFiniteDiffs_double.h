/* Copyright 2019 The MathWorks, Inc.*/

#ifndef _MFL_INTERP_double_H_
#define _MFL_INTERP_double_H_

#include "mfl_interp_util.h" /* MFL_INTERP_UINT */

/**
 * \file
 * Akima finite differences and 1-D weights.
 */

/**
 * N-D Akima finite differences and 1-D weights.
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
 * \param[in]  numelFiniteDiffs  Array numel for N-D Akima finite differences.
 * \param[in]  finiteDiffsSize  \b Pre-allocated workspace for sizes of each finite difference.
 *
 * \param[out]  finiteDiffs  Akima finite differences.
 * \param[out]  weights1D    Akima 1-D weights.
 */
void akimaFiniteDiffs_double
(
    /* INPUTS:  */
    const double** gridVectors,
    const double*  gridValues,
    const MFL_INTERP_UINT*        gridSize,
    const MFL_INTERP_UINT         N,
    const MFL_INTERP_UINT         numelFiniteDiffs,    
    MFL_INTERP_UINT*              finiteDiffsSize,
    /* OUTPUTS: */
    double*        finiteDiffs,
    double*        weights1D
);

/**
 * 1-D Akima finite differences along a specified dimension of an N-D array.
 *
 * \param[in]  x       Grid coordinates x for a specified dimension \p dim.
 * \param[in]  F       N-D array of values at \p x.
 * \param[in]  sizeF   <tt>[sizeF(1), ..., sizeF(N)] = size(F)</tt>.
 * \param[in]  numelF  <tt>numel(F)</tt>.
 * \param[in]  dim     \p dim to work along (0-based).
 * \param[in]  sizeFD  <tt>size(FD)</tt>.
 *
 * \param[out]  FD     1-D Akima finite differences along \p dim.
 */
void akimaFiniteDiffs1D_double
(
    /* INPUTS:  */
    const double* x,
    const double* F,
    const MFL_INTERP_UINT*       sizeF,
    const MFL_INTERP_UINT        numelF,
    const MFL_INTERP_UINT        dim,
    const MFL_INTERP_UINT*       sizeFD,
    /* OUTPUTS: */
    double*       FD
);

#endif /* _MFL_INTERP_AKIMAFINITEDIFFS_double_H_ */
