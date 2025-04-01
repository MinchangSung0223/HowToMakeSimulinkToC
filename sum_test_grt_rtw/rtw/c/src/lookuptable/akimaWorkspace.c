/* Copyright 2019 The MathWorks, Inc.*/

#include "akimaWorkspace.h"
#include "akimaStrides.h"

/**
 * \file
 * Compute workspace size required for Akima polynomial coefficients, polynomial basis,
 * and interpolation.
 */

/**
 * Compute workspace size for akimaFixedGrid_precompute_double() and
 * akimaFixedGrid_precompute_float().
 *
 * \param[in]  N            Number of dimensions of underlying N-D grid, i.e., \p N.
 * \param[in]  gridSize     Size of the underlying N-D grid. In MATLAB notation:
 *                          <tt>[gridSize(1), ..., gridSize(N)] = size(ndgrid(x1, ..., xN))</tt>,
 *                          where <tt>x1, ..., xN </tt> are the \p N vectors defining the N-D grid.
 *
 * \param[out]  numelWorkspace1   Workspace size for floating-point quantities.
 * \param[out]  numelWorkspace2   Workspace size for MFL_INTERP_UINT quantities.
 * \param[out]  numelCoefficients Workspace size to store the Akima coefficients.
 */

void akimaFixedGrid_precomputeWS
(
    /* INPUTS:  */
    const MFL_INTERP_UINT  N,
    const MFL_INTERP_UINT* gridSize,
    /* OUTPUTS: */
    MFL_INTERP_UINT* numelWorkspaceCoefficients,
    MFL_INTERP_UINT* numelWorkspaceIndices,
    MFL_INTERP_UINT* numelCoefficients
)
{
    MFL_INTERP_UINT gridNumel, pow2toN, ii, numelFiniteDiffs, numelWeights1D, numelDerivativesNDCube;

    gridNumel = akimaProd(gridSize,N); /* n1*n2* ... *nN */
    pow2toN = ((MFL_INTERP_UINT)1) << N;

    /*
     * Temporary Floating-point workspace for N-D Akima finite differences:
     */
    numelFiniteDiffs = akimaFiniteDiffsWorkspace(gridSize,N);

    /*
     * Sizes for 1-D Akima weights:
     *   Weights for x1            = n1+2  x  n2    x  n3    x  ...  x  nN
     *   Weights for x2            = n1    x  n2+2  x  n3    x  ...  x  nN
     *   Weights for x3            = n1    x  n2    x  n3+2  x  ...  x  nN
     *   ...
     *   Weights for xN            = n1    x  n2    x  n3    x  ...  x  nN+2
     *
     * Which add up to a total 1-D weights workspace requirement of (MATLAB notation):
     *   sum(prod(gridSize+2*diag(ones(N,1)),2))
     */
    numelWeights1D = N*gridNumel;
    for (ii = 0; ii < N; ++ii) {
        numelWeights1D += 2*(gridNumel/gridSize[ii]);
    }

    /*
     * Need a temporary contiguous N-D cube to compute an Akima derivative and a 4N-vector.
     */
    numelDerivativesNDCube = pow2toN + 4*N;

    /*
     * Therefore, total temporary floating-point workspace for computing the coefficients:
     */
    *numelWorkspaceCoefficients = numelFiniteDiffs + numelWeights1D + numelDerivativesNDCube;

    /* Array used for intermediate indices and sizes 3*N+(2*N+3)*2^N: */
    *numelWorkspaceIndices = 3*N + (2*N+3)*pow2toN;

    /*
     * Each Akima coefficient is an N-D array of size   n1 x n2 x ... x nN.
     * And we have a total of 2^N coefficient arrays.
     */
    *numelCoefficients = gridNumel*pow2toN;
}

