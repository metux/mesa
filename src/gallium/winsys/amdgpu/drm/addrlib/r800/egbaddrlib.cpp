/*
 * Copyright © 2014 Advanced Micro Devices, Inc.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NON-INFRINGEMENT. IN NO EVENT SHALL THE COPYRIGHT HOLDERS, AUTHORS
 * AND/OR ITS SUPPLIERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 */

/**
***************************************************************************************************
* @file  egbaddrlib.cpp
* @brief Contains the EgBasedAddrLib class implementation
***************************************************************************************************
*/

#include <stdint.h>

#include "egbaddrlib.h"

/**
***************************************************************************************************
*   EgBasedAddrLib::EgBasedAddrLib
*
*   @brief
*       Constructor
*
*   @note
*
***************************************************************************************************
*/
EgBasedAddrLib::EgBasedAddrLib(const AddrClient* pClient) :
    AddrLib(pClient),
    m_ranks(0),
    m_logicalBanks(0),
    m_bankInterleave(1)
{
}

/**
***************************************************************************************************
*   EgBasedAddrLib::~EgBasedAddrLib
*
*   @brief
*       Destructor
***************************************************************************************************
*/
EgBasedAddrLib::~EgBasedAddrLib()
{
}

/**
***************************************************************************************************
*   EgBasedAddrLib::DispatchComputeSurfaceInfo
*
*   @brief
*       Compute surface sizes include padded pitch,height,slices,total size in bytes,
*       meanwhile output suitable tile mode and base alignment might be changed in this
*       call as well. Results are returned through output parameters.
*
*   @return
*       TRUE if no error occurs
***************************************************************************************************
*/
BOOL_32 EgBasedAddrLib::DispatchComputeSurfaceInfo(
    const ADDR_COMPUTE_SURFACE_INFO_INPUT*  pIn,    ///< [in] input structure
    ADDR_COMPUTE_SURFACE_INFO_OUTPUT*       pOut    ///< [out] output structure
    ) const
{
    AddrTileMode        tileMode      = pIn->tileMode;
    uint32_t            bpp           = pIn->bpp;
    uint32_t            numSamples    = pIn->numSamples;
    uint32_t            numFrags      = ((pIn->numFrags == 0) ? numSamples : pIn->numFrags);
    uint32_t            pitch         = pIn->width;
    uint32_t            height        = pIn->height;
    uint32_t            numSlices     = pIn->numSlices;
    uint32_t            mipLevel      = pIn->mipLevel;
    ADDR_SURFACE_FLAGS  flags         = pIn->flags;

    ADDR_TILEINFO       tileInfoDef   = {0};
    ADDR_TILEINFO*      pTileInfo     = &tileInfoDef;

    uint32_t            padDims = 0;
    BOOL_32             valid;

    tileMode = DegradeLargeThickTile(tileMode, bpp);

    // Only override numSamples for NI above
    if (m_chipFamily >= ADDR_CHIP_FAMILY_NI)
    {
        if (numFrags != numSamples) // This means EQAA
        {
            // The real surface size needed is determined by number of fragments
            numSamples = numFrags;
        }

        // Save altered numSamples in pOut
        pOut->numSamples = numSamples;
    }

    // Caller makes sure pOut->pTileInfo is not NULL, see HwlComputeSurfaceInfo
    ADDR_ASSERT(pOut->pTileInfo);

    if (pOut->pTileInfo != NULL)
    {
        pTileInfo = pOut->pTileInfo;
    }

    // Set default values
    if (pIn->pTileInfo != NULL)
    {
        if (pTileInfo != pIn->pTileInfo)
        {
            *pTileInfo = *pIn->pTileInfo;
        }
    }
    else
    {
        memset(pTileInfo, 0, sizeof(ADDR_TILEINFO));
    }

    // For macro tile mode, we should calculate default tiling parameters
    HwlSetupTileInfo(tileMode,
                     flags,
                     bpp,
                     pitch,
                     height,
                     numSamples,
                     pIn->pTileInfo,
                     pTileInfo,
                     pIn->tileType,
                     pOut);

    if (flags.cube)
    {
        if (mipLevel == 0)
        {
            padDims = 2;
        }

        if (numSlices == 1)
        {
            // This is calculating one face, remove cube flag
            flags.cube = 0;
        }
    }

    switch (tileMode)
    {
        case ADDR_TM_LINEAR_GENERAL://fall through
        case ADDR_TM_LINEAR_ALIGNED:
            valid = ComputeSurfaceInfoLinear(pIn, pOut, padDims);
            break;

        case ADDR_TM_1D_TILED_THIN1://fall through
        case ADDR_TM_1D_TILED_THICK:
            valid = ComputeSurfaceInfoMicroTiled(pIn, pOut, padDims, tileMode);
            break;

        case ADDR_TM_2D_TILED_THIN1:    //fall through
        case ADDR_TM_2D_TILED_THICK:    //fall through
        case ADDR_TM_3D_TILED_THIN1:    //fall through
        case ADDR_TM_3D_TILED_THICK:    //fall through
        case ADDR_TM_2D_TILED_XTHICK:   //fall through
        case ADDR_TM_3D_TILED_XTHICK:   //fall through
        case ADDR_TM_PRT_TILED_THIN1:   //fall through
        case ADDR_TM_PRT_2D_TILED_THIN1://fall through
        case ADDR_TM_PRT_3D_TILED_THIN1://fall through
        case ADDR_TM_PRT_TILED_THICK:   //fall through
        case ADDR_TM_PRT_2D_TILED_THICK://fall through
        case ADDR_TM_PRT_3D_TILED_THICK:
            valid = ComputeSurfaceInfoMacroTiled(pIn, pOut, padDims, tileMode);
            break;

        default:
            valid = FALSE;
            ADDR_ASSERT_ALWAYS();
            break;
    }

    return valid;
}

/**
***************************************************************************************************
*   EgBasedAddrLib::ComputeSurfaceInfoLinear
*
*   @brief
*       Compute linear surface sizes include padded pitch, height, slices, total size in
*       bytes, meanwhile alignments as well. Since it is linear mode, so output tile mode
*       will not be changed here. Results are returned through output parameters.
*
*   @return
*       TRUE if no error occurs
***************************************************************************************************
*/
BOOL_32 EgBasedAddrLib::ComputeSurfaceInfoLinear(
    const ADDR_COMPUTE_SURFACE_INFO_INPUT*  pIn,    ///< [in] Input structure
    ADDR_COMPUTE_SURFACE_INFO_OUTPUT*       pOut,   ///< [out] Output structure
    uint32_t                                padDims ///< [in] Dimensions to padd
    ) const
{
    uint32_t expPitch = pIn->width;
    uint32_t expHeight = pIn->height;
    uint32_t expNumSlices = pIn->numSlices;

    // No linear MSAA on real H/W, keep this for TGL
    uint32_t numSamples = pOut->numSamples;

    const uint32_t microTileThickness = 1;

    //
    // Compute the surface alignments.
    //
    ComputeSurfaceAlignmentsLinear(pIn->tileMode,
                                   pIn->bpp,
                                   pIn->flags,
                                   &pOut->baseAlign,
                                   &pOut->pitchAlign,
                                   &pOut->heightAlign);

    if ((pIn->tileMode == ADDR_TM_LINEAR_GENERAL) && pIn->flags.color && (pIn->height > 1))
    {
#if !ALT_TEST
        // When linear_general surface is accessed in multiple lines, it requires 8 pixels in pitch
        // alignment since PITCH_TILE_MAX is in unit of 8 pixels.
        // It is OK if it is accessed per line.
        ADDR_ASSERT((pIn->width % 8) == 0);
#endif
    }

    pOut->depthAlign = microTileThickness;

    expPitch = HwlPreHandleBaseLvl3xPitch(pIn, expPitch);

    //
    // Pad pitch and height to the required granularities.
    //
    PadDimensions(pIn->tileMode,
                  pIn->bpp,
                  pIn->flags,
                  numSamples,
                  pOut->pTileInfo,
                  padDims,
                  pIn->mipLevel,
                  &expPitch, pOut->pitchAlign,
                  &expHeight, pOut->heightAlign,
                  &expNumSlices, microTileThickness);

    expPitch = HwlPostHandleBaseLvl3xPitch(pIn, expPitch);

    //
    // Adjust per HWL
    //

    uint64_t logicalSliceSize;

    logicalSliceSize = HwlGetSizeAdjustmentLinear(pIn->tileMode,
                                                  pIn->bpp,
                                                  numSamples,
                                                  pOut->baseAlign,
                                                  pOut->pitchAlign,
                                                  &expPitch,
                                                  &expHeight,
                                                  &pOut->heightAlign);


    pOut->pitch = expPitch;
    pOut->height = expHeight;
    pOut->depth = expNumSlices;

    pOut->surfSize = logicalSliceSize * expNumSlices;

    pOut->tileMode = pIn->tileMode;

    return TRUE;
}

/**
***************************************************************************************************
*   EgBasedAddrLib::ComputeSurfaceInfoMicroTiled
*
*   @brief
*       Compute 1D/Micro Tiled surface sizes include padded pitch, height, slices, total
*       size in bytes, meanwhile alignments as well. Results are returned through output
*       parameters.
*
*   @return
*       TRUE if no error occurs
***************************************************************************************************
*/
BOOL_32 EgBasedAddrLib::ComputeSurfaceInfoMicroTiled(
    const ADDR_COMPUTE_SURFACE_INFO_INPUT*  pIn,        ///< [in] Input structure
    ADDR_COMPUTE_SURFACE_INFO_OUTPUT*       pOut,       ///< [out] Output structure
    uint32_t                                padDims,    ///< [in] Dimensions to padd
    AddrTileMode                            expTileMode ///< [in] Expected tile mode
    ) const
{
    BOOL_32 valid = TRUE;

    uint32_t microTileThickness;
    uint32_t expPitch = pIn->width;
    uint32_t expHeight = pIn->height;
    uint32_t expNumSlices = pIn->numSlices;

    // No 1D MSAA on real H/W, keep this for TGL
    uint32_t numSamples = pOut->numSamples;

    //
    // Compute the micro tile thickness.
    //
    microTileThickness = ComputeSurfaceThickness(expTileMode);

    //
    // Extra override for mip levels
    //
    if (pIn->mipLevel > 0)
    {
        //
        // Reduce tiling mode from thick to thin if the number of slices is less than the
        // micro tile thickness.
        //
        if ((expTileMode == ADDR_TM_1D_TILED_THICK) &&
            (expNumSlices < ThickTileThickness))
        {
            expTileMode = HwlDegradeThickTileMode(ADDR_TM_1D_TILED_THICK, expNumSlices, NULL);
            if (expTileMode != ADDR_TM_1D_TILED_THICK)
            {
                microTileThickness = 1;
            }
        }
    }

    //
    // Compute the surface restrictions.
    //
    ComputeSurfaceAlignmentsMicroTiled(expTileMode,
                                       pIn->bpp,
                                       pIn->flags,
                                       pIn->mipLevel,
                                       numSamples,
                                       &pOut->baseAlign,
                                       &pOut->pitchAlign,
                                       &pOut->heightAlign);

    pOut->depthAlign = microTileThickness;

    //
    // Pad pitch and height to the required granularities.
    // Compute surface size.
    // Return parameters.
    //
    PadDimensions(expTileMode,
                  pIn->bpp,
                  pIn->flags,
                  numSamples,
                  pOut->pTileInfo,
                  padDims,
                  pIn->mipLevel,
                  &expPitch, pOut->pitchAlign,
                  &expHeight, pOut->heightAlign,
                  &expNumSlices, microTileThickness);

    //
    // Get HWL specific pitch adjustment
    //
    uint64_t logicalSliceSize = HwlGetSizeAdjustmentMicroTiled(microTileThickness,
                                                              pIn->bpp,
                                                              pIn->flags,
                                                              numSamples,
                                                              pOut->baseAlign,
                                                              pOut->pitchAlign,
                                                              &expPitch,
                                                              &expHeight);


    pOut->pitch = expPitch;
    pOut->height = expHeight;
    pOut->depth = expNumSlices;

    pOut->surfSize = logicalSliceSize * expNumSlices;

    pOut->tileMode = expTileMode;

    return valid;
}


/**
***************************************************************************************************
*   EgBasedAddrLib::ComputeSurfaceInfoMacroTiled
*
*   @brief
*       Compute 2D/macro tiled surface sizes include padded pitch, height, slices, total
*       size in bytes, meanwhile output suitable tile mode and alignments might be changed
*       in this call as well. Results are returned through output parameters.
*
*   @return
*       TRUE if no error occurs
***************************************************************************************************
*/
BOOL_32 EgBasedAddrLib::ComputeSurfaceInfoMacroTiled(
    const ADDR_COMPUTE_SURFACE_INFO_INPUT*  pIn,        ///< [in] Input structure
    ADDR_COMPUTE_SURFACE_INFO_OUTPUT*       pOut,       ///< [out] Output structure
    uint32_t                                 padDims,    ///< [in] Dimensions to padd
    AddrTileMode                            expTileMode ///< [in] Expected tile mode
    ) const
{
    BOOL_32 valid = TRUE;

    AddrTileMode origTileMode = expTileMode;
    uint32_t microTileThickness;

    uint32_t paddedPitch;
    uint32_t paddedHeight;
    uint64_t bytesPerSlice;

    uint32_t expPitch     = pIn->width;
    uint32_t expHeight    = pIn->height;
    uint32_t expNumSlices = pIn->numSlices;

    uint32_t numSamples = pOut->numSamples;

    //
    // Compute the surface restrictions as base
    // SanityCheckMacroTiled is called in ComputeSurfaceAlignmentsMacroTiled
    //
    valid = ComputeSurfaceAlignmentsMacroTiled(expTileMode,
                                               pIn->bpp,
                                               pIn->flags,
                                               pIn->mipLevel,
                                               numSamples,
                                               pOut->pTileInfo,
                                               &pOut->baseAlign,
                                               &pOut->pitchAlign,
                                               &pOut->heightAlign);

    if (valid)
    {
        //
        // Compute the micro tile thickness.
        //
        microTileThickness = ComputeSurfaceThickness(expTileMode);

        //
        // Find the correct tiling mode for mip levels
        //
        if (pIn->mipLevel > 0)
        {
            //
            // Try valid tile mode
            //
            expTileMode = ComputeSurfaceMipLevelTileMode(expTileMode,
                                                         pIn->bpp,
                                                         expPitch,
                                                         expHeight,
                                                         expNumSlices,
                                                         numSamples,
                                                         pOut->pitchAlign,
                                                         pOut->heightAlign,
                                                         pOut->pTileInfo);

            if (!IsMacroTiled(expTileMode)) // Downgraded to micro-tiled
            {
                return ComputeSurfaceInfoMicroTiled(pIn, pOut, padDims, expTileMode);
            }
            else
            {
                if (microTileThickness != ComputeSurfaceThickness(expTileMode))
                {
                    //
                    // Re-compute if thickness changed since bank-height may be changed!
                    //
                    return ComputeSurfaceInfoMacroTiled(pIn, pOut, padDims, expTileMode);
                }
            }
        }

        paddedPitch     = expPitch;
        paddedHeight    = expHeight;

        //
        // Re-cal alignment
        //
        if (expTileMode != origTileMode) // Tile mode is changed but still macro-tiled
        {
            valid = ComputeSurfaceAlignmentsMacroTiled(expTileMode,
                                                       pIn->bpp,
                                                       pIn->flags,
                                                       pIn->mipLevel,
                                                       numSamples,
                                                       pOut->pTileInfo,
                                                       &pOut->baseAlign,
                                                       &pOut->pitchAlign,
                                                       &pOut->heightAlign);
        }

        //
        // Do padding
        //
        PadDimensions(expTileMode,
                      pIn->bpp,
                      pIn->flags,
                      numSamples,
                      pOut->pTileInfo,
                      padDims,
                      pIn->mipLevel,
                      &paddedPitch, pOut->pitchAlign,
                      &paddedHeight, pOut->heightAlign,
                      &expNumSlices, microTileThickness);

        if (pIn->flags.qbStereo &&
            (pOut->pStereoInfo != NULL) &&
            HwlStereoCheckRightOffsetPadding())
        {
            // Eye height's bank bits are different from y == 0?
            // Since 3D rendering treats right eye buffer starting from y == "eye height" while
            // display engine treats it to be 0, so the bank bits may be different, we pad
            // more in height to make sure y == "eye height" has the same bank bits as y == 0.
            uint32_t checkMask = pOut->pTileInfo->banks - 1;
            uint32_t bankBits = 0;
            do
            {
                bankBits = (paddedHeight / 8 / pOut->pTileInfo->bankHeight) & checkMask;

                if (bankBits)
                {
                   paddedHeight += pOut->heightAlign;
                }
            } while (bankBits);
        }

        //
        // Compute the size of a slice.
        //
        bytesPerSlice = BITS_TO_BYTES(static_cast<uint64_t>(paddedPitch) *
                                      paddedHeight * NextPow2(pIn->bpp) * numSamples);

        pOut->pitch = paddedPitch;
        // Put this check right here to workaround special mipmap cases which the original height
        // is needed.
        // The original height is pre-stored in pOut->height in PostComputeMipLevel and
        // pOut->pitch is needed in HwlCheckLastMacroTiledLvl, too.
        if (m_configFlags.checkLast2DLevel && numSamples == 1) // Don't check MSAA
        {
            // Set a TRUE in pOut if next Level is the first 1D sub level
            HwlCheckLastMacroTiledLvl(pIn, pOut);
        }
        pOut->height = paddedHeight;

        pOut->depth = expNumSlices;

        pOut->surfSize = bytesPerSlice * expNumSlices;

        pOut->tileMode = expTileMode;

        pOut->depthAlign = microTileThickness;

    } // if (valid)

    return valid;
}

/**
***************************************************************************************************
*   EgBasedAddrLib::ComputeSurfaceAlignmentsLinear
*
*   @brief
*       Compute linear surface alignment, calculation results are returned through
*       output parameters.
*
*   @return
*       TRUE if no error occurs
***************************************************************************************************
*/
BOOL_32 EgBasedAddrLib::ComputeSurfaceAlignmentsLinear(
    AddrTileMode        tileMode,          ///< [in] tile mode
    uint32_t            bpp,               ///< [in] bits per pixel
    ADDR_SURFACE_FLAGS  flags,             ///< [in] surface flags
    uint32_t*           pBaseAlign,        ///< [out] base address alignment in bytes
    uint32_t*           pPitchAlign,       ///< [out] pitch alignment in pixels
    uint32_t*           pHeightAlign       ///< [out] height alignment in pixels
    ) const
{
    BOOL_32 valid = TRUE;

    switch (tileMode)
    {
        case ADDR_TM_LINEAR_GENERAL:
            //
            // The required base alignment and pitch and height granularities is to 1 element.
            //
            *pBaseAlign   = (bpp > 8) ? bpp / 8 : 1;
            *pPitchAlign  = 1;
            *pHeightAlign = 1;
            break;
        case ADDR_TM_LINEAR_ALIGNED:
            //
            // The required alignment for base is the pipe interleave size.
            // The required granularity for pitch is hwl dependent.
            // The required granularity for height is one row.
            //
            *pBaseAlign     = m_pipeInterleaveBytes;
            *pPitchAlign    = HwlGetPitchAlignmentLinear(bpp, flags);
            *pHeightAlign   = 1;
            break;
        default:
            *pBaseAlign     = 1;
            *pPitchAlign    = 1;
            *pHeightAlign   = 1;
            ADDR_UNHANDLED_CASE();
            break;
    }

    AdjustPitchAlignment(flags, pPitchAlign);

    return valid;
}

