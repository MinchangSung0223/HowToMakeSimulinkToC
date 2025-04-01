/* Copyright 2019 The MathWorks, Inc.*/

#include <string.h> /* memcpy, memset */

#include "akimaCoefficients_double.h"
#include "akimaDerivative_double.h"
#include "akimaFiniteDiffs_double.h"
#include "akimaStrides.h"
#include "akimaUtils_double.h"
#include "akimaWorkspace.h"

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
void akimaCoefficients_double
(
    /* INPUTS:  */
    const double** gridVectors,
    const double*  gridValues,
    const MFL_INTERP_UINT*        gridSize,
    const MFL_INTERP_UINT         N,
    double*        workspaceCoefficients,
    MFL_INTERP_UINT*              workspaceIndices,
    /* OUTPUTS: */
    double* coefficients
)
{
    MFL_INTERP_UINT gridNumel, pow2toN, numelFiniteDiffs;
    MFL_INTERP_UINT ii, ii2, jj, kk, pp, ppFD, qq, thisStride, thisStrideW;
    MFL_INTERP_UINT* gridSizeCumprod;
    MFL_INTERP_UINT* finiteDiffsSizeCumprod;
    MFL_INTERP_UINT* indicesX;
    MFL_INTERP_UINT* indicesXnnz;
    MFL_INTERP_UINT* indXjj;
    MFL_INTERP_UINT* indWjj;
    MFL_INTERP_UINT* ppW;

    double* workspaceDerivatives;
    double* workspaceFiniteDiffs;
    double* workspaceWeights;

    gridNumel = akimaProd(gridSize,N);
    pow2toN = ((MFL_INTERP_UINT)1) << N;
    numelFiniteDiffs = akimaFiniteDiffsWorkspace(gridSize,N);

    /* Temporary floating-point workspace */
    workspaceDerivatives = workspaceCoefficients;
    workspaceFiniteDiffs = workspaceCoefficients + pow2toN + 4*N;
    workspaceWeights = workspaceFiniteDiffs + numelFiniteDiffs;
    
    /*
     * Compute Akima N-D finite differences FD.
     */
    akimaFiniteDiffs_double(
                            gridVectors,
                            gridValues,
                            gridSize,
                            N,
                            numelFiniteDiffs,
                            workspaceIndices,
                            workspaceFiniteDiffs,
                            workspaceWeights);

    /*
     * Compute Akima derivative estimates. These are the N-D Akima coefficients.
     */

    /* The first Akima coefficient is simply a copy of the gridValues */
    memcpy(coefficients,gridValues,gridNumel*sizeof(double));

    /*
     * Temporary indices workspace:
     *  workspaceIndices[0,N*2^N)                         - Nx2^N matrix for finite diffs cumprod
     *  workspaceIndices[N*2^N,2*N*2^N)                   - Nx2^N matrix of derivative indices
     *  workspaceIndices[2*N*2^N,(2*N+1)*2^N)             - N-vector nnz of each derivative index
     *  workspaceIndices[(2*N+1)*2^N,N+(2*N+1)*2^N)       - N-vector for weights indices
     *  workspaceIndices[N+(2*N+1)*2^N,2*N+(2*N+1)*2^N)   - N-vector for weights increment
     *  workspaceIndices[2*N+(2*N+1)*2^N,2*N+(2*N+2)*2^N) - 2^N-vector for strides
     *  workspaceIndices[2*N+(2*N+2)*2^N,2*N+(2*N+3)*2^N) - 2^N-vector for heads of finite diffs
     *  workspaceIndices[2*N+(2*N+3)*2^N,3*N+(2*N+3)*2^N) - N-vector for heads of weights
     */

    /*
     * We use derivative indices indXjj which follow the pattern:
     *   1                       corresponding to d/dx1
     *
     *       2                   corresponding to d/dx2
     *   1   2                   corresponding to d/dx1dx2
     *
     *           3               corresponding to d/dx3
     *   1       3               corresponding to d/dx1dx3
     *       2   3               corresponding to d/dx2dx3
     *   1   2   3               corresponding to d/dx1dx2dx3
     *
     *               4           corresponding to d/dx4
     *   1           4           corresponding to d/dx1dx4
     *       2       4           corresponding to d/dx2dx4
     *   1   2       4           corresponding to d/dx1dx2dx4
     *           3   4           corresponding to d/dx3dx4
     *   1       3   4           corresponding to d/dx1dx3dx4
     *       2   3   4           corresponding to d/dx2dx3dx4
     *   1   2   3   4           corresponding to d/dx1dx2dx3dx4
     *   ...
     */
    indicesX    = workspaceIndices + N * pow2toN;
    indicesXnnz = indicesX         + N * pow2toN;
    memset(indicesX,0,N*pow2toN*sizeof(MFL_INTERP_UINT));
    memset(indicesXnnz,0,pow2toN*sizeof(MFL_INTERP_UINT));
    indWjj = indicesXnnz + pow2toN;
    ppW = indWjj + N;

    /* NOTE: workspaceIndices[0,N*2^N) already contains the size of each finite difference. */
    gridSizeCumprod = workspaceIndices;
    akimaCumprod(gridSizeCumprod,N);
    kk = 1;

    for (ii = 0; ii < N; ++ii) {

        /*
         * Compute in the same order that we store the coefficients C and finite differences FD:
         * ii = 0: compute Cx1
         * ii = 1: compute Cx2, Cx1x2
         * ii = 2: compute Cx3, Cx1x3, Cx2x3, Cx1x2x3
         * ii = 3: compute Cx4, Cx1x4, Cx2x4, Cx1x2x4, Cx3x4, Cx1x3x4, Cx2x3x4, Cx1x2x3x4
         * ...
         * 
         * ii:  0    1 1    2 2 2 2    3  3  3  3  3  3  3  3    ...
         * jj:  1    2 3    4 5 6 7    8  9 10 11 12 13 14 15    ...
         */
        ii2 = ((MFL_INTERP_UINT)1) << ii;

        for (jj = ii2; jj < 2*ii2; ++jj, ++kk) { /* skip jj = 0 */

            /*
             * We need all these indices because pp walks through an n1 x n2 x ... x nN array
             * of coefficients, BUT we need to take into account that the sizes of the finite
             * differences FD and weights W are larger than the sizes of the coefficients.
             * We cannot directly do FD(pp) and W(pp).
             */

            /*
             * indXjj is a 0-based version of this MATLAB command:
             * indXjj = find(str2double(num2cell(flip(dec2bin(jj-1,N)))));
             * namely: ii jj  indXjj
             *         0  1   0
             *
             *         1  2   1
             *         1  3   01
             *
             *         2  4   2
             *         2  5   02
             *         2  6   12
             *         2  7   012
             *          ...
             */
            indXjj = indicesX + jj*N;
            memcpy(indXjj,indicesX + (jj-ii2)*N,indicesXnnz[jj-ii2]*sizeof(MFL_INTERP_UINT));
            indXjj[ indicesXnnz[jj-ii2] ] = ii;
            indicesXnnz[jj] = indicesXnnz[jj-ii2] + 1;

            /* indWjj is used as ind2sub for the weights W */
            memset(indWjj,0,indicesXnnz[jj]*sizeof(MFL_INTERP_UINT));

            finiteDiffsSizeCumprod = workspaceIndices + kk*N;
            akimaCumprod(finiteDiffsSizeCumprod,N);
            ppFD = 0;
            memset(ppW,0,N*sizeof(MFL_INTERP_UINT));

            for (pp = 0; pp < gridNumel; ++pp, ++ppFD) {

                /*
                 * Compute Akima coefficient using the Akima derivative formula.
                 */
                akimaDerivative_double(
                                        workspaceFiniteDiffs,
                                        ppW[N + pow2toN + kk],
                                        workspaceWeights,
                                        ppW + N + 2*pow2toN,
                                        ppFD,
                                        ppW,
                                        gridSizeCumprod,
                                        finiteDiffsSizeCumprod,
                                        indXjj,
                                        indicesXnnz[jj],
                                        workspaceDerivatives,
                                        workspaceDerivatives + pow2toN,
                                        ppW + N,
                                        coefficients + kk*gridNumel + pp);

                /*
                 * Make sure we correctly jump to the next FD and W
                 *
                 * For example, if the gridValues have size n1 x n2, then the x1x2 finite
                 * difference FDx1x2 has size (n1+1) x (n2+1), and we want ppFD to jump the
                 * (n1+1) row and the (n2+1) column.
                 * Similarly, the 1-D weights w1 have size (n1+2) x n2 and we want to jump the
                 * (n1+1) and (n1+2) rows. And w2 has size n1 x (n2+2) and we want to jump the
                 * (n1+1) and (n1+2) columns.
                 * And these jumps must work for n-D arrays of finite differences and weights,
                 * and for jumping along a general dimension dim <= N.
                 */
                for (qq = 0; qq < indicesXnnz[jj]; ++qq) {

                    thisStride = finiteDiffsSizeCumprod[ indXjj[qq] ];

                    if ( (ppFD+1) % thisStride == 0 ) {
                        ++(indWjj[qq]);

                        if (indWjj[qq] == gridSize[ indXjj[qq] ]) {
                            /* Account for FD size being ... x (ni+1) x ... */
                            indWjj[qq] = 0;
                            ppFD += thisStride;

                            /* Account for W size being ... x (ni+2) x ... */
                            thisStrideW = gridSizeCumprod[ indXjj[qq] ];
                            ppW[qq] += 2*thisStrideW;
                        }
                    }

                    ++(ppW[qq]);
                }
            }
        }
    }
}


