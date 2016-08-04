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
* @file  egbaddrlib.h
* @brief Contains the EgBasedAddrLib class definition.
***************************************************************************************************
*/

#ifndef __EG_BASED_ADDR_LIB_H__
#define __EG_BASED_ADDR_LIB_H__

#include <stdint.h>

#include "addrlib.h"


/// Structures for functions
struct CoordFromBankPipe
{
    uint32_t xBits : 3;
    uint32_t yBits : 4;

    uint32_t xBit3 : 1;
    uint32_t xBit4 : 1;
    uint32_t xBit5 : 1;
    uint32_t yBit3 : 1;
    uint32_t yBit4 : 1;
    uint32_t yBit5 : 1;
    uint32_t yBit6 : 1;
};

/**
***************************************************************************************************
* @brief This class is the Evergreen based address library
* @note  Abstract class
***************************************************************************************************
*/
class EgBasedAddrLib : public AddrLib
{
protected:
    EgBasedAddrLib(const AddrClient* pClient);
    virtual ~EgBasedAddrLib();

public:

    /// Surface info functions

    // NOTE: DispatchComputeSurfaceInfo using TileInfo takes both an input and an output.
    //       On input:
    //       One or more fields may be 0 to be calculated/defaulted - pre-SI h/w.
    //       H/W using tile mode index only accepts none or all 0's - SI and newer h/w.
    //       It then returns the actual tiling configuration used.
    //       Other methods' TileInfo must be valid on entry
    BOOL_32 DispatchComputeSurfaceInfo(
        const ADDR_COMPUTE_SURFACE_INFO_INPUT* pIn,
        ADDR_COMPUTE_SURFACE_INFO_OUTPUT* pOut) const;

    ADDR_E_RETURNCODE DispatchComputeFmaskInfo(
        const ADDR_COMPUTE_FMASK_INFO_INPUT* pIn,
        ADDR_COMPUTE_FMASK_INFO_OUTPUT* pOut);

protected:
    // Hwl interface
    virtual ADDR_E_RETURNCODE HwlComputeSurfaceInfo(
        const ADDR_COMPUTE_SURFACE_INFO_INPUT* pIn,
        ADDR_COMPUTE_SURFACE_INFO_OUTPUT* pOut) const;

    virtual ADDR_E_RETURNCODE HwlComputeSurfaceAddrFromCoord(
        const ADDR_COMPUTE_SURFACE_ADDRFROMCOORD_INPUT* pIn,
        ADDR_COMPUTE_SURFACE_ADDRFROMCOORD_OUTPUT* pOut) const;

    virtual ADDR_E_RETURNCODE HwlComputeSurfaceCoordFromAddr(
        const ADDR_COMPUTE_SURFACE_COORDFROMADDR_INPUT* pIn,
        ADDR_COMPUTE_SURFACE_COORDFROMADDR_OUTPUT* pOut) const;

    virtual ADDR_E_RETURNCODE HwlComputeSliceTileSwizzle(
        const ADDR_COMPUTE_SLICESWIZZLE_INPUT* pIn,
        ADDR_COMPUTE_SLICESWIZZLE_OUTPUT* pOut) const;

    virtual ADDR_E_RETURNCODE HwlExtractBankPipeSwizzle(
        const ADDR_EXTRACT_BANKPIPE_SWIZZLE_INPUT* pIn,
        ADDR_EXTRACT_BANKPIPE_SWIZZLE_OUTPUT* pOut) const;

    virtual ADDR_E_RETURNCODE HwlCombineBankPipeSwizzle(
        uint32_t bankSwizzle, uint32_t pipeSwizzle, ADDR_TILEINFO*  pTileInfo,
        uint64_t baseAddr, uint32_t* pTileSwizzle) const;

    virtual ADDR_E_RETURNCODE HwlComputeBaseSwizzle(
        const ADDR_COMPUTE_BASE_SWIZZLE_INPUT* pIn,
        ADDR_COMPUTE_BASE_SWIZZLE_OUTPUT* pOut) const;

    virtual ADDR_E_RETURNCODE HwlConvertTileInfoToHW(
        const ADDR_CONVERT_TILEINFOTOHW_INPUT* pIn,
        ADDR_CONVERT_TILEINFOTOHW_OUTPUT* pOut) const;

