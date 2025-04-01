/* Copyright 2019 The MathWorks, Inc.*/

#ifndef _MFL_INTERP_AKIMAHERMITEBASIS_double_H_
#define _MFL_INTERP_AKIMAHERMITEBASIS_double_H_

#include "mfl_interp_util.h" /* MFL_INTERP_UINT */

/**
 * \file
 * Evaluate N-D Akima cubic polynomial using a Hermite basis.
 */

/**
 * Evaluate N-D Akima cubic interpolant at query points using a Hermite basis.
 *
 * <b>NOTE: The Akima coefficients only need to be computed once. You can then reuse them to
 *          evaluate the Akima cubic polynomial at different query points.</b>
 *
 * \param[in]  gridVectors  Vectors of grid coordinates: <tt> x1, x2, ..., xN</tt>.
 *                          In MATLAB notation, the underlying N-D grid is given by:
 *                          <tt>[XX1,...,XXN] = ndgrid(x1,...,xN)</tt>.
 * \param[in]  gridSize     Size of the underlying N-D grid. In MATLAB notation:
 *                          <tt>[gridSize(1), ..., gridSize(N)] = size(ndgrid(x1, ..., xN))</tt>,
 *                          where <tt>x1, ..., xN </tt> are the \p N vectors defining the N-D grid.
 * \param[in]  N            Number of dimensions of underlying N-D grid, i.e., \p N.
 *
 * \param[in]  extrapMethod  Extrapolation: 0 (Akima), 1 (NearestBoundary), 2 (LinearBoundary).
 *
 * \param[in]  workspaceEvaluation  \b Pre-allocated workspace for forming the N-D Akima polynomial.
 * \param[in]  workspaceIndices     \b Pre-allocated workspace for sizes of each finite difference.
 *
 * \param[in]  coefficients  \b Pre-computed Akima cubic polynomial coefficients.
 *
 * \param[in]  noDerivatives 1 for interpolation/extrapolation and 0 for derivative computations.
 *
 * \param[in]  numVq  Number of query points.
 *                    In MATLAB notation: <tt>numVq = numel(xqi) = numel(Vq)</tt>.
 * \param[in]  Xq     Query vectors <tt> xq1, ..., xqN</tt> of length \p numVq, representing
 *                    N-D coordinates for each query point.
 *
 * \param[in]  binsXq Bins (grid intervals) containing the given query points \p Xq.
 *                    For grid [x1, x2, ..., xn], the bins are:
 *                      bin 0:   xq <= x2, OR xq is NaN
 *                      bin 1:   x1 <= xq < x2
 *                      ...
 *                      bin n-2: xn-1 <= xq
 *                    binsXq has the same size as Xq and must be \b pre-allocated.
 *                    Use null binsXq to indicate that the bins have not been pre-computed.
 *
 * \param[out] Vq  If \p noDerivatives = 1, returns interpolation results at query points \p Xq.
 *                 Else, returns derivatives at query points \p Xq.
 *                 Must be \b pre-allocated.
 */
void akimaEvaluationViaHermiteBasis_double
(
    /* INPUTS:  */
    const double** gridVectors,
    const MFL_INTERP_UINT*        gridSize,
    const MFL_INTERP_UINT         N,
    const MFL_INTERP_UINT         extrapMethod,
    double*        workspaceEvaluation,
    MFL_INTERP_UINT*              workspaceIndices,
    double*        coefficients,
    const MFL_INTERP_UINT         noDerivatives,
    const MFL_INTERP_UINT         numVq,
    const double** Xq,
    MFL_INTERP_UINT**             binsXq,
    double*        hermXq,
    /* OUTPUTS: */
    double*        Vq
);

/**
 * Evaluate 1-D Akima cubic interpolant at query points using a Hermite basis.
 *
 * <b>NOTE: This is a faster 1-D alternative to the general N-D code <tt>(N = 1,2,3,...)</tt>.
 *
 * <b>NOTE: The Akima coefficients only need to be computed once. You can then reuse them to
 *          evaluate the Akima cubic polynomial at different query points.</b>
 *
 * \param[in]  x             Vector of 1-D grid coordinates.
 * \param[in]  nx            Number of 1-D grid coordinates.
 * \param[in]  extrapMethod  Extrapolation: 0 (Akima), 1 (NearestBoundary), 2 (LinearBoundary).
 * \param[in]  coefficients  \b Pre-computed Akima cubic polynomial coefficients.
 * \param[in]  noDerivatives 1 for interpolation/extrapolation and 0 for derivative computations.
 * \param[in]  numVq         Number of 1-D query points.
 * \param[in]  xq            1-D query points.
 * \param[in]  bq            Pre-computed bins for the query points. If NULL, compute the bins.
 *
 * \param[out] vq  If \p noDerivatives = 1, returns interpolation results at query points \p xq.
 *                 Else, returns derivatives at query points \p xq.
 *                 Must be \b pre-allocated.
 */
void akimaEvaluationViaHermiteBasis1D_double
(
    /* INPUTS:  */
    const double* x,
    const MFL_INTERP_UINT        nx,
    const MFL_INTERP_UINT        extrapMethod,
    double*       coefficients,
    const MFL_INTERP_UINT        noDerivatives,
    const MFL_INTERP_UINT        numVq,
    const double* xq,
    const MFL_INTERP_UINT*       bq,
    /* OUTPUTS: */
    double*       vq
);

