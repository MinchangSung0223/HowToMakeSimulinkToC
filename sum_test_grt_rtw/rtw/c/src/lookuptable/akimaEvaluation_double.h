/* Copyright 2019 The MathWorks, Inc.*/

#ifndef _MFL_INTERP_AKIMAEVALUATION_double_H_
#define _MFL_INTERP_AKIMAEVALUATION_double_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "mfl_interp_util.h" /* MFL_INTERP_UINT */

/**
 * \file
 * External interfaces for Akima cubic interpolation.
 */

/**
 * Pre-compute Akima cubic polynomial coefficients for fixed grid vectors and grid values,
 * but different query points.
 *
 * \param[in]  N            Number of dimensions of underlying N-D grid, i.e., \p N.
 * \param[in]  gridSize     Size of the underlying N-D grid. In MATLAB notation:
 *                          <tt>[gridSize(1), ..., gridSize(N)] = size(ndgrid(x1, ..., xN))</tt>,
 *                          where <tt>x1, ..., xN </tt> are the \p N vectors defining the N-D grid.
 * \param[in]  gridVectors  Vectors of grid coordinates: <tt> x1, x2, ..., xN</tt>.
 *                          In MATLAB notation, the underlying N-D grid is given by:
 *                          <tt>[XX1,...,XXN] = ndgrid(x1,...,xN)</tt>.
 * \param[in]  gridValues   Values at each grid node. In MATLAB notation,
 *                          <tt> gridValues(i, j, ...) = value at (x1(i), x2(j), ...) </tt>.
 *                          The size of this N-D array equals <tt>size(ndgrid(x1, ..., xN)) </tt>.
 * \param[in]  workspace1   \b Pre-allocated workspace for floating-point quantities.
 * \param[in]  workspace2   \b Pre-allocated workspace for MFL_INTERP_UINT.
 *
 * \param[out]  coefficients  Akima cubic polynomial coefficients.
 */

void akimaFixedGrid_precompute_double
(
    /* INPUTS:  */
    const MFL_INTERP_UINT         N,
    const MFL_INTERP_UINT*        gridSize,
    const double** gridVectors,
    const double*  gridValues,
    double*        work1,
    MFL_INTERP_UINT*              work2,
    /* OUTPUTS: */
    double*        coefficients
);

/**
 * Interpolate using pre-computed Akima cubic polynomial coefficients for fixed grid vectors and
 * grid values, but different query points.
 *
 * \param[in]  N            Number of dimensions of underlying N-D grid, i.e., \p N.
 * \param[in]  gridSize     Size of the underlying N-D grid. In MATLAB notation:
 *                          <tt>[gridSize(1), ..., gridSize(N)] = size(ndgrid(x1, ..., xN))</tt>,
 *                          where <tt>x1, ..., xN </tt> are the \p N vectors defining the N-D grid.
 * \param[in]  gridVectors  Vectors of grid coordinates: <tt> x1, x2, ..., xN</tt>.
 *                          In MATLAB notation, the underlying N-D grid is given by:
 *                          <tt>[XX1,...,XXN] = ndgrid(x1,...,xN)</tt>.
 * \param[in]  extrapMethod Specifies the extrapolation method:
 *                              0 for Akima extrapolation,
 *                              1 for Akima nearest-boundary extrapolation, or
 *                              2 for Akima linear-boundary extrapolation.
 * \param[in]  noDerivatives Switches between computing interpolation values or derivatives:
 *                              0 for computing derivatives, or
 *                              1 for computing values.
 * \param[in]  workspace1   \b Pre-allocated workspace for floating-point quantities.
 * \param[in]  workspace2   \b Pre-allocated workspace for MFL_INTERP_UINT.
 * \param[in]  coefficients \b Pre-computed Akima cubic polynomial coefficients.
 * \param[in]  numQ   Number of query points.
 *                    In MATLAB notation: <tt>numQ = numel(xqi) = numel(Vq)</tt>.
 * \param[in]  Xq     Query points vectors <tt> xq1, ..., xqN</tt> of length \p numQ, representing
 *                    N-D coordinates for each query point.
 * \param[in]  binsXq Bins (grid intervals) containing the given query points \p Xq.
 *                    For grid [x1, x2, ..., xn], the bins are:
 *                      bin 0:   xq <= x2, OR xq is NaN
 *                      bin 1:   x1 <= xq < x2
 *                      ...
 *                      bin n-2: xn-1 <= xq
 *                    binsXq has the same size as Xq and must be \b pre-allocated.
 *                    Use null (0) binsXq to indicate that the bins have not been pre-computed.
 *
 * \param[out] Vq  Interpolation result at given query points \p Xq. Must be \b pre-allocated.
 */