    virtual uint32_t HwlComputeHtileBpp(
        BOOL_32 isWidth8, BOOL_32 isHeight8) const;

    virtual uint32_t HwlComputeHtileBaseAlign(
        BOOL_32 isTcCompatible, BOOL_32 isLinear, ADDR_TILEINFO* pTileInfo) const;

    virtual ADDR_E_RETURNCODE HwlComputeFmaskInfo(
        const ADDR_COMPUTE_FMASK_INFO_INPUT* pIn,
        ADDR_COMPUTE_FMASK_INFO_OUTPUT* pOut);

    virtual ADDR_E_RETURNCODE HwlComputeFmaskAddrFromCoord(
        const ADDR_COMPUTE_FMASK_ADDRFROMCOORD_INPUT* pIn,
        ADDR_COMPUTE_FMASK_ADDRFROMCOORD_OUTPUT* pOut) const;

    virtual ADDR_E_RETURNCODE HwlComputeFmaskCoordFromAddr(
        const ADDR_COMPUTE_FMASK_COORDFROMADDR_INPUT* pIn,
        ADDR_COMPUTE_FMASK_COORDFROMADDR_OUTPUT* pOut) const;

    virtual BOOL_32 HwlDegradeBaseLevel(
        const ADDR_COMPUTE_SURFACE_INFO_INPUT* pIn) const;

    virtual uint32_t HwlComputeQbStereoRightSwizzle(
        ADDR_COMPUTE_SURFACE_INFO_OUTPUT* pInfo) const;

    virtual VOID HwlComputePixelCoordFromOffset(
        uint32_t offset, uint32_t bpp, uint32_t numSamples,
        AddrTileMode tileMode, uint32_t tileBase, uint32_t compBits,
        uint32_t* pX, uint32_t* pY, uint32_t* pSlice, uint32_t* pSample,
        AddrTileType microTileType, BOOL_32 isDepthSampleOrder) const;

    /// Return Cmask block max
    virtual BOOL_32 HwlGetMaxCmaskBlockMax() const
    {
        return 16383; // 14 bits
    }

    // Sub-hwl interface
    /// Pure virtual function to setup tile info (indices) if client requests to do so
    virtual VOID HwlSetupTileInfo(
        AddrTileMode tileMode, ADDR_SURFACE_FLAGS flags,
        uint32_t bpp, uint32_t pitch, uint32_t height, uint32_t numSamples,
        ADDR_TILEINFO* inputTileInfo, ADDR_TILEINFO* outputTileInfo,
        AddrTileType inTileType, ADDR_COMPUTE_SURFACE_INFO_OUTPUT* pOut) const = 0;

    /// Pure virtual function to get pitch alignment for linear modes
    virtual uint32_t HwlGetPitchAlignmentLinear(uint32_t bpp, ADDR_SURFACE_FLAGS flags) const = 0;

    /// Pure virtual function to get size adjustment for linear modes
    virtual uint64_t HwlGetSizeAdjustmentLinear(
        AddrTileMode tileMode,
        uint32_t bpp, uint32_t numSamples, uint32_t baseAlign, uint32_t pitchAlign,
        uint32_t *pPitch, uint32_t *pHeight, uint32_t *pHeightAlign) const = 0;

    virtual uint32_t HwlGetPitchAlignmentMicroTiled(
        AddrTileMode tileMode, uint32_t bpp, ADDR_SURFACE_FLAGS flags, uint32_t numSamples) const;

    virtual uint64_t HwlGetSizeAdjustmentMicroTiled(
        uint32_t thickness, uint32_t bpp, ADDR_SURFACE_FLAGS flags, uint32_t numSamples,
        uint32_t baseAlign, uint32_t pitchAlign,
        uint32_t *pPitch, uint32_t *pHeight) const;

        /// Pure virtual function to do extra sanity check
    virtual BOOL_32 HwlSanityCheckMacroTiled(
        ADDR_TILEINFO* pTileInfo) const = 0;