/**
***************************************************************************************************
*   EgBasedAddrLib::ComputeSurfaceAlignmentsMicroTiled
*
*   @brief
*       Compute 1D tiled surface alignment, calculation results are returned through
*       output parameters.
*
*   @return
*       TRUE if no error occurs
***************************************************************************************************
*/
BOOL_32 EgBasedAddrLib::ComputeSurfaceAlignmentsMicroTiled(
    AddrTileMode        tileMode,          ///< [in] tile mode
    uint32_t            bpp,               ///< [in] bits per pixel
    ADDR_SURFACE_FLAGS  flags,             ///< [in] surface flags
    uint32_t            mipLevel,          ///< [in] mip level
    uint32_t            numSamples,        ///< [in] number of samples
    uint32_t*           pBaseAlign,        ///< [out] base address alignment in bytes
    uint32_t*           pPitchAlign,       ///< [out] pitch alignment in pixels
    uint32_t*           pHeightAlign       ///< [out] height alignment in pixels
    ) const
{
    BOOL_32 valid = TRUE;

    //
    // The required alignment for base is the pipe interleave size.
    //
    *pBaseAlign   = m_pipeInterleaveBytes;

    *pPitchAlign  = HwlGetPitchAlignmentMicroTiled(tileMode, bpp, flags, numSamples);

    *pHeightAlign = MicroTileHeight;

    AdjustPitchAlignment(flags, pPitchAlign);

    // ECR#393489
    // Workaround 2 for 1D tiling -  There is HW bug for Carrizo
    // where it requires the following alignments for 1D tiling.
    if (flags.czDispCompatible && (mipLevel == 0))
    {
        *pBaseAlign  = PowTwoAlign(*pBaseAlign, 4096);                         //Base address MOD 4096 = 0
        *pPitchAlign = PowTwoAlign(*pPitchAlign, 512 / (BITS_TO_BYTES(bpp))); //(8 lines * pitch * bytes per pixel) MOD 4096 = 0
    }
    // end Carrizo workaround for 1D tilling

    return valid;
}


/**
***************************************************************************************************
*   EgBasedAddrLib::HwlReduceBankWidthHeight
*
*   @brief
*       Additional checks, reduce bankHeight/bankWidth if needed and possible
*       tileSize*BANK_WIDTH*BANK_HEIGHT <= ROW_SIZE
*
*   @return
*       TRUE if no error occurs
***************************************************************************************************
*/
BOOL_32 EgBasedAddrLib::HwlReduceBankWidthHeight(
    uint32_t            tileSize,           ///< [in] tile size
    uint32_t            bpp,                ///< [in] bits per pixel
    ADDR_SURFACE_FLAGS  flags,              ///< [in] surface flags
    uint32_t            numSamples,         ///< [in] number of samples
    uint32_t            bankHeightAlign,    ///< [in] bank height alignment
    uint32_t            pipes,              ///< [in] pipes
    ADDR_TILEINFO*      pTileInfo           ///< [in/out] bank structure.
    ) const
{
    uint32_t macroAspectAlign;
    BOOL_32 valid = TRUE;

    if (tileSize * pTileInfo->bankWidth * pTileInfo->bankHeight > m_rowSize)
    {
        BOOL_32 stillGreater = TRUE;

        // Try reducing bankWidth first
        if (stillGreater && pTileInfo->bankWidth > 1)
        {
            while (stillGreater && pTileInfo->bankWidth > 0)
            {
                pTileInfo->bankWidth >>= 1;

                if (pTileInfo->bankWidth == 0)
                {
                    pTileInfo->bankWidth = 1;
                    break;
                }

                stillGreater =
                    tileSize * pTileInfo->bankWidth * pTileInfo->bankHeight > m_rowSize;
            }

            // bankWidth is reduced above, so we need to recalculate bankHeight and ratio
            bankHeightAlign = Max(1u,
                                  m_pipeInterleaveBytes * m_bankInterleave /
                                  (tileSize * pTileInfo->bankWidth)
                                  );

            // We cannot increase bankHeight so just assert this case.
            ADDR_ASSERT((pTileInfo->bankHeight % bankHeightAlign) == 0);

            if (numSamples == 1)
            {
                macroAspectAlign = Max(1u,
                                   m_pipeInterleaveBytes * m_bankInterleave /
                                   (tileSize * pipes * pTileInfo->bankWidth)
                                   );
                pTileInfo->macroAspectRatio = PowTwoAlign(pTileInfo->macroAspectRatio,
                                                          macroAspectAlign);
            }
        }

        // Early quit bank_height degradation for "64" bit z buffer
        if (flags.depth && bpp >= 64)
        {
            stillGreater = FALSE;
        }

        // Then try reducing bankHeight
        if (stillGreater && pTileInfo->bankHeight > bankHeightAlign)
        {
            while (stillGreater && pTileInfo->bankHeight > bankHeightAlign)
            {
                pTileInfo->bankHeight >>= 1;

                if (pTileInfo->bankHeight < bankHeightAlign)
                {
                    pTileInfo->bankHeight = bankHeightAlign;
                    break;
                }

                stillGreater =
                    tileSize * pTileInfo->bankWidth * pTileInfo->bankHeight > m_rowSize;
            }
        }

        valid = !stillGreater;

        // Generate a warning if we still fail to meet this constraint
        if (!valid)
        {
            ADDR_WARN(
                0, ("TILE_SIZE(%d)*BANK_WIDTH(%d)*BANK_HEIGHT(%d) <= ROW_SIZE(%d)",
                tileSize, pTileInfo->bankWidth, pTileInfo->bankHeight, m_rowSize));
        }
    }

    return valid;
}

/**
***************************************************************************************************
*   EgBasedAddrLib::ComputeSurfaceAlignmentsMacroTiled
*
*   @brief
*       Compute 2D tiled surface alignment, calculation results are returned through
*       output parameters.
*
*   @return
*       TRUE if no error occurs
***************************************************************************************************
*/
BOOL_32 EgBasedAddrLib::ComputeSurfaceAlignmentsMacroTiled(
    AddrTileMode        tileMode,           ///< [in] tile mode
    uint32_t            bpp,                ///< [in] bits per pixel
    ADDR_SURFACE_FLAGS  flags,              ///< [in] surface flags
    uint32_t            mipLevel,           ///< [in] mip level
    uint32_t            numSamples,         ///< [in] number of samples
    ADDR_TILEINFO*      pTileInfo,          ///< [in/out] bank structure.
    uint32_t*           pBaseAlign,         ///< [out] base address alignment in bytes
    uint32_t*           pPitchAlign,        ///< [out] pitch alignment in pixels
    uint32_t*           pHeightAlign        ///< [out] height alignment in pixels
    ) const
{
    BOOL_32 valid = SanityCheckMacroTiled(pTileInfo);

    if (valid)
    {
        uint32_t macroTileWidth;
        uint32_t macroTileHeight;

        uint32_t tileSize;
        uint32_t bankHeightAlign;
        uint32_t macroAspectAlign;

        uint32_t thickness = ComputeSurfaceThickness(tileMode);
        uint32_t pipes = HwlGetPipes(pTileInfo);

        //
        // Align bank height first according to latest h/w spec
        //

        // tile_size = MIN(tile_split, 64 * tile_thickness * element_bytes * num_samples)
        tileSize = Min(pTileInfo->tileSplitBytes,
                       BITS_TO_BYTES(64 * thickness * bpp * numSamples));

        // bank_height_align =
        // MAX(1, (pipe_interleave_bytes * bank_interleave)/(tile_size*bank_width))
        bankHeightAlign = Max(1u,
                              m_pipeInterleaveBytes * m_bankInterleave /
                              (tileSize * pTileInfo->bankWidth)
                              );

        pTileInfo->bankHeight = PowTwoAlign(pTileInfo->bankHeight, bankHeightAlign);

        // num_pipes * bank_width * macro_tile_aspect >=
        // (pipe_interleave_size * bank_interleave) / tile_size
        if (numSamples == 1)
        {
            // this restriction is only for mipmap (mipmap's numSamples must be 1)
            macroAspectAlign = Max(1u,
                               m_pipeInterleaveBytes * m_bankInterleave /
                               (tileSize * pipes * pTileInfo->bankWidth)
                               );
            pTileInfo->macroAspectRatio = PowTwoAlign(pTileInfo->macroAspectRatio, macroAspectAlign);
        }

        valid = HwlReduceBankWidthHeight(tileSize,
                                      bpp,
                                      flags,
                                      numSamples,
                                      bankHeightAlign,
                                      pipes,
                                      pTileInfo);

        //
        // The required granularity for pitch is the macro tile width.
        //
        macroTileWidth = MicroTileWidth * pTileInfo->bankWidth * pipes *
            pTileInfo->macroAspectRatio;

        *pPitchAlign = macroTileWidth;

        AdjustPitchAlignment(flags, pPitchAlign);

        //
        // The required granularity for height is the macro tile height.
        //
        macroTileHeight = MicroTileHeight * pTileInfo->bankHeight * pTileInfo->banks /
            pTileInfo->macroAspectRatio;

        *pHeightAlign = macroTileHeight;

        //
        // Compute base alignment
        //
        *pBaseAlign = pipes *
            pTileInfo->bankWidth * pTileInfo->banks * pTileInfo->bankHeight * tileSize;

        if ((mipLevel == 0) && (flags.prt) && (m_chipFamily == ADDR_CHIP_FAMILY_SI))
        {
            static const uint32_t PrtTileSize = 0x10000;

            uint32_t macroTileSize = macroTileWidth * macroTileHeight * numSamples * bpp / 8;

            if (macroTileSize < PrtTileSize)
            {
                uint32_t numMacroTiles = PrtTileSize / macroTileSize;

                ADDR_ASSERT((PrtTileSize % macroTileSize) == 0);

                *pPitchAlign *= numMacroTiles;
                *pBaseAlign  *= numMacroTiles;
            }
        }
    }

    return valid;
}

/**
***************************************************************************************************
*   EgBasedAddrLib::SanityCheckMacroTiled
*
*   @brief
*       Check if macro-tiled parameters are valid
*   @return
*       TRUE if valid
***************************************************************************************************
*/
BOOL_32 EgBasedAddrLib::SanityCheckMacroTiled(
    ADDR_TILEINFO* pTileInfo   ///< [in] macro-tiled parameters
    ) const
{
    BOOL_32 valid       = TRUE;
    uint32_t numPipes    = HwlGetPipes(pTileInfo);

    switch (pTileInfo->banks)
    {
        case 2: //fall through
        case 4: //fall through
        case 8: //fall through
        case 16:
            break;
        default:
            valid = FALSE;
            break;

    }

    if (valid)
    {
        switch (pTileInfo->bankWidth)
        {
            case 1: //fall through
            case 2: //fall through
            case 4: //fall through
            case 8:
                break;
            default:
                valid = FALSE;
                break;
        }
    }

    if (valid)
    {
        switch (pTileInfo->bankHeight)
        {
            case 1: //fall through
            case 2: //fall through
            case 4: //fall through
            case 8:
                break;
            default:
                valid = FALSE;
                break;
        }
    }

    if (valid)
    {
        switch (pTileInfo->macroAspectRatio)
        {
            case 1: //fall through
            case 2: //fall through
            case 4: //fall through
            case 8:
                break;
            default:
                valid = FALSE;
                break;
        }
    }

    if (valid)
    {
        if (pTileInfo->banks < pTileInfo->macroAspectRatio)
        {
            // This will generate macro tile height <= 1
            valid = FALSE;
        }
    }

    if (valid)
    {
        if (pTileInfo->tileSplitBytes > m_rowSize)
        {
            valid = FALSE;
        }
    }

    if (valid)
    {
        valid = HwlSanityCheckMacroTiled(pTileInfo);
    }

    ADDR_ASSERT(valid == TRUE);

    // Add this assert for guidance
    ADDR_ASSERT(numPipes * pTileInfo->banks >= 4);

    return valid;
}

/**
***************************************************************************************************
*   EgBasedAddrLib::ComputeSurfaceMipLevelTileMode
*
*   @brief
*       Compute valid tile mode for surface mipmap sub-levels
*
*   @return
*       Suitable tile mode
***************************************************************************************************
*/
AddrTileMode EgBasedAddrLib::ComputeSurfaceMipLevelTileMode(
    AddrTileMode        baseTileMode,   ///< [in] base tile mode
    uint32_t            bpp,            ///< [in] bits per pixels
    uint32_t            pitch,          ///< [in] current level pitch
    uint32_t            height,         ///< [in] current level height
    uint32_t            numSlices,      ///< [in] current number of slices
    uint32_t            numSamples,     ///< [in] number of samples
    uint32_t            pitchAlign,     ///< [in] pitch alignment
    uint32_t            heightAlign,    ///< [in] height alignment
    ADDR_TILEINFO*      pTileInfo       ///< [in] ptr to bank structure
    ) const
{
    uint32_t bytesPerTile;

    AddrTileMode expTileMode = baseTileMode;
    uint32_t microTileThickness = ComputeSurfaceThickness(expTileMode);
    uint32_t interleaveSize = m_pipeInterleaveBytes * m_bankInterleave;

    //
    // Compute the size of a slice.
    //
    bytesPerTile = BITS_TO_BYTES(MicroTilePixels * microTileThickness * NextPow2(bpp) * numSamples);

    //
    // Reduce tiling mode from thick to thin if the number of slices is less than the
    // micro tile thickness.
    //
    if (numSlices < microTileThickness)
    {
        expTileMode = HwlDegradeThickTileMode(expTileMode, numSlices, &bytesPerTile);
    }

    if (bytesPerTile > pTileInfo->tileSplitBytes)
    {
        bytesPerTile = pTileInfo->tileSplitBytes;
    }

    uint32_t threshold1 =
        bytesPerTile * HwlGetPipes(pTileInfo) * pTileInfo->bankWidth * pTileInfo->macroAspectRatio;

    uint32_t threshold2 =
        bytesPerTile * pTileInfo->bankWidth * pTileInfo->bankHeight;

    //
    // Reduce the tile mode from 2D/3D to 1D in following conditions
    //
    switch (expTileMode)
    {
        case ADDR_TM_2D_TILED_THIN1: //fall through
        case ADDR_TM_3D_TILED_THIN1:
        case ADDR_TM_PRT_TILED_THIN1:
        case ADDR_TM_PRT_2D_TILED_THIN1:
        case ADDR_TM_PRT_3D_TILED_THIN1:
            if ((pitch < pitchAlign) ||
                (height < heightAlign) ||
                (interleaveSize > threshold1) ||
                (interleaveSize > threshold2))
            {
                expTileMode = ADDR_TM_1D_TILED_THIN1;
            }
            break;
        case ADDR_TM_2D_TILED_THICK: //fall through
        case ADDR_TM_3D_TILED_THICK:
        case ADDR_TM_2D_TILED_XTHICK:
        case ADDR_TM_3D_TILED_XTHICK:
        case ADDR_TM_PRT_TILED_THICK:
        case ADDR_TM_PRT_2D_TILED_THICK:
        case ADDR_TM_PRT_3D_TILED_THICK:
            if ((pitch < pitchAlign) ||
                (height < heightAlign))
            {
                expTileMode = ADDR_TM_1D_TILED_THICK;
            }
            break;
        default:
            break;
    }

    return expTileMode;
}

/**
***************************************************************************************************
*   EgBasedAddrLib::HwlDegradeBaseLevel
*   @brief
*       Check if degrade is needed for base level
*   @return
*       TRUE if degrade is suggested
***************************************************************************************************
*/
BOOL_32 EgBasedAddrLib::HwlDegradeBaseLevel(
    const ADDR_COMPUTE_SURFACE_INFO_INPUT* pIn) const
{
    BOOL_32 degrade = FALSE;
    BOOL_32 valid = TRUE;

    ADDR_ASSERT(IsMacroTiled(pIn->tileMode));

    uint32_t baseAlign;
    uint32_t pitchAlign;
    uint32_t heightAlign;

    ADDR_ASSERT(pIn->pTileInfo);
    ADDR_TILEINFO tileInfo = *pIn->pTileInfo;
    ADDR_COMPUTE_SURFACE_INFO_OUTPUT out = {0};

    if (UseTileIndex(pIn->tileIndex))
    {
        out.tileIndex = pIn->tileIndex;
        out.macroModeIndex = TileIndexInvalid;
    }

    HwlSetupTileInfo(pIn->tileMode,
                     pIn->flags,
                     pIn->bpp,
                     pIn->width,
                     pIn->height,
                     pIn->numSamples,
                     &tileInfo,
                     &tileInfo,
                     pIn->tileType,
                     &out);

    valid = ComputeSurfaceAlignmentsMacroTiled(pIn->tileMode,
                                               pIn->bpp,
                                               pIn->flags,
                                               pIn->mipLevel,
                                               pIn->numSamples,
                                               &tileInfo,
                                               &baseAlign,
                                               &pitchAlign,
                                               &heightAlign);

    if (valid)
    {
        degrade = (pIn->width < pitchAlign || pIn->height < heightAlign);
    }
    else
    {
        degrade = TRUE;
    }

    return degrade;
}

/**
***************************************************************************************************
*   EgBasedAddrLib::HwlDegradeThickTileMode
*
*   @brief
*       Degrades valid tile mode for thick modes if needed
*
*   @return
*       Suitable tile mode
***************************************************************************************************
*/
AddrTileMode EgBasedAddrLib::HwlDegradeThickTileMode(
    AddrTileMode        baseTileMode,   ///< [in] base tile mode
    uint32_t            numSlices,      ///< [in] current number of slices
    uint32_t*           pBytesPerTile   ///< [in/out] pointer to bytes per slice
    ) const
{
    ADDR_ASSERT(numSlices < ComputeSurfaceThickness(baseTileMode));
    // if pBytesPerTile is NULL, this is a don't-care....
    uint32_t bytesPerTile = pBytesPerTile != NULL ? *pBytesPerTile : 64;

    AddrTileMode expTileMode = baseTileMode;
    switch (baseTileMode)
    {
        case ADDR_TM_1D_TILED_THICK:
            expTileMode = ADDR_TM_1D_TILED_THIN1;
            bytesPerTile >>= 2;
            break;
        case ADDR_TM_2D_TILED_THICK:
            expTileMode = ADDR_TM_2D_TILED_THIN1;
            bytesPerTile >>= 2;
            break;
        case ADDR_TM_3D_TILED_THICK:
            expTileMode = ADDR_TM_3D_TILED_THIN1;
            bytesPerTile >>= 2;
            break;
        case ADDR_TM_2D_TILED_XTHICK:
            if (numSlices < ThickTileThickness)
            {
                expTileMode = ADDR_TM_2D_TILED_THIN1;
                bytesPerTile >>= 3;
            }
            else
            {
                expTileMode = ADDR_TM_2D_TILED_THICK;
                bytesPerTile >>= 1;
            }
            break;
        case ADDR_TM_3D_TILED_XTHICK:
            if (numSlices < ThickTileThickness)
            {
                expTileMode = ADDR_TM_3D_TILED_THIN1;
                bytesPerTile >>= 3;
            }
            else
            {
                expTileMode = ADDR_TM_3D_TILED_THICK;
                bytesPerTile >>= 1;
            }
            break;
        default:
            ADDR_ASSERT_ALWAYS();
            break;
    }

    if (pBytesPerTile != NULL)
    {
        *pBytesPerTile = bytesPerTile;
    }

    return expTileMode;
}