/**
 * Compute 1-D Akima cubic polynomial coefficients.
 *
 * <b>NOTE: This is a faster 1-D alternative to the general N-D code <tt>(N = 1,2,3,...)</tt> in
 *          <tt>akimaCoefficients_double</tt>.</b>
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
void akimaCoefficients_1D_double
(
    /* INPUTS:  */
    const double* x,
    const double* v,
    const MFL_INTERP_UINT        nx,
    double*       workspace,
    /* OUTPUTS: */
    double*       coefficients
)
{
    MFL_INTERP_UINT ii;
    double sL, sU, wL, wU, denW, s1, s2;
    double* s; /* slopes */
    double* w; /* weights */
    double* d; /* modified Akima derivative estimates */

    /* The first Akima coefficient is simply a copy of the grid value v */
    memcpy(coefficients,v,nx*sizeof(double));

    /* The second Akima coefficient is the modified Akima derivative estimate */
    d = coefficients + nx;

    s = workspace;          /* slopes */
    w = workspace + nx + 1; /* weights */
    
    /* Must be initialized to zero to work for the case of two grid nodes. */
    memset(s,0,(nx + 1)*sizeof(double));
    
    /* s(2:n) = diff(v) ./ diff(x) */
    for (ii = 1; ii < nx; ++ii) {
        s[ii] = ( v[ii] - v[ii-1] ) / ( x[ii] - x[ii-1] );
    }
    /* End points */
    s[0]  = 2*s[1]    - s[2];
    s[nx] = 2*s[nx-1] - s[nx-2];

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
    for (ii = 1; ii < nx+1; ++ii) {
        w[ii] = akimaAbs_double( s[ii] - s[ii-1] ) + 
                akimaAbs_double( s[ii]/2 + s[ii-1]/2 );
    }
    
    /* Extrapolate the first weight */
    s2 = s[0];
    s1 = 2*s[0] - s[1];
    w[0] = akimaAbs_double( s2 - s1 ) +
           akimaAbs_double( s2/2 + s1/2 );

    /* Extrapolate the last weight */
    s2 = s[nx];
    s1 = 2*s[nx] - s[nx-1];
    w[nx+1] = akimaAbs_double( s2 - s1 ) +
              akimaAbs_double( s2/2 + s1/2 );

    for (ii = 0; ii < nx; ++ii) {

        sL = s[ii];   /* lower slope */
        sU = s[ii+1]; /* upper slope */
        wL = w[ii];   /* lower weight */
        wU = w[ii+2]; /* upper weight */

        /*
         * The derivative estimate is a weighted average of lower and upper slopes
         * Also, for MATLAB's 'makima' formula, if the data is constant for more than four
         * consecutive grid nodes, then the weights denominator is zero and the formula
         * produces an unwanted NaN result. Replace this NaN with 0:
         */
        denW = wL + wU;
        d[ii] = (denW == 0) ? 0 : (wU/denW) * sL + (wL/denW) * sU;
    }
}