/**
 * Compute workspace size for akimaFixedGrid_precompute_1D_double() and
 * akimaFixedGrid_precompute_1D_float().
 *
 * \param[in]  nx                 Number of 1-D grid coordinates.
 *
 * \param[out]  numelWorkspace    Workspace size for floating-point quantities.
 * \param[out]  numelCoefficients Workspace size to store the Akima coefficients.
 */

void akimaFixedGrid_precomputeWS_1D
(
    /* INPUTS:  */
    const MFL_INTERP_UINT nx,
    /* OUTPUTS: */
    MFL_INTERP_UINT* numelWorkspace,
    MFL_INTERP_UINT* numelCoefficients
)
{
    *numelWorkspace = 2*nx+3;
    *numelCoefficients = 2*nx;
}

/**
 * Compute workspace size for akimaFixedGrid_interpolate_double() and
 * akimaFixedGrid_interpolate_float().
 *
 * \param[in]  N                  Number of dimensions of underlying N-D grid, i.e., \p N.
 *
 * \param[out]  numelWorkspace1   Workspace size for floating-point quantities.
 * \param[out]  numelWorkspace2   Workspace size for MFL_INTERP_UINT quantities.
 */

void akimaFixedGrid_interpolateWS
(
    /* INPUTS:  */
    const MFL_INTERP_UINT  N,
    /* OUTPUTS: */
    MFL_INTERP_UINT* numelWorkspaceEvaluation,
    MFL_INTERP_UINT* numelWorkspaceIndices
)
{
    MFL_INTERP_UINT pow2toN = ((MFL_INTERP_UINT)1) << N;
    /* Temporary Floating-point workspace for N-D cube and polynomial terms: */
    *numelWorkspaceEvaluation = 4*N+pow2toN;
    /* Intermediate N-D indices */
    *numelWorkspaceIndices = N+(N+1)*pow2toN;
}

/**
 * Compute workspace size for akimaFixedQuery_precompute_double() and
 * akimaFixedQuery_precompute_float().
 *
 * \param[in]  N     Number of dimensions of underlying N-D grid, i.e., \p N.
 * \param[in]  numQ  Number of query points.
 *                   In MATLAB notation: <tt>numQ = numel(xqi) = numel(Vq)</tt>.
 *
 * \param[out]  numelAkimaBasis   Workspace size to store Akima basis.
 */

MFL_INTERP_UINT akimaFixedQuery_precomputeWS
(
    const MFL_INTERP_UINT N,
    const MFL_INTERP_UINT numQ
)
{
    return 4*N*numQ;
}

/**
 * Compute workspace size for akimaFixedQuery_precompute_1D_double() and
 * akimaFixedQuery_precompute_1D_float().
 *
 * \param[in]  numQ  Number of query points.
 *                   In MATLAB notation: <tt>numQ = numel(xqi) = numel(Vq)</tt>.
 *
 * \param[out]  numelAkimaBasis  Workspace size to store Akima basis.
 */

MFL_INTERP_UINT akimaFixedQuery_precomputeWS_1D
(
    const MFL_INTERP_UINT numQ
)
{
    return akimaFixedQuery_precomputeWS(1,numQ);
}

/**
 * Compute workspace size for akimaFixedQuery_interpolate_double() and
 * akimaFixedQuery_interpolate_float().
 *
 * \param[in]  N         Number of dimensions of underlying N-D grid, i.e., \p N.
 * \param[in]  gridSize  Size of the underlying N-D grid. In MATLAB notation:
 *                       <tt>[gridSize(1), ..., gridSize(N)] = size(ndgrid(x1, ..., xN))</tt>,
 *                       where <tt>x1, ..., xN </tt> are the \p N vectors defining the N-D grid.
 *
 * \param[out]  numelWorkspace1   Workspace size for floating-point quantities.
 * \param[out]  numelWorkspace2   Workspace size for MFL_INTERP_UINT quantities.
 */