/**
***************************************************************************************************
*   EgBasedAddrLib::DispatchComputeSurfaceAddrFromCoord
*
*   @brief
*       Compute surface address from given coord (x, y, slice,sample)
*
*   @return
*       Address in bytes
***************************************************************************************************
*/
uint64_t EgBasedAddrLib::DispatchComputeSurfaceAddrFromCoord(
    const ADDR_COMPUTE_SURFACE_ADDRFROMCOORD_INPUT* pIn,    ///< [in] input structure
    ADDR_COMPUTE_SURFACE_ADDRFROMCOORD_OUTPUT*      pOut    ///< [out] output structure
    ) const
{
    uint32_t             x                 = pIn->x;
    uint32_t             y                 = pIn->y;
    uint32_t             slice             = pIn->slice;
    uint32_t             sample            = pIn->sample;
    uint32_t             bpp               = pIn->bpp;
    uint32_t             pitch             = pIn->pitch;
    uint32_t             height            = pIn->height;
    uint32_t             numSlices         = pIn->numSlices;
    uint32_t             numSamples        = ((pIn->numSamples == 0) ? 1 : pIn->numSamples);
    uint32_t             numFrags          = ((pIn->numFrags == 0) ? numSamples : pIn->numFrags);
    AddrTileMode        tileMode           = pIn->tileMode;
    AddrTileType        microTileType      = pIn->tileType;
    BOOL_32             ignoreSE           = pIn->ignoreSE;
    BOOL_32             isDepthSampleOrder = pIn->isDepth;
    ADDR_TILEINFO*      pTileInfo          = pIn->pTileInfo;

    uint32_t*            pBitPosition      = &pOut->bitPosition;
    uint64_t             addr;

#if ADDR_AM_BUILD
    uint32_t             addr5Bit          = 0;
    uint32_t             addr5Swizzle      = pIn->addr5Swizzle;
    BOOL_32             is32ByteTile       = pIn->is32ByteTile;
#endif

    // ADDR_DEPTH_SAMPLE_ORDER = non-disp + depth-sample-order
    if (microTileType == ADDR_DEPTH_SAMPLE_ORDER)
    {
        isDepthSampleOrder = TRUE;
    }

    if (m_chipFamily >= ADDR_CHIP_FAMILY_NI)
    {
        if (numFrags != numSamples)
        {
            numSamples = numFrags;
            ADDR_ASSERT(sample < numSamples);
        }

        /// @note
        /// 128 bit/thick tiled surface doesn't support display tiling and
        /// mipmap chain must have the same tileType, so please fill tileType correctly
        if (!IsLinear(pIn->tileMode))
        {
            if (bpp >= 128 || ComputeSurfaceThickness(tileMode) > 1)
            {
                ADDR_ASSERT(microTileType != ADDR_DISPLAYABLE);
            }
        }
    }

    switch (tileMode)
    {
        case ADDR_TM_LINEAR_GENERAL://fall through
        case ADDR_TM_LINEAR_ALIGNED:
            addr = ComputeSurfaceAddrFromCoordLinear(x,
                                                     y,
                                                     slice,
                                                     sample,
                                                     bpp,
                                                     pitch,
                                                     height,
                                                     numSlices,
                                                     pBitPosition);
            break;
        case ADDR_TM_1D_TILED_THIN1://fall through
        case ADDR_TM_1D_TILED_THICK:
            addr = ComputeSurfaceAddrFromCoordMicroTiled(x,
                                                         y,
                                                         slice,
                                                         sample,
                                                         bpp,
                                                         pitch,
                                                         height,
                                                         numSamples,
                                                         tileMode,
                                                         microTileType,
                                                         isDepthSampleOrder,
                                                         pBitPosition);
            break;
        case ADDR_TM_2D_TILED_THIN1:    //fall through
        case ADDR_TM_2D_TILED_THICK:    //fall through
        case ADDR_TM_3D_TILED_THIN1:    //fall through
        case ADDR_TM_3D_TILED_THICK:    //fall through
        case ADDR_TM_2D_TILED_XTHICK:   //fall through
        case ADDR_TM_3D_TILED_XTHICK:   //fall through
        case ADDR_TM_PRT_TILED_THIN1:   //fall through
        case ADDR_TM_PRT_2D_TILED_THIN1://fall through
        case ADDR_TM_PRT_3D_TILED_THIN1://fall through
        case ADDR_TM_PRT_TILED_THICK:   //fall through
        case ADDR_TM_PRT_2D_TILED_THICK://fall through
        case ADDR_TM_PRT_3D_TILED_THICK:
            uint32_t pipeSwizzle;
            uint32_t bankSwizzle;

            if (m_configFlags.useCombinedSwizzle)
            {
                ExtractBankPipeSwizzle(pIn->tileSwizzle, pIn->pTileInfo,
                                       &bankSwizzle, &pipeSwizzle);
            }
            else
            {
                pipeSwizzle = pIn->pipeSwizzle;
                bankSwizzle = pIn->bankSwizzle;
            }

            addr = ComputeSurfaceAddrFromCoordMacroTiled(x,
                                                         y,
                                                         slice,
                                                         sample,
                                                         bpp,
                                                         pitch,
                                                         height,
                                                         numSamples,
                                                         tileMode,
                                                         microTileType,
                                                         ignoreSE,
                                                         isDepthSampleOrder,
                                                         pipeSwizzle,
                                                         bankSwizzle,
                                                         pTileInfo,
                                                         pBitPosition);
            break;
        default:
            addr = 0;
            ADDR_ASSERT_ALWAYS();
            break;
    }

#if ADDR_AM_BUILD
    if (m_chipFamily >= ADDR_CHIP_FAMILY_NI)
    {
        if (addr5Swizzle && isDepthSampleOrder && is32ByteTile)
        {
            uint32_t tx = x >> 3;
            uint32_t ty = y >> 3;
            uint32_t tileBits = ((ty&0x3) << 2) | (tx&0x3);

            tileBits = tileBits & addr5Swizzle;
            addr5Bit = XorReduce(tileBits, 4);

            addr = addr | static_cast<uint64_t>(addr5Bit << 5);
        }
    }
#endif

    return addr;
}

/**
***************************************************************************************************
*   EgBasedAddrLib::ComputeSurfaceAddrFromCoordMicroTiled
*
*   @brief
*       Computes the surface address and bit position from a
*       coordinate for 2D tilied (macro tiled)
*   @return
*       The byte address
***************************************************************************************************
*/
uint64_t EgBasedAddrLib::ComputeSurfaceAddrFromCoordMacroTiled(
    uint32_t             x,                     ///< [in] x coordinate
    uint32_t             y,                     ///< [in] y coordinate
    uint32_t             slice,                 ///< [in] slice index
    uint32_t             sample,                ///< [in] sample index
    uint32_t             bpp,                   ///< [in] bits per pixel
    uint32_t             pitch,                 ///< [in] surface pitch, in pixels
    uint32_t             height,                ///< [in] surface height, in pixels
    uint32_t             numSamples,            ///< [in] number of samples
    AddrTileMode        tileMode,               ///< [in] tile mode
    AddrTileType        microTileType,          ///< [in] micro tiling type
    BOOL_32             ignoreSE,               ///< [in] TRUE if shader enginers can be ignored
    BOOL_32             isDepthSampleOrder,     ///< [in] TRUE if it depth sample ordering is used
    uint32_t             pipeSwizzle,           ///< [in] pipe swizzle
    uint32_t             bankSwizzle,           ///< [in] bank swizzle
    ADDR_TILEINFO*      pTileInfo,              ///< [in] bank structure
                                                ///  **All fields to be valid on entry**
    uint32_t*            pBitPosition           ///< [out] bit position, e.g. FMT_1 will use this
    ) const
{
    uint64_t addr;

    uint32_t microTileBytes;
    uint32_t microTileBits;
    uint32_t sampleOffset;
    uint32_t pixelIndex;
    uint32_t pixelOffset;
    uint32_t elementOffset;
    uint32_t tileSplitSlice;
    uint32_t pipe;
    uint32_t bank;
    uint64_t sliceBytes;
    uint64_t sliceOffset;
    uint32_t macroTilePitch;
    uint32_t macroTileHeight;
    uint32_t macroTilesPerRow;
    uint32_t macroTilesPerSlice;
    uint64_t macroTileBytes;
    uint32_t macroTileIndexX;
    uint32_t macroTileIndexY;
    uint64_t macroTileOffset;
    uint64_t totalOffset;
    uint64_t pipeInterleaveMask;
    uint64_t bankInterleaveMask;
    uint64_t pipeInterleaveOffset;
    uint32_t bankInterleaveOffset;
    uint64_t offset;
    uint32_t tileRowIndex;
    uint32_t tileColumnIndex;
    uint32_t tileIndex;
    uint32_t tileOffset;

    uint32_t microTileThickness = ComputeSurfaceThickness(tileMode);

    //
    // Compute the number of group, pipe, and bank bits.
    //
    uint32_t numPipes              = HwlGetPipes(pTileInfo);
    uint32_t numPipeInterleaveBits = Log2(m_pipeInterleaveBytes);
    uint32_t numPipeBits           = Log2(numPipes);
    uint32_t numBankInterleaveBits = Log2(m_bankInterleave);
    uint32_t numBankBits           = Log2(pTileInfo->banks);

    //
    // Compute the micro tile size.
    //
    microTileBits = MicroTilePixels * microTileThickness * bpp * numSamples;

    microTileBytes = microTileBits / 8;
    //
    // Compute the pixel index within the micro tile.
    //
    pixelIndex = ComputePixelIndexWithinMicroTile(x,
                                                  y,
                                                  slice,
                                                  bpp,
                                                  tileMode,
                                                  microTileType);

    //
    // Compute the sample offset and pixel offset.
    //
    if (isDepthSampleOrder)
    {
        //
        // For depth surfaces, samples are stored contiguously for each element, so the sample
        // offset is the sample number times the element size.
        //
        sampleOffset = sample * bpp;
        pixelOffset  = pixelIndex * bpp * numSamples;
    }
    else
    {
        //
        // For color surfaces, all elements for a particular sample are stored contiguously, so
        // the sample offset is the sample number times the micro tile size divided yBit the number
        // of samples.
        //
        sampleOffset = sample * (microTileBits / numSamples);
        pixelOffset  = pixelIndex * bpp;
    }

    //
    // Compute the element offset.
    //
    elementOffset = pixelOffset + sampleOffset;

    *pBitPosition = static_cast<uint32_t>(elementOffset % 8);

    elementOffset /= 8; //bit-to-byte

    //
    // Determine if tiles need to be split across slices.
    //
    // If the size of the micro tile is larger than the tile split size, then the tile will be
    // split across multiple slices.
    //
    uint32_t slicesPerTile = 1;

    if ((microTileBytes > pTileInfo->tileSplitBytes) && (microTileThickness == 1))
    {   //don't support for thick mode

        //
        // Compute the number of slices per tile.
        //
        slicesPerTile = microTileBytes / pTileInfo->tileSplitBytes;

        //
        // Compute the tile split slice number for use in rotating the bank.
        //
        tileSplitSlice = elementOffset / pTileInfo->tileSplitBytes;

        //
        // Adjust the element offset to account for the portion of the tile that is being moved to
        // a new slice..
        //
        elementOffset %= pTileInfo->tileSplitBytes;

        //
        // Adjust the microTileBytes size to tileSplitBytes size since
        // a new slice..
        //
        microTileBytes = pTileInfo->tileSplitBytes;
    }
    else
    {
        tileSplitSlice = 0;
    }

    //
    // Compute macro tile pitch and height.
    //
    macroTilePitch  =
        (MicroTileWidth  * pTileInfo->bankWidth  * numPipes) * pTileInfo->macroAspectRatio;
    macroTileHeight =
        (MicroTileHeight * pTileInfo->bankHeight * pTileInfo->banks) / pTileInfo->macroAspectRatio;

    //
    // Compute the number of bytes per macro tile. Note: bytes of the same bank/pipe actually
    //
    macroTileBytes =
        static_cast<uint64_t>(microTileBytes) *
        (macroTilePitch / MicroTileWidth) * (macroTileHeight / MicroTileHeight) /
        (numPipes * pTileInfo->banks);

    //
    // Compute the number of macro tiles per row.
    //
    macroTilesPerRow = pitch / macroTilePitch;

    //
    // Compute the offset to the macro tile containing the specified coordinate.
    //
    macroTileIndexX = x / macroTilePitch;
    macroTileIndexY = y / macroTileHeight;
    macroTileOffset = ((macroTileIndexY * macroTilesPerRow) + macroTileIndexX) * macroTileBytes;

    //
    // Compute the number of macro tiles per slice.
    //
    macroTilesPerSlice = macroTilesPerRow  * (height / macroTileHeight);

    //
    // Compute the slice size.
    //
    sliceBytes = macroTilesPerSlice * macroTileBytes;

    //
    // Compute the slice offset.
    //
    sliceOffset = sliceBytes * (tileSplitSlice + slicesPerTile * (slice / microTileThickness));

    //
    // Compute tile offest
    //
    tileRowIndex    = (y / MicroTileHeight) % pTileInfo->bankHeight;
    tileColumnIndex = ((x / MicroTileWidth) / numPipes) % pTileInfo->bankWidth;
    tileIndex        = (tileRowIndex * pTileInfo->bankWidth) + tileColumnIndex;
    tileOffset       = tileIndex * microTileBytes;

    //
    // Combine the slice offset and macro tile offset with the pixel and sample offsets, accounting
    // for the pipe and bank bits in the middle of the address.
    //
    totalOffset = sliceOffset + macroTileOffset + elementOffset + tileOffset;

    //
    // Get the pipe and bank.
    //

    // when the tileMode is PRT type, then adjust x and y coordinates
    if (IsPrtNoRotationTileMode(tileMode))
    {
        x = x % macroTilePitch;
        y = y % macroTileHeight;
    }

    pipe = ComputePipeFromCoord(x,
                                y,
                                slice,
                                tileMode,
                                pipeSwizzle,
                                ignoreSE,
                                pTileInfo);

    bank = ComputeBankFromCoord(x,
                                y,
                                slice,
                                tileMode,
                                bankSwizzle,
                                tileSplitSlice,
                                pTileInfo);


    //
    // Split the offset to put some bits below the pipe+bank bits and some above.
    //
    pipeInterleaveMask = (1 << numPipeInterleaveBits) - 1;
    bankInterleaveMask = (1 << numBankInterleaveBits) - 1;
    pipeInterleaveOffset = totalOffset & pipeInterleaveMask;
    bankInterleaveOffset = static_cast<uint32_t>((totalOffset >> numPipeInterleaveBits) &
                                                bankInterleaveMask);
    offset               =  totalOffset >> (numPipeInterleaveBits + numBankInterleaveBits);

    //
    // Assemble the address from its components.
    //
    addr  = pipeInterleaveOffset;
    // This is to remove /analyze warnings
    uint32_t pipeBits            = pipe                 <<  numPipeInterleaveBits;
    uint32_t bankInterleaveBits  = bankInterleaveOffset << (numPipeInterleaveBits + numPipeBits);
    uint32_t bankBits            = bank                 << (numPipeInterleaveBits + numPipeBits +
                                                           numBankInterleaveBits);
    uint64_t offsetBits          = offset               << (numPipeInterleaveBits + numPipeBits +
                                                           numBankInterleaveBits + numBankBits);

    addr |= pipeBits;
    addr |= bankInterleaveBits;
    addr |= bankBits;
    addr |= offsetBits;

    return addr;
}

/**
***************************************************************************************************
*   EgBasedAddrLib::ComputeSurfaceAddrFromCoordMicroTiled
*
*   @brief
*       Computes the surface address and bit position from a coordinate for 1D tilied
*       (micro tiled)
*   @return
*       The byte address
***************************************************************************************************
*/
uint64_t EgBasedAddrLib::ComputeSurfaceAddrFromCoordMicroTiled(
    uint32_t             x,                     ///< [in] x coordinate
    uint32_t             y,                     ///< [in] y coordinate
    uint32_t             slice,                 ///< [in] slice index
    uint32_t             sample,                ///< [in] sample index
    uint32_t             bpp,                   ///< [in] bits per pixel
    uint32_t             pitch,                 ///< [in] pitch, in pixels
    uint32_t             height,                ///< [in] height, in pixels
    uint32_t             numSamples,            ///< [in] number of samples
    AddrTileMode        tileMode,               ///< [in] tile mode
    AddrTileType        microTileType,          ///< [in] micro tiling type
    BOOL_32             isDepthSampleOrder,     ///< [in] TRUE if depth sample ordering is used
    uint32_t*            pBitPosition           ///< [out] bit position, e.g. FMT_1 will use this
    ) const
{
    uint64_t addr = 0;

    uint32_t microTileBytes;
    uint64_t sliceBytes;
    uint32_t microTilesPerRow;
    uint32_t microTileIndexX;
    uint32_t microTileIndexY;
    uint32_t microTileIndexZ;
    uint64_t sliceOffset;
    uint64_t microTileOffset;
    uint32_t sampleOffset;
    uint32_t pixelIndex;
    uint32_t pixelOffset;

    uint32_t microTileThickness = ComputeSurfaceThickness(tileMode);

    //
    // Compute the micro tile size.
    //
    microTileBytes = BITS_TO_BYTES(MicroTilePixels * microTileThickness * bpp * numSamples);

    //
    // Compute the slice size.
    //
    sliceBytes =
        BITS_TO_BYTES(static_cast<uint64_t>(pitch) * height * microTileThickness * bpp * numSamples);

    //
    // Compute the number of micro tiles per row.
    //
    microTilesPerRow = pitch / MicroTileWidth;

    //
    // Compute the micro tile index.
    //
    microTileIndexX = x     / MicroTileWidth;
    microTileIndexY = y     / MicroTileHeight;
    microTileIndexZ = slice / microTileThickness;

    //
    // Compute the slice offset.
    //
    sliceOffset = static_cast<uint64_t>(microTileIndexZ) * sliceBytes;

    //
    // Compute the offset to the micro tile containing the specified coordinate.
    //
    microTileOffset = (static_cast<uint64_t>(microTileIndexY) * microTilesPerRow + microTileIndexX) *
        microTileBytes;

    //
    // Compute the pixel index within the micro tile.
    //
    pixelIndex = ComputePixelIndexWithinMicroTile(x,
                                                  y,
                                                  slice,
                                                  bpp,
                                                  tileMode,
                                                  microTileType);

    // Compute the sample offset.
    //
    if (isDepthSampleOrder)
    {
        //
        // For depth surfaces, samples are stored contiguously for each element, so the sample
        // offset is the sample number times the element size.
        //
        sampleOffset = sample * bpp;
        pixelOffset = pixelIndex * bpp * numSamples;
    }
    else
    {
        //
        // For color surfaces, all elements for a particular sample are stored contiguously, so
        // the sample offset is the sample number times the micro tile size divided yBit the number
        // of samples.
        //
        sampleOffset = sample * (microTileBytes*8 / numSamples);
        pixelOffset = pixelIndex * bpp;
    }

    //
    // Compute the bit position of the pixel.  Each element is stored with one bit per sample.
    //

    uint32_t elemOffset = sampleOffset + pixelOffset;

    *pBitPosition = elemOffset % 8;
    elemOffset /= 8;

    //
    // Combine the slice offset, micro tile offset, sample offset, and pixel offsets.
    //
    addr = sliceOffset + microTileOffset + elemOffset;

    return addr;
}

