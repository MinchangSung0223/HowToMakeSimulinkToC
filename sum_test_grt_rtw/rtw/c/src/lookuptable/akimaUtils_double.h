/* Copyright 2019 The MathWorks, Inc.*/

#ifndef _MFL_INTERP_AKIMAUTILS_double_H_
#define _MFL_INTERP_AKIMAUTILS_double_H_

#include "mfl_interp_util.h" /* MFL_INTERP_UINT */

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
double akimaAbs_double(const double a);


/**
 * Find the bins (grid intervals) containing a given 1-D query point.
 *
 * \param[in]  x   Vector of 1-D grid coordinates.
 * \param[in]  n   <tt>numel(x)</tt>.
 * \param[in]  xq  Query point.
 *
 * \return  Bin (interval) number of x which contains xq.
 */
MFL_INTERP_UINT akimaFindGridInterval1D_double
(
    const double*  x,
    const MFL_INTERP_UINT         n,
    const double   xq
);

/**
 * Check that 1-D grid coordinates are strictly increasing and finite.
 *
 * \param[in]  x        Vector of 1-D grid coordinates.
 * \param[in]  nx       Number of 1-D grid coordinates.
 *
 * \return     isValid  1 for valid grid coordinates.
 */
MFL_INTERP_UINT akimaGridVectorsAreValid1D_double
(
    const double* x,
    const MFL_INTERP_UINT        nx
);

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
MFL_INTERP_UINT akimaGridVectorsAreValid_double
(
    const double** gridVectors,
    const MFL_INTERP_UINT*        gridSize,
    const MFL_INTERP_UINT         N
);

#endif /* _MFL_INTERP_AKIMAUTILS_double_H_ */
