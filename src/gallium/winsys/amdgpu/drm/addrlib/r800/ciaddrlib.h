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
* @file  ciaddrlib.h
* @brief Contains the CIAddrLib class definition.
***************************************************************************************************
*/

#ifndef __CI_ADDR_LIB_H__
#define __CI_ADDR_LIB_H__

#include <stdint.h>

#include "addrlib.h"
#include "siaddrlib.h"

/**
***************************************************************************************************
* @brief CI specific settings structure.
***************************************************************************************************
*/
struct CIChipSettings
{
    struct
    {
        uint32_t isSeaIsland : 1;
        uint32_t isBonaire   : 1;
        uint32_t isKaveri    : 1;
        uint32_t isSpectre   : 1;
        uint32_t isSpooky    : 1;
        uint32_t isKalindi   : 1;
        // Hawaii is GFXIP 7.2, similar with CI (Bonaire)
        uint32_t isHawaii    : 1;

        // VI
        uint32_t isVolcanicIslands : 1;
        uint32_t isIceland         : 1;
        uint32_t isTonga           : 1;
        uint32_t isFiji            : 1;
        uint32_t isPolaris10       : 1;
        uint32_t isPolaris11       : 1;
        // VI fusion (Carrizo)
        uint32_t isCarrizo         : 1;
    };
};

/**
***************************************************************************************************
* @brief This class is the CI specific address library
*        function set.
***************************************************************************************************
*/
class CIAddrLib : public SIAddrLib
{
public:
    /// Creates CIAddrLib object
    static AddrLib* CreateObj(const AddrClient* pClient)
    {
        return new(pClient) CIAddrLib(pClient);
    }

private:
    CIAddrLib(const AddrClient* pClient);
    virtual ~CIAddrLib();

protected:

    // Hwl interface - defined in AddrLib
    virtual ADDR_E_RETURNCODE HwlComputeSurfaceInfo(
        const ADDR_COMPUTE_SURFACE_INFO_INPUT* pIn,
        ADDR_COMPUTE_SURFACE_INFO_OUTPUT* pOut) const;

    virtual ADDR_E_RETURNCODE HwlComputeFmaskInfo(
        const ADDR_COMPUTE_FMASK_INFO_INPUT* pIn,
        ADDR_COMPUTE_FMASK_INFO_OUTPUT* pOut);

    virtual AddrChipFamily HwlConvertChipFamily(
        uint32_t uChipFamily, uint32_t uChipRevision);

    virtual BOOL_32 HwlInitGlobalParams(
        const ADDR_CREATE_INPUT* pCreateIn);

    virtual ADDR_E_RETURNCODE HwlSetupTileCfg(
        int32_t index, int32_t macroModeIndex, ADDR_TILEINFO* pInfo,
        AddrTileMode* pMode = 0, AddrTileType* pType = 0) const;

    virtual VOID HwlComputeTileDataWidthAndHeightLinear(
        uint32_t* pMacroWidth, uint32_t* pMacroHeight,
        uint32_t bpp, ADDR_TILEINFO* pTileInfo) const;

    virtual int32_t HwlComputeMacroModeIndex(
        int32_t tileIndex, ADDR_SURFACE_FLAGS flags, uint32_t bpp, uint32_t numSamples,
        ADDR_TILEINFO* pTileInfo, AddrTileMode* pTileMode = NULL, AddrTileType* pTileType = NULL
        ) const;

    // Sub-hwl interface - defined in EgBasedAddrLib
    virtual VOID HwlSetupTileInfo(
        AddrTileMode tileMode, ADDR_SURFACE_FLAGS flags,
        uint32_t bpp, uint32_t pitch, uint32_t height, uint32_t numSamples,
        ADDR_TILEINFO* inputTileInfo, ADDR_TILEINFO* outputTileInfo,
        AddrTileType inTileType, ADDR_COMPUTE_SURFACE_INFO_OUTPUT* pOut) const;

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

    virtual AddrTileMode HwlDegradeThickTileMode(
        AddrTileMode baseTileMode, uint32_t numSlices, uint32_t* pBytesPerTile) const;

    virtual BOOL_32 HwlOverrideTileMode(
        const ADDR_COMPUTE_SURFACE_INFO_INPUT* pIn,
        AddrTileMode* pTileMode,
        AddrTileType* pTileType) const;

    virtual BOOL_32 HwlStereoCheckRightOffsetPadding() const;

    virtual ADDR_E_RETURNCODE HwlComputeDccInfo(
        const ADDR_COMPUTE_DCCINFO_INPUT* pIn,
        ADDR_COMPUTE_DCCINFO_OUTPUT* pOut) const;

    virtual ADDR_E_RETURNCODE HwlComputeCmaskAddrFromCoord(
        const ADDR_COMPUTE_CMASK_ADDRFROMCOORD_INPUT* pIn,
        ADDR_COMPUTE_CMASK_ADDRFROMCOORD_OUTPUT* pOut) const;

protected:
    virtual VOID HwlPadDimensions(
        AddrTileMode tileMode, uint32_t bpp, ADDR_SURFACE_FLAGS flags,
        uint32_t numSamples, ADDR_TILEINFO* pTileInfo, uint32_t padDims, uint32_t mipLevel,
        uint32_t* pPitch, uint32_t pitchAlign, uint32_t* pHeight, uint32_t heightAlign,
        uint32_t* pSlices, uint32_t sliceAlign) const;

private:
    VOID ReadGbTileMode(
        uint32_t regValue, ADDR_TILECONFIG* pCfg) const;

    VOID ReadGbMacroTileCfg(
        uint32_t regValue, ADDR_TILEINFO* pCfg) const;

    uint32_t GetPrtSwitchP4Threshold() const;

    BOOL_32 InitTileSettingTable(
        const uint32_t *pSetting, uint32_t noOfEntries);

    BOOL_32 InitMacroTileCfgTable(
        const uint32_t *pSetting, uint32_t noOfEntries);

    uint64_t HwlComputeMetadataNibbleAddress(
        uint64_t uncompressedDataByteAddress,
        uint64_t dataBaseByteAddress,
        uint64_t metadataBaseByteAddress,
        uint32_t metadataBitSize,
        uint32_t elementBitSize,
        uint32_t blockByteSize,
        uint32_t pipeInterleaveBytes,
        uint32_t numOfPipes,
        uint32_t numOfBanks,
        uint32_t numOfSamplesPerSplit) const;

    static const uint32_t    MacroTileTableSize = 16;
    ADDR_TILEINFO           m_macroTileTable[MacroTileTableSize];
    uint32_t                 m_noOfMacroEntries;
    BOOL_32                 m_allowNonDispThickModes;

    CIChipSettings          m_settings;
};

#endif