/**
***************************************************************************************************
*   EgBasedAddrLib::HwlComputePixelCoordFromOffset
*
*   @brief
*       Compute pixel coordinate from offset inside a micro tile
*   @return
*       N/A
***************************************************************************************************
*/
VOID EgBasedAddrLib::HwlComputePixelCoordFromOffset(
    uint32_t         offset,            ///< [in] offset inside micro tile in bits
    uint32_t         bpp,               ///< [in] bits per pixel
    uint32_t         numSamples,        ///< [in] number of samples
    AddrTileMode    tileMode,           ///< [in] tile mode
    uint32_t         tileBase,          ///< [in] base offset within a tile
    uint32_t         compBits,          ///< [in] component bits actually needed(for planar surface)
    uint32_t*        pX,                ///< [out] x coordinate
    uint32_t*        pY,                ///< [out] y coordinate
    uint32_t*        pSlice,            ///< [out] slice index
    uint32_t*        pSample,           ///< [out] sample index
    AddrTileType    microTileType,      ///< [in] micro tiling type
    BOOL_32         isDepthSampleOrder  ///< [in] TRUE if depth sample order in microtile is used
    ) const
{
    uint32_t x = 0;
    uint32_t y = 0;
    uint32_t z = 0;
    uint32_t thickness = ComputeSurfaceThickness(tileMode);

    // For planar surface, we adjust offset acoording to tile base
    if ((bpp != compBits) && (compBits != 0) && isDepthSampleOrder)
    {
        offset -= tileBase;

        ADDR_ASSERT(microTileType == ADDR_NON_DISPLAYABLE ||
                    microTileType == ADDR_DEPTH_SAMPLE_ORDER);

        bpp = compBits;
    }

    uint32_t sampleTileBits;
    uint32_t samplePixelBits;
    uint32_t pixelIndex;

    if (isDepthSampleOrder)
    {
        samplePixelBits = bpp * numSamples;
        pixelIndex = offset / samplePixelBits;
        *pSample = (offset % samplePixelBits) / bpp;
    }
    else
    {
        sampleTileBits = MicroTilePixels * bpp * thickness;
        *pSample = offset / sampleTileBits;
        pixelIndex = (offset % sampleTileBits) / bpp;
    }

    if (microTileType != ADDR_THICK)
    {
        if (microTileType == ADDR_DISPLAYABLE) // displayable
        {
            switch (bpp)
            {
                case 8:
                    x = pixelIndex & 0x7;
                    y = Bits2Number(3, _BIT(pixelIndex,5),_BIT(pixelIndex,3),_BIT(pixelIndex,4));
                    break;
                case 16:
                    x = pixelIndex & 0x7;
                    y = Bits2Number(3, _BIT(pixelIndex,5),_BIT(pixelIndex,4),_BIT(pixelIndex,3));
                    break;
                case 32:
                    x = Bits2Number(3, _BIT(pixelIndex,3),_BIT(pixelIndex,1),_BIT(pixelIndex,0));
                    y = Bits2Number(3, _BIT(pixelIndex,5),_BIT(pixelIndex,4),_BIT(pixelIndex,2));
                    break;
                case 64:
                    x = Bits2Number(3, _BIT(pixelIndex,3),_BIT(pixelIndex,2),_BIT(pixelIndex,0));
                    y = Bits2Number(3, _BIT(pixelIndex,5),_BIT(pixelIndex,4),_BIT(pixelIndex,1));
                    break;
                case 128:
                    x = Bits2Number(3, _BIT(pixelIndex,3),_BIT(pixelIndex,2),_BIT(pixelIndex,1));
                    y = Bits2Number(3, _BIT(pixelIndex,5),_BIT(pixelIndex,4),_BIT(pixelIndex,0));
                    break;
                default:
                    break;
            }
        }
        else if (microTileType == ADDR_NON_DISPLAYABLE || microTileType == ADDR_DEPTH_SAMPLE_ORDER)
        {
            x = Bits2Number(3, _BIT(pixelIndex,4),_BIT(pixelIndex,2),_BIT(pixelIndex,0));
            y = Bits2Number(3, _BIT(pixelIndex,5),_BIT(pixelIndex,3),_BIT(pixelIndex,1));
        }
        else if (microTileType == ADDR_ROTATED)
        {
            /*
                8-Bit Elements
                element_index[5:0] = { x[2], x[0], x[1], y[2], y[1], y[0] }

                16-Bit Elements
                element_index[5:0] = { x[2], x[1], x[0], y[2], y[1], y[0] }

                32-Bit Elements
                element_index[5:0] = { x[2], x[1], y[2], x[0], y[1], y[0] }

                64-Bit Elements
                element_index[5:0] = { y[2], x[2], x[1], y[1], x[0], y[0] }
            */
            switch(bpp)
            {
                case 8:
                    x = Bits2Number(3, _BIT(pixelIndex,5),_BIT(pixelIndex,3),_BIT(pixelIndex,4));
                    y = pixelIndex & 0x7;
                    break;
                case 16:
                    x = Bits2Number(3, _BIT(pixelIndex,5),_BIT(pixelIndex,4),_BIT(pixelIndex,3));
                    y = pixelIndex & 0x7;
                    break;
                case 32:
                    x = Bits2Number(3, _BIT(pixelIndex,5),_BIT(pixelIndex,4),_BIT(pixelIndex,2));
                    y = Bits2Number(3, _BIT(pixelIndex,3),_BIT(pixelIndex,1),_BIT(pixelIndex,0));
                    break;
                case 64:
                    x = Bits2Number(3, _BIT(pixelIndex,4),_BIT(pixelIndex,3),_BIT(pixelIndex,1));
                    y = Bits2Number(3, _BIT(pixelIndex,5),_BIT(pixelIndex,2),_BIT(pixelIndex,0));
                    break;
                default:
                    ADDR_ASSERT_ALWAYS();
                    break;
            }
        }

        if (thickness > 1) // thick
        {
            z = Bits2Number(3, _BIT(pixelIndex,8),_BIT(pixelIndex,7),_BIT(pixelIndex,6));
        }
    }
    else
    {
        ADDR_ASSERT((m_chipFamily >= ADDR_CHIP_FAMILY_CI) && (thickness > 1));
        /*
            8-Bit Elements and 16-Bit Elements
            element_index[7:0] = { y[2], x[2], z[1], z[0], y[1], x[1], y[0], x[0] }

            32-Bit Elements
            element_index[7:0] = { y[2], x[2], z[1], y[1], z[0], x[1], y[0], x[0] }

            64-Bit Elements and 128-Bit Elements
            element_index[7:0] = { y[2], x[2], z[1], y[1], x[1], z[0], y[0], x[0] }

            The equation to compute the element index for the extra thick tile:
            element_index[8] = z[2]
        */
        switch (bpp)
        {
            case 8:
            case 16: // fall-through
                x = Bits2Number(3, _BIT(pixelIndex,6),_BIT(pixelIndex,2),_BIT(pixelIndex,0));
                y = Bits2Number(3, _BIT(pixelIndex,7),_BIT(pixelIndex,3),_BIT(pixelIndex,1));
                z = Bits2Number(2, _BIT(pixelIndex,5),_BIT(pixelIndex,4));
                break;
            case 32:
                x = Bits2Number(3, _BIT(pixelIndex,6),_BIT(pixelIndex,2),_BIT(pixelIndex,0));
                y = Bits2Number(3, _BIT(pixelIndex,7),_BIT(pixelIndex,4),_BIT(pixelIndex,1));
                z = Bits2Number(2, _BIT(pixelIndex,5),_BIT(pixelIndex,3));
                break;
            case 64:
            case 128: // fall-through
                x = Bits2Number(3, _BIT(pixelIndex,6),_BIT(pixelIndex,3),_BIT(pixelIndex,0));
                y = Bits2Number(3, _BIT(pixelIndex,7),_BIT(pixelIndex,4),_BIT(pixelIndex,1));
                z = Bits2Number(2, _BIT(pixelIndex,5),_BIT(pixelIndex,2));
                break;
            default:
                ADDR_ASSERT_ALWAYS();
                break;
        }

        if (thickness == 8)
        {
            z += Bits2Number(3,_BIT(pixelIndex,8),0,0);
        }
    }

    *pX = x;
    *pY = y;
    *pSlice += z;
}


/**
***************************************************************************************************
*   EgBasedAddrLib::DispatchComputeSurfaceCoordFromAddrDispatch
*
*   @brief
*       Compute (x,y,slice,sample) coordinates from surface address
*   @return
*       N/A
***************************************************************************************************
*/
VOID EgBasedAddrLib::DispatchComputeSurfaceCoordFromAddr(
    const ADDR_COMPUTE_SURFACE_COORDFROMADDR_INPUT* pIn,    ///< [in] input structure
    ADDR_COMPUTE_SURFACE_COORDFROMADDR_OUTPUT*      pOut    ///< [out] output structure
    ) const
{
    uint64_t            addr               = pIn->addr;
    uint32_t            bitPosition        = pIn->bitPosition;
    uint32_t            bpp                = pIn->bpp;
    uint32_t            pitch              = pIn->pitch;
    uint32_t            height             = pIn->height;
    uint32_t            numSlices          = pIn->numSlices;
    uint32_t            numSamples         = ((pIn->numSamples == 0) ? 1 : pIn->numSamples);
    uint32_t            numFrags           = ((pIn->numFrags == 0) ? numSamples : pIn->numFrags);
    AddrTileMode        tileMode           = pIn->tileMode;
    uint32_t            tileBase           = pIn->tileBase;
    uint32_t            compBits           = pIn->compBits;
    AddrTileType        microTileType      = pIn->tileType;
    BOOL_32             ignoreSE           = pIn->ignoreSE;
    BOOL_32             isDepthSampleOrder = pIn->isDepth;
    ADDR_TILEINFO*      pTileInfo          = pIn->pTileInfo;

    uint32_t*           pX                 = &pOut->x;
    uint32_t*           pY                 = &pOut->y;
    uint32_t*           pSlice             = &pOut->slice;
    uint32_t*           pSample            = &pOut->sample;

    if (microTileType == ADDR_DEPTH_SAMPLE_ORDER)
    {
        isDepthSampleOrder = TRUE;
    }

    if (m_chipFamily >= ADDR_CHIP_FAMILY_NI)
    {
        if (numFrags != numSamples)
        {
            numSamples = numFrags;
        }

        /// @note
        /// 128 bit/thick tiled surface doesn't support display tiling and
        /// mipmap chain must have the same tileType, so please fill tileType correctly
        if (!IsLinear(pIn->tileMode))
        {
            if (bpp >= 128 || ComputeSurfaceThickness(tileMode) > 1)
            {
                ADDR_ASSERT(microTileType != ADDR_DISPLAYABLE);
            }
        }
    }

    switch (tileMode)
    {
        case ADDR_TM_LINEAR_GENERAL://fall through
        case ADDR_TM_LINEAR_ALIGNED:
            ComputeSurfaceCoordFromAddrLinear(addr,
                                              bitPosition,
                                              bpp,
                                              pitch,
                                              height,
                                              numSlices,
                                              pX,
                                              pY,
                                              pSlice,
                                              pSample);
            break;
        case ADDR_TM_1D_TILED_THIN1://fall through
        case ADDR_TM_1D_TILED_THICK:
            ComputeSurfaceCoordFromAddrMicroTiled(addr,
                                                  bitPosition,
                                                  bpp,
                                                  pitch,
                                                  height,
                                                  numSamples,
                                                  tileMode,
                                                  tileBase,
                                                  compBits,
                                                  pX,
                                                  pY,
                                                  pSlice,
                                                  pSample,
                                                  microTileType,
                                                  isDepthSampleOrder);
            break;
        case ADDR_TM_2D_TILED_THIN1:    //fall through
        case ADDR_TM_2D_TILED_THICK:    //fall through
        case ADDR_TM_3D_TILED_THIN1:    //fall through
        case ADDR_TM_3D_TILED_THICK:    //fall through
        case ADDR_TM_2D_TILED_XTHICK:   //fall through
        case ADDR_TM_3D_TILED_XTHICK:   //fall through
        case ADDR_TM_PRT_TILED_THIN1:   //fall through
        case ADDR_TM_PRT_2D_TILED_THIN1://fall through
        case ADDR_TM_PRT_3D_TILED_THIN1://fall through
        case ADDR_TM_PRT_TILED_THICK:   //fall through
        case ADDR_TM_PRT_2D_TILED_THICK://fall through
        case ADDR_TM_PRT_3D_TILED_THICK:
            uint32_t pipeSwizzle;
            uint32_t bankSwizzle;

            if (m_configFlags.useCombinedSwizzle)
            {
                ExtractBankPipeSwizzle(pIn->tileSwizzle, pIn->pTileInfo,
                                       &bankSwizzle, &pipeSwizzle);
            }
            else
            {
                pipeSwizzle = pIn->pipeSwizzle;
                bankSwizzle = pIn->bankSwizzle;
            }

            ComputeSurfaceCoordFromAddrMacroTiled(addr,
                                                  bitPosition,
                                                  bpp,
                                                  pitch,
                                                  height,
                                                  numSamples,
                                                  tileMode,
                                                  tileBase,
                                                  compBits,
                                                  microTileType,
                                                  ignoreSE,
                                                  isDepthSampleOrder,
                                                  pipeSwizzle,
                                                  bankSwizzle,
                                                  pTileInfo,
                                                  pX,
                                                  pY,
                                                  pSlice,
                                                  pSample);
            break;
        default:
            ADDR_ASSERT_ALWAYS();
    }
}


/**
***************************************************************************************************
*   EgBasedAddrLib::ComputeSurfaceCoordFromAddrMacroTiled
*
*   @brief
*       Compute surface coordinates from address for macro tiled surface
*   @return
*       N/A
***************************************************************************************************
*/
VOID EgBasedAddrLib::ComputeSurfaceCoordFromAddrMacroTiled(
    uint64_t            addr,               ///< [in] byte address
    uint32_t            bitPosition,        ///< [in] bit position
    uint32_t            bpp,                ///< [in] bits per pixel
    uint32_t            pitch,              ///< [in] pitch in pixels
    uint32_t            height,             ///< [in] height in pixels
    uint32_t            numSamples,         ///< [in] number of samples
    AddrTileMode        tileMode,           ///< [in] tile mode
    uint32_t            tileBase,           ///< [in] tile base offset
    uint32_t            compBits,           ///< [in] component bits (for planar surface)
    AddrTileType        microTileType,      ///< [in] micro tiling type
    BOOL_32             ignoreSE,           ///< [in] TRUE if shader engines can be ignored
    BOOL_32             isDepthSampleOrder, ///< [in] TRUE if depth sample order is used
    uint32_t            pipeSwizzle,        ///< [in] pipe swizzle
    uint32_t            bankSwizzle,        ///< [in] bank swizzle
    ADDR_TILEINFO*      pTileInfo,          ///< [in] bank structure.
                                            ///  **All fields to be valid on entry**
    uint32_t*           pX,                 ///< [out] X coord
    uint32_t*           pY,                 ///< [out] Y coord
    uint32_t*           pSlice,             ///< [out] slice index
    uint32_t*           pSample             ///< [out] sample index
    ) const
{
    uint32_t mx;
    uint32_t my;
    uint64_t tileBits;
    uint64_t macroTileBits;
    uint32_t slices;
    uint32_t tileSlices;
    uint64_t elementOffset;
    uint64_t macroTileIndex;
    uint32_t tileIndex;
    uint64_t totalOffset;


    uint32_t bank;
    uint32_t pipe;
    uint32_t groupBits = m_pipeInterleaveBytes << 3;
    uint32_t pipes = HwlGetPipes(pTileInfo);
    uint32_t banks = pTileInfo->banks;

    uint32_t bankInterleave = m_bankInterleave;

    uint64_t addrBits = BYTES_TO_BITS(addr) + bitPosition;

    //
    // remove bits for bank and pipe
    //
    totalOffset = (addrBits % groupBits) +
        (((addrBits / groupBits / pipes) % bankInterleave) * groupBits) +
        (((addrBits / groupBits / pipes) / bankInterleave) / banks) * groupBits * bankInterleave;

    uint32_t microTileThickness = ComputeSurfaceThickness(tileMode);

    uint32_t microTileBits = bpp * microTileThickness * MicroTilePixels * numSamples;

    uint32_t microTileBytes = BITS_TO_BYTES(microTileBits);
    //
    // Determine if tiles need to be split across slices.
    //
    // If the size of the micro tile is larger than the tile split size, then the tile will be
    // split across multiple slices.
    //
    uint32_t slicesPerTile = 1; //_State->TileSlices

    if ((microTileBytes > pTileInfo->tileSplitBytes) && (microTileThickness == 1))
    {   //don't support for thick mode

        //
        // Compute the number of slices per tile.
        //
        slicesPerTile = microTileBytes / pTileInfo->tileSplitBytes;
    }

    tileBits = microTileBits / slicesPerTile; // micro tile bits

    // in micro tiles because not MicroTileWidth timed.
    uint32_t macroWidth  = pTileInfo->bankWidth * pipes * pTileInfo->macroAspectRatio;
    // in micro tiles as well
    uint32_t macroHeight = pTileInfo->bankHeight * banks / pTileInfo->macroAspectRatio;

    uint32_t pitchInMacroTiles = pitch / MicroTileWidth / macroWidth;

    macroTileBits = (macroWidth * macroHeight) * tileBits / (banks * pipes);

    macroTileIndex = totalOffset / macroTileBits;

    // pitchMacros * height / heightMacros;  macroTilesPerSlice == _State->SliceMacros
    uint32_t macroTilesPerSlice = (pitch / (macroWidth * MicroTileWidth)) * height /
        (macroHeight * MicroTileWidth);

    slices = static_cast<uint32_t>(macroTileIndex / macroTilesPerSlice);

    *pSlice = static_cast<uint32_t>(slices / slicesPerTile * microTileThickness);

    //
    // calculate element offset and x[2:0], y[2:0], z[1:0] for thick
    //
    tileSlices = slices % slicesPerTile;

    elementOffset  = tileSlices * tileBits;
    elementOffset += totalOffset % tileBits;

    uint32_t coordZ = 0;

    HwlComputePixelCoordFromOffset(static_cast<uint32_t>(elementOffset),
                                   bpp,
                                   numSamples,
                                   tileMode,
                                   tileBase,
                                   compBits,
                                   pX,
                                   pY,
                                   &coordZ,
                                   pSample,
                                   microTileType,
                                   isDepthSampleOrder);

    macroTileIndex = macroTileIndex % macroTilesPerSlice;
    *pY += static_cast<uint32_t>(macroTileIndex / pitchInMacroTiles * macroHeight * MicroTileHeight);
    *pX += static_cast<uint32_t>(macroTileIndex % pitchInMacroTiles * macroWidth * MicroTileWidth);

    *pSlice += coordZ;

    tileIndex = static_cast<uint32_t>((totalOffset % macroTileBits) / tileBits);

    my = (tileIndex / pTileInfo->bankWidth) % pTileInfo->bankHeight * MicroTileHeight;
    mx = (tileIndex % pTileInfo->bankWidth) * pipes * MicroTileWidth;

    *pY += my;
    *pX += mx;

    bank = ComputeBankFromAddr(addr, banks, pipes);
    pipe = ComputePipeFromAddr(addr, pipes);

    HwlComputeSurfaceCoord2DFromBankPipe(tileMode,
                                         pX,
                                         pY,
                                         *pSlice,
                                         bank,
                                         pipe,
                                         bankSwizzle,
                                         pipeSwizzle,
                                         tileSlices,
                                         ignoreSE,
                                         pTileInfo);
}

