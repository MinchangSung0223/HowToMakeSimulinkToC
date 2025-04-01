/*
 * sum_test.c
 *
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * Code generation for model "sum_test".
 *
 * Model version              : 1.1
 * Simulink Coder version : 23.2 (R2023b) 01-Aug-2023
 * C source code generated on : Tue Apr  1 09:31:00 2025
 *
 * Target selection: grt.tlc
 * Note: GRT includes extra infrastructure and instrumentation for prototyping
 * Embedded hardware selection: Intel->x86-64 (Linux 64)
 * Code generation objectives: Unspecified
 * Validation result: Not run
 */

#include "sum_test.h"
#include <string.h>
#include "sum_test_private.h"
#include "rt_nonfinite.h"

/* External inputs (root inport signals with default storage) */
ExtU_sum_test_T sum_test_U;

/* External outputs (root outports fed by signals with default storage) */
ExtY_sum_test_T sum_test_Y;

/* Real-time model */
static RT_MODEL_sum_test_T sum_test_M_;
RT_MODEL_sum_test_T *const sum_test_M = &sum_test_M_;

/* Model step function */
void sum_test_step(void)
{
  /* Outport: '<Root>/Out1' incorporates:
   *  Inport: '<Root>/Input'
   *  Inport: '<Root>/Input1'
   *  Sum: '<Root>/Sum1'
   */
  sum_test_Y.Out1 = sum_test_U.Input + sum_test_U.Input1;

  /* Matfile logging */
  rt_UpdateTXYLogVars(sum_test_M->rtwLogInfo, (&sum_test_M->Timing.taskTime0));

  /* signal main to stop simulation */
  {                                    /* Sample time: [0.001s, 0.0s] */
    if ((rtmGetTFinal(sum_test_M)!=-1) &&
        !((rtmGetTFinal(sum_test_M)-sum_test_M->Timing.taskTime0) >
          sum_test_M->Timing.taskTime0 * (DBL_EPSILON))) {
      rtmSetErrorStatus(sum_test_M, "Simulation finished");
    }
  }

  /* Update absolute time for base rate */
  /* The "clockTick0" counts the number of times the code of this task has
   * been executed. The absolute time is the multiplication of "clockTick0"
   * and "Timing.stepSize0". Size of "clockTick0" ensures timer will not
   * overflow during the application lifespan selected.
   * Timer of this task consists of two 32 bit unsigned integers.
   * The two integers represent the low bits Timing.clockTick0 and the high bits
   * Timing.clockTickH0. When the low bit overflows to 0, the high bits increment.
   */
  if (!(++sum_test_M->Timing.clockTick0)) {
    ++sum_test_M->Timing.clockTickH0;
  }

  sum_test_M->Timing.taskTime0 = sum_test_M->Timing.clockTick0 *
    sum_test_M->Timing.stepSize0 + sum_test_M->Timing.clockTickH0 *
    sum_test_M->Timing.stepSize0 * 4294967296.0;
}

/* Model initialize function */
void sum_test_initialize(void)
{
  /* Registration code */

  /* initialize non-finites */
  rt_InitInfAndNaN(sizeof(real_T));

  /* initialize real-time model */
  (void) memset((void *)sum_test_M, 0,
                sizeof(RT_MODEL_sum_test_T));
  rtmSetTFinal(sum_test_M, -1);
  sum_test_M->Timing.stepSize0 = 0.001;

  /* Setup for data logging */
  {
    static RTWLogInfo rt_DataLoggingInfo;
    rt_DataLoggingInfo.loggingInterval = (NULL);
    sum_test_M->rtwLogInfo = &rt_DataLoggingInfo;
  }

  /* Setup for data logging */
  {
    rtliSetLogXSignalInfo(sum_test_M->rtwLogInfo, (NULL));
    rtliSetLogXSignalPtrs(sum_test_M->rtwLogInfo, (NULL));
    rtliSetLogT(sum_test_M->rtwLogInfo, "tout");
    rtliSetLogX(sum_test_M->rtwLogInfo, "");
    rtliSetLogXFinal(sum_test_M->rtwLogInfo, "");
    rtliSetLogVarNameModifier(sum_test_M->rtwLogInfo, "rt_");
    rtliSetLogFormat(sum_test_M->rtwLogInfo, 4);
    rtliSetLogMaxRows(sum_test_M->rtwLogInfo, 0);
    rtliSetLogDecimation(sum_test_M->rtwLogInfo, 1);
    rtliSetLogY(sum_test_M->rtwLogInfo, "");
    rtliSetLogYSignalInfo(sum_test_M->rtwLogInfo, (NULL));
    rtliSetLogYSignalPtrs(sum_test_M->rtwLogInfo, (NULL));
  }

  /* external inputs */
  (void)memset(&sum_test_U, 0, sizeof(ExtU_sum_test_T));

  /* external outputs */
  sum_test_Y.Out1 = 0.0;

  /* Matfile logging */
  rt_StartDataLoggingWithStartTime(sum_test_M->rtwLogInfo, 0.0, rtmGetTFinal
    (sum_test_M), sum_test_M->Timing.stepSize0, (&rtmGetErrorStatus(sum_test_M)));
}

/* Model terminate function */
void sum_test_terminate(void)
{
  /* (no terminate code required) */
}
