/* Copyright 2019 The MathWorks, Inc.*/

#ifndef _MFL_INTERP_AKIMADERIVATIVE_double_H_
#define _MFL_INTERP_AKIMADERIVATIVE_double_H_

#include "mfl_interp_util.h" /* MFL_INTERP_UINT */

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
);

#endif /* _MFL_INTERP_AKIMADERIVATIVE_H_ */