/**
***************************************************************************************************
*   EgBasedAddrLib::ComputeSurfaceCoord2DFromBankPipe
*
*   @brief
*       Compute surface x,y coordinates from bank/pipe info
*   @return
*       N/A
***************************************************************************************************
*/
VOID EgBasedAddrLib::ComputeSurfaceCoord2DFromBankPipe(
    AddrTileMode        tileMode,   ///< [in] tile mode
    uint32_t            x,          ///< [in] x coordinate
    uint32_t            y,          ///< [in] y coordinate
    uint32_t            slice,      ///< [in] slice index
    uint32_t            bank,       ///< [in] bank number
    uint32_t            pipe,       ///< [in] pipe number
    uint32_t            bankSwizzle,///< [in] bank swizzle
    uint32_t            pipeSwizzle,///< [in] pipe swizzle
    uint32_t            tileSlices, ///< [in] slices in a micro tile
    ADDR_TILEINFO*      pTileInfo,  ///< [in] bank structure. **All fields to be valid on entry**
    CoordFromBankPipe*  pOutput     ///< [out] pointer to extracted x/y bits
    ) const
{
    uint32_t yBit3 = 0;
    uint32_t yBit4 = 0;
    uint32_t yBit5 = 0;
    uint32_t yBit6 = 0;

    uint32_t xBit3 = 0;
    uint32_t xBit4 = 0;
    uint32_t xBit5 = 0;

    uint32_t tileSplitRotation;

    uint32_t numPipes = HwlGetPipes(pTileInfo);

    uint32_t bankRotation = ComputeBankRotation(tileMode,
                                               pTileInfo->banks, numPipes);

    uint32_t pipeRotation = ComputePipeRotation(tileMode, numPipes);

    uint32_t xBit = x / (MicroTileWidth * pTileInfo->bankWidth * numPipes);
    uint32_t yBit = y / (MicroTileHeight * pTileInfo->bankHeight);

    //calculate the bank and pipe before rotation and swizzle

    switch (tileMode)
    {
        case ADDR_TM_2D_TILED_THIN1:  //fall through
        case ADDR_TM_2D_TILED_THICK:  //fall through
        case ADDR_TM_2D_TILED_XTHICK: //fall through
        case ADDR_TM_3D_TILED_THIN1:  //fall through
        case ADDR_TM_3D_TILED_THICK:  //fall through
        case ADDR_TM_3D_TILED_XTHICK:
            tileSplitRotation = ((pTileInfo->banks / 2) + 1);
            break;
        default:
            tileSplitRotation =  0;
            break;
    }

    uint32_t microTileThickness = ComputeSurfaceThickness(tileMode);

    bank ^= tileSplitRotation * tileSlices;
    if (pipeRotation == 0)
    {
        bank ^= bankRotation * (slice / microTileThickness) + bankSwizzle;
        bank %= pTileInfo->banks;
        pipe ^= pipeSwizzle;
    }
    else
    {
        bank ^= bankRotation * (slice / microTileThickness) / numPipes + bankSwizzle;
        bank %= pTileInfo->banks;
        pipe ^= pipeRotation * (slice / microTileThickness) + pipeSwizzle;
    }

    if (pTileInfo->macroAspectRatio == 1)
    {
        switch (pTileInfo->banks)
        {
            case 2:
                yBit3 = _BIT(bank, 0) ^ _BIT(xBit,0);
                break;
            case 4:
                yBit4 = _BIT(bank, 0) ^ _BIT(xBit,0);
                yBit3 = _BIT(bank, 1) ^ _BIT(xBit,1);
                break;
            case 8:
                yBit3 = _BIT(bank, 2) ^ _BIT(xBit,2);
                yBit5 = _BIT(bank, 0) ^ _BIT(xBit,0);
                yBit4 = _BIT(bank, 1) ^ _BIT(xBit,1) ^ yBit5;
                break;
            case 16:
                yBit3 = _BIT(bank, 3) ^ _BIT(xBit, 3);
                yBit4 = _BIT(bank, 2) ^ _BIT(xBit, 2);
                yBit6 = _BIT(bank, 0) ^ _BIT(xBit, 0);
                yBit5 = _BIT(bank, 1) ^ _BIT(xBit, 1) ^ yBit6;
                break;
            default:
                break;
        }

    }
    else if (pTileInfo->macroAspectRatio == 2)
    {
        switch (pTileInfo->banks)
        {
            case 2: //xBit3 = yBit3^b0
                xBit3 = _BIT(bank, 0) ^ _BIT(yBit,0);
                break;
            case 4: //xBit3=yBit4^b0; yBit3=xBit4^b1
                xBit3 = _BIT(bank, 0) ^ _BIT(yBit,1);
                yBit3 = _BIT(bank, 1) ^ _BIT(xBit,1);
                break;
            case 8: //xBit4, xBit5, yBit5 are known
                xBit3 = _BIT(bank, 0) ^ _BIT(yBit,2);
                yBit3 = _BIT(bank, 2) ^ _BIT(xBit,2);
                yBit4 = _BIT(bank, 1) ^ _BIT(xBit,1) ^ _BIT(yBit, 2);
                break;
            case 16://x4,x5,x6,y6 are known
                xBit3 = _BIT(bank, 0) ^ _BIT(yBit, 3); //x3 = y6 ^ b0
                yBit3 = _BIT(bank, 3) ^ _BIT(xBit, 3); //y3 = x6 ^ b3
                yBit4 = _BIT(bank, 2) ^ _BIT(xBit, 2); //y4 = x5 ^ b2
                yBit5 = _BIT(bank, 1) ^ _BIT(xBit, 1) ^ _BIT(yBit, 3); //y5=x4^y6^b1
                break;
            default:
                break;
        }
    }
    else if (pTileInfo->macroAspectRatio == 4)
    {
        switch (pTileInfo->banks)
        {
            case 4: //yBit3, yBit4
                xBit3 = _BIT(bank, 0) ^ _BIT(yBit,1);
                xBit4 = _BIT(bank, 1) ^ _BIT(yBit,0);
                break;
            case 8: //xBit5, yBit4, yBit5
                xBit3 = _BIT(bank, 0) ^ _BIT(yBit,2);
                yBit3 = _BIT(bank, 2) ^ _BIT(xBit,2);
                xBit4 = _BIT(bank, 1) ^ _BIT(yBit,1) ^  _BIT(yBit,2);
                break;
            case 16: //xBit5, xBit6, yBit5, yBit6
                xBit3 = _BIT(bank, 0) ^ _BIT(yBit, 3);//x3 = b0 ^ y6
                xBit4 = _BIT(bank, 1) ^ _BIT(yBit, 2) ^ _BIT(yBit, 3);//x4 = b1 ^ y5 ^ y6;
                yBit3 = _BIT(bank, 3) ^ _BIT(xBit, 3); //y3 = b3 ^ x6;
                yBit4 = _BIT(bank, 2) ^ _BIT(xBit, 2); //y4 = b2 ^ x5;
                break;
            default:
                break;
        }
    }
    else if (pTileInfo->macroAspectRatio == 8)
    {
        switch (pTileInfo->banks)
        {
            case 8: //yBit3, yBit4, yBit5
                xBit3 = _BIT(bank, 0) ^ _BIT(yBit,2); //x3 = b0 ^ y5;
                xBit4 = _BIT(bank, 1) ^ _BIT(yBit,1) ^ _BIT(yBit, 2);//x4 = b1 ^ y4 ^ y5;
                xBit5 = _BIT(bank, 2) ^ _BIT(yBit,0);
                break;
            case 16: //xBit6, yBit4, yBit5, yBit6
                xBit3 = _BIT(bank, 0) ^ _BIT(yBit, 3);//x3 = y6 ^ b0
                xBit4 = _BIT(bank, 1) ^ _BIT(yBit, 2) ^ _BIT(yBit, 3);//x4 = y5 ^ y6 ^ b1
                xBit5 = _BIT(bank, 2) ^ _BIT(yBit, 1);//x5 = y4 ^ b2
                yBit3 = _BIT(bank, 3) ^ _BIT(xBit, 3); //y3 = x6 ^ b3
                break;
            default:
                break;
        }
    }

    pOutput->xBits = xBit;
    pOutput->yBits = yBit;

    pOutput->xBit3 = xBit3;
    pOutput->xBit4 = xBit4;
    pOutput->xBit5 = xBit5;
    pOutput->yBit3 = yBit3;
    pOutput->yBit4 = yBit4;
    pOutput->yBit5 = yBit5;
    pOutput->yBit6 = yBit6;
}

/**
***************************************************************************************************
*   EgBasedAddrLib::HwlExtractBankPipeSwizzle
*   @brief
*       Entry of EgBasedAddrLib ExtractBankPipeSwizzle
*   @return
*       ADDR_E_RETURNCODE
***************************************************************************************************
*/
ADDR_E_RETURNCODE EgBasedAddrLib::HwlExtractBankPipeSwizzle(
    const ADDR_EXTRACT_BANKPIPE_SWIZZLE_INPUT*  pIn,   ///< [in] input structure
    ADDR_EXTRACT_BANKPIPE_SWIZZLE_OUTPUT*       pOut   ///< [out] output structure
    ) const
{
    ExtractBankPipeSwizzle(pIn->base256b,
                           pIn->pTileInfo,
                           &pOut->bankSwizzle,
                           &pOut->pipeSwizzle);

    return ADDR_OK;
}


/**
***************************************************************************************************
*   EgBasedAddrLib::HwlCombineBankPipeSwizzle
*   @brief
*       Combine bank/pipe swizzle
*   @return
*       ADDR_E_RETURNCODE
***************************************************************************************************
*/
ADDR_E_RETURNCODE EgBasedAddrLib::HwlCombineBankPipeSwizzle(
    uint32_t        bankSwizzle,    ///< [in] bank swizzle
    uint32_t        pipeSwizzle,    ///< [in] pipe swizzle
    ADDR_TILEINFO*  pTileInfo,      ///< [in] tile info
    uint64_t        baseAddr,       ///< [in] base address
    uint32_t*       pTileSwizzle    ///< [out] combined swizzle
    ) const
{
    ADDR_E_RETURNCODE retCode = ADDR_OK;

    if (pTileSwizzle)
    {
        *pTileSwizzle = GetBankPipeSwizzle(bankSwizzle, pipeSwizzle, baseAddr, pTileInfo);
    }
    else
    {
        retCode = ADDR_INVALIDPARAMS;
    }

    return retCode;
}

/**
***************************************************************************************************
*   EgBasedAddrLib::HwlComputeBaseSwizzle
*   @brief
*       Compute base swizzle
*   @return
*       ADDR_E_RETURNCODE
***************************************************************************************************
*/
ADDR_E_RETURNCODE EgBasedAddrLib::HwlComputeBaseSwizzle(
    const ADDR_COMPUTE_BASE_SWIZZLE_INPUT* pIn,
    ADDR_COMPUTE_BASE_SWIZZLE_OUTPUT* pOut
    ) const
{
    uint32_t bankSwizzle = 0;
    uint32_t pipeSwizzle = 0;
    ADDR_TILEINFO* pTileInfo = pIn->pTileInfo;

    ADDR_ASSERT(IsMacroTiled(pIn->tileMode));
    ADDR_ASSERT(pIn->pTileInfo);

    /// This is a legacy misreading of h/w doc, use it as it doesn't hurt.
    static const uint8_t bankRotationArray[4][16] = {
        { 0, 0,  0, 0,  0, 0,  0, 0, 0,  0, 0,  0, 0,  0, 0, 0 }, // ADDR_SURF_2_BANK
        { 0, 1,  2, 3,  0, 0,  0, 0, 0,  0, 0,  0, 0,  0, 0, 0 }, // ADDR_SURF_4_BANK
        { 0, 3,  6, 1,  4, 7,  2, 5, 0,  0, 0,  0, 0,  0, 0, 0 }, // ADDR_SURF_8_BANK
        { 0, 7, 14, 5, 12, 3, 10, 1, 8, 15, 6, 13, 4, 11, 2, 9 }, // ADDR_SURF_16_BANK
    };

    uint32_t banks = pTileInfo ? pTileInfo->banks : 2;
    uint32_t hwNumBanks;

    // Uses less bank swizzle bits
    if (pIn->option.reduceBankBit && banks > 2)
    {
        banks >>= 1;
    }

    switch (banks)
    {
        case 2:
            hwNumBanks = 0;
            break;
        case 4:
            hwNumBanks = 1;
            break;
        case 8:
            hwNumBanks = 2;
            break;
        case 16:
            hwNumBanks = 3;
            break;
        default:
            ADDR_ASSERT_ALWAYS();
            hwNumBanks = 0;
            break;
    }

    if (pIn->option.genOption == ADDR_SWIZZLE_GEN_LINEAR)
    {
        bankSwizzle = pIn->surfIndex & (banks - 1);
    }
    else // (pIn->option.genOption == ADDR_SWIZZLE_GEN_DEFAULT)
    {
        bankSwizzle = bankRotationArray[hwNumBanks][pIn->surfIndex & (banks - 1)];
    }

    if (IsMacro3dTiled(pIn->tileMode))
    {
        pipeSwizzle = pIn->surfIndex & (HwlGetPipes(pTileInfo) - 1);
    }

    return HwlCombineBankPipeSwizzle(bankSwizzle, pipeSwizzle, pTileInfo, 0, &pOut->tileSwizzle);
}

/**
***************************************************************************************************
*   EgBasedAddrLib::ExtractBankPipeSwizzle
*   @brief
*       Extract bank/pipe swizzle from base256b
*   @return
*       N/A
***************************************************************************************************
*/
VOID EgBasedAddrLib::ExtractBankPipeSwizzle(
    uint32_t        base256b,       ///< [in] input base256b register value
    ADDR_TILEINFO*  pTileInfo,      ///< [in] 2D tile parameters. Client must provide all data
    uint32_t*       pBankSwizzle,   ///< [out] bank swizzle
    uint32_t*       pPipeSwizzle    ///< [out] pipe swizzle
    ) const
{
    uint32_t bankSwizzle = 0;
    uint32_t pipeSwizzle = 0;

    if (base256b != 0)
    {
        uint32_t numPipes        = HwlGetPipes(pTileInfo);
        uint32_t bankBits        = QLog2(pTileInfo->banks);
        uint32_t pipeBits        = QLog2(numPipes);
        uint32_t groupBytes      = m_pipeInterleaveBytes;
        uint32_t bankInterleave  = m_bankInterleave;

        pipeSwizzle =
            (base256b / (groupBytes >> 8)) & ((1<<pipeBits)-1);

        bankSwizzle =
            (base256b / (groupBytes >> 8) / numPipes / bankInterleave) & ((1 << bankBits) - 1);
    }

    *pPipeSwizzle = pipeSwizzle;
    *pBankSwizzle = bankSwizzle;
}

/**
***************************************************************************************************
*   EgBasedAddrLib::GetBankPipeSwizzle
*   @brief
*       Combine bank/pipe swizzle
*   @return
*       Base256b bits (only filled bank/pipe bits)
***************************************************************************************************
*/
uint32_t EgBasedAddrLib::GetBankPipeSwizzle(
    uint32_t        bankSwizzle,    ///< [in] bank swizzle
    uint32_t        pipeSwizzle,    ///< [in] pipe swizzle
    uint64_t        baseAddr,       ///< [in] base address
    ADDR_TILEINFO*  pTileInfo       ///< [in] tile info
    ) const
{
    uint32_t pipeBits = QLog2(HwlGetPipes(pTileInfo));
    uint32_t bankInterleaveBits = QLog2(m_bankInterleave);
    uint32_t tileSwizzle = pipeSwizzle + ((bankSwizzle << bankInterleaveBits) << pipeBits);

    baseAddr ^= tileSwizzle * m_pipeInterleaveBytes;
    baseAddr >>= 8;

    return static_cast<uint32_t>(baseAddr);
}

/**
***************************************************************************************************
*   EgBasedAddrLib::ComputeSliceTileSwizzle
*   @brief
*       Compute cubemap/3d texture faces/slices tile swizzle
*   @return
*       Tile swizzle
***************************************************************************************************
*/
uint32_t EgBasedAddrLib::ComputeSliceTileSwizzle(
    AddrTileMode   tileMode,            ///< [in] Tile mode
    uint32_t       baseSwizzle,         ///< [in] Base swizzle
    uint32_t       slice,               ///< [in] Slice index, Cubemap face index, 0 means +X
    uint64_t       baseAddr,            ///< [in] Base address
    ADDR_TILEINFO* pTileInfo            ///< [in] Bank structure
    ) const
{
    uint32_t tileSwizzle = 0;

    if (IsMacroTiled(tileMode)) // Swizzle only for macro tile mode
    {
        uint32_t firstSlice = slice / ComputeSurfaceThickness(tileMode);

        uint32_t numPipes = HwlGetPipes(pTileInfo);
        uint32_t numBanks = pTileInfo->banks;

        uint32_t pipeRotation;
        uint32_t bankRotation;

        uint32_t bankSwizzle = 0;
        uint32_t pipeSwizzle = 0;

        pipeRotation = ComputePipeRotation(tileMode, numPipes);
        bankRotation = ComputeBankRotation(tileMode, numBanks, numPipes);

        if (baseSwizzle != 0)
        {
            ExtractBankPipeSwizzle(baseSwizzle,
                                   pTileInfo,
                                   &bankSwizzle,
                                   &pipeSwizzle);
        }

        if (pipeRotation == 0) //2D mode
        {
            bankSwizzle += firstSlice * bankRotation;
            bankSwizzle %= numBanks;
        }
        else //3D mode
        {
            pipeSwizzle += firstSlice * pipeRotation;
            pipeSwizzle %= numPipes;
            bankSwizzle += firstSlice * bankRotation / numPipes;
            bankSwizzle %= numBanks;
        }

        tileSwizzle = GetBankPipeSwizzle(bankSwizzle,
                                         pipeSwizzle,
                                         baseAddr,
                                         pTileInfo);
    }

    return tileSwizzle;
}

/**
***************************************************************************************************
*   EgBasedAddrLib::HwlComputeQbStereoRightSwizzle
*
*   @brief
*       Compute right eye swizzle
*   @return
*       swizzle
***************************************************************************************************
*/
uint32_t EgBasedAddrLib::HwlComputeQbStereoRightSwizzle(
    ADDR_COMPUTE_SURFACE_INFO_OUTPUT* pInfo  ///< [in] Surface info, must be valid
    ) const
{
    uint32_t bankBits    = 0;
    uint32_t swizzle     = 0;

    // The assumption is default swizzle for left eye is 0
    if (IsMacroTiled(pInfo->tileMode) && pInfo->pStereoInfo && pInfo->pTileInfo)
    {
        bankBits = ComputeBankFromCoord(0, pInfo->height, 0,
                                        pInfo->tileMode, 0, 0, pInfo->pTileInfo);

        if (bankBits)
        {
            HwlCombineBankPipeSwizzle(bankBits, 0, pInfo->pTileInfo, 0, &swizzle);
        }
    }

    return swizzle;
}

