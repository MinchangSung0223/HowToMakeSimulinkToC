/* Copyright 2019 The MathWorks, Inc.*/

#ifndef _MFL_INTERP_AKIMAWORKSPACE_H_
#define _MFL_INTERP_AKIMAWORKSPACE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "mfl_interp_util.h" /* MFL_INTERP_UINT */

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
    MFL_INTERP_UINT* numelWorkspace1,
    MFL_INTERP_UINT* numelWorkspace2,
    MFL_INTERP_UINT* numelCoefficients
);

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
);

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
    MFL_INTERP_UINT* numelWorkspace1,
    MFL_INTERP_UINT* numelWorkspace2
);

/**
 * Compute workspace size for akimaFixedQuery_precompute_double() and
 * akimaFixedQuery_precompute_float().
 *
 * \param[in]  N     Number of dimensions of underlying N-D grid, i.e., \p N.
 * \param[in]  numQ  Number of query points.
 *                   In MATLAB notation: <tt>numQ = numel(xqi) = numel(Vq)</tt>.
 *
 * \param[out]  numelAkimaBasis  Workspace size to store Akima basis.
 */

MFL_INTERP_UINT akimaFixedQuery_precomputeWS
(
    const MFL_INTERP_UINT N,
    const MFL_INTERP_UINT numQ
);

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
);

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
    MFL_INTERP_UINT*       numelWorkspace1,
    MFL_INTERP_UINT*       numelWorkspace2
);

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
    const MFL_INTERP_UINT  nx
);

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
);

#ifdef __cplusplus
}
#endif

#endif /* _MFL_INTERP_AKIMAWORKSPACE_H_ */
