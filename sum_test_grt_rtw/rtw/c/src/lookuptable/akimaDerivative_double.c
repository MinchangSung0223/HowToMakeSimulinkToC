/* Copyright 2019 The MathWorks, Inc.*/

#include "akimaDerivative_double.h"
#include "akimaStrides.h"
#include "akimaUtils_double.h"

/**
 * \file
 * Modified Akima formula for estimating derivatives.
 */

/**
 * Modified Akima formula for estimating derivatives.
 *
 * \param[in]  FD      Multiple N-D arrays of finite differences.
 * \param[in]  headFD  Start of each N-D array of finite differences.
 * \param[in]  W       Multiple N-D arrays of 1-D weights.
 * \param[in]  headW   Start of each N-D array of 1-D weights.
 * \param[in]  ppFD    Current location in \p FD array.
 * \param[in]  ppW     Current location in \p W array.
 * \param[in]  gridSizeCumprod         <tt>[1 cumprod(gridSize(1:N-1))]</tt> (in MATLAB)
 * \param[in]  finiteDiffsSizeCumprod  <tt>[1 cumprod(sizeFD(1:N-1))]</tt> (in MATLAB)
 * \param[in]  indX       If \p FD corresponds to <tt>x1x3x4</tt>, indX holds <tt>[0 2 3]</tt>.
 * \param[in]  indXNumel  <tt>numel(indX)</tt>
 * \param[in]  ndcube   Workspace for derivative computation.
 * \param[in]  localW   Workspace for derivative computation.
 * \param[in]  strides  Workspace for temporary indices.
 *
 * \param[out] derivativeEst  Modified Akima derivative estimate.
 */
void akimaDerivative_double
(
    /* INPUTS:  */
    const double*  FD,
    const MFL_INTERP_UINT         headFD,
    const double*  W,
    const MFL_INTERP_UINT*        headsW,
    const MFL_INTERP_UINT         ppFD,
    const MFL_INTERP_UINT*        ppW,
    const MFL_INTERP_UINT*        gridSizeCumprod,
    const MFL_INTERP_UINT*        finiteDiffsSizeCumprod,
    const MFL_INTERP_UINT*        indX,
    const MFL_INTERP_UINT         indXNumel,
    double*        ndcube,
    double*        localW,
    MFL_INTERP_UINT*              strides,
    /* OUTPUTS: */
    double*        derivativeEst
)
{
    MFL_INTERP_UINT ii, ii2, jj, lastStride, thisStride, thisStrideW;
    double wL, wU, fdL, fdU, denW;

    /*
     * Copy strided N-D cube finite difference data into a contiguous n-D cube.
     * For example, strided 3-D cube for FDx2x4x5:
     *   pp
     *   pp+s2
     *
     *   pp+   s4
     *   pp+s2+s4
     *
     *   pp      +s5
     *   pp+s2   +s5
     *   pp+   s4+s5
     *   pp+s2+s4+s5
     *
     *   ...
     */
    strides[0] = 0;
    ndcube[0] = FD[headFD + ppFD];

    for (ii = 0; ii < indXNumel; ++ii) {

        thisStride = finiteDiffsSizeCumprod[ indX[ii] ];
        ii2 = ((MFL_INTERP_UINT)1) << ii;
        for (jj = ii2; jj < 2*ii2; ++jj) { /* skip jj = 0 */
            lastStride = strides[jj-ii2];
            strides[jj] = lastStride + thisStride;
            ndcube[jj] = FD[headFD + ppFD + strides[jj]];
        }

        thisStrideW = gridSizeCumprod[ indX[ii] ];
        localW[2*ii  ] = W[ headsW[indX[ii]] + ppW[ii] ];
        localW[2*ii+1] = W[ headsW[indX[ii]] + ppW[ii] + 2*thisStrideW ]; /* ... x (ni+2) x ...*/
    }

    /*
     * Finally, compute on the contiguous N-D cube.
     */
    for (ii = indXNumel; ii > 0; --ii) {
        /*
         * Accumulate Akima derivative formula in this dimension and then reduce
         * the cube's dimension by 1 and move on to the next higher dimension:
         */

        /*
         * Hiroshi Akima's derivative estimate formula, i.e., eq. (1) from:
         *   H. Akima, "A New Method of Interpolation and Smooth Curve Fitting
         *   Based on Local Procedures", Journal of the ACM (JACM), v. 17-4, p.589-602, 1970.
         *
         * later generalized to 2-D in:
         *   H. Akima, "A method of bivariate interpolation and smooth surface fitting
         *   based on local procedures", Communications of the ACM, v. 17-1, p.18-20, 1974.
         *
         * It's a weighted average of lower and upper slopes.
         * Reformulate Akima's formula  (wU*fdL + wL*fdU) / denW  to get better numerics.
         *
         * Also, for MATLAB's 'makima' formula, if the data is constant for more than four
         * consecutive grid nodes, then the weights denominator is zero and the formula
         * produces an unwanted NaN result. Replace this NaN with 0:
         */
        wL = localW[2*indXNumel-2*ii  ];    /* Lower Akima weight */
        wU = localW[2*indXNumel-2*ii+1];    /* Upper Akima weight */
        denW = wL + wU;
        wL = (denW == 0) ? 0 : wL/denW;
        wU = (denW == 0) ? 0 : wU/denW;

        ii2 = ((MFL_INTERP_UINT)1) << (ii-1);
        for (jj = 0; jj < ii2; ++jj) {
            fdL = ndcube[2*jj  ];    /* Lower finite difference, i.e., lower slope */
            fdU = ndcube[2*jj+1];    /* Upper finite difference, i.e., upper slope  */
            ndcube[jj] = wU*fdL + wL*fdU;
        }
    }

    /*
     * Akima derivative estimate was accumulated in the first N-D cube entry:
     */
    derivativeEst[0] = ndcube[0];
}