/**
***************************************************************************************************
*   EgBasedAddrLib::ComputeBankFromCoord
*
*   @brief
*       Compute bank number from coordinates
*   @return
*       Bank number
***************************************************************************************************
*/
uint32_t EgBasedAddrLib::ComputeBankFromCoord(
    uint32_t        x,              ///< [in] x coordinate
    uint32_t        y,              ///< [in] y coordinate
    uint32_t        slice,          ///< [in] slice index
    AddrTileMode    tileMode,       ///< [in] tile mode
    uint32_t        bankSwizzle,    ///< [in] bank swizzle
    uint32_t        tileSplitSlice, ///< [in] If the size of the pixel offset is larger than the
                                    ///  tile split size, then the pixel will be moved to a separate
                                    ///  slice. This value equals pixelOffset / tileSplitBytes
                                    ///  in this case. Otherwise this is 0.
    ADDR_TILEINFO*  pTileInfo       ///< [in] tile info
    ) const
{
    uint32_t pipes = HwlGetPipes(pTileInfo);
    uint32_t bankBit0 = 0;
    uint32_t bankBit1 = 0;
    uint32_t bankBit2 = 0;
    uint32_t bankBit3 = 0;
    uint32_t sliceRotation;
    uint32_t tileSplitRotation;
    uint32_t bank;
    uint32_t numBanks    = pTileInfo->banks;
    uint32_t bankWidth   = pTileInfo->bankWidth;
    uint32_t bankHeight  = pTileInfo->bankHeight;

    uint32_t tx = x / MicroTileWidth / (bankWidth * pipes);
    uint32_t ty = y / MicroTileHeight / bankHeight;

    uint32_t x3 = _BIT(tx,0);
    uint32_t x4 = _BIT(tx,1);
    uint32_t x5 = _BIT(tx,2);
    uint32_t x6 = _BIT(tx,3);
    uint32_t y3 = _BIT(ty,0);
    uint32_t y4 = _BIT(ty,1);
    uint32_t y5 = _BIT(ty,2);
    uint32_t y6 = _BIT(ty,3);

    switch (numBanks)
    {
        case 16:
            bankBit0 = x3 ^ y6;
            bankBit1 = x4 ^ y5 ^ y6;
            bankBit2 = x5 ^ y4;
            bankBit3 = x6 ^ y3;
            break;
        case 8:
            bankBit0 = x3 ^ y5;
            bankBit1 = x4 ^ y4 ^ y5;
            bankBit2 = x5 ^ y3;
            break;
        case 4:
            bankBit0 = x3 ^ y4;
            bankBit1 = x4 ^ y3;
            break;
        case 2:
            bankBit0 = x3 ^ y3;
            break;
        default:
            ADDR_ASSERT_ALWAYS();
            break;
    }

    bank = bankBit0 | (bankBit1 << 1) | (bankBit2 << 2) | (bankBit3 << 3);

    //Bits2Number(4, bankBit3, bankBit2, bankBit1, bankBit0);

    bank = HwlPreAdjustBank((x / MicroTileWidth), bank, pTileInfo);
    //
    // Compute bank rotation for the slice.
    //
    uint32_t microTileThickness = ComputeSurfaceThickness(tileMode);

    switch (tileMode)
    {
        case ADDR_TM_2D_TILED_THIN1:  // fall through
        case ADDR_TM_2D_TILED_THICK:  // fall through
        case ADDR_TM_2D_TILED_XTHICK:
            sliceRotation = ((numBanks / 2) - 1) * (slice / microTileThickness);
            break;
        case ADDR_TM_3D_TILED_THIN1:  // fall through
        case ADDR_TM_3D_TILED_THICK:  // fall through
        case ADDR_TM_3D_TILED_XTHICK:
            sliceRotation =
                Max(1u, (pipes / 2) - 1) * (slice / microTileThickness) / pipes;
            break;
        default:
            sliceRotation =  0;
            break;
    }


    //
    // Compute bank rotation for the tile split slice.
    //
    // The sample slice will be non-zero if samples must be split across multiple slices.
    // This situation arises when the micro tile size multiplied yBit the number of samples exceeds
    // the split size (set in GB_ADDR_CONFIG).
    //
    switch (tileMode)
    {
        case ADDR_TM_2D_TILED_THIN1: //fall through
        case ADDR_TM_3D_TILED_THIN1: //fall through
        case ADDR_TM_PRT_2D_TILED_THIN1: //fall through
        case ADDR_TM_PRT_3D_TILED_THIN1: //fall through
            tileSplitRotation = ((numBanks / 2) + 1) * tileSplitSlice;
            break;
        default:
            tileSplitRotation =  0;
            break;
    }

    //
    // Apply bank rotation for the slice and tile split slice.
    //
    bank ^= bankSwizzle + sliceRotation;
    bank ^= tileSplitRotation;

    bank &= (numBanks - 1);

    return bank;
}

/**
***************************************************************************************************
*   EgBasedAddrLib::ComputeBankFromAddr
*
*   @brief
*       Compute the bank number from an address
*   @return
*       Bank number
***************************************************************************************************
*/
uint32_t EgBasedAddrLib::ComputeBankFromAddr(
    uint64_t addr,      ///< [in] address
    uint32_t numBanks,  ///< [in] number of banks
    uint32_t numPipes   ///< [in] number of pipes
    ) const
{
    uint32_t bank;

    //
    // The LSBs of the address are arranged as follows:
    //   bank | bankInterleave | pipe | pipeInterleave
    //
    // To get the bank number, shift off the pipe interleave, pipe, and bank interlave bits and
    // mask the bank bits.
    //
    bank = static_cast<uint32_t>(
        (addr >> Log2(m_pipeInterleaveBytes * numPipes * m_bankInterleave)) &
        (numBanks - 1)
        );

    return bank;
}

/**
***************************************************************************************************
*   EgBasedAddrLib::ComputePipeRotation
*
*   @brief
*       Compute pipe rotation value
*   @return
*       Pipe rotation
***************************************************************************************************
*/
uint32_t EgBasedAddrLib::ComputePipeRotation(
    AddrTileMode tileMode,  ///< [in] tile mode
    uint32_t     numPipes   ///< [in] number of pipes
    ) const
{
   uint32_t rotation;

    switch (tileMode)
    {
        case ADDR_TM_3D_TILED_THIN1:        //fall through
        case ADDR_TM_3D_TILED_THICK:        //fall through
        case ADDR_TM_3D_TILED_XTHICK:       //fall through
        case ADDR_TM_PRT_3D_TILED_THIN1:    //fall through
        case ADDR_TM_PRT_3D_TILED_THICK:
            rotation = (numPipes < 4) ? 1 : (numPipes / 2 - 1);
            break;
        default:
            rotation = 0;
    }

    return rotation;
}



/**
***************************************************************************************************
*   EgBasedAddrLib::ComputeBankRotation
*
*   @brief
*       Compute bank rotation value
*   @return
*       Bank rotation
***************************************************************************************************
*/
uint32_t EgBasedAddrLib::ComputeBankRotation(
    AddrTileMode tileMode,  ///< [in] tile mode
    uint32_t     numBanks,  ///< [in] number of banks
    uint32_t     numPipes   ///< [in] number of pipes
    ) const
{
    uint32_t rotation;

    switch (tileMode)
    {
        case ADDR_TM_2D_TILED_THIN1: // fall through
        case ADDR_TM_2D_TILED_THICK: // fall through
        case ADDR_TM_2D_TILED_XTHICK:
        case ADDR_TM_PRT_2D_TILED_THIN1:
        case ADDR_TM_PRT_2D_TILED_THICK:
            // Rotate banks per Z-slice yBit 1 for 4-bank or 3 for 8-bank
            rotation =  numBanks / 2 - 1;
            break;
        case ADDR_TM_3D_TILED_THIN1: // fall through
        case ADDR_TM_3D_TILED_THICK: // fall through
        case ADDR_TM_3D_TILED_XTHICK:
        case ADDR_TM_PRT_3D_TILED_THIN1:
        case ADDR_TM_PRT_3D_TILED_THICK:
            rotation = (numPipes < 4) ? 1 : (numPipes / 2 - 1);    // rotate pipes & banks
            break;
        default:
            rotation = 0;
    }

    return rotation;
}


/**
***************************************************************************************************
*   EgBasedAddrLib::ComputeHtileBytes
*
*   @brief
*       Compute htile size in bytes
*
*   @return
*       Htile size in bytes
***************************************************************************************************
*/
uint64_t EgBasedAddrLib::ComputeHtileBytes(
    uint32_t  pitch,      ///< [in] pitch
    uint32_t  height,     ///< [in] height
    uint32_t  bpp,        ///< [in] bits per pixel
    BOOL_32   isLinear,   ///< [in] if it is linear mode
    uint32_t  numSlices,  ///< [in] number of slices
    uint64_t* sliceBytes, ///< [out] bytes per slice
    uint32_t  baseAlign   ///< [in] base alignments
    ) const
{
    uint64_t surfBytes;

    const uint64_t HtileCacheLineSize = BITS_TO_BYTES(HtileCacheBits);

    *sliceBytes = BITS_TO_BYTES(static_cast<uint64_t>(pitch) * height * bpp / 64);

    if (m_configFlags.useHtileSliceAlign)
    {
        // Align the sliceSize to htilecachelinesize * pipes at first
        *sliceBytes = PowTwoAlign(*sliceBytes, HtileCacheLineSize * m_pipes);
        surfBytes  = *sliceBytes * numSlices;
    }
    else
    {
        // Align the surfSize to htilecachelinesize * pipes at last
        surfBytes  = *sliceBytes * numSlices;
        surfBytes  = PowTwoAlign(surfBytes, HtileCacheLineSize * m_pipes);
    }

    return surfBytes;
}

/**
***************************************************************************************************
*   EgBasedAddrLib::DispatchComputeFmaskInfo
*
*   @brief
*       Compute fmask sizes include padded pitch, height, slices, total size in bytes,
*       meanwhile output suitable tile mode and alignments as well. Results are returned
*       through output parameters.
*
*   @return
*       ADDR_E_RETURNCODE
***************************************************************************************************
*/
ADDR_E_RETURNCODE EgBasedAddrLib::DispatchComputeFmaskInfo(
    const ADDR_COMPUTE_FMASK_INFO_INPUT*    pIn,   ///< [in] input structure
    ADDR_COMPUTE_FMASK_INFO_OUTPUT*         pOut)  ///< [out] output structure
{
    ADDR_E_RETURNCODE retCode = ADDR_OK;

    ADDR_COMPUTE_SURFACE_INFO_INPUT  surfIn     = {0};
    ADDR_COMPUTE_SURFACE_INFO_OUTPUT surfOut    = {0};

    // Setup input structure
    surfIn.tileMode          = pIn->tileMode;
    surfIn.width             = pIn->pitch;
    surfIn.height            = pIn->height;
    surfIn.numSlices         = pIn->numSlices;
    surfIn.pTileInfo         = pIn->pTileInfo;
    surfIn.tileType          = ADDR_NON_DISPLAYABLE;
    surfIn.flags.fmask       = 1;

    // Setup output structure
    surfOut.pTileInfo       = pOut->pTileInfo;

    // Setup hwl specific fields
    HwlFmaskPreThunkSurfInfo(pIn, pOut, &surfIn, &surfOut);

    surfIn.bpp = HwlComputeFmaskBits(pIn, &surfIn.numSamples);

    // ComputeSurfaceInfo needs numSamples in surfOut as surface routines need adjusted numSamples
    surfOut.numSamples = surfIn.numSamples;

    retCode = HwlComputeSurfaceInfo(&surfIn, &surfOut);

    // Save bpp field for surface dump support
    surfOut.bpp = surfIn.bpp;

    if (retCode == ADDR_OK)
    {
        pOut->bpp               = surfOut.bpp;
        pOut->pitch             = surfOut.pitch;
        pOut->height            = surfOut.height;
        pOut->numSlices         = surfOut.depth;
        pOut->fmaskBytes        = surfOut.surfSize;
        pOut->baseAlign         = surfOut.baseAlign;
        pOut->pitchAlign        = surfOut.pitchAlign;
        pOut->heightAlign       = surfOut.heightAlign;

        if (surfOut.depth > 1)
        {
            // For fmask, expNumSlices is stored in depth.
            pOut->sliceSize = surfOut.surfSize / surfOut.depth;
        }
        else
        {
            pOut->sliceSize = surfOut.surfSize;
        }

        // Save numSamples field for surface dump support
        pOut->numSamples        = surfOut.numSamples;

        HwlFmaskPostThunkSurfInfo(&surfOut, pOut);
    }

    return retCode;
}

/**
***************************************************************************************************
*   EgBasedAddrLib::HwlFmaskSurfaceInfo
*   @brief
*       Entry of EgBasedAddrLib ComputeFmaskInfo
*   @return
*       ADDR_E_RETURNCODE
***************************************************************************************************
*/
ADDR_E_RETURNCODE EgBasedAddrLib::HwlComputeFmaskInfo(
    const ADDR_COMPUTE_FMASK_INFO_INPUT*    pIn,   ///< [in] input structure
    ADDR_COMPUTE_FMASK_INFO_OUTPUT*         pOut   ///< [out] output structure
    )
{
    ADDR_E_RETURNCODE retCode = ADDR_OK;

    ADDR_TILEINFO tileInfo = {0};

    // Use internal tile info if pOut does not have a valid pTileInfo
    if (pOut->pTileInfo == NULL)
    {
        pOut->pTileInfo = &tileInfo;
    }

    retCode = DispatchComputeFmaskInfo(pIn, pOut);

    if (retCode == ADDR_OK)
    {
        pOut->tileIndex =
            HwlPostCheckTileIndex(pOut->pTileInfo, pIn->tileMode, ADDR_NON_DISPLAYABLE,
                                  pOut->tileIndex);
    }

    // Resets pTileInfo to NULL if the internal tile info is used
    if (pOut->pTileInfo == &tileInfo)
    {
        pOut->pTileInfo = NULL;
    }

    return retCode;
}

/**
***************************************************************************************************
*   EgBasedAddrLib::HwlComputeFmaskAddrFromCoord
*   @brief
*       Entry of EgBasedAddrLib ComputeFmaskAddrFromCoord
*   @return
*       ADDR_E_RETURNCODE
***************************************************************************************************
*/
ADDR_E_RETURNCODE EgBasedAddrLib::HwlComputeFmaskAddrFromCoord(
    const ADDR_COMPUTE_FMASK_ADDRFROMCOORD_INPUT*   pIn,    ///< [in] input structure
    ADDR_COMPUTE_FMASK_ADDRFROMCOORD_OUTPUT*        pOut    ///< [out] output structure
    ) const
{
    ADDR_E_RETURNCODE retCode = ADDR_OK;

#if ADDR_AM_BUILD
    if ((pIn->x > pIn->pitch)               ||
        (pIn->y > pIn->height)              ||
        (pIn->numSamples > m_maxSamples)    ||
        (pIn->sample >= m_maxSamples))
    {
        retCode = ADDR_INVALIDPARAMS;
    }
    else
    {
        pOut->addr = DispatchComputeFmaskAddrFromCoord(pIn, pOut);
    }
#endif

    return retCode;
}

/**
***************************************************************************************************
*   EgBasedAddrLib::HwlComputeFmaskCoordFromAddr
*   @brief
*       Entry of EgBasedAddrLib ComputeFmaskCoordFromAddr
*   @return
*       ADDR_E_RETURNCODE
***************************************************************************************************
*/
ADDR_E_RETURNCODE EgBasedAddrLib::HwlComputeFmaskCoordFromAddr(
    const ADDR_COMPUTE_FMASK_COORDFROMADDR_INPUT*   pIn,    ///< [in] input structure
    ADDR_COMPUTE_FMASK_COORDFROMADDR_OUTPUT*        pOut    ///< [out] output structure
    ) const
{
    ADDR_E_RETURNCODE retCode = ADDR_OK;

#if ADDR_AM_BUILD
    if ((pIn->bitPosition >= 8) ||
        (pIn->numSamples > m_maxSamples))
    {
        retCode = ADDR_INVALIDPARAMS;
    }
    else
    {
        DispatchComputeFmaskCoordFromAddr(pIn, pOut);
    }
#endif

    return retCode;
}

#if ADDR_AM_BUILD
/**
***************************************************************************************************
*   EgBasedAddrLib::DispatchComputeFmaskAddrFromCoord
*
*   @brief
*       Computes the FMASK address and bit position from a coordinate.
*   @return
*       The byte address
***************************************************************************************************
*/
uint64_t EgBasedAddrLib::DispatchComputeFmaskAddrFromCoord(
    const ADDR_COMPUTE_FMASK_ADDRFROMCOORD_INPUT*   pIn,    ///< [in] input structure
    ADDR_COMPUTE_FMASK_ADDRFROMCOORD_OUTPUT*        pOut    ///< [out] output structure
    ) const
{
    uint32_t            x                 = pIn->x;
    uint32_t            y                 = pIn->y;
    uint32_t            slice             = pIn->slice;
    uint32_t            sample            = pIn->sample;
    uint32_t            plane             = pIn->plane;
    uint32_t            pitch             = pIn->pitch;
    uint32_t            height            = pIn->height;
    uint32_t            numSamples        = pIn->numSamples;
    AddrTileMode        tileMode          = pIn->tileMode;
    BOOL_32             ignoreSE          = pIn->ignoreSE;
    ADDR_TILEINFO*      pTileInfo         = pIn->pTileInfo;
    BOOL_32             resolved          = pIn->resolved;

    uint32_t* pBitPosition = &pOut->bitPosition;
    uint64_t addr          = 0;

    ADDR_ASSERT(numSamples > 1);
    ADDR_ASSERT(ComputeSurfaceThickness(tileMode) == 1);

    switch (tileMode)
    {
        case ADDR_TM_1D_TILED_THIN1:
            addr = ComputeFmaskAddrFromCoordMicroTiled(x,
                                                       y,
                                                       slice,
                                                       sample,
                                                       plane,
                                                       pitch,
                                                       height,
                                                       numSamples,
                                                       tileMode,
                                                       resolved,
                                                       pBitPosition);
            break;
        case ADDR_TM_2D_TILED_THIN1: //fall through
        case ADDR_TM_3D_TILED_THIN1:
            uint32_t pipeSwizzle;
            uint32_t bankSwizzle;

            if (m_configFlags.useCombinedSwizzle)
            {
                ExtractBankPipeSwizzle(pIn->tileSwizzle, pIn->pTileInfo,
                                       &bankSwizzle, &pipeSwizzle);
            }
            else
            {
                pipeSwizzle = pIn->pipeSwizzle;
                bankSwizzle = pIn->bankSwizzle;
            }

            addr = ComputeFmaskAddrFromCoordMacroTiled(x,
                                                       y,
                                                       slice,
                                                       sample,
                                                       plane,
                                                       pitch,
                                                       height,
                                                       numSamples,
                                                       tileMode,
                                                       pipeSwizzle,
                                                       bankSwizzle,
                                                       ignoreSE,
                                                       pTileInfo,
                                                       resolved,
                                                       pBitPosition);
            break;
        default:
            *pBitPosition = 0;
            break;
    }

    return addr;
}