    /// Pure virtual function to check current level to be the last macro tiled one
    virtual VOID HwlCheckLastMacroTiledLvl(
        const ADDR_COMPUTE_SURFACE_INFO_INPUT* pIn,
        ADDR_COMPUTE_SURFACE_INFO_OUTPUT* pOut) const = 0;

    /// Adjusts bank before bank is modified by rotation
    virtual uint32_t HwlPreAdjustBank(
        uint32_t tileX, uint32_t bank, ADDR_TILEINFO*  pTileInfo) const = 0;

    virtual VOID HwlComputeSurfaceCoord2DFromBankPipe(
        AddrTileMode tileMode, uint32_t* pX, uint32_t* pY, uint32_t slice,
        uint32_t bank, uint32_t pipe,
        uint32_t bankSwizzle, uint32_t pipeSwizzle, uint32_t tileSlices,
        BOOL_32 ignoreSE,
        ADDR_TILEINFO* pTileInfo) const = 0;

    virtual BOOL_32 HwlTileInfoEqual(
        const ADDR_TILEINFO* pLeft, const ADDR_TILEINFO* pRight) const;

    virtual AddrTileMode HwlDegradeThickTileMode(
        AddrTileMode baseTileMode, uint32_t numSlices, uint32_t* pBytesPerTile) const;

    virtual int32_t HwlPostCheckTileIndex(
        const ADDR_TILEINFO* pInfo, AddrTileMode mode, AddrTileType type,
        INT curIndex = TileIndexInvalid) const
    {
        return TileIndexInvalid;
    }

    virtual VOID HwlFmaskPreThunkSurfInfo(
        const ADDR_COMPUTE_FMASK_INFO_INPUT* pFmaskIn,
        const ADDR_COMPUTE_FMASK_INFO_OUTPUT* pFmaskOut,
        ADDR_COMPUTE_SURFACE_INFO_INPUT* pSurfIn,
        ADDR_COMPUTE_SURFACE_INFO_OUTPUT* pSurfOut) const
    {
    }

    virtual VOID HwlFmaskPostThunkSurfInfo(
        const ADDR_COMPUTE_SURFACE_INFO_OUTPUT* pSurfOut,
        ADDR_COMPUTE_FMASK_INFO_OUTPUT* pFmaskOut) const
    {
    }

    /// Virtual function to check if the height needs extra padding
    /// for stereo right eye offset, to avoid bank pipe swizzle
    virtual BOOL_32 HwlStereoCheckRightOffsetPadding() const
    {
        return FALSE;
    }

    virtual BOOL_32 HwlReduceBankWidthHeight(
        uint32_t tileSize, uint32_t bpp, ADDR_SURFACE_FLAGS flags, uint32_t numSamples,
        uint32_t bankHeightAlign, uint32_t pipes,
        ADDR_TILEINFO* pTileInfo) const;

    // Protected non-virtual functions

    /// Mip level functions
    AddrTileMode ComputeSurfaceMipLevelTileMode(
        AddrTileMode baseTileMode, uint32_t bpp,
        uint32_t pitch, uint32_t height, uint32_t numSlices, uint32_t numSamples,
        uint32_t pitchAlign, uint32_t heightAlign,
        ADDR_TILEINFO* pTileInfo) const;

    /// Swizzle functions
    VOID ExtractBankPipeSwizzle(
        uint32_t base256b, ADDR_TILEINFO* pTileInfo,
        uint32_t* pBankSwizzle, uint32_t* pPipeSwizzle) const;

    uint32_t GetBankPipeSwizzle(
        uint32_t bankSwizzle, uint32_t pipeSwizzle,
        uint64_t baseAddr, ADDR_TILEINFO*  pTileInfo) const;

    uint32_t ComputeSliceTileSwizzle(
        AddrTileMode tileMode, uint32_t baseSwizzle, uint32_t slice, uint64_t baseAddr,
        ADDR_TILEINFO* pTileInfo) const;

    /// Addressing functions
    uint32_t ComputeBankFromCoord(
        uint32_t x, uint32_t y, uint32_t slice,
        AddrTileMode tileMode, uint32_t bankSwizzle, uint32_t tileSpitSlice,
        ADDR_TILEINFO* pTileInfo) const;

    uint32_t ComputeBankFromAddr(
        uint64_t addr, uint32_t numBanks, uint32_t numPipes) const;

