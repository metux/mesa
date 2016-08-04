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
* @file  siaddrlib.h
* @brief Contains the R800AddrLib class definition.
***************************************************************************************************
*/

#ifndef __SI_ADDR_LIB_H__
#define __SI_ADDR_LIB_H__

#include <stdint.h>

#include "addrlib.h"
#include "egbaddrlib.h"

/**
***************************************************************************************************
* @brief Describes the information in tile mode table
***************************************************************************************************
*/
struct ADDR_TILECONFIG
{
    AddrTileMode  mode;
    AddrTileType  type;
    ADDR_TILEINFO info;
};

/**
***************************************************************************************************
* @brief SI specific settings structure.
***************************************************************************************************
*/
struct SIChipSettings
{
    struct
    {
        uint32_t isSouthernIsland    : 1;
        uint32_t isTahiti            : 1;
        uint32_t isPitCairn          : 1;
        uint32_t isCapeVerde         : 1;
        /// Oland/Hainan are of GFXIP 6.0, similar with SI
        uint32_t isOland             : 1;
        uint32_t isHainan            : 1;
    };
};

/**
***************************************************************************************************
* @brief This class is the SI specific address library
*        function set.
***************************************************************************************************
*/
class SIAddrLib : public EgBasedAddrLib
{
public:
    /// Creates SIAddrLib object
    static AddrLib* CreateObj(const AddrClient* pClient)
    {
        return new(pClient) SIAddrLib(pClient);
    }

protected:
    SIAddrLib(const AddrClient* pClient);
    virtual ~SIAddrLib();

    // Hwl interface - defined in AddrLib
    virtual ADDR_E_RETURNCODE HwlComputeSurfaceInfo(
        const ADDR_COMPUTE_SURFACE_INFO_INPUT* pIn,
        ADDR_COMPUTE_SURFACE_INFO_OUTPUT* pOut) const;

    virtual ADDR_E_RETURNCODE HwlConvertTileInfoToHW(
        const ADDR_CONVERT_TILEINFOTOHW_INPUT* pIn,
        ADDR_CONVERT_TILEINFOTOHW_OUTPUT* pOut) const;

    virtual uint64_t HwlComputeXmaskAddrFromCoord(
        uint32_t pitch, uint32_t height, uint32_t x, uint32_t y, uint32_t slice, uint32_t numSlices,
        uint32_t factor, BOOL_32 isLinear, BOOL_32 isWidth8, BOOL_32 isHeight8,
        ADDR_TILEINFO* pTileInfo, uint32_t* pBitPosition) const;

    virtual VOID HwlComputeXmaskCoordFromAddr(
        uint64_t addr, uint32_t bitPosition, uint32_t pitch, uint32_t height, uint32_t numSlices,
        uint32_t factor, BOOL_32 isLinear, BOOL_32 isWidth8, BOOL_32 isHeight8,
        ADDR_TILEINFO* pTileInfo, uint32_t* pX, uint32_t* pY, uint32_t* pSlice) const;

    virtual ADDR_E_RETURNCODE HwlGetTileIndex(
        const ADDR_GET_TILEINDEX_INPUT* pIn,
        ADDR_GET_TILEINDEX_OUTPUT*      pOut) const;

    virtual BOOL_32 HwlComputeMipLevel(
        ADDR_COMPUTE_SURFACE_INFO_INPUT* pIn) const;

    virtual AddrChipFamily HwlConvertChipFamily(
        uint32_t uChipFamily, uint32_t uChipRevision);

    virtual BOOL_32 HwlInitGlobalParams(
        const ADDR_CREATE_INPUT* pCreateIn);

    virtual ADDR_E_RETURNCODE HwlSetupTileCfg(
        int32_t index, int32_t macroModeIndex,
        ADDR_TILEINFO* pInfo, AddrTileMode* pMode = 0, AddrTileType* pType = 0) const;

    virtual VOID HwlComputeTileDataWidthAndHeightLinear(
        uint32_t* pMacroWidth, uint32_t* pMacroHeight,
        uint32_t bpp, ADDR_TILEINFO* pTileInfo) const;

    virtual uint64_t HwlComputeHtileBytes(
        uint32_t pitch, uint32_t height, uint32_t bpp,
        BOOL_32 isLinear, uint32_t numSlices, uint64_t* pSliceBytes, uint32_t baseAlign) const;

    virtual uint32_t ComputePipeFromCoord(
        uint32_t x, uint32_t y, uint32_t slice,
        AddrTileMode tileMode, uint32_t pipeSwizzle, BOOL_32 ignoreSE,
        ADDR_TILEINFO* pTileInfo) const;

    virtual uint32_t HwlGetPipes(const ADDR_TILEINFO* pTileInfo) const;

    /// Pre-handler of 3x pitch (96 bit) adjustment
    virtual uint32_t HwlPreHandleBaseLvl3xPitch(
        const ADDR_COMPUTE_SURFACE_INFO_INPUT* pIn, uint32_t expPitch) const;
    /// Post-handler of 3x pitch adjustment
    virtual uint32_t HwlPostHandleBaseLvl3xPitch(
        const ADDR_COMPUTE_SURFACE_INFO_INPUT* pIn, uint32_t expPitch) const;

    /// Dummy function to finalize the inheritance
    virtual uint32_t HwlComputeXmaskCoordYFrom8Pipe(
        uint32_t pipe, uint32_t x) const;

