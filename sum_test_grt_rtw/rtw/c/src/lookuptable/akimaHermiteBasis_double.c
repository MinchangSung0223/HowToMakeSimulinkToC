/* Copyright 2019 The MathWorks, Inc.*/

#include <string.h> /* memcpy, memset */

#include "akimaHermiteBasis_double.h"
#include "akimaStrides.h"
#include "akimaUtils_double.h"

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
)
{
    MFL_INTERP_UINT pow2toN, gridNumel, ii, ii2, jj, hh, kk, qq, thisStride, lastStride;
    MFL_INTERP_UINT* gridSizeCumprod;
    MFL_INTERP_UINT* strides;
    MFL_INTERP_UINT* indH;
    double* ndcube;
    double* H;
    double* dH;

    pow2toN = ((MFL_INTERP_UINT)1) << N;
    gridNumel = akimaProd(gridSize,N);

    /*
     * Indices workspace usage:
     *  workspaceIndices[0,N)                         - for A = [1 cumprod(A(1:N-1))] (in MATLAB)
     *  workspaceIndices[N,N + 2^N)                   - 2^N-vector for strides
     *  workspaceIndices[N+2^N,N+2^N + N*2^N)         - Nx2^N matrix of indices
     */
    gridSizeCumprod = workspaceIndices;
    memcpy(gridSizeCumprod,gridSize,N*sizeof(MFL_INTERP_UINT));
    akimaCumprod(gridSizeCumprod,N);
    strides = gridSizeCumprod + N; /* don't have to be 0-initialized */
    strides[0] = 0; /* except for the first stride */
    indH = strides + pow2toN;
    memset(indH,0,N*pow2toN*sizeof(MFL_INTERP_UINT)); /* must be 0-initialized */

    /*
     * Evaluation workspace usage:
     *  workspaceEvaluation[0,2^N)             - for contiguous N-D cube
     *  workspaceEvaluation[2^N,2*N+2^N)       - for H coefficients
     *  workspaceEvaluation[2*N+2^N,4*N+2^N)   - for dH coefficients
     */
    ndcube = workspaceEvaluation; /* don't have to be 0-initialized */
    H = ndcube + pow2toN; /* don't have to be 0-initialized */
    dH = H + 2*N; /* don't have to be 0-initialized */

    /*
     * indH alternates between the 2 entries of each Hermite coefficient
     * H using this 2^N x N pattern, for example for H1, H2, H3, H4:
     *   0 0 0 0
     *   1 0 0 0
     *
     *   0 1 0 0
     *   1 1 0 0
     *
     *   0 0 1 0
     *   1 0 1 0
     *   0 1 1 0
     *   1 1 1 0
     *   ...
     */
    for (ii = 0; ii < N; ++ii) {
        ii2 = ((MFL_INTERP_UINT)1) << ii;
        for (jj = ii2; jj < 2*ii2; ++jj) { /* skip jj = 0 */
            for (hh = 0; hh < N; ++hh) {
                indH[ jj*N + hh ] = (hh == ii) ? 1 : indH[ (jj - ii2)*N + hh ];
            }
        }
    }

    /*
     * Evaluate N-D Akima polynomial interpolant.
     */
    for (kk = 0; kk < numVq; ++kk) {

        /*
         * Form Hermite basis for each N-D query point.
         */
        qq = akimaHermiteBasisND_double(
                                    kk,
                                    gridVectors,
                                    gridSize,
                                    gridSizeCumprod,
                                    N,
                                    extrapMethod,
                                    noDerivatives,
                                    numVq,
                                    Xq,
                                    binsXq,
                                    hermXq,
                                    H,
                                    dH);

        /*
         * Sum interpolation result obtained at each corner of the N-D cube
         * containing the query point Xq, i.e., sum interpolation results at:
         *   qq
         *   qq+1
         *
         *   qq  +n1
         *   qq+1+n1
         *
         *   qq     +n1*n2
         *   qq+1   +n1*n2
         *   qq  +n1+n1*n2
         *   qq+1+n1+n1*n2
         *   ...
         */

        /* Evaluate at a single N-D grid point: qq+strides[0] */
        Vq[kk] = akimaHermitePolynomialND_double(
                                                qq+strides[0],
                                                N,
                                                gridNumel,
                                                coefficients,
                                                H,
                                                dH,
                                                indH,
                                                ndcube);

        for (ii = 0; ii < N; ++ii) {

            thisStride = gridSizeCumprod[ii];

            ii2 = ((MFL_INTERP_UINT)1) << ii;
            for (jj = ii2; jj < 2*ii2; ++jj) { /* skip jj = 0 */

                lastStride = strides[jj-ii2];
                strides[jj] = lastStride + thisStride;

                /* Evaluate at a single N-D grid point: qq+strides[jj] */
                Vq[kk] += akimaHermitePolynomialND_double(
                                                        qq+strides[jj],
                                                        N,
                                                        gridNumel,
                                                        coefficients,
                                                        H,
                                                        dH,
                                                        indH + jj*N,
                                                        ndcube);
            }
        }
    }
}

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
)
{
    MFL_INTERP_UINT kk, b;
    double h1, h2, dh1, dh2;

    /*
     * Evaluate 1-D Akima polynomial interpolant.
     */
    for (kk = 0; kk < numVq; ++kk) {

        /* Compute the bins if needed */
        b = bq ? bq[kk] : akimaFindGridInterval1D_double(x,nx,xq[kk]);

        /*
         * Form Hermite basis for each 1-D query point.
         */
        akimaHermiteBasis1D_double(
                                    x,
                                    nx,
                                    extrapMethod,
                                    noDerivatives,
                                    xq[kk],
                                    b,
                                    &h1,
                                    &h2,
                                    &dh1,
                                    &dh2);

        /*
         * Evaluate cubic polynomial using the Hermite basis.
         */
        vq[kk] = (coefficients[b]   * h1 + coefficients[nx + b]     * dh1) +
                 (coefficients[b+1] * h2 + coefficients[nx + b + 1] * dh2);
    }
}

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
)
{
    MFL_INTERP_UINT ii, jj, ii2, jjh;
    /*
     * Place Akima coefficients c0, c1, c2, c12, c3, c13, c23, c123 ... on an N-D cube:
     *
     *   c0     c2  |     c3      c23  |    ...
     *   c1    c12  |    c13     c123  |    ...
     *
     * to compute the Akima interpolantion formula given by:
     *
     *   (( c0*H0 +   c1*dH0) *  H1 +
     *    ( c2*H0 +  c12*dH0) * dH1 + ) *  H2 +
     *   (( c3*H0 +  c13*dH0) *  H1 +
     *    (c23*H0 + c123*dH0) * dH1 + ) * dH2
     *   ...
     */

    /*
     * Copy Akima coefficients to a contiguous N-D cube:
     */
    ndcube[0] = C[qq];
    for (ii = 0; ii < N; ++ii) {
        ii2 = ((MFL_INTERP_UINT)1) << ii;
        for (jj = ii2; jj < 2*ii2; ++jj) { /* skip jj = 0 */
            ndcube[jj] = C[jj*gridNumel + qq];
        }
    }

    /*
     * Finally, compute on the contiguous N-D cube:
     */
    for (ii = N; ii > 0; --ii) {
        /*
         * Evaluate polynomial in this dimension and then reduce
         * the cube's dimension by 1 and move on to the next higher dimension:
         */

        jjh = 2*(N-ii) + indH[N-ii];
        ii2 = ((MFL_INTERP_UINT)1) << (ii-1);
        for (jj = 0; jj < ii2; ++jj) {

            ndcube[jj] = ndcube[2*jj] * H[jjh] + ndcube[2*jj+1] * dH[jjh];

        }
    }

    /*
     * Akima formula was accumulated in the first cube entry:
     */
    return ndcube[0];
}

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
)
{
    MFL_INTERP_UINT jj, b, qq, jj2, kk2;

    qq = 0;
    kk2 = 2*kk;
    for (jj = 0; jj < N; ++jj) {

        b = binsXq ? binsXq[jj][kk]
                   : akimaFindGridInterval1D_double(gridVectors[jj],
                                                                   gridSize[jj],
                                                                   Xq[jj][kk]);

        if (hermXq != 0) {
            /* Just copy over the pre-computed Hermite basis coefficients: */
            jj2 = jj*2*numVq;
            H[jj*2] = hermXq[jj2+kk2];
            H[jj*2+1] = hermXq[jj2+kk2+1];
            dH[jj*2] = hermXq[N*2*numVq+jj2+kk2];
            dH[jj*2+1] = hermXq[N*2*numVq+jj2+kk2+1];
        }
        else {
            /* Compute the first and second Hermite basis coefficients: */
            jj2 = 2*jj;
            akimaHermiteBasis1D_double(
                                        gridVectors[jj],
                                        gridSize[jj],
                                        extrapMethod,
                                        noDerivatives,
                                        Xq[jj][kk],
                                        b,
                                        H+jj2,
                                        H+jj2+1,
                                        dH+jj2,
                                        dH+jj2+1);
        }

        /* sub2ind for the bins to get linear index of Xq relative to the grid: */
        qq += b*gridSizeCumprod[jj];
    }
    return qq;
}

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
)
{
    double dx, s, s2, s3;

    if (nx < 3 && extrapMethod != 1) {
        /*
         * Simply use the linear formula if we have only 2 grid points
         * and we are not doing nearestBoundary extrapolation
         *   y = yb * ((x-xa)/(xb-xa)) - ya * ((x-xb)/(xb-xa))
         *     = yb * s + ya * (1-s), where s = (x-xa)/(xb-xa)
         */
        dx = x[b+1] - x[b];
        s  = (xq - x[b]) / dx;
        *h1  = 1-s;
        *h2  = s;
        *dh1 = 0;
        *dh2 = 0;
    }
    else {
        if (extrapMethod > 0 && xq < x[0]) {

            /*
             * Akima-linear-boundary or Akima-nearest-boundary extrapolation in [-Inf,x[0])
             */
            if (noDerivatives) {
                *h1  = 1;
                *h2  = 0;
                *dh1 = (xq - x[0])*(extrapMethod-1);
                *dh2 = 0;
            }
            else {
                *h1  = 0;
                *h2  = 0;
                *dh1 = (double)(extrapMethod-1);
                *dh2 = 0;
            }
        }
        else if (extrapMethod > 0 && xq > x[nx-1]) {

            /*
             * Akima-linear-boundary or Akima-nearest-boundary extrapolation in (x[n-1],Inf]
             */
            if (noDerivatives) {
                *h1  = 0;
                *h2  = 1;
                *dh1 = 0;
                *dh2 = (xq - x[nx-1])*(extrapMethod-1);
            }
            else {
                *h1  = 0;
                *h2  = 0;
                *dh1 = 0;
                *dh2 = (double)(extrapMethod-1);
            }
        }
        else {

            /*
             * Akima cubic interpolation and Akima cubic extrapolation
             */

            /* NOTE: NaN xq flow through here and generate NaN polynomial coefficients. */

            dx = x[b+1] - x[b];
            s  = (xq - x[b]) / dx;

            if (noDerivatives) {
                s2 = s*s;
                s3 = s2*s;

                *h2  = -(2*s3 - 3*s2);
                *h1  = -h2[0] + 1;
                *dh1 = (s3 - 2*s2 + s) * dx;
                *dh2 = (s3 - s2) * dx;
            }
            else {
                /* Evaluate derivatives of the cubic Hermite interpolating polynomial bases. */
                s2 = s*s;
                s3 = 6*(s2-s) / dx;

                *h1  = s3;
                *h2  = -s3;
                *dh1 = 3*s2 - 4*s + 1;
                *dh2 = 3*s2 - 2*s;
            }
        }
    }
}
