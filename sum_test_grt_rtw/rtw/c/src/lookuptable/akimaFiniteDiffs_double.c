/* Copyright 2019 The MathWorks, Inc.*/

#include <string.h> /* memcpy */

#include "akimaFiniteDiffs_double.h"
#include "akimaStrides.h"
#include "akimaUtils_double.h"

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
)
{
    MFL_INTERP_UINT gridNumel, pow2toN, weights1DNumel, prevFD, thisFD, prevFDsize, thisFDsize,
        ii, ii2, jj;
    MFL_INTERP_UINT* finiteDiffsNumel;
    MFL_INTERP_UINT* finiteDiffsHeads;
    MFL_INTERP_UINT* weights1DHeads;

    gridNumel = akimaProd(gridSize,N);
    pow2toN = ((MFL_INTERP_UINT)1) << N;

    /*
     * Workspace usage:
     *  finiteDiffsSize[0,N*2^N)                         - Nx2^N matrix for finite diffs sizes
     *  finiteDiffsSize[N*2^N,(N+1)*2^N)                 - 2^N-vector for finite diffs numel
     *  finiteDiffsSize[2*N+(2*N+2)*2^N,2*N+(2*N+3)*2^N) - 2^N-vector for heads of finite diffs
     *  finiteDiffsSize[2*N+(2*N+3)*2^N,3*N+(2*N+3)*2^N) - N-vector for heads of weights
     *  finiteDiffs[0,(2*n1+1)*(2*n2+1)*...*(2*nN+1)]    - finite differences
     *  weights1D[0,sum_{i=1:N}( n1*n2* ... *(ni + 2)* ... *nN ) - 1-D weights
     */

    /*
     * Compute individual sizes for Akima finite differences:
     *   size F             = n1   x n2   x n3 x ... x nN
     *   size FD along x1   = n1+1 x n2   x n3 x ... x nN
     *   size FD along x2   = n1   x n2+1 x n3 x ... x nN
     *   size FD along x1x2 = n1+1 x n2+1 x n3 x ... x nN
     *   ...
     *
     * Compute Akima finite differences:
     * Loop N times for each xi. Do 1D finite differences on previous results:
     *    F                      f.d. along x1 => (F, FDx1)
     *   (F, FDx1)               f.d. along x2 => (FDx2, FDx1x2)
     *   (F, FDx1, FDx2, FDx1x2) f.d. along x3 => (FDx3, FDx1x3, FDx2x3, FDx1x2x3)
     *   ...
     * Which produces the finite differences FD =
     *    F,   FDx1,   FDx2, FDx1x2,   FDx3, FDx1x3, FDx2x3, FDx1x2x3,   ...
     */
    
    /* Must be initialized to zero to work for the case of two grid nodes. */
    memset(finiteDiffs,0,numelFiniteDiffs*sizeof(double));
    
    /* The first finite difference is a copy of the gridValues */
    memcpy(finiteDiffs,gridValues,gridNumel*sizeof(double));
    memcpy(finiteDiffsSize,gridSize,N*sizeof(MFL_INTERP_UINT));

    /* finiteDiffsSize[N*2^N,(N+1)*2^N) - number of elements for each finite difference */
    finiteDiffsNumel = finiteDiffsSize + N*pow2toN;
    finiteDiffsNumel[0] = gridNumel;

    finiteDiffsHeads = finiteDiffsSize + 2 * ( N + (N+1)*pow2toN );
    finiteDiffsHeads[0] = 0;
    weights1DNumel = 0;
    weights1DHeads = finiteDiffsHeads + pow2toN;
    weights1DHeads[0] = 0;

    thisFD = gridNumel; /* where the current to-be-computed FD starts */

    for (ii = 0; ii < N; ++ii) {

        /*
         * Compute in the same order that we store in finiteDiffs:
         * ii = 0: compute FDx1
         * ii = 1: compute FDx2, FDx1x2
         * ii = 2: compute FDx3, FDx1x3, FDx2x3, FDx1x2x3
         * ii = 3: compute FDx4, FDx1x4, FDx2x4, FDx1x2x4, FDx3x4, FDx1x3x4, FDx2x3x4, FDx1x2x3x4
         * ...
         */
        prevFD = 0;
        ii2 = ((MFL_INTERP_UINT)1) << ii;

        for (jj = ii2; jj < 2*ii2; ++jj) { /* skip jj = 0 */

            /* Compute size of current finite difference: */
            prevFDsize = (jj-ii2)*N; /* e.g. FDx2   */
            thisFDsize = jj*N;       /* and  FDx2x3 */
            memcpy(finiteDiffsSize+thisFDsize,finiteDiffsSize+prevFDsize,N*sizeof(MFL_INTERP_UINT));
            finiteDiffsSize[thisFDsize+ii] += 1;
            finiteDiffsNumel[jj] = finiteDiffsNumel[jj-ii2] + finiteDiffsNumel[jj-ii2]/gridSize[ii];
            finiteDiffsHeads[jj] = finiteDiffsHeads[jj-1] + finiteDiffsNumel[jj-1];

            /* Compute current Akima finite difference using previous finite difference: */
            akimaFiniteDiffs1D_double(
                                       gridVectors[ii],
                                       finiteDiffs+prevFD,
                                       finiteDiffsSize+prevFDsize,
                                       finiteDiffsNumel[jj-ii2],
                                       ii,
                                       finiteDiffsSize+thisFDsize,
                                       finiteDiffs+thisFD);

            if (jj == ii2) {
                /*
                 * If we are computing the 1-D finite differences FDx1, FDx2, FDx3, ..., FDxN,
                 * then also compute the Akima 1-D weights while we are here:
                 */

                /*
                 * Sizes for Akima weights:
                 *   Weights for x1            = n1+2  x  n2    x  n3    x  ...  x  nN
                 *   Weights for x2            = n1    x  n2+2  x  n3    x  ...  x  nN
                 *   Weights for x3            = n1    x  n2    x  n3+2  x  ...  x  nN
                 *   ...
                 *   Weights for xN            = n1    x  n2    x  n3    x  ...  x  nN+2
                 */

                /*
                 * Compute the size of this weight Wxi:
                 * Namely size(Wxi) == size(F) except for size(Wxi,i) = size(F,i) + 2
                 */
                if (ii > 0) {
                    weights1DHeads[ii] = weights1DHeads[ii-1] + weights1DNumel;
                }
                weights1DNumel = gridNumel + 2*(gridNumel/gridSize[ii]);
                memcpy(finiteDiffsSize+thisFDsize+N,finiteDiffsSize+prevFDsize,N*sizeof(MFL_INTERP_UINT));
                finiteDiffsSize[thisFDsize+N+ii] += 2;

                /*
                 * Compute 1-D weights at xi by applying finite differences again
                 * on the 1-D Akima finite differences FDxi:
                 */
                akimaFiniteDiffs1D_double(
                                           (void *)0 /* use the 1-D weights formula */,
                                           finiteDiffs+thisFD,
                                           finiteDiffsSize+thisFDsize,
                                           finiteDiffsNumel[ii2],
                                           ii,
                                           finiteDiffsSize+thisFDsize+N,
                                           weights1D);

                weights1D += weights1DNumel;
            }

            /*
             * Keep track of the heads for the previous finite difference and
             * the current finite difference. The current finite difference
             * is computed from the previous finite difference via compute1D.
             */
            prevFD += finiteDiffsNumel[jj-ii2];
            thisFD += finiteDiffsNumel[jj];
        }
    }
}

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
)
{
    MFL_INTERP_UINT strideF, numelPageF, numelVectorF, strideFD, numelPageFD, numelVectorFD;
    MFL_INTERP_UINT iiF, iiFD, jjF, jjFD, jjFend, jjFDend, kk, skk, skkFD;
    double f1, f2;

    /* Compute strides: */
    akimaStrideData(sizeF ,dim,&strideF ,&numelPageF ,&numelVectorF );
    akimaStrideData(sizeFD,dim,&strideFD,&numelPageFD,&numelVectorFD);

    /* For each page: */
    for (iiF = 0, iiFD = 0; iiF < numelF; iiF += numelPageF, iiFD += numelPageFD) {

        /* For each (strided) vector on a page: */
        for (jjF = iiF, jjFD = iiFD; jjF < iiF + strideF; ++jjF, ++jjFD) {

            /*
             * For a vector f of length n, its 1-D Akima finite difference
             * is a vector fd of length n+1, given by a finite difference of order one:
             *
             *  fd(2:n) = ( f(2:n)-f(1:n-1) ) / ( x(2:n)-x(1:n-1) )
             *
             * AND the end points fd(1) and fd(n+1) are computed with by extrapolating fd(2:n).
             */

            /*
             * We also account for a dim and an N-D array F of strided vectors along dim.
             * So we actually diff the strided vector F(jj:stride:(end-stride))
             * while keeping track of the fact that size(FD) == size(F)
             * except for size(FD,dim) = size(F,dim) + 1
             */
            for (kk = 1; kk < numelVectorF; ++kk) {

                /* Skip the first and last FD end points (via skkFD): */
                skk   = jjF  + (kk-1)*strideF;
                skkFD = jjFD +     kk*strideFD;

                f1 = F[skk];
                f2 = F[skk + strideF];

                if (x) { /* Used when computing 1-D Akima finite differences FD */

                    /*
                     * Classic finite differences of order one to compute slopes s(i):
                     *
                     *   s(i) = ( f(i+1)-f(i) ) / ( x(i+1)-x(i) )
                     */
                    FD[skkFD] = ( f2 - f1 ) / ( x[kk] - x[kk-1] );
                }
                else { /* Used when computing the 1-D weights W */

                    /*
                     * Hiroshi Akima's derivative estimate formula, i.e., eq. (1) from:
                     *   H. Akima, "A New Method of Interpolation and Smooth Curve Fitting
                     *   Based on Local Procedures", Journal of the ACM (JACM),
                     *   v. 17-4, p.589-602, 1970
                     *
                     * uses the weights:
                     *
                     *   w(i) = abs( s(i+1) - s(i) ) .
                     *
                     * By adding a second averaging term we eliminate overshoot and undershoot when
                     * data is constant for more than two consecutive grid nodes:
                     *
                     *   w(i) = abs( s(i+1) - s(i) ) + abs( s(i+1) + s(i) )/2 .
                     *
                     * This is the 'makima' formula in MATLAB.
                     */
                    FD[skkFD] = akimaAbs_double( f2 - f1 ) +
                                akimaAbs_double( f2/2 + f1/2 );
                }
            }

            jjFDend = jjFD + numelVectorF*strideFD;
            if (x) { /* Used when computing 1-D Akima finite differences FD */
                /*
                 * Now handle the first and last FD end points.
                 * Extrapolate the above Akima finite differences:
                 */
                FD[jjFD   ] = 2*FD[jjFD   +strideFD] - FD[jjFD   +2*strideFD];
                FD[jjFDend] = 2*FD[jjFDend-strideFD] - FD[jjFDend-2*strideFD];
            }
            else { /* Used when computing the 1-D weights W */
                /* Extrapolate the first weight */
                f2 = F[jjF];
                f1 = 2*f2 - F[jjF+strideF];
                FD[jjFD] = akimaAbs_double( f2 - f1 ) +
                           akimaAbs_double( f2/2 + f1/2 );

                /* Extrapolate the last weight */
                jjFend = jjF + (numelVectorF-1)*strideF;
                f2 = F[jjFend];
                f1 = 2*f2 - F[jjFend-strideF];
                FD[jjFDend] = akimaAbs_double( f2 - f1 ) +
                              akimaAbs_double( f2/2 + f1/2 );
            }
        }
    }
}