/**
 * Multiply Hermite basis with Akima coefficients to evaluate cubic interpolant at one N-D node
 * in the N-D grid.
 *
 * \param[in]  qq         Current linear index into N-D Akima coefficients.
 * \param[in]  N          Number of dimensions of underlying N-D grid, i.e., \p N.
 * \param[in]  gridNumel  Number of nodes in the grid: <tt>prod(gridSize)</tt>.
 * \param[in]  C          Akima cubic polynomial coefficients.
 * \param[in]  H          First Hermite basis coefficient.
 * \param[in]  dH         Second Hermite basis coefficient.
 * \param[in]  indH       Indices to loop through the Hermite coefficients in the right order.
 * \param[in]  ndcube     Workspace for N-D polynomial evaluation.
 *
 * \return  Interpolation result at one N-D grid node
 */
double akimaHermitePolynomialND_double
(
    const MFL_INTERP_UINT         qq,
    const MFL_INTERP_UINT         N,
    const MFL_INTERP_UINT         gridNumel,
    const double*  C,
    const double*  H,
    const double*  dH,
    const MFL_INTERP_UINT*        indH,
    double*        ndcube
);

/**
 * Form Hermite basis for an N-D query point.
 *
 * \param[in]  kk              Current query point: <tt>xq1(kk), ..., xqN(kk)</tt>.
 *
 * \param[in]  gridVectors     Vectors of grid coordinates: <tt> x1, x2, ..., xN</tt>.
 *                             In MATLAB notation, the underlying N-D grid is given by:
 *                             <tt>[XX1,...,XXN] = ndgrid(x1,...,xN)</tt>.
 * \param[in]  gridSize        Size of the underlying N-D grid. In MATLAB notation:
 *                             <tt>[gridSize(1), ..., gridSize(N)] = size(ndgrid(x1, ..., xN))</tt>,
 *                             where <tt>x1, ..., xN </tt> are the \p N vectors defining the N-D grid.
 * \param[in]  gridSizeCumprod Cumprod of size of the underlying N-D grid. In MATLAB notation:
 *                             <tt>[1 cumprod(gridSize(1:N-1))]</tt>,
 * \param[in]  N               Number of dimensions of underlying N-D grid, i.e., \p N.
 *
 * \param[in]  extrapMethod    Extrapolation: 0 (Akima), 1 (NearestBoundary), 2 (LinearBoundary).
 *
 * \param[in]  noDerivatives   1 for interpolation/extrapolation and 0 for derivative computations.
 *
 * \param[in]  Xq              Query coordinate vectors <tt> xq1, ..., xqN</tt>.
 *
 * \param[in]  binsXq Bins (grid intervals) containing the given query points \p Xq.
 *                    For grid [x1, x2, ..., xn], the bins are:
 *                      bin 0:   xq <= x2, OR xq is NaN
 *                      bin 1:   x1 <= xq < x2
 *                      ...
 *                      bin n-2: xn-1 <= xq
 *                    binsXq has the same size as Xq and must be \b pre-allocated.
 *                    Use null binsXq to indicate that the bins have not been pre-computed.
 *
 * \param[out]  H      First Hermite basis coefficient.
 * \param[out]  dH     Second Hermite basis coefficient.
 * \return             Linear index of query point relative to the N-D grid
 */
MFL_INTERP_UINT akimaHermiteBasisND_double
(
    /* INPUTS:  */
    const MFL_INTERP_UINT         kk,
    const double** gridVectors,
    const MFL_INTERP_UINT*        gridSize,
    const MFL_INTERP_UINT*        gridSizeCumprod,
    const MFL_INTERP_UINT         N,
    const MFL_INTERP_UINT         extrapMethod,
    const MFL_INTERP_UINT         noDerivatives,
    const MFL_INTERP_UINT         numVq,
    const double** Xq,
    MFL_INTERP_UINT**             binsXq,
    double*        hermXq,
    /* OUTPUTS: */
    double*        H,
    double*        dH
);

/**
 * Form Hermite basis for an 1-D query point.
 *
 * \param[in]  x             Vector of 1-D grid coordinates.
 * \param[in]  nx            Number of 1-D grid coordinates.
 * \param[in]  extrapMethod  Extrapolation: 0 (Akima), 1 (NearestBoundary), 2 (LinearBoundary).
 * \param[in]  noDerivatives 1 for interpolation/extrapolation and 0 for derivative computations.
 * \param[in]  xq            1-D query point.
 * \param[in]  b             Bins (intervals) for the query points.
 *
 * \param[out]  h1  First Hermite basis coefficient for the left node.
 * \param[out]  h2  First Hermite basis coefficient for the right node.
 * \param[out]  dh1 Second Hermite basis coefficient for the left node.
 * \param[out]  dh2 Second Hermite basis coefficient for the right node.
 */
void akimaHermiteBasis1D_double
(
    /* INPUTS:  */
    const double* x,
    const MFL_INTERP_UINT        nx,
    const MFL_INTERP_UINT        extrapMethod,
    const MFL_INTERP_UINT        noDerivatives,
    const double  xq,
    const MFL_INTERP_UINT        b,
    /* OUTPUTS: */
    double*       h1,
    double*       h2,
    double*       dh1,
    double*       dh2
);

#endif /* _MFL_INTERP_AKIMAHERMITEBASIS_double_H_ */