    uint32_t ComputePipeRotation(
        AddrTileMode tileMode, uint32_t numPipes) const;

    uint32_t ComputeBankRotation(
        AddrTileMode tileMode, uint32_t numBanks,
        uint32_t numPipes) const;

    VOID ComputeSurfaceCoord2DFromBankPipe(
        AddrTileMode tileMode, uint32_t x, uint32_t y, uint32_t slice,
        uint32_t bank, uint32_t pipe,
        uint32_t bankSwizzle, uint32_t pipeSwizzle, uint32_t tileSlices,
        ADDR_TILEINFO* pTileInfo,
        CoordFromBankPipe *pOutput) const;

    /// Htile/Cmask functions
    uint64_t ComputeHtileBytes(
        uint32_t pitch, uint32_t height, uint32_t bpp,
        BOOL_32 isLinear, uint32_t numSlices, uint64_t* sliceBytes, uint32_t baseAlign) const;

    // Static functions
    static BOOL_32 IsTileInfoAllZero(ADDR_TILEINFO* pTileInfo);
    static uint32_t ComputeFmaskNumPlanesFromNumSamples(uint32_t numSamples);
    static uint32_t ComputeFmaskResolvedBppFromNumSamples(uint32_t numSamples);

private:

    BOOL_32 ComputeSurfaceInfoLinear(
        const ADDR_COMPUTE_SURFACE_INFO_INPUT* pIn,
        ADDR_COMPUTE_SURFACE_INFO_OUTPUT* pOut,
        uint32_t padDims) const;

    BOOL_32 ComputeSurfaceInfoMicroTiled(
        const ADDR_COMPUTE_SURFACE_INFO_INPUT* pIn,
        ADDR_COMPUTE_SURFACE_INFO_OUTPUT* pOut,
        uint32_t padDims,
        AddrTileMode expTileMode) const;

    BOOL_32 ComputeSurfaceInfoMacroTiled(
        const ADDR_COMPUTE_SURFACE_INFO_INPUT* pIn,
        ADDR_COMPUTE_SURFACE_INFO_OUTPUT* pOut,
        uint32_t padDims,
        AddrTileMode expTileMode) const;

    BOOL_32 ComputeSurfaceAlignmentsLinear(
        AddrTileMode tileMode, uint32_t bpp, ADDR_SURFACE_FLAGS flags,
        uint32_t* pBaseAlign, uint32_t* pPitchAlign, uint32_t* pHeightAlign) const;

    BOOL_32 ComputeSurfaceAlignmentsMicroTiled(
        AddrTileMode tileMode, uint32_t bpp, ADDR_SURFACE_FLAGS flags,
        uint32_t mipLevel, uint32_t numSamples,
        uint32_t* pBaseAlign, uint32_t* pPitchAlign, uint32_t* pHeightAlign) const;

    BOOL_32 ComputeSurfaceAlignmentsMacroTiled(
        AddrTileMode tileMode, uint32_t bpp, ADDR_SURFACE_FLAGS flags,
        uint32_t mipLevel, uint32_t numSamples,
        ADDR_TILEINFO* pTileInfo,
        uint32_t* pBaseAlign, uint32_t* pPitchAlign, uint32_t* pHeightAlign) const;

    /// Surface addressing functions
    uint64_t DispatchComputeSurfaceAddrFromCoord(
        const ADDR_COMPUTE_SURFACE_ADDRFROMCOORD_INPUT* pIn,
        ADDR_COMPUTE_SURFACE_ADDRFROMCOORD_OUTPUT* pOut) const;

    VOID    DispatchComputeSurfaceCoordFromAddr(
        const ADDR_COMPUTE_SURFACE_COORDFROMADDR_INPUT* pIn,
        ADDR_COMPUTE_SURFACE_COORDFROMADDR_OUTPUT* pOut) const;

    uint64_t ComputeSurfaceAddrFromCoordMicroTiled(
        uint32_t x, uint32_t y, uint32_t slice, uint32_t sample,
        uint32_t bpp, uint32_t pitch, uint32_t height, uint32_t numSamples,
        AddrTileMode tileMode,
        AddrTileType microTileType, BOOL_32 isDepthSampleOrder,
        uint32_t* pBitPosition) const;