/**
***************************************************************************************************
*   EgBasedAddrLib::ComputeFmaskAddrFromCoordMicroTiled
*
*   @brief
*       Computes the FMASK address and bit position from a coordinate for 1D tilied (micro
*       tiled)
*   @return
*       The byte address
***************************************************************************************************
*/
uint64_t EgBasedAddrLib::ComputeFmaskAddrFromCoordMicroTiled(
    uint32_t            x,              ///< [in] x coordinate
    uint32_t            y,              ///< [in] y coordinate
    uint32_t            slice,          ///< [in] slice index
    uint32_t            sample,         ///< [in] sample number
    uint32_t            plane,          ///< [in] plane number
    uint32_t            pitch,          ///< [in] surface pitch in pixels
    uint32_t            height,         ///< [in] surface height in pixels
    uint32_t            numSamples,     ///< [in] number of samples
    AddrTileMode        tileMode,       ///< [in] tile mode
    BOOL_32             resolved,       ///< [in] TRUE if this is for resolved fmask
    uint32_t*           pBitPosition    ///< [out] pointer to returned bit position
    ) const
{
    uint64_t addr = 0;
    uint32_t effectiveBpp;
    uint32_t effectiveSamples;

    //
    // 2xAA use the same layout as 4xAA
    //
    if (numSamples == 2)
    {
        numSamples = 4;
    }

    //
    // Compute the number of planes.
    //
    if (!resolved)
    {
        effectiveSamples = ComputeFmaskNumPlanesFromNumSamples(numSamples);
        effectiveBpp = numSamples;

        //
        // Compute the address just like a color surface with numSamples bits per element and
        // numPlanes samples.
        //
        addr = ComputeSurfaceAddrFromCoordMicroTiled(x,
                                                     y,
                                                     slice,
                                                     plane, // sample
                                                     effectiveBpp,
                                                     pitch,
                                                     height,
                                                     effectiveSamples,
                                                     tileMode,
                                                     ADDR_NON_DISPLAYABLE,
                                                     FALSE,
                                                     pBitPosition);

        //
        // Compute the real bit position. Each (sample, plane) is stored with one bit per sample.
        //

        //
        // Compute the pixel index with in the micro tile
        //
        uint32_t pixelIndex = ComputePixelIndexWithinMicroTile(x % 8,
                                                              y % 8,
                                                              slice,
                                                              1,
                                                              tileMode,
                                                              ADDR_NON_DISPLAYABLE);

        *pBitPosition = ((pixelIndex * numSamples) + sample) & (BITS_PER_BYTE-1);

        uint64_t bitAddr = BYTES_TO_BITS(addr) + *pBitPosition;

        addr = bitAddr / 8;
    }
    else
    {
        effectiveBpp = ComputeFmaskResolvedBppFromNumSamples(numSamples);
        effectiveSamples = 1;

        //
        // Compute the address just like a color surface with numSamples bits per element and
        // numPlanes samples.
        //
        addr = ComputeSurfaceAddrFromCoordMicroTiled(x,
                                                     y,
                                                     slice,
                                                     sample,
                                                     effectiveBpp,
                                                     pitch,
                                                     height,
                                                     effectiveSamples,
                                                     tileMode,
                                                     ADDR_NON_DISPLAYABLE,
                                                     TRUE,
                                                     pBitPosition);
    }

    return addr;
}

/**
***************************************************************************************************
*   EgBasedAddrLib::ComputeFmaskAddrFromCoordMacroTiled
*
*   @brief
*       Computes the FMASK address and bit position from a coordinate for 2D tilied (macro
*       tiled)
*   @return
*       The byte address
***************************************************************************************************
*/
uint64_t EgBasedAddrLib::ComputeFmaskAddrFromCoordMacroTiled(
    uint32_t            x,              ///< [in] x coordinate
    uint32_t            y,              ///< [in] y coordinate
    uint32_t            slice,          ///< [in] slice index
    uint32_t            sample,         ///< [in] sample number
    uint32_t            plane,          ///< [in] plane number
    uint32_t            pitch,          ///< [in] surface pitch in pixels
    uint32_t            height,         ///< [in] surface height in pixels
    uint32_t            numSamples,     ///< [in] number of samples
    AddrTileMode        tileMode,       ///< [in] tile mode
    uint32_t            pipeSwizzle,    ///< [in] pipe swizzle
    uint32_t            bankSwizzle,    ///< [in] bank swizzle
    BOOL_32             ignoreSE,       ///< [in] TRUE if ignore shader engine
    ADDR_TILEINFO*      pTileInfo,      ///< [in] bank structure.**All fields to be valid on entry**
    BOOL_32             resolved,       ///< [in] TRUE if this is for resolved fmask
    uint32_t*           pBitPosition    ///< [out] pointer to returned bit position
    ) const
{
    uint64_t addr = 0;
    uint32_t effectiveBpp;
    uint32_t effectiveSamples;

    //
    // 2xAA use the same layout as 4xAA
    //
    if (numSamples == 2)
    {
        numSamples = 4;
    }

    //
    // Compute the number of planes.
    //
    if (!resolved)
    {
        effectiveSamples = ComputeFmaskNumPlanesFromNumSamples(numSamples);
        effectiveBpp = numSamples;

        //
        // Compute the address just like a color surface with numSamples bits per element and
        // numPlanes samples.
        //
        addr = ComputeSurfaceAddrFromCoordMacroTiled(x,
                                                     y,
                                                     slice,
                                                     plane, // sample
                                                     effectiveBpp,
                                                     pitch,
                                                     height,
                                                     effectiveSamples,
                                                     tileMode,
                                                     ADDR_NON_DISPLAYABLE,// isdisp
                                                     ignoreSE,// ignore_shader
                                                     FALSE,// depth_sample_order
                                                     pipeSwizzle,
                                                     bankSwizzle,
                                                     pTileInfo,
                                                     pBitPosition);

        //
        // Compute the real bit position. Each (sample, plane) is stored with one bit per sample.
        //


        //
        // Compute the pixel index with in the micro tile
        //
        uint32_t pixelIndex = ComputePixelIndexWithinMicroTile(x ,
                                                              y ,
                                                              slice,
                                                              effectiveBpp,
                                                              tileMode,
                                                              ADDR_NON_DISPLAYABLE);

        *pBitPosition = ((pixelIndex * numSamples) + sample) & (BITS_PER_BYTE-1);

        uint64_t bitAddr = BYTES_TO_BITS(addr) + *pBitPosition;

        addr = bitAddr / 8;

    }
    else
    {
        effectiveBpp = ComputeFmaskResolvedBppFromNumSamples(numSamples);
        effectiveSamples = 1;

        //
        // Compute the address just like a color surface with numSamples bits per element and
        // numPlanes samples.
        //
        addr = ComputeSurfaceAddrFromCoordMacroTiled(x,
                                                     y,
                                                     slice,
                                                     sample,
                                                     effectiveBpp,
                                                     pitch,
                                                     height,
                                                     effectiveSamples,
                                                     tileMode,
                                                     ADDR_NON_DISPLAYABLE,
                                                     ignoreSE,
                                                     TRUE,
                                                     pipeSwizzle,
                                                     bankSwizzle,
                                                     pTileInfo,
                                                     pBitPosition);
    }

    return addr;
}

/**
***************************************************************************************************
*   EgBasedAddrLib::ComputeFmaskCoordFromAddrMicroTiled
*
*   @brief
*       Compute (x,y,slice,sample,plane) coordinates from fmask address
*   @return
*       N/A
*
***************************************************************************************************
*/
VOID EgBasedAddrLib::ComputeFmaskCoordFromAddrMicroTiled(
    uint64_t            addr,       ///< [in] byte address
    uint32_t            bitPosition,///< [in] bit position
    uint32_t            pitch,      ///< [in] pitch in pixels
    uint32_t            height,     ///< [in] height in pixels
    uint32_t            numSamples, ///< [in] number of samples (of color buffer)
    AddrTileMode        tileMode,   ///< [in] tile mode
    BOOL_32             resolved,   ///< [in] TRUE if it is resolved fmask
    uint32_t*           pX,         ///< [out] X coord
    uint32_t*           pY,         ///< [out] Y coord
    uint32_t*           pSlice,     ///< [out] slice index
    uint32_t*           pSample,    ///< [out] sample index
    uint32_t*           pPlane      ///< [out] plane index
    ) const
{
    uint32_t effectiveBpp;
    uint32_t effectiveSamples;

    // 2xAA use the same layout as 4xAA
    if (numSamples == 2)
    {
        numSamples = 4;
    }

    if (!resolved)
    {
        effectiveSamples = ComputeFmaskNumPlanesFromNumSamples(numSamples);
        effectiveBpp  = numSamples;

        ComputeSurfaceCoordFromAddrMicroTiled(addr,
                                              bitPosition,
                                              effectiveBpp,
                                              pitch,
                                              height,
                                              effectiveSamples,
                                              tileMode,
                                              0, // tileBase
                                              0, // compBits
                                              pX,
                                              pY,
                                              pSlice,
                                              pPlane,
                                              ADDR_NON_DISPLAYABLE, // microTileType
                                              FALSE  // isDepthSampleOrder
                                              );


        if ( pSample )
        {
            *pSample = bitPosition % numSamples;
        }
    }
    else
    {
        effectiveBpp = ComputeFmaskResolvedBppFromNumSamples(numSamples);
        effectiveSamples = 1;

        ComputeSurfaceCoordFromAddrMicroTiled(addr,
                                              bitPosition,
                                              effectiveBpp,
                                              pitch,
                                              height,
                                              effectiveSamples,
                                              tileMode,
                                              0,     // tileBase
                                              0,     // compBits
                                              pX,
                                              pY,
                                              pSlice,
                                              pSample,
                                              ADDR_NON_DISPLAYABLE, // microTileType
                                              TRUE   // isDepthSampleOrder
                                              );
    }
}

/**
***************************************************************************************************
*   EgBasedAddrLib::ComputeFmaskCoordFromAddrMacroTiled
*
*   @brief
*       Compute (x,y,slice,sample,plane) coordinates from
*       fmask address
*   @return
*       N/A
*
***************************************************************************************************
*/
VOID EgBasedAddrLib::ComputeFmaskCoordFromAddrMacroTiled(
    uint64_t            addr,       ///< [in] byte address
    uint32_t            bitPosition,///< [in] bit position
    uint32_t            pitch,      ///< [in] pitch in pixels
    uint32_t            height,     ///< [in] height in pixels
    uint32_t            numSamples, ///< [in] number of samples (of color buffer)
    AddrTileMode        tileMode,   ///< [in] tile mode
    uint32_t            pipeSwizzle,///< [in] pipe swizzle
    uint32_t            bankSwizzle,///< [in] bank swizzle
    BOOL_32             ignoreSE,   ///< [in] TRUE if ignore shader engine
    ADDR_TILEINFO*      pTileInfo,  ///< [in] bank structure. **All fields to be valid on entry**
    BOOL_32             resolved,   ///< [in] TRUE if it is resolved fmask
    uint32_t*           pX,         ///< [out] X coord
    uint32_t*           pY,         ///< [out] Y coord
    uint32_t*           pSlice,     ///< [out] slice index
    uint32_t*           pSample,    ///< [out] sample index
    uint32_t*           pPlane      ///< [out] plane index
    ) const
{
    uint32_t effectiveBpp;
    uint32_t effectiveSamples;

    // 2xAA use the same layout as 4xAA
    if (numSamples == 2)
    {
        numSamples = 4;
    }

    //
    // Compute the number of planes.
    //
    if (!resolved)
    {
        effectiveSamples = ComputeFmaskNumPlanesFromNumSamples(numSamples);
        effectiveBpp  = numSamples;

        ComputeSurfaceCoordFromAddrMacroTiled(addr,
                                              bitPosition,
                                              effectiveBpp,
                                              pitch,
                                              height,
                                              effectiveSamples,
                                              tileMode,
                                              0, // No tileBase
                                              0, // No compBits
                                              ADDR_NON_DISPLAYABLE,
                                              ignoreSE,
                                              FALSE,
                                              pipeSwizzle,
                                              bankSwizzle,
                                              pTileInfo,
                                              pX,
                                              pY,
                                              pSlice,
                                              pPlane);

        if (pSample)
        {
            *pSample = bitPosition % numSamples;
        }
    }
    else
    {
        effectiveBpp = ComputeFmaskResolvedBppFromNumSamples(numSamples);
        effectiveSamples = 1;

        ComputeSurfaceCoordFromAddrMacroTiled(addr,
                                              bitPosition,
                                              effectiveBpp,
                                              pitch,
                                              height,
                                              effectiveSamples,
                                              tileMode,
                                              0, // No tileBase
                                              0, // No compBits
                                              ADDR_NON_DISPLAYABLE,
                                              ignoreSE,
                                              TRUE,
                                              pipeSwizzle,
                                              bankSwizzle,
                                              pTileInfo,
                                              pX,
                                              pY,
                                              pSlice,
                                              pSample);
    }
}

/**
***************************************************************************************************
*   EgBasedAddrLib::DispatchComputeFmaskCoordFromAddr
*
*   @brief
*       Compute (x,y,slice,sample,plane) coordinates from
*       fmask address
*   @return
*       N/A
*
***************************************************************************************************
*/
VOID EgBasedAddrLib::DispatchComputeFmaskCoordFromAddr(
    const ADDR_COMPUTE_FMASK_COORDFROMADDR_INPUT*   pIn,    ///< [in] input structure
    ADDR_COMPUTE_FMASK_COORDFROMADDR_OUTPUT*        pOut    ///< [out] output structure
    ) const
{
    uint64_t            addr              = pIn->addr;
    uint32_t            bitPosition       = pIn->bitPosition;
    uint32_t            pitch             = pIn->pitch;
    uint32_t            height            = pIn->height;
    uint32_t            numSamples        = pIn->numSamples;
    AddrTileMode        tileMode          = pIn->tileMode;
    BOOL_32             ignoreSE          = pIn->ignoreSE;
    ADDR_TILEINFO*      pTileInfo         = pIn->pTileInfo;
    BOOL_32             resolved          = pIn->resolved;

    uint32_t*           pX      = &pOut->x;
    uint32_t*           pY      = &pOut->y;
    uint32_t*           pSlice  = &pOut->slice;
    uint32_t*           pSample = &pOut->sample;
    uint32_t*           pPlane  = &pOut->plane;

    switch (tileMode)
    {
        case ADDR_TM_1D_TILED_THIN1:
            ComputeFmaskCoordFromAddrMicroTiled(addr,
                                                bitPosition,
                                                pitch,
                                                height,
                                                numSamples,
                                                tileMode,
                                                resolved,
                                                pX,
                                                pY,
                                                pSlice,
                                                pSample,
                                                pPlane);
            break;
        case ADDR_TM_2D_TILED_THIN1://fall through
        case ADDR_TM_3D_TILED_THIN1:
            uint32_t pipeSwizzle;
            uint32_t bankSwizzle;

            if (m_configFlags.useCombinedSwizzle)
            {
                ExtractBankPipeSwizzle(pIn->tileSwizzle, pIn->pTileInfo,
                                       &bankSwizzle, &pipeSwizzle);
            }
            else
            {
                pipeSwizzle = pIn->pipeSwizzle;
                bankSwizzle = pIn->bankSwizzle;
            }

            ComputeFmaskCoordFromAddrMacroTiled(addr,
                                                bitPosition,
                                                pitch,
                                                height,
                                                numSamples,
                                                tileMode,
                                                pipeSwizzle,
                                                bankSwizzle,
                                                ignoreSE,
                                                pTileInfo,
                                                resolved,
                                                pX,
                                                pY,
                                                pSlice,
                                                pSample,
                                                pPlane);
            break;
        default:
            ADDR_ASSERT_ALWAYS();
            break;

    }
}
#endif

/**
***************************************************************************************************
*   EgBasedAddrLib::ComputeFmaskNumPlanesFromNumSamples
*
*   @brief
*       Compute fmask number of planes from number of samples
*
*   @return
*       Number of planes
***************************************************************************************************
*/
uint32_t EgBasedAddrLib::ComputeFmaskNumPlanesFromNumSamples(
    uint32_t numSamples)     ///< [in] number of samples
{
    uint32_t numPlanes;

    //
    // FMASK is stored such that each micro tile is composed of elements containing N bits, where
    // N is the number of samples.  There is a micro tile for each bit in the FMASK address, and
    // micro tiles for each address bit, sometimes referred to as a plane, are stored sequentially.
    // The FMASK for a 2-sample surface looks like a general surface with 2 bits per element.
    // The FMASK for a 4-sample surface looks like a general surface with 4 bits per element and
    // 2 samples.  The FMASK for an 8-sample surface looks like a general surface with 8 bits per
    // element and 4 samples.  R6xx and R7xx only stored 3 planes for 8-sample FMASK surfaces.
    // This was changed for R8xx to simplify the logic in the CB.
    //
    switch (numSamples)
    {
        case 2:
            numPlanes = 1;
            break;
        case 4:
            numPlanes = 2;
            break;
        case 8:
            numPlanes = 4;
            break;
        default:
            ADDR_UNHANDLED_CASE();
            numPlanes = 0;
            break;
    }
    return numPlanes;
}

/**
***************************************************************************************************
*   EgBasedAddrLib::ComputeFmaskResolvedBppFromNumSamples
*
*   @brief
*       Compute resolved fmask effective bpp based on number of samples
*
*   @return
*       bpp
***************************************************************************************************
*/
uint32_t EgBasedAddrLib::ComputeFmaskResolvedBppFromNumSamples(
    uint32_t numSamples)     ///< number of samples
{
    uint32_t bpp;

    //
    // Resolved FMASK surfaces are generated yBit the CB and read yBit the texture unit
    // so that the texture unit can read compressed multi-sample color data.
    // These surfaces store each index value packed per element.
    // Each element contains at least num_samples * log2(num_samples) bits.
    // Resolved FMASK surfaces are addressed as follows:
    // 2-sample Addressed similarly to a color surface with 8 bits per element and 1 sample.
    // 4-sample Addressed similarly to a color surface with 8 bits per element and 1 sample.
    // 8-sample Addressed similarly to a color surface with 32 bits per element and 1 sample.

    switch (numSamples)
    {
        case 2:
            bpp = 8;
            break;
        case 4:
            bpp = 8;
            break;
        case 8:
            bpp = 32;
            break;
        default:
            ADDR_UNHANDLED_CASE();
            bpp = 0;
            break;
    }
    return bpp;
}

/**
***************************************************************************************************
*   EgBasedAddrLib::IsTileInfoAllZero
*
*   @brief
*       Return TRUE if all field are zero
*   @note
*       Since NULL input is consider to be all zero
***************************************************************************************************
*/
BOOL_32 EgBasedAddrLib::IsTileInfoAllZero(
    ADDR_TILEINFO* pTileInfo)
{
    BOOL_32 allZero = TRUE;

    if (pTileInfo)
    {
        if ((pTileInfo->banks            != 0)  ||
            (pTileInfo->bankWidth        != 0)  ||
            (pTileInfo->bankHeight       != 0)  ||
            (pTileInfo->macroAspectRatio != 0)  ||
            (pTileInfo->tileSplitBytes   != 0)  ||
            (pTileInfo->pipeConfig       != 0)
            )
        {
            allZero = FALSE;
        }
    }

    return allZero;
}