    // Sub-hwl interface - defined in EgBasedAddrLib
    virtual VOID HwlSetupTileInfo(
        AddrTileMode tileMode, ADDR_SURFACE_FLAGS flags,
        uint32_t bpp, uint32_t pitch, uint32_t height, uint32_t numSamples,
        ADDR_TILEINFO* inputTileInfo, ADDR_TILEINFO* outputTileInfo,
        AddrTileType inTileType, ADDR_COMPUTE_SURFACE_INFO_OUTPUT* pOut) const;

    virtual uint32_t HwlGetPitchAlignmentMicroTiled(
        AddrTileMode tileMode, uint32_t bpp, ADDR_SURFACE_FLAGS flags, uint32_t numSamples) const;

    virtual uint64_t HwlGetSizeAdjustmentMicroTiled(
        uint32_t thickness, uint32_t bpp, ADDR_SURFACE_FLAGS flags, uint32_t numSamples,
        uint32_t baseAlign, uint32_t pitchAlign,
        uint32_t *pPitch, uint32_t *pHeight) const;

    virtual VOID HwlCheckLastMacroTiledLvl(
        const ADDR_COMPUTE_SURFACE_INFO_INPUT* pIn, ADDR_COMPUTE_SURFACE_INFO_OUTPUT* pOut) const;

    virtual BOOL_32 HwlTileInfoEqual(
        const ADDR_TILEINFO* pLeft, const ADDR_TILEINFO* pRight) const;

    virtual AddrTileMode HwlDegradeThickTileMode(
        AddrTileMode baseTileMode, uint32_t numSlices, uint32_t* pBytesPerTile) const;

    virtual BOOL_32 HwlOverrideTileMode(
        const ADDR_COMPUTE_SURFACE_INFO_INPUT* pIn,
        AddrTileMode* pTileMode,
        AddrTileType* pTileType) const;

    virtual BOOL_32 HwlSanityCheckMacroTiled(
        ADDR_TILEINFO* pTileInfo) const
    {
        return TRUE;
    }

    virtual uint32_t HwlGetPitchAlignmentLinear(uint32_t bpp, ADDR_SURFACE_FLAGS flags) const;

    virtual uint64_t HwlGetSizeAdjustmentLinear(
        AddrTileMode tileMode,
        uint32_t bpp, uint32_t numSamples, uint32_t baseAlign, uint32_t pitchAlign,
        uint32_t *pPitch, uint32_t *pHeight, uint32_t *pHeightAlign) const;

    virtual VOID HwlComputeSurfaceCoord2DFromBankPipe(
        AddrTileMode tileMode, uint32_t* pX, uint32_t* pY, uint32_t slice,
        uint32_t bank, uint32_t pipe,
        uint32_t bankSwizzle, uint32_t pipeSwizzle, uint32_t tileSlices,
        BOOL_32 ignoreSE,
        ADDR_TILEINFO* pTileInfo) const;

    virtual uint32_t HwlPreAdjustBank(
        uint32_t tileX, uint32_t bank, ADDR_TILEINFO* pTileInfo) const;

    virtual int32_t HwlPostCheckTileIndex(
        const ADDR_TILEINFO* pInfo, AddrTileMode mode, AddrTileType type,
        INT curIndex = TileIndexInvalid) const;

    virtual VOID   HwlFmaskPreThunkSurfInfo(
        const ADDR_COMPUTE_FMASK_INFO_INPUT* pFmaskIn,
        const ADDR_COMPUTE_FMASK_INFO_OUTPUT* pFmaskOut,
        ADDR_COMPUTE_SURFACE_INFO_INPUT* pSurfIn,
        ADDR_COMPUTE_SURFACE_INFO_OUTPUT* pSurfOut) const;

    virtual VOID   HwlFmaskPostThunkSurfInfo(
        const ADDR_COMPUTE_SURFACE_INFO_OUTPUT* pSurfOut,
        ADDR_COMPUTE_FMASK_INFO_OUTPUT* pFmaskOut) const;

    virtual uint32_t HwlComputeFmaskBits(
        const ADDR_COMPUTE_FMASK_INFO_INPUT* pIn,
        uint32_t* pNumSamples) const;

    virtual BOOL_32 HwlReduceBankWidthHeight(
        uint32_t tileSize, uint32_t bpp, ADDR_SURFACE_FLAGS flags, uint32_t numSamples,
        uint32_t bankHeightAlign, uint32_t pipes,
        ADDR_TILEINFO* pTileInfo) const
    {
        return TRUE;
    }

    // Protected non-virtual functions
    VOID ComputeTileCoordFromPipeAndElemIdx(
        uint32_t elemIdx, uint32_t pipe, AddrPipeCfg pipeCfg, uint32_t pitchInMacroTile,
        uint32_t x, uint32_t y, uint32_t* pX, uint32_t* pY) const;

    uint32_t TileCoordToMaskElementIndex(
        uint32_t tx, uint32_t ty, AddrPipeCfg  pipeConfig,
        uint32_t *macroShift, uint32_t *elemIdxBits) const;

    BOOL_32 DecodeGbRegs(
        const ADDR_REGISTER_VALUE* pRegValue);

    const ADDR_TILECONFIG* GetTileSetting(
        uint32_t index) const;

    static const uint32_t    TileTableSize = 32;
    ADDR_TILECONFIG         m_tileTable[TileTableSize];
    uint32_t                 m_noOfEntries;

private:

    uint32_t GetPipePerSurf(AddrPipeCfg pipeConfig) const;

    VOID ReadGbTileMode(
        uint32_t regValue, ADDR_TILECONFIG* pCfg) const;
    BOOL_32 InitTileSettingTable(
        const uint32_t *pSetting, uint32_t noOfEntries);

    SIChipSettings          m_settings;
};

#endif