void akimaFixedQuery_interpolateWS
(
    /* INPUTS:  */
    const MFL_INTERP_UINT  N,
    const MFL_INTERP_UINT* gridSize,
    /* OUTPUTS: */
    MFL_INTERP_UINT*       numelFloatingPointWS,
    MFL_INTERP_UINT*       numelUnsignedIntWS
)
{
    MFL_INTERP_UINT numelWorkspaceCoeffs, numelCoeffs;
    MFL_INTERP_UINT numelWorkspaceEval, numelWorkspaceEvalIndices;

    /* Compute workspace size for coefficients. */
    akimaFixedGrid_precomputeWS(N, gridSize, &numelWorkspaceCoeffs, numelUnsignedIntWS, &numelCoeffs);

    /* Compute workspace size for evaluation. */
    akimaFixedGrid_interpolateWS(N, &numelWorkspaceEval, &numelWorkspaceEvalIndices);

    *numelFloatingPointWS = ((numelWorkspaceCoeffs < numelWorkspaceEval) ? numelWorkspaceEval : numelWorkspaceCoeffs) + numelCoeffs;
    if (*numelUnsignedIntWS < numelWorkspaceEvalIndices) {
        *numelUnsignedIntWS = numelWorkspaceEvalIndices;
    }
}

/**
 * Compute workspace size for akimaFixedQuery_interpolate_1D_double() and
 * akimaFixedQuery_interpolate_1D_float().
 *
 * \param[in]  nx  Number of 1-D grid coordinates.
 *
 * \param[out] numelWorkspace  Workspace size for floating-point quantities.
 */

MFL_INTERP_UINT akimaFixedQuery_interpolateWS_1D
(
    const MFL_INTERP_UINT  gridSize
)
{
    MFL_INTERP_UINT numelWorkspace, numelCoeffs;
    akimaFixedGrid_precomputeWS_1D(gridSize,&numelWorkspace,&numelCoeffs);
    return (numelWorkspace + numelCoeffs);
}

/**
 * Get workspace size for N-D Akima finite differences.
 *
 * \param[in]  gridSize  Size of the underlying N-D grid. In MATLAB notation:
 *                       <tt>[gridSize(1), ..., gridSize(N)] = size(ndgrid(x1, ..., xN))</tt>,
 *                       where <tt>x1, ..., xN </tt> are the \p N vectors defining the N-D grid.
 * \param[in]  N         Number of dimensions of underlying N-D grid, i.e., \p N.
 *
 * \return Array numel for N-D Akima finite differences.
 */
MFL_INTERP_UINT akimaFiniteDiffsWorkspace
(
    const MFL_INTERP_UINT* gridSize,
    const MFL_INTERP_UINT  N
)
{
    MFL_INTERP_UINT numelFiniteDiffs, ii;

    /*
     * Sizes for Akima finite differences:
     *   size F                    = n1    x  n2    x  n3    x  ...  x  nN
     *   size FD along x1          = n1+1  x  n2    x  n3    x  ...  x  nN
     *   size FD along x2          = n1    x  n2+1  x  n3    x  ...  x  nN
     *   size FD along x1x2        = n1+1  x  n2+1  x  n3    x  ...  x  nN
     *   size Fd along x3          = n1    x  n2    x  n3+1  x  ...  x  nN
     *   size FD along x1x3        = n1+1  x  n2    x  n3+1  x  ...  x  nN
     *   size FD along x2x3        = n1    x  n2+1  x  n3+1  x  ...  x  nN
     *   size FD along x1x2x3      = n1+1  x  n2+1  x  n3+1  x  ...  x  nN
     *   ...
     *   size FD along x1x2x3...xN = n1+1  x  n2+1  x  n3+1  x  ...  x  nN+1
     *
     * Which add up to a total finite difference workspace requirement of:
     *   (2*n1+1)*(2*n2+1)*...*(2*nN+1)
     */
    numelFiniteDiffs = 1;
    for (ii = 0; ii < N; ++ii) {
        numelFiniteDiffs *= 2*gridSize[ii]+1;
    }
    return numelFiniteDiffs;
}