/**
***************************************************************************************************
*   EgBasedAddrLib::HwlTileInfoEqual
*
*   @brief
*       Return TRUE if all field are equal
*   @note
*       Only takes care of current HWL's data
***************************************************************************************************
*/
BOOL_32 EgBasedAddrLib::HwlTileInfoEqual(
    const ADDR_TILEINFO* pLeft, ///<[in] Left compare operand
    const ADDR_TILEINFO* pRight ///<[in] Right compare operand
    ) const
{
    BOOL_32 equal = FALSE;

    if (pLeft->banks == pRight->banks           &&
        pLeft->bankWidth == pRight->bankWidth   &&
        pLeft->bankHeight == pRight->bankHeight &&
        pLeft->macroAspectRatio == pRight->macroAspectRatio &&
        pLeft->tileSplitBytes == pRight->tileSplitBytes)
    {
        equal = TRUE;
    }

    return equal;
}

/**
***************************************************************************************************
*   EgBasedAddrLib::HwlConvertTileInfoToHW
*   @brief
*       Entry of EgBasedAddrLib ConvertTileInfoToHW
*   @return
*       ADDR_E_RETURNCODE
***************************************************************************************************
*/
ADDR_E_RETURNCODE EgBasedAddrLib::HwlConvertTileInfoToHW(
    const ADDR_CONVERT_TILEINFOTOHW_INPUT* pIn, ///< [in] input structure
    ADDR_CONVERT_TILEINFOTOHW_OUTPUT* pOut      ///< [out] output structure
    ) const
{
    ADDR_E_RETURNCODE retCode   = ADDR_OK;

    ADDR_TILEINFO *pTileInfoIn  = pIn->pTileInfo;
    ADDR_TILEINFO *pTileInfoOut = pOut->pTileInfo;

    if ((pTileInfoIn != NULL) && (pTileInfoOut != NULL))
    {
        if (pIn->reverse == FALSE)
        {
            switch (pTileInfoIn->banks)
            {
                case 2:
                    pTileInfoOut->banks = 0;
                    break;
                case 4:
                    pTileInfoOut->banks = 1;
                    break;
                case 8:
                    pTileInfoOut->banks = 2;
                    break;
                case 16:
                    pTileInfoOut->banks = 3;
                    break;
                default:
                    ADDR_ASSERT_ALWAYS();
                    retCode = ADDR_INVALIDPARAMS;
                    pTileInfoOut->banks = 0;
                    break;
            }

            switch (pTileInfoIn->bankWidth)
            {
                case 1:
                    pTileInfoOut->bankWidth = 0;
                    break;
                case 2:
                    pTileInfoOut->bankWidth = 1;
                    break;
                case 4:
                    pTileInfoOut->bankWidth = 2;
                    break;
                case 8:
                    pTileInfoOut->bankWidth = 3;
                    break;
                default:
                    ADDR_ASSERT_ALWAYS();
                    retCode = ADDR_INVALIDPARAMS;
                    pTileInfoOut->bankWidth = 0;
                    break;
            }

            switch (pTileInfoIn->bankHeight)
            {
                case 1:
                    pTileInfoOut->bankHeight = 0;
                    break;
                case 2:
                    pTileInfoOut->bankHeight = 1;
                    break;
                case 4:
                    pTileInfoOut->bankHeight = 2;
                    break;
                case 8:
                    pTileInfoOut->bankHeight = 3;
                    break;
                default:
                    ADDR_ASSERT_ALWAYS();
                    retCode = ADDR_INVALIDPARAMS;
                    pTileInfoOut->bankHeight = 0;
                    break;
            }

            switch (pTileInfoIn->macroAspectRatio)
            {
                case 1:
                    pTileInfoOut->macroAspectRatio = 0;
                    break;
                case 2:
                    pTileInfoOut->macroAspectRatio = 1;
                    break;
                case 4:
                    pTileInfoOut->macroAspectRatio = 2;
                    break;
                case 8:
                    pTileInfoOut->macroAspectRatio = 3;
                    break;
                default:
                    ADDR_ASSERT_ALWAYS();
                    retCode = ADDR_INVALIDPARAMS;
                    pTileInfoOut->macroAspectRatio = 0;
                    break;
            }

            switch (pTileInfoIn->tileSplitBytes)
            {
                case 64:
                    pTileInfoOut->tileSplitBytes = 0;
                    break;
                case 128:
                    pTileInfoOut->tileSplitBytes = 1;
                    break;
                case 256:
                    pTileInfoOut->tileSplitBytes = 2;
                    break;
                case 512:
                    pTileInfoOut->tileSplitBytes = 3;
                    break;
                case 1024:
                    pTileInfoOut->tileSplitBytes = 4;
                    break;
                case 2048:
                    pTileInfoOut->tileSplitBytes = 5;
                    break;
                case 4096:
                    pTileInfoOut->tileSplitBytes = 6;
                    break;
                default:
                    ADDR_ASSERT_ALWAYS();
                    retCode = ADDR_INVALIDPARAMS;
                    pTileInfoOut->tileSplitBytes = 0;
                    break;
            }
        }
        else
        {
            switch (pTileInfoIn->banks)
            {
                case 0:
                    pTileInfoOut->banks = 2;
                    break;
                case 1:
                    pTileInfoOut->banks = 4;
                    break;
                case 2:
                    pTileInfoOut->banks = 8;
                    break;
                case 3:
                    pTileInfoOut->banks = 16;
                    break;
                default:
                    ADDR_ASSERT_ALWAYS();
                    retCode = ADDR_INVALIDPARAMS;
                    pTileInfoOut->banks = 2;
                    break;
            }

            switch (pTileInfoIn->bankWidth)
            {
                case 0:
                    pTileInfoOut->bankWidth = 1;
                    break;
                case 1:
                    pTileInfoOut->bankWidth = 2;
                    break;
                case 2:
                    pTileInfoOut->bankWidth = 4;
                    break;
                case 3:
                    pTileInfoOut->bankWidth = 8;
                    break;
                default:
                    ADDR_ASSERT_ALWAYS();
                    retCode = ADDR_INVALIDPARAMS;
                    pTileInfoOut->bankWidth = 1;
                    break;
            }

            switch (pTileInfoIn->bankHeight)
            {
                case 0:
                    pTileInfoOut->bankHeight = 1;
                    break;
                case 1:
                    pTileInfoOut->bankHeight = 2;
                    break;
                case 2:
                    pTileInfoOut->bankHeight = 4;
                    break;
                case 3:
                    pTileInfoOut->bankHeight = 8;
                    break;
                default:
                    ADDR_ASSERT_ALWAYS();
                    retCode = ADDR_INVALIDPARAMS;
                    pTileInfoOut->bankHeight = 1;
                    break;
            }

            switch (pTileInfoIn->macroAspectRatio)
            {
                case 0:
                    pTileInfoOut->macroAspectRatio = 1;
                    break;
                case 1:
                    pTileInfoOut->macroAspectRatio = 2;
                    break;
                case 2:
                    pTileInfoOut->macroAspectRatio = 4;
                    break;
                case 3:
                    pTileInfoOut->macroAspectRatio = 8;
                    break;
                default:
                    ADDR_ASSERT_ALWAYS();
                    retCode = ADDR_INVALIDPARAMS;
                    pTileInfoOut->macroAspectRatio = 1;
                    break;
            }

            switch (pTileInfoIn->tileSplitBytes)
            {
                case 0:
                    pTileInfoOut->tileSplitBytes = 64;
                    break;
                case 1:
                    pTileInfoOut->tileSplitBytes = 128;
                    break;
                case 2:
                    pTileInfoOut->tileSplitBytes = 256;
                    break;
                case 3:
                    pTileInfoOut->tileSplitBytes = 512;
                    break;
                case 4:
                    pTileInfoOut->tileSplitBytes = 1024;
                    break;
                case 5:
                    pTileInfoOut->tileSplitBytes = 2048;
                    break;
                case 6:
                    pTileInfoOut->tileSplitBytes = 4096;
                    break;
                default:
                    ADDR_ASSERT_ALWAYS();
                    retCode = ADDR_INVALIDPARAMS;
                    pTileInfoOut->tileSplitBytes = 64;
                    break;
            }
        }

        if (pTileInfoIn != pTileInfoOut)
        {
            pTileInfoOut->pipeConfig = pTileInfoIn->pipeConfig;
        }
    }
    else
    {
        ADDR_ASSERT_ALWAYS();
        retCode = ADDR_INVALIDPARAMS;
    }

    return retCode;
}

/**
***************************************************************************************************
*   EgBasedAddrLib::HwlComputeSurfaceInfo
*   @brief
*       Entry of EgBasedAddrLib ComputeSurfaceInfo
*   @return
*       ADDR_E_RETURNCODE
***************************************************************************************************
*/
ADDR_E_RETURNCODE EgBasedAddrLib::HwlComputeSurfaceInfo(
    const ADDR_COMPUTE_SURFACE_INFO_INPUT*  pIn,    ///< [in] input structure
    ADDR_COMPUTE_SURFACE_INFO_OUTPUT*       pOut    ///< [out] output structure
    ) const
{
    ADDR_E_RETURNCODE retCode = ADDR_OK;

    if (pIn->numSamples < pIn->numFrags)
    {
        retCode = ADDR_INVALIDPARAMS;
    }

    ADDR_TILEINFO tileInfo = {0};

    if (retCode == ADDR_OK)
    {
        // Uses internal tile info if pOut does not have a valid pTileInfo
        if (pOut->pTileInfo == NULL)
        {
            pOut->pTileInfo = &tileInfo;
        }

        if (!DispatchComputeSurfaceInfo(pIn, pOut))
        {
            retCode = ADDR_INVALIDPARAMS;
        }

        // Returns an index
        pOut->tileIndex = HwlPostCheckTileIndex(pOut->pTileInfo,
                                                pOut->tileMode,
                                                pOut->tileType,
                                                pOut->tileIndex);

        if (IsMacroTiled(pOut->tileMode) && (pOut->macroModeIndex == TileIndexInvalid))
        {
            pOut->macroModeIndex = HwlComputeMacroModeIndex(pOut->tileIndex,
                                                            pIn->flags,
                                                            pIn->bpp,
                                                            pIn->numSamples,
                                                            pOut->pTileInfo);
        }

        // Resets pTileInfo to NULL if the internal tile info is used
        if (pOut->pTileInfo == &tileInfo)
        {
#if DEBUG
            // Client does not pass in a valid pTileInfo
            if (IsMacroTiled(pOut->tileMode))
            {
                // If a valid index is returned, then no pTileInfo is okay
                ADDR_ASSERT(!m_configFlags.useTileIndex || pOut->tileIndex != TileIndexInvalid);

                if (!IsTileInfoAllZero(pIn->pTileInfo))
                {
                    // The initial value of pIn->pTileInfo is copied to tileInfo
                    // We do not expect any of these value to be changed nor any 0 of inputs
                    ADDR_ASSERT(tileInfo.banks == pIn->pTileInfo->banks);
                    ADDR_ASSERT(tileInfo.bankWidth == pIn->pTileInfo->bankWidth);
                    ADDR_ASSERT(tileInfo.bankHeight == pIn->pTileInfo->bankHeight);
                    ADDR_ASSERT(tileInfo.macroAspectRatio == pIn->pTileInfo->macroAspectRatio);
                    ADDR_ASSERT(tileInfo.tileSplitBytes == pIn->pTileInfo->tileSplitBytes);
                }
            }
#endif
            pOut->pTileInfo = NULL;
        }
    }

    return retCode;
}

/**
***************************************************************************************************
*   EgBasedAddrLib::HwlComputeSurfaceAddrFromCoord
*   @brief
*       Entry of EgBasedAddrLib ComputeSurfaceAddrFromCoord
*   @return
*       ADDR_E_RETURNCODE
***************************************************************************************************
*/
ADDR_E_RETURNCODE EgBasedAddrLib::HwlComputeSurfaceAddrFromCoord(
    const ADDR_COMPUTE_SURFACE_ADDRFROMCOORD_INPUT* pIn,    ///< [in] input structure
    ADDR_COMPUTE_SURFACE_ADDRFROMCOORD_OUTPUT*      pOut    ///< [out] output structure
    ) const
{
    ADDR_E_RETURNCODE retCode = ADDR_OK;

    if (
#if !ALT_TEST // Overflow test needs this out-of-boundary coord
        (pIn->x > pIn->pitch)   ||
        (pIn->y > pIn->height)  ||
#endif
        (pIn->numSamples > m_maxSamples))
    {
        retCode = ADDR_INVALIDPARAMS;
    }
    else
    {
        pOut->addr = DispatchComputeSurfaceAddrFromCoord(pIn, pOut);
    }

    return retCode;
}

/**
***************************************************************************************************
*   EgBasedAddrLib::HwlComputeSurfaceCoordFromAddr
*   @brief
*       Entry of EgBasedAddrLib ComputeSurfaceCoordFromAddr
*   @return
*       ADDR_E_RETURNCODE
***************************************************************************************************
*/
ADDR_E_RETURNCODE EgBasedAddrLib::HwlComputeSurfaceCoordFromAddr(
    const ADDR_COMPUTE_SURFACE_COORDFROMADDR_INPUT* pIn,    ///< [in] input structure
    ADDR_COMPUTE_SURFACE_COORDFROMADDR_OUTPUT*      pOut    ///< [out] output structure
    ) const
{
    ADDR_E_RETURNCODE retCode = ADDR_OK;

    if ((pIn->bitPosition >= 8) ||
        (pIn->numSamples > m_maxSamples))
    {
        retCode = ADDR_INVALIDPARAMS;
    }
    else
    {
        DispatchComputeSurfaceCoordFromAddr(pIn, pOut);
    }
    return retCode;
}

/**
***************************************************************************************************
*   EgBasedAddrLib::HwlComputeSliceTileSwizzle
*   @brief
*       Entry of EgBasedAddrLib ComputeSurfaceCoordFromAddr
*   @return
*       ADDR_E_RETURNCODE
***************************************************************************************************
*/
ADDR_E_RETURNCODE EgBasedAddrLib::HwlComputeSliceTileSwizzle(
    const ADDR_COMPUTE_SLICESWIZZLE_INPUT*  pIn,    ///< [in] input structure
    ADDR_COMPUTE_SLICESWIZZLE_OUTPUT*       pOut    ///< [out] output structure
    ) const
{
    ADDR_E_RETURNCODE retCode = ADDR_OK;

    if (pIn->pTileInfo && (pIn->pTileInfo->banks > 0))
    {

        pOut->tileSwizzle = ComputeSliceTileSwizzle(pIn->tileMode,
                                                    pIn->baseSwizzle,
                                                    pIn->slice,
                                                    pIn->baseAddr,
                                                    pIn->pTileInfo);
    }
    else
    {
        retCode = ADDR_INVALIDPARAMS;
    }

    return retCode;
}

/**
***************************************************************************************************
*   EgBasedAddrLib::HwlComputeHtileBpp
*
*   @brief
*       Compute htile bpp
*
*   @return
*       Htile bpp
***************************************************************************************************
*/
uint32_t EgBasedAddrLib::HwlComputeHtileBpp(
    BOOL_32 isWidth8,   ///< [in] TRUE if block width is 8
    BOOL_32 isHeight8   ///< [in] TRUE if block height is 8
    ) const
{
    // only support 8x8 mode
    ADDR_ASSERT(isWidth8 && isHeight8);
    return 32;
}

/**
***************************************************************************************************
*   EgBasedAddrLib::HwlComputeHtileBaseAlign
*
*   @brief
*       Compute htile base alignment
*
*   @return
*       Htile base alignment
***************************************************************************************************
*/
uint32_t EgBasedAddrLib::HwlComputeHtileBaseAlign(
    BOOL_32         isTcCompatible, ///< [in] if TC compatible
    BOOL_32         isLinear,       ///< [in] if it is linear mode
    ADDR_TILEINFO*  pTileInfo       ///< [in] Tile info
    ) const
{
    uint32_t baseAlign = m_pipeInterleaveBytes * HwlGetPipes(pTileInfo);

    if (isTcCompatible)
    {
        ADDR_ASSERT(pTileInfo != NULL);
        if (pTileInfo)
        {
            baseAlign *= pTileInfo->banks;
        }
    }

    return baseAlign;
}

/**
***************************************************************************************************
*   EgBasedAddrLib::HwlGetPitchAlignmentMicroTiled
*
*   @brief
*       Compute 1D tiled surface pitch alignment, calculation results are returned through
*       output parameters.
*
*   @return
*       pitch alignment
***************************************************************************************************
*/
uint32_t EgBasedAddrLib::HwlGetPitchAlignmentMicroTiled(
    AddrTileMode        tileMode,          ///< [in] tile mode
    uint32_t            bpp,               ///< [in] bits per pixel
    ADDR_SURFACE_FLAGS  flags,             ///< [in] surface flags
    uint32_t            numSamples         ///< [in] number of samples
    ) const
{
    uint32_t pitchAlign;

    uint32_t microTileThickness = ComputeSurfaceThickness(tileMode);

    uint32_t pixelsPerMicroTile;
    uint32_t pixelsPerPipeInterleave;
    uint32_t microTilesPerPipeInterleave;

    //
    // Special workaround for depth/stencil buffer, use 8 bpp to meet larger requirement for
    // stencil buffer since pitch alignment is related to bpp.
    // For a depth only buffer do not set this.
    //
    // Note: this actually does not work for mipmap but mipmap depth texture is not really
    // sampled with mipmap.
    //
    if (flags.depth && !flags.noStencil)
    {
        bpp = 8;
    }

    pixelsPerMicroTile = MicroTilePixels * microTileThickness;
    pixelsPerPipeInterleave = BYTES_TO_BITS(m_pipeInterleaveBytes) / (bpp * numSamples);
    microTilesPerPipeInterleave = pixelsPerPipeInterleave / pixelsPerMicroTile;

    pitchAlign = Max(MicroTileWidth, microTilesPerPipeInterleave * MicroTileWidth);

    return pitchAlign;
}

/**
***************************************************************************************************
*   EgBasedAddrLib::HwlGetSizeAdjustmentMicroTiled
*
*   @brief
*       Adjust 1D tiled surface pitch and slice size
*
*   @return
*       Logical slice size in bytes
***************************************************************************************************
*/
uint64_t EgBasedAddrLib::HwlGetSizeAdjustmentMicroTiled(
    uint32_t            thickness,      ///< [in] thickness
    uint32_t            bpp,            ///< [in] bits per pixel
    ADDR_SURFACE_FLAGS  flags,          ///< [in] surface flags
    uint32_t            numSamples,     ///< [in] number of samples
    uint32_t            baseAlign,      ///< [in] base alignment
    uint32_t            pitchAlign,     ///< [in] pitch alignment
    uint32_t*           pPitch,         ///< [in/out] pointer to pitch
    uint32_t*           pHeight         ///< [in/out] pointer to height
    ) const
{
    uint64_t logicalSliceSize;
    uint64_t physicalSliceSize;

    uint32_t pitch   = *pPitch;
    uint32_t height  = *pHeight;

    // Logical slice: pitch * height * bpp * numSamples (no 1D MSAA so actually numSamples == 1)
    logicalSliceSize = BITS_TO_BYTES(static_cast<uint64_t>(pitch) * height * bpp * numSamples);

    // Physical slice: multiplied by thickness
    physicalSliceSize =  logicalSliceSize * thickness;

    //
    // R800 will always pad physical slice size to baseAlign which is pipe_interleave_bytes
    //
    ADDR_ASSERT((physicalSliceSize % baseAlign) == 0)

    return logicalSliceSize;
}