void akimaFixedGrid_interpolate_double
(
    /* INPUTS:  */
    const MFL_INTERP_UINT         N,
    const MFL_INTERP_UINT*        gridSize,
    const double** gridVectors,
    const MFL_INTERP_UINT         extrapMethod,
    const MFL_INTERP_UINT         noDerivatives,
    double*        workspaceEvaluation,
    MFL_INTERP_UINT*              workspaceIndices,
    double*        coefficients,
    const MFL_INTERP_UINT         numQ,
    const double** Xq,
    MFL_INTERP_UINT**             binsXq,
    /* OUTPUTS: */
    double*        Vq
);


/**
 * Pre-compute Akima cubic Hermite basis for fixed grid vectors and query points,
 * but different grid values.
 *
 * \param[in]  N            Number of dimensions of underlying N-D grid, i.e., \p N.
 * \param[in]  gridSize     Size of the underlying N-D grid. In MATLAB notation:
 *                          <tt>[gridSize(1), ..., gridSize(N)] = size(ndgrid(x1, ..., xN))</tt>,
 *                          where <tt>x1, ..., xN </tt> are the \p N vectors defining the N-D grid.
 * \param[in]  gridVectors  Vectors of grid coordinates: <tt> x1, x2, ..., xN</tt>.
 *                          In MATLAB notation, the underlying N-D grid is given by:
 *                          <tt>[XX1,...,XXN] = ndgrid(x1,...,xN)</tt>.
 * \param[in]  extrapMethod Specifies the extrapolation method:
 *                              0 for Akima extrapolation,
 *                              1 for Akima nearest-boundary extrapolation, or
 *                              2 for Akima linear-boundary extrapolation.
 * \param[in]  noDerivatives Switches between computing interpolation values or derivatives:
 *                              0 for computing derivatives, or
 *                              1 for computing values.
 * \param[in]  numQ   Number of query points.
 *                    In MATLAB notation: <tt>numQ = numel(xqi) = numel(Vq)</tt>.
 * \param[in]  Xq     Query points vectors <tt> xq1, ..., xqN</tt> of length \p numQ, representing
 *                    N-D coordinates for each query point.
 * \param[in]  binsXq Bins (grid intervals) containing the given query points \p Xq.
 *                    For grid [x1, x2, ..., xn], the bins are:
 *                      bin 0:   xq <= x2, OR xq is NaN
 *                      bin 1:   x1 <= xq < x2
 *                      ...
 *                      bin n-2: xn-1 <= xq
 *                    binsXq has the same size as Xq and must be \b pre-allocated.
 *                    Use null (0) binsXq to indicate that the bins have not been pre-computed.
 *
 * \param[out]  akimaBasis  Akima basis.
 */

void akimaFixedQuery_precompute_double
(
    /* INPUTS:  */
    const MFL_INTERP_UINT         N,
    const MFL_INTERP_UINT*        gridSize,
    const double** gridVectors,
    const MFL_INTERP_UINT         extrapMethod,
    const MFL_INTERP_UINT         noDerivatives,
    const MFL_INTERP_UINT         numQ,
    const double** Xq,
    MFL_INTERP_UINT**             binsXq,
    /* OUTPUTS: */
    double*        akimaBasis
);