    uint64_t ComputeSurfaceAddrFromCoordMacroTiled(
        uint32_t x, uint32_t y, uint32_t slice, uint32_t sample,
        uint32_t bpp, uint32_t pitch, uint32_t height, uint32_t numSamples,
        AddrTileMode tileMode,
        AddrTileType microTileType, BOOL_32 ignoreSE, BOOL_32 isDepthSampleOrder,
        uint32_t pipeSwizzle, uint32_t bankSwizzle,
        ADDR_TILEINFO* pTileInfo,
        uint32_t* pBitPosition) const;

    VOID    ComputeSurfaceCoordFromAddrMacroTiled(
        uint64_t addr, uint32_t bitPosition,
        uint32_t bpp, uint32_t pitch, uint32_t height, uint32_t numSamples,
        AddrTileMode tileMode, uint32_t tileBase, uint32_t compBits,
        AddrTileType microTileType, BOOL_32 ignoreSE, BOOL_32 isDepthSampleOrder,
        uint32_t pipeSwizzle, uint32_t bankSwizzle,
        ADDR_TILEINFO* pTileInfo,
        uint32_t* pX, uint32_t* pY, uint32_t* pSlice, uint32_t* pSample) const;

    /// Fmask functions
    uint64_t DispatchComputeFmaskAddrFromCoord(
        const ADDR_COMPUTE_FMASK_ADDRFROMCOORD_INPUT* pIn,
        ADDR_COMPUTE_FMASK_ADDRFROMCOORD_OUTPUT* pOut) const;

    VOID    DispatchComputeFmaskCoordFromAddr(
        const ADDR_COMPUTE_FMASK_COORDFROMADDR_INPUT* pIn,
        ADDR_COMPUTE_FMASK_COORDFROMADDR_OUTPUT* pOut) const;

    // FMASK related methods - private
    uint64_t ComputeFmaskAddrFromCoordMicroTiled(
        uint32_t x, uint32_t y, uint32_t slice, uint32_t sample, uint32_t plane,
        uint32_t pitch, uint32_t height, uint32_t numSamples, AddrTileMode tileMode,
        BOOL_32 resolved, uint32_t* pBitPosition) const;

    VOID    ComputeFmaskCoordFromAddrMicroTiled(
        uint64_t addr, uint32_t bitPosition,
        uint32_t pitch, uint32_t height, uint32_t numSamples,
        AddrTileMode tileMode, BOOL_32 resolved,
        uint32_t* pX, uint32_t* pY, uint32_t* pSlice, uint32_t* pSample, uint32_t* pPlane) const;

    VOID    ComputeFmaskCoordFromAddrMacroTiled(
        uint64_t addr, uint32_t bitPosition,
        uint32_t pitch, uint32_t height, uint32_t numSamples, AddrTileMode tileMode,
        uint32_t pipeSwizzle, uint32_t bankSwizzle,
        BOOL_32 ignoreSE,
        ADDR_TILEINFO* pTileInfo,
        BOOL_32 resolved,
        uint32_t* pX, uint32_t* pY, uint32_t* pSlice, uint32_t* pSample, uint32_t* pPlane) const;

    uint64_t ComputeFmaskAddrFromCoordMacroTiled(
        uint32_t x, uint32_t y, uint32_t slice, uint32_t sample, uint32_t plane,
        uint32_t pitch, uint32_t height, uint32_t numSamples,
        AddrTileMode tileMode, uint32_t pipeSwizzle, uint32_t bankSwizzle,
        BOOL_32 ignoreSE,
        ADDR_TILEINFO* pTileInfo,
        BOOL_32 resolved,
        uint32_t* pBitPosition) const;

    /// Sanity check functions
    BOOL_32 SanityCheckMacroTiled(
        ADDR_TILEINFO* pTileInfo) const;

protected:
    uint32_t m_ranks;                ///< Number of ranks - MC_ARB_RAMCFG.NOOFRANK
    uint32_t m_logicalBanks;         ///< Logical banks = m_banks * m_ranks if m_banks != 16
    uint32_t m_bankInterleave;       ///< Bank interleave, as a multiple of pipe interleave size
};

#endif
