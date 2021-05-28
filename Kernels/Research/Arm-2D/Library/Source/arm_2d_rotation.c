/*
 * Copyright (C) 2010-2020 Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/* ----------------------------------------------------------------------
 * Project:      Arm-2D Library
 * Title:        arm-2d_rotation.c
 * Description:  APIs for tile rotation
 *
 * $Date:        29 April 2021
 * $Revision:    V.0.1.0
 *
 * Target Processor:  Cortex-M cores
 *
 * -------------------------------------------------------------------- */


/*============================ INCLUDES ======================================*/
#define __ARM_2D_IMPL__

#include "arm_2d.h"
#include "__arm_2d_impl.h"
#include "math.h"


#ifdef   __cplusplus
extern "C" {
#endif

#if defined(__clang__)
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wincompatible-pointer-types-discards-qualifiers"
#   pragma clang diagnostic ignored "-Wcast-qual"
#   pragma clang diagnostic ignored "-Wcast-align"
#   pragma clang diagnostic ignored "-Wextra-semi-stmt"
#   pragma clang diagnostic ignored "-Wsign-conversion"
#   pragma clang diagnostic ignored "-Wunused-function"
#   pragma clang diagnostic ignored "-Wimplicit-int-float-conversion"
#   pragma clang diagnostic ignored "-Wdouble-promotion"
#   pragma clang diagnostic ignored "-Wunused-parameter"
#   pragma clang diagnostic ignored "-Wimplicit-float-conversion"
#   pragma clang diagnostic ignored "-Wimplicit-int-conversion"
#   pragma clang diagnostic ignored "-Wtautological-pointer-compare"
#   pragma clang diagnostic ignored "-Wsign-compare"
#   pragma clang diagnostic ignored "-Wfloat-conversion"
#   pragma clang diagnostic ignored "-Wmissing-prototypes"
#   pragma clang diagnostic ignored "-Wpadded"
#   pragma clang diagnostic ignored "-Wundef"
#elif defined(__IS_COMPILER_ARM_COMPILER_5__)
#   pragma diag_suppress 174,177,188,68,513,144
#elif __IS_COMPILER_GCC__
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wdiscarded-qualifiers"
#endif

#include <arm_math.h>


/*----------------------------------------------------------------------------*
 * Code Template                                                              *
 *----------------------------------------------------------------------------*/

#define __API_COLOUR                rgb565
#define __API_INT_TYPE              uint16_t
#define __API_PIXEL_BLENDING        __ARM_2D_PIXEL_BLENDING_RGB565

#include "__arm_2d_rotate.inc"

#define __API_COLOUR                rgb888
#define __API_INT_TYPE              uint32_t
#define __API_PIXEL_BLENDING        __ARM_2D_PIXEL_BLENDING_RGB888

#include "__arm_2d_rotate.inc"

/*============================ MACROS ========================================*/
#undef __PI
#define __PI                3.1415926f

#define __CALIB             0.009f
/* faster ATAN */
#define FAST_ATAN_F32_1(x, xabs)    \
                            (x * (PI / 4.0f) + 0.273f * x * (1.0f - xabs))
#define EPS_ATAN2           1e-5f

/*============================ MACROFIED FUNCTIONS ===========================*/

/* faster atan(y/x) float version */
static
float32_t __atan2_f32(float32_t y, float32_t x)
{
    float32_t       xabs = fabsf(x);
    float32_t       yabs = fabsf(y);
    float32_t       atan2est, div;

    if (xabs >= yabs) {
        /* division is in the [-1 +1] range */
        div = yabs / (xabs + EPS_ATAN2);
        atan2est = FAST_ATAN_F32_1(div, div);
    } else {
        /* division is in the ]1 x*1e5] range */
        div = xabs / (yabs + EPS_ATAN2);
        atan2est = PI / 2 - FAST_ATAN_F32_1(div, div);
    }
    /* append sign */
    return copysignf(1.0f, y) * copysignf(1.0f, x) * atan2est;
}

/*============================ TYPES =========================================*/
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ PROTOTYPES ====================================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ IMPLEMENTATION ================================*/




ARM_NONNULL(1,2,4)
static
arm_2d_point_float_t *__arm_2d_rotate_point(const arm_2d_location_t *ptLocation,
                                            const arm_2d_location_t *ptCenter,
                                            float fAngle,
                                            arm_2d_point_float_t *ptOutBuffer)
{
    int16_t iX = ptLocation->iX - ptCenter->iX;
    int16_t iY = ptLocation->iY - ptCenter->iY;

    float fX,fY;

    float fR;
    arm_sqrt_f32( iX * iX + iY * iY, &fR);

    float fAlpha;
#ifdef ORIG_ROT
    fAlpha = fAngle;
    if (0 != iX) {
        fAlpha += atanf((float)iY / (float)iX);
        if (iX < 0) {
            fAlpha += __PI;
        }
    } else if (iY > 0) {
        fAlpha += __PI / 2.0f;
    } else if (iY < 0) {
        fAlpha -= __PI / 2.0f;
    }
#else
    /* optimized */
    fAlpha = __atan2_f32((float)iY,  (float)iX);
    if (iX < 0) fAlpha += __PI;
    fAlpha += fAngle;
#endif

    //fX = fR * cosf(fAlpha) + ptCenter->iX;
    //fY = fR * sinf(fAlpha) + ptCenter->iY;


    float   sinAlpha = arm_sin_f32(fAlpha);
    float   cosAlpha = arm_cos_f32(fAlpha);

#if 0
    arm_sqrt_f32(1.0f - cosAlpha*cosAlpha, &sinAlpha);
	int fAlphaInt = (int)floorf(fAlpha*1.0f/PI);
    /* apply sign */
	if(fAlphaInt&1) sinAlpha = -sinAlpha;
#endif
    fX = fR * cosAlpha + ptCenter->iX;
    fY = fR * sinAlpha + ptCenter->iY;
    if (fX > 0) {
        ptOutBuffer->fX = fX + __CALIB;
    } else {
        ptOutBuffer->fX = fX - __CALIB;
    }
    if (fY > 0) {
        ptOutBuffer->fY = fY + __CALIB;
    } else {
        ptOutBuffer->fY = fY - __CALIB;
    }

    return ptOutBuffer;
}


static arm_2d_err_t __arm_2d_rotate_preprocess_source(arm_2d_op_rotate_t *ptThis)
{
    arm_2d_tile_t *ptSource = this.Source.ptTile;

    memset(ptSource, 0, sizeof(*ptSource));

    ptSource->tInfo = this.Origin.ptTile->tInfo;
    ptSource->bIsRoot = true;
    ptSource->pchBuffer = NULL;                 //!< special case

    arm_2d_region_t tOrigValidRegion;
    if (NULL == arm_2d_tile_get_root(this.Origin.ptTile, &tOrigValidRegion, NULL)) {
        return ARM_2D_ERR_OUT_OF_REGION;
    }

    //! angle validation
    this.tRotate.fAngle = fmodf(this.tRotate.fAngle, ARM_2D_ANGLE(360));

    //! calculate the source region
    do {
        arm_2d_point_float_t tPoint;

        arm_2d_location_t tTopLeft = {.iX = INT16_MAX, .iY = INT16_MAX};
        arm_2d_location_t tBottomRight = {.iX = INT16_MIN, .iY = INT16_MIN};

        //! Top Left
        arm_2d_location_t tCornerPoint = tOrigValidRegion.tLocation;
        __arm_2d_rotate_point(  &tCornerPoint,
                                &this.tRotate.tCenter,
                                this.tRotate.fAngle,
                                &tPoint);

        do {
            tTopLeft.iX = MIN(tTopLeft.iX, tPoint.fX);
            tTopLeft.iY = MIN(tTopLeft.iY, tPoint.fY);

            tBottomRight.iX = MAX(tBottomRight.iX, tPoint.fX);
            tBottomRight.iY = MAX(tBottomRight.iY, tPoint.fY);
        } while(0);

        //! Bottom Left
        tCornerPoint.iY += tOrigValidRegion.tSize.iHeight - 1;
        __arm_2d_rotate_point(  &tCornerPoint,
                                &this.tRotate.tCenter,
                                this.tRotate.fAngle,
                                &tPoint);

        do {
            tTopLeft.iX = MIN(tTopLeft.iX, tPoint.fX);
            tTopLeft.iY = MIN(tTopLeft.iY, tPoint.fY);

            tBottomRight.iX = MAX(tBottomRight.iX, tPoint.fX);
            tBottomRight.iY = MAX(tBottomRight.iY, tPoint.fY);
        } while(0);

        //! Top Right
        tCornerPoint = tOrigValidRegion.tLocation;
        tCornerPoint.iX += tOrigValidRegion.tSize.iWidth - 1;

        __arm_2d_rotate_point(  &tCornerPoint,
                                &this.tRotate.tCenter,
                                this.tRotate.fAngle,
                                &tPoint);

        do {
            tTopLeft.iX = MIN(tTopLeft.iX, tPoint.fX);
            tTopLeft.iY = MIN(tTopLeft.iY, tPoint.fY);

            tBottomRight.iX = MAX(tBottomRight.iX, tPoint.fX);
            tBottomRight.iY = MAX(tBottomRight.iY, tPoint.fY);
        } while(0);

        //! Bottom Right
        tCornerPoint.iY += tOrigValidRegion.tSize.iHeight - 1;
        __arm_2d_rotate_point(  &tCornerPoint,
                                &this.tRotate.tCenter,
                                this.tRotate.fAngle,
                                &tPoint);

        do {
            tTopLeft.iX = MIN(tTopLeft.iX, tPoint.fX);
            tTopLeft.iY = MIN(tTopLeft.iY, tPoint.fY);

            tBottomRight.iX = MAX(tBottomRight.iX, tPoint.fX);
            tBottomRight.iY = MAX(tBottomRight.iY, tPoint.fY);
        } while(0);

        //! calculate the region
        this.tRotate.tDummySourceOffset = tTopLeft;

        ptSource->tRegion.tSize.iHeight = tBottomRight.iY - tTopLeft.iY + 1;
        ptSource->tRegion.tSize.iWidth = tBottomRight.iX - tTopLeft.iX + 1;

        //this.tRotate.tTargetRegion.tSize = ptSource->tRegion.tSize;
    } while(0);

    return ARM_2D_ERR_NONE;
}


static void __arm_2d_rotate_preprocess_target(arm_2d_op_rotate_t *ptThis)
{
    this.tRotate.tTargetRegion.tSize = this.Source.ptTile->tRegion.tSize;

    arm_2d_region_t *ptTargetRegion = this.Target.ptRegion;
    if (NULL == ptTargetRegion) {
        ptTargetRegion = &this.Target.ptTile->tRegion;
    }
    this.Target.ptRegion = &this.tRotate.tTargetRegion;

    this.tRotate.tTargetRegion.tLocation = ptTargetRegion->tLocation;

    //! align with the specified center point
    do {

        arm_2d_location_t tOffset = {
            .iX = this.tRotate.tCenter.iX - this.tRotate.tDummySourceOffset.iX,
            .iY = this.tRotate.tCenter.iY - this.tRotate.tDummySourceOffset.iY,
        };

        arm_2d_location_t tTargetCenter = {
            .iX = ptTargetRegion->tSize.iWidth >> 1,
            .iY = ptTargetRegion->tSize.iHeight >> 1,
        };

        tOffset.iX = tTargetCenter.iX - tOffset.iX;
        tOffset.iY = tTargetCenter.iY - tOffset.iY;

        this.tRotate.tTargetRegion.tLocation.iX += tOffset.iX;
        this.tRotate.tTargetRegion.tLocation.iY += tOffset.iY;

    } while(0);
}


ARM_NONNULL(2)
arm_2d_err_t arm_2dp_rgb565_tile_rotation_prepare(
                                            arm_2d_op_rotate_t *ptOP,
                                            const arm_2d_tile_t *ptSource,
                                            const arm_2d_location_t tCentre,
                                            float fAngle,
                                            uint16_t hwFillColour)
{
    assert(NULL != ptSource);

    ARM_2D_IMPL(arm_2d_op_rotate_t, ptOP);

    OP_CORE.ptOp = &ARM_2D_OP_ROTATE_RGB565;

    this.Source.ptTile = &this.Origin.tDummySource;
    this.Origin.ptTile = ptSource;
    this.wMode = 0;
    this.tRotate.fAngle = fAngle;
    this.tRotate.tCenter = tCentre;
    this.tRotate.Mask.hwColour = hwFillColour;

    return __arm_2d_rotate_preprocess_source(ptThis);
}

ARM_NONNULL(2)
arm_2d_err_t arm_2dp_rgb888_tile_rotation_prepare(
                                            arm_2d_op_rotate_t *ptOP,
                                            const arm_2d_tile_t *ptSource,
                                            const arm_2d_location_t tCentre,
                                            float fAngle,
                                            uint32_t wFillColour)
{
    assert(NULL != ptSource);

    ARM_2D_IMPL(arm_2d_op_rotate_t, ptOP);

    OP_CORE.ptOp = &ARM_2D_OP_ROTATE_RGB888;

    this.Source.ptTile = &this.Origin.tDummySource;
    this.Origin.ptTile = ptSource;
    this.wMode = 0;
    this.tRotate.fAngle = fAngle;
    this.tRotate.tCenter = tCentre;
    this.tRotate.Mask.hwColour = wFillColour;

    return __arm_2d_rotate_preprocess_source(ptThis);
}

arm_fsm_rt_t __arm_2d_rgb565_sw_rotate(__arm_2d_sub_task_t *ptTask)
{
    ARM_2D_IMPL(arm_2d_op_rotate_t, ptTask->ptOP);
    assert(ARM_2D_COLOUR_RGB565 == OP_CORE.ptOp->Info.Colour.chScheme);

    __arm_2d_impl_rgb565_rotate(&(ptTask->Param.tCopyOrig),
                                &this.tRotate);


    return arm_fsm_rt_cpl;
}

arm_fsm_rt_t __arm_2d_rgb888_sw_rotate(__arm_2d_sub_task_t *ptTask)
{
    ARM_2D_IMPL(arm_2d_op_rotate_t, ptTask->ptOP);
    assert(ARM_2D_COLOUR_SZ_32BIT == OP_CORE.ptOp->Info.Colour.u3ColourSZ);


    __arm_2d_impl_rgb888_rotate(&(ptTask->Param.tCopyOrig),
                                &this.tRotate);

    return arm_fsm_rt_cpl;
}


ARM_NONNULL(2)
arm_2d_err_t arm_2dp_rgb565_tile_rotation_with_alpha_prepare(
                                        arm_2d_op_rotate_alpha_t *ptOP,
                                        const arm_2d_tile_t *ptSource,
                                        const arm_2d_location_t tCentre,
                                        float fAngle,
                                        uint16_t hwFillColour,
                                        uint_fast8_t chRatio)
{
    assert(NULL != ptSource);

    ARM_2D_IMPL(arm_2d_op_rotate_alpha_t, ptOP);

    OP_CORE.ptOp = &ARM_2D_OP_ROTATE_WITH_ALPHA_RGB565;

    this.Source.ptTile = &this.Origin.tDummySource;
    this.Origin.ptTile = ptSource;
    this.wMode = 0;
    this.tRotate.fAngle = fAngle;
    this.tRotate.tCenter = tCentre;
    this.tRotate.Mask.hwColour = hwFillColour;
    this.chRatio = chRatio;

    return __arm_2d_rotate_preprocess_source((arm_2d_op_rotate_t *)ptThis);
}

ARM_NONNULL(2)
arm_2d_err_t arm_2dp_rgb888_tile_rotation_with_alpha_prepare(
                                        arm_2d_op_rotate_alpha_t *ptOP,
                                        const arm_2d_tile_t *ptSource,
                                        const arm_2d_location_t tCentre,
                                        float fAngle,
                                        uint32_t wFillColour,
                                        uint_fast8_t chRatio)
{
    assert(NULL != ptSource);

    ARM_2D_IMPL(arm_2d_op_rotate_alpha_t, ptOP);

    OP_CORE.ptOp = &ARM_2D_OP_ROTATE_WITH_ALPHA_RGB888;

    this.Source.ptTile = &this.Origin.tDummySource;
    this.Origin.ptTile = ptSource;
    this.wMode = 0;
    this.tRotate.fAngle = fAngle;
    this.tRotate.tCenter = tCentre;
    this.tRotate.Mask.wColour = wFillColour;
    this.chRatio = chRatio;

    return __arm_2d_rotate_preprocess_source((arm_2d_op_rotate_t *)ptThis);
}

arm_fsm_rt_t __arm_2d_rgb565_sw_rotate_with_alpha(__arm_2d_sub_task_t *ptTask)
{
    ARM_2D_IMPL(arm_2d_op_rotate_alpha_t, ptTask->ptOP);
    assert(ARM_2D_COLOUR_RGB565 == OP_CORE.ptOp->Info.Colour.chScheme);

    __arm_2d_impl_rgb565_rotate_alpha(  &(ptTask->Param.tCopyOrig),
                                        &this.tRotate,
                                        this.chRatio);


    return arm_fsm_rt_cpl;
}

arm_fsm_rt_t __arm_2d_rgb888_sw_rotate_with_alpha(__arm_2d_sub_task_t *ptTask)
{
    ARM_2D_IMPL(arm_2d_op_rotate_alpha_t, ptTask->ptOP);
    assert(ARM_2D_COLOUR_SZ_32BIT == OP_CORE.ptOp->Info.Colour.u3ColourSZ);

    __arm_2d_impl_rgb888_rotate_alpha(&(ptTask->Param.tCopyOrig),
                                        &this.tRotate,
                                        this.chRatio);

    return arm_fsm_rt_cpl;
}


arm_fsm_rt_t arm_2dp_tile_rotate(arm_2d_op_rotate_t *ptOP,
                                 const arm_2d_tile_t *ptTarget,
                                 const arm_2d_region_t *ptRegion)
{
    ARM_2D_IMPL(arm_2d_op_rotate_t, ptOP);

    this.Target.ptTile = ptTarget;
    this.Target.ptRegion = ptRegion;

    __arm_2d_rotate_preprocess_target(ptThis);
    return __arm_2d_op_invoke((arm_2d_op_core_t *)ptThis);
}




/*----------------------------------------------------------------------------*
 * Accelerable Low Level APIs                                                 *
 *----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*
 * Draw a point whose cordinates is stored as float point.                    *
 *----------------------------------------------------------------------------*/

#if 0
static arm_2d_region_t *__arm_2d_calculate_region(  const arm_2d_point_float_t *ptLocation,
                                                    arm_2d_region_t *ptRegion)
{
    assert(NULL != ptLocation);
    assert(NULL != ptRegion);

    /* +-----+-----+
     * |  P0 |  P1 |
     * +---- p ----+
     * |  P2 |  -- |
     * +-----+-----+
     */

    arm_2d_location_t tPoints[3];

    tPoints[0].iX = (int16_t)ptLocation->fX;
    tPoints[2].iX = (int16_t)ptLocation->fX;
    tPoints[1].iX = (int16_t)(ptLocation->fX + 0.99f);
    ptRegion->tSize.iWidth = tPoints[1].iX - tPoints[0].iX + 1;

    tPoints[0].iY = (int16_t)ptLocation->fY;
    tPoints[2].iY = (int16_t)ptLocation->fY;
    tPoints[1].iY = (int16_t)(ptLocation->fY + 0.99f);
    ptRegion->tSize.iHeight = tPoints[2].iY - tPoints[0].iY + 1;

    ptRegion->tLocation = tPoints[0];

    return ptRegion;
}
#endif



#if defined(__clang__)
#   pragma clang diagnostic pop
#elif __IS_COMPILER_ARM_COMPILER_5__
#   pragma diag_warning 174,177,188,68,513,144
#elif __IS_COMPILER_GCC__
#   pragma GCC diagnostic pop
#endif

#ifdef   __cplusplus
}
#endif