/**
 * Interpolate using pre-computed Akima cubic Hermite basis for fixed grid vectors and query points,
 * but different grid values.
 *
 * \param[in]  N            Number of dimensions of underlying N-D grid, i.e., \p N.
 * \param[in]  gridSize     Size of the underlying N-D grid. In MATLAB notation:
 *                          <tt>[gridSize(1), ..., gridSize(N)] = size(ndgrid(x1, ..., xN))</tt>,
 *                          where <tt>x1, ..., xN </tt> are the \p N vectors defining the N-D grid.
 * \param[in]  gridVectors  Vectors of grid coordinates: <tt> x1, x2, ..., xN</tt>.
 *                          In MATLAB notation, the underlying N-D grid is given by:
 *                          <tt>[XX1,...,XXN] = ndgrid(x1,...,xN)</tt>.
 * \param[in]  gridValues   Values at each grid node. In MATLAB notation,
 *                          <tt> gridValues(i, j, ...) = value at (x1(i), x2(j), ...) </tt>.
 *                          The size of this N-D array equals <tt>size(ndgrid(x1, ..., xN)) </tt>.
 * \param[in]  extrapMethod Specifies the extrapolation method:
 *                              0 for Akima extrapolation,
 *                              1 for Akima nearest-boundary extrapolation, or
 *                              2 for Akima linear-boundary extrapolation.
 * \param[in]  noDerivatives Switches between computing interpolation values or derivatives:
 *                              0 for computing derivatives, or
 *                              1 for computing values.
 * \param[in]  workspace1   \b Pre-allocated workspace for floating-point quantities.
 * \param[in]  workspace2   \b Pre-allocated workspace for MFL_INTERP_UINT.
 * \param[in]  akimaBasis   \b Pre-computed Akima basis.
 * \param[in]  numQ   Number of query points.
 *                    In MATLAB notation: <tt>numQ = numel(xqi) = numel(Vq)</tt>.
 * \param[in]  Xq     Query points vectors <tt> xq1, ..., xqN</tt> of length \p numQ, representing
 *                    N-D coordinates for each query point.
 * \param[in]  binsXq Bins (grid intervals) containing the given query points \p Xq.
 *                    For grid [x1, x2, ..., xn], the bins are:
 *                      bin 0:   xq <= x2, OR xq is NaN
 *                      bin 1:   x1 <= xq < x2
 *                      ...
 *                      bin n-2: xn-1 <= xq
 *                    binsXq has the same size as Xq and must be \b pre-allocated.
 *                    Use null (0) binsXq to indicate that the bins have not been pre-computed.
 *
 * \param[out] Vq  Interpolation result. Must be \b pre-allocated.
 */

void akimaFixedQuery_interpolate_double
(
    /* INPUTS:  */
    const MFL_INTERP_UINT         N,
    const MFL_INTERP_UINT*        gridSize,
    const double** gridVectors,
    const double*  gridValues,
    const MFL_INTERP_UINT         extrapMethod,
    const MFL_INTERP_UINT         noDerivatives,
    double*        work1,
    MFL_INTERP_UINT*              work2,
    double*        akimaBasis,
    const MFL_INTERP_UINT         numQ,
    const double** Xq,
    MFL_INTERP_UINT**             binsXq,
    /* OUTPUTS: */
    double*        Vq
);


 /**
 * Find the bins (grid intervals) which contain the query points.
 *
 * \param[in]  N            Number of dimensions of underlying N-D grid, i.e., \p N.
 * \param[in]  gridSize     Size of the underlying N-D grid. In MATLAB notation:
 *                          <tt>[gridSize(1), ..., gridSize(N)] = size(ndgrid(x1, ..., xN))</tt>,
 *                          where <tt>x1, ..., xN </tt> are the \p N vectors defining the N-D grid.
 * \param[in]  gridVectors  Vectors of grid coordinates: <tt> x1, x2, ..., xN</tt>.
 *                          In MATLAB notation, the underlying N-D grid is given by:
 *                          <tt>[XX1,...,XXN] = ndgrid(x1,...,xN)</tt>.
 * \param[in]  numQ   Number of query points.
 *                    In MATLAB notation: <tt>numQ = numel(xqi) = numel(Vq)</tt>.
 * \param[in]  Xq     Query points vectors <tt> xq1, ..., xqN</tt> of length \p numQ, representing
 *                    N-D coordinates for each query point.
 *
 * \param[out] binsXq Bins (grid intervals) containing the given query points \p Xq.
 *                    For grid [x1, x2, ..., xn], the bins are:
 *                      bin 0:   xq <= x2, OR xq is NaN
 *                      bin 1:   x1 <= xq < x2
 *                      ...
 *                      bin n-2: xn-1 <= xq
 *                    binsXq has the same size as Xq and must be \b pre-allocated.
 */

void akimaQueryBins_double
(
    /* INPUTS:  */
    const MFL_INTERP_UINT         N,
    const MFL_INTERP_UINT*        gridSize,
    const double** gridVectors,
    const MFL_INTERP_UINT         numQ,
    const double** Xq,
    /* OUTPUTS: */
    MFL_INTERP_UINT**             binsXq
);


/*------------------------------------------------------------------------------------------------*/
/* 1-D optimizations:                                                                             */
/*------------------------------------------------------------------------------------------------*/

/**
 * Optimized 1-D function.
 *
 * <b>NOTE: This is a faster 1-D alternative to the general N-D code <tt>(N = 1,2,3,...)</tt>.
 *
 * Pre-compute Akima cubic polynomial coefficients for fixed grid vectors
 * and grid values, but different query points.
 *
 * \param[in]  nx            Number of 1-D grid coordinates.
 * \param[in]  x             Vector of 1-D grid coordinates.
 * \param[in]  v             Vector of 1-D grid values.
 * \param[in]  workspace     \b Pre-allocated workspace for floating-point quantities.
 *
 * \param[out]  coefficients  Akima cubic polynomial coefficients.
 */

void akimaFixedGrid_precompute_1D_double
(
    /* INPUTS:  */
    const MFL_INTERP_UINT        nx,
    const double* x,
    const double* v,
    double*       workspace,
    /* OUTPUTS: */
    double*       coefficients
);

/**
 * Optimized 1-D function.
 *
 * <b>NOTE: This is a faster 1-D alternative to the general N-D code <tt>(N = 1,2,3,...)</tt>.
 *
 * Interpolate using pre-computed Akima cubic polynomial coefficients for fixed grid vectors and
 * grid values, but different query points.
 *
 * \param[in]  nx           Number of 1-D grid coordinates.
 * \param[in]  x            Vector of 1-D grid coordinates.
 * \param[in]  extrapMethod Specifies the extrapolation method:
 *                              0 for Akima extrapolation,
 *                              1 for Akima nearest-boundary extrapolation, or
 *                              2 for Akima linear-boundary extrapolation.
 * \param[in]  noDerivatives Switches between computing interpolation values or derivatives:
 *                              0 for computing derivatives, or
 *                              1 for computing values.
 * \param[in]  coefficients \b Pre-computed Akima cubic polynomial coefficients.
 * \param[in]  numQ   Number of query points.
 *                    In MATLAB notation: <tt>numQ = numel(xqi) = numel(Vq)</tt>.
 * \param[in]  xq     Query points vector representing coordinates for each query point.
 * \param[in]  binsXq Bins (grid intervals) containing the given query points \p Xq.
 *                    For grid [x1, x2, ..., xn], the bins are:
 *                      bin 0:   xq <= x2, OR xq is NaN
 *                      bin 1:   x1 <= xq < x2
 *                      ...
 *                      bin n-2: xn-1 <= xq
 *                    binsXq has the same size as Xq and must be \b pre-allocated.
 *                    Use null (0) binsXq to indicate that the bins have not been pre-computed.
 *
 * \param[out] vq  Interpolation result at given query points \p xq. Must be \b pre-allocated.
 */

void akimaFixedGrid_interpolate_1D_double
(
    /* INPUTS:  */
    const MFL_INTERP_UINT        nx,
    const double* x,
    const MFL_INTERP_UINT        extrapMethod,
    const MFL_INTERP_UINT        noDerivatives,
    double*       coefficients,
    const MFL_INTERP_UINT        numQ,
    const double* xq,
    const MFL_INTERP_UINT*       binsXq,
    /* OUTPUTS: */
    double*       vq
);


/**
 * Optimized 1-D function.
 *
 * <b>NOTE: This is a faster 1-D alternative to the general N-D code <tt>(N = 1,2,3,...)</tt>.
 *
 * Pre-compute Akima cubic Hermite basis for fixed grid vectors and query points,
 * but different grid values.
 *
 * \param[in]  nx           Number of 1-D grid coordinates.
 * \param[in]  x            Vector of 1-D grid coordinates.
 * \param[in]  extrapMethod Specifies the extrapolation method:
 *                              0 for Akima extrapolation,
 *                              1 for Akima nearest-boundary extrapolation, or
 *                              2 for Akima linear-boundary extrapolation.
 * \param[in]  noDerivatives Switches between computing interpolation values or derivatives:
 *                              0 for computing derivatives, or
 *                              1 for computing values.
 * \param[in]  numQ   Number of query points.
 *                    In MATLAB notation: <tt>numQ = numel(xqi) = numel(Vq)</tt>.
 * \param[in]  xq     Query points vector representing coordinates for each query point.
 * \param[in]  binsXq Bins (grid intervals) containing the given query points \p Xq.
 *                    For grid [x1, x2, ..., xn], the bins are:
 *                      bin 0:   xq <= x2, OR xq is NaN
 *                      bin 1:   x1 <= xq < x2
 *                      ...
 *                      bin n-2: xn-1 <= xq
 *                    binsXq has the same size as Xq and must be \b pre-allocated.
 *                    Use null (0) binsXq to indicate that the bins have not been pre-computed.
 *
 * \param[out]  akimaBasis  Akima basis.
 */

void akimaFixedQuery_precompute_1D_double
(
    /* INPUTS:  */
    const MFL_INTERP_UINT        nx,
    const double* x,
    const MFL_INTERP_UINT        extrapMethod,
    const MFL_INTERP_UINT        noDerivatives,
    const MFL_INTERP_UINT        numQ,
    const double* xq,
    const MFL_INTERP_UINT*       binsXq,
    /* OUTPUTS: */
    double*       akimaBasis
);

/**
 * Optimized 1-D function.
 *
 * <b>NOTE: This is a faster 1-D alternative to the general N-D code <tt>(N = 1,2,3,...)</tt>.
 *
 * Interpolate using pre-computed Akima cubic Hermite basis for fixed grid vectors and query points,
 * but different grid values.
 *
 * \param[in]  nx           Number of 1-D grid coordinates.
 * \param[in]  x            Vector of 1-D grid coordinates.
 * \param[in]  v            Vector of 1-D grid values.
 * \param[in]  extrapMethod Specifies the extrapolation method:
 *                              0 for Akima extrapolation,
 *                              1 for Akima nearest-boundary extrapolation, or
 *                              2 for Akima linear-boundary extrapolation.
 * \param[in]  noDerivatives Switches between computing interpolation values or derivatives:
 *                              0 for computing derivatives, or
 *                              1 for computing values.
 * \param[in]  workspace   \b Pre-allocated workspace for floating-point quantities.
 * \param[in]  akimaBasis  \b Pre-computed Akima basis.
 * \param[in]  numQ   Number of query points.
 *                    In MATLAB notation: <tt>numQ = numel(xqi) = numel(Vq)</tt>.
 * \param[in]  xq     Query points vector representing coordinates for each query point.
 * \param[in]  binsXq Bins (grid intervals) containing the given query points \p Xq.
 *                    For grid [x1, x2, ..., xn], the bins are:
 *                      bin 0:   xq <= x2, OR xq is NaN
 *                      bin 1:   x1 <= xq < x2
 *                      ...
 *                      bin n-2: xn-1 <= xq
 *                    binsXq has the same size as Xq and must be \b pre-allocated.
 *                    Use null (0) binsXq to indicate that the bins have not been pre-computed.
 *
 * \param[out] vq  Interpolation result. Must be \b pre-allocated.
 */

void akimaFixedQuery_interpolate_1D_double
(
    /* INPUTS:  */
    const MFL_INTERP_UINT        nx,
    const double* x,
    const double* v,
    const MFL_INTERP_UINT        extrapMethod,
    const MFL_INTERP_UINT        noDerivatives,
    double*       workspace,
    double*       akimaBasis,
    const MFL_INTERP_UINT        numQ,
    const double* xq,
    const MFL_INTERP_UINT*       binsXq,
    /* OUTPUTS: */
    double*       vq
);

/**
 * Optimized 1-D function.
 *
 * Find the bins (grid intervals) which contain the query points.
 *
 * \param[in]  nx     Number of 1-D grid coordinates.
 * \param[in]  x      Vector of 1-D grid coordinates.
 * \param[in]  numQ   Number of query points.
 *                    In MATLAB notation: <tt>numQ = numel(xqi) = numel(Vq)</tt>.
 * \param[in]  xq     Query points vector representing coordinates for each query point.
 *
 * \param[out] binsXq Bins (grid intervals) containing the given query points \p Xq.
 *                    For grid [x1, x2, ..., xn], the bins are:
 *                      bin 0:   xq <= x2, OR xq is NaN
 *                      bin 1:   x1 <= xq < x2
 *                      ...
 *                      bin n-2: xn-1 <= xq
 *                    binsXq has the same size as Xq and must be \b pre-allocated.
 */

void akimaQueryBins_1D_double
(
    /* INPUTS:  */
    const MFL_INTERP_UINT         nx,
    const double*  x,
    const MFL_INTERP_UINT         numQ,
    const double*  xq,
    /* OUTPUTS: */
    MFL_INTERP_UINT*              binsXq
);

#ifdef __cplusplus
}
#endif

#endif /* _MFL_INTERP_AKIMAEVALUATION_double_H_ */
