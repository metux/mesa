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
* @file  addrlib.h
* @brief Contains the AddrLib base class definition.
***************************************************************************************************
*/

#ifndef __ADDR_LIB_H__
#define __ADDR_LIB_H__

#include <stdint.h>

#include "addrinterface.h"
#include "addrobject.h"
#include "addrelemlib.h"

#if BRAHMA_BUILD
#include "amdgpu_id.h"
#else
#include "atiid.h"
#endif

#ifndef CIASICIDGFXENGINE_R600
#define CIASICIDGFXENGINE_R600 0x00000006
#endif

#ifndef CIASICIDGFXENGINE_R800
#define CIASICIDGFXENGINE_R800 0x00000008
#endif

#ifndef CIASICIDGFXENGINE_SOUTHERNISLAND
#define CIASICIDGFXENGINE_SOUTHERNISLAND 0x0000000A
#endif

#ifndef CIASICIDGFXENGINE_SEAISLAND
#define CIASICIDGFXENGINE_SEAISLAND 0x0000000B
#endif
/**
***************************************************************************************************
* @brief Neutral enums that define pipeinterleave
***************************************************************************************************
*/
enum AddrPipeInterleave
{
    ADDR_PIPEINTERLEAVE_256B = 256,
    ADDR_PIPEINTERLEAVE_512B = 512,
};

/**
***************************************************************************************************
* @brief Neutral enums that define DRAM row size
***************************************************************************************************
*/
enum AddrRowSize
{
    ADDR_ROWSIZE_1KB = 1024,
    ADDR_ROWSIZE_2KB = 2048,
    ADDR_ROWSIZE_4KB = 4096,
    ADDR_ROWSIZE_8KB = 8192,
};

/**
***************************************************************************************************
* @brief Neutral enums that define bank interleave
***************************************************************************************************
*/
enum AddrBankInterleave
{
    ADDR_BANKINTERLEAVE_1 = 1,
    ADDR_BANKINTERLEAVE_2 = 2,
    ADDR_BANKINTERLEAVE_4 = 4,
    ADDR_BANKINTERLEAVE_8 = 8,
};

/**
***************************************************************************************************
* @brief Neutral enums that define MGPU chip tile size
***************************************************************************************************
*/
enum AddrChipTileSize
{
    ADDR_CHIPTILESIZE_16 = 16,
    ADDR_CHIPTILESIZE_32 = 32,
    ADDR_CHIPTILESIZE_64 = 64,
    ADDR_CHIPTILESIZE_128 = 128,
};

/**
***************************************************************************************************
* @brief Neutral enums that define shader engine tile size
***************************************************************************************************
*/
enum AddrEngTileSize
{
    ADDR_SE_TILESIZE_16 = 16,
    ADDR_SE_TILESIZE_32 = 32,
};

/**
***************************************************************************************************
* @brief Neutral enums that define bank swap size
***************************************************************************************************
*/
enum AddrBankSwapSize
{
    ADDR_BANKSWAP_128B = 128,
    ADDR_BANKSWAP_256B = 256,
    ADDR_BANKSWAP_512B = 512,
    ADDR_BANKSWAP_1KB = 1024,
};

/**
***************************************************************************************************
* @brief Neutral enums that define bank swap size
***************************************************************************************************
*/
enum AddrSampleSplitSize
{
    ADDR_SAMPLESPLIT_1KB = 1024,
    ADDR_SAMPLESPLIT_2KB = 2048,
    ADDR_SAMPLESPLIT_4KB = 4096,
    ADDR_SAMPLESPLIT_8KB = 8192,
};

/**
***************************************************************************************************
* @brief Flags for AddrTileMode
***************************************************************************************************
*/
struct AddrTileModeFlags
{
    uint32_t thickness       : 4;
    uint32_t isLinear        : 1;
    uint32_t isMicro         : 1;
    uint32_t isMacro         : 1;
    uint32_t isMacro3d       : 1;
    uint32_t isPrt           : 1;
    uint32_t isPrtNoRotation : 1;
    uint32_t isBankSwapped   : 1;
};

/**
***************************************************************************************************
* @brief This class contains asic independent address lib functionalities
***************************************************************************************************
*/
class AddrLib : public AddrObject
{
public:
    virtual ~AddrLib();

    static ADDR_E_RETURNCODE Create(
        const ADDR_CREATE_INPUT* pCreateInfo, ADDR_CREATE_OUTPUT* pCreateOut);

    /// Pair of Create
    VOID Destroy()
    {
        delete this;
    }

    static AddrLib* GetAddrLib(
        ADDR_HANDLE hLib);

    /// Returns AddrLib version (from compiled binary instead include file)
    uint32_t GetVersion()
    {
        return m_version;
    }

    /// Returns asic chip family name defined by AddrLib
    AddrChipFamily GetAddrChipFamily()
    {
        return m_chipFamily;
    }

    /// Returns tileIndex support
    BOOL_32 UseTileIndex(int32_t index) const
    {
        return m_configFlags.useTileIndex && (index != TileIndexInvalid);
    }

    /// Returns combined swizzle support
    BOOL_32 UseCombinedSwizzle() const
    {
        return m_configFlags.useCombinedSwizzle;
    }

    //
    // Interface stubs
    //
    ADDR_E_RETURNCODE ComputeSurfaceInfo(
        const ADDR_COMPUTE_SURFACE_INFO_INPUT* pIn,
        ADDR_COMPUTE_SURFACE_INFO_OUTPUT* pOut) const;

    ADDR_E_RETURNCODE ComputeSurfaceAddrFromCoord(
        const ADDR_COMPUTE_SURFACE_ADDRFROMCOORD_INPUT* pIn,
        ADDR_COMPUTE_SURFACE_ADDRFROMCOORD_OUTPUT* pOut) const;

    ADDR_E_RETURNCODE ComputeSurfaceCoordFromAddr(
        const ADDR_COMPUTE_SURFACE_COORDFROMADDR_INPUT*  pIn,
        ADDR_COMPUTE_SURFACE_COORDFROMADDR_OUTPUT* pOut) const;

    ADDR_E_RETURNCODE ComputeSliceTileSwizzle(
        const ADDR_COMPUTE_SLICESWIZZLE_INPUT*  pIn,
        ADDR_COMPUTE_SLICESWIZZLE_OUTPUT* pOut) const;

    ADDR_E_RETURNCODE ExtractBankPipeSwizzle(
        const ADDR_EXTRACT_BANKPIPE_SWIZZLE_INPUT* pIn,
        ADDR_EXTRACT_BANKPIPE_SWIZZLE_OUTPUT* pOut) const;

    ADDR_E_RETURNCODE CombineBankPipeSwizzle(
        const ADDR_COMBINE_BANKPIPE_SWIZZLE_INPUT*  pIn,
        ADDR_COMBINE_BANKPIPE_SWIZZLE_OUTPUT* pOut) const;

    ADDR_E_RETURNCODE ComputeBaseSwizzle(
        const ADDR_COMPUTE_BASE_SWIZZLE_INPUT*  pIn,
        ADDR_COMPUTE_BASE_SWIZZLE_OUTPUT* pOut) const;

    ADDR_E_RETURNCODE ComputeFmaskInfo(
        const ADDR_COMPUTE_FMASK_INFO_INPUT*  pIn,
        ADDR_COMPUTE_FMASK_INFO_OUTPUT* pOut);

    ADDR_E_RETURNCODE ComputeFmaskAddrFromCoord(
        const ADDR_COMPUTE_FMASK_ADDRFROMCOORD_INPUT*  pIn,
        ADDR_COMPUTE_FMASK_ADDRFROMCOORD_OUTPUT* pOut) const;

    ADDR_E_RETURNCODE ComputeFmaskCoordFromAddr(
        const ADDR_COMPUTE_FMASK_COORDFROMADDR_INPUT*  pIn,
        ADDR_COMPUTE_FMASK_COORDFROMADDR_OUTPUT* pOut) const;

    ADDR_E_RETURNCODE ConvertTileInfoToHW(
        const ADDR_CONVERT_TILEINFOTOHW_INPUT* pIn,
        ADDR_CONVERT_TILEINFOTOHW_OUTPUT* pOut) const;

    ADDR_E_RETURNCODE ConvertTileIndex(
        const ADDR_CONVERT_TILEINDEX_INPUT* pIn,
        ADDR_CONVERT_TILEINDEX_OUTPUT* pOut) const;

    ADDR_E_RETURNCODE ConvertTileIndex1(
        const ADDR_CONVERT_TILEINDEX1_INPUT* pIn,
        ADDR_CONVERT_TILEINDEX_OUTPUT* pOut) const;

    ADDR_E_RETURNCODE GetTileIndex(
        const ADDR_GET_TILEINDEX_INPUT* pIn,
        ADDR_GET_TILEINDEX_OUTPUT* pOut) const;

    ADDR_E_RETURNCODE ComputeHtileInfo(
        const ADDR_COMPUTE_HTILE_INFO_INPUT* pIn,
        ADDR_COMPUTE_HTILE_INFO_OUTPUT* pOut) const;

    ADDR_E_RETURNCODE ComputeCmaskInfo(
        const ADDR_COMPUTE_CMASK_INFO_INPUT* pIn,
        ADDR_COMPUTE_CMASK_INFO_OUTPUT* pOut) const;

    ADDR_E_RETURNCODE ComputeDccInfo(
        const ADDR_COMPUTE_DCCINFO_INPUT* pIn,
        ADDR_COMPUTE_DCCINFO_OUTPUT* pOut) const;

    ADDR_E_RETURNCODE ComputeHtileAddrFromCoord(
        const ADDR_COMPUTE_HTILE_ADDRFROMCOORD_INPUT*  pIn,
        ADDR_COMPUTE_HTILE_ADDRFROMCOORD_OUTPUT* pOut) const;

    ADDR_E_RETURNCODE ComputeCmaskAddrFromCoord(
        const ADDR_COMPUTE_CMASK_ADDRFROMCOORD_INPUT*  pIn,
        ADDR_COMPUTE_CMASK_ADDRFROMCOORD_OUTPUT* pOut) const;

    ADDR_E_RETURNCODE ComputeHtileCoordFromAddr(
        const ADDR_COMPUTE_HTILE_COORDFROMADDR_INPUT*  pIn,
        ADDR_COMPUTE_HTILE_COORDFROMADDR_OUTPUT* pOut) const;

    ADDR_E_RETURNCODE ComputeCmaskCoordFromAddr(
        const ADDR_COMPUTE_CMASK_COORDFROMADDR_INPUT*  pIn,
        ADDR_COMPUTE_CMASK_COORDFROMADDR_OUTPUT* pOut) const;

    ADDR_E_RETURNCODE ComputePrtInfo(
        const ADDR_PRT_INFO_INPUT*  pIn,
        ADDR_PRT_INFO_OUTPUT*       pOut) const;

    ADDR_E_RETURNCODE Flt32ToDepthPixel(
        const ELEM_FLT32TODEPTHPIXEL_INPUT* pIn,
        ELEM_FLT32TODEPTHPIXEL_OUTPUT* pOut) const;

    ADDR_E_RETURNCODE Flt32ToColorPixel(
        const ELEM_FLT32TOCOLORPIXEL_INPUT* pIn,
        ELEM_FLT32TOCOLORPIXEL_OUTPUT* pOut) const;

    BOOL_32 GetExportNorm(
        const ELEM_GETEXPORTNORM_INPUT* pIn) const;

protected:
    AddrLib();  // Constructor is protected
    AddrLib(const AddrClient* pClient);

    /// Pure Virtual function for Hwl computing surface info
    virtual ADDR_E_RETURNCODE HwlComputeSurfaceInfo(
        const ADDR_COMPUTE_SURFACE_INFO_INPUT* pIn,
        ADDR_COMPUTE_SURFACE_INFO_OUTPUT* pOut) const = 0;

    /// Pure Virtual function for Hwl computing surface address from coord
    virtual ADDR_E_RETURNCODE HwlComputeSurfaceAddrFromCoord(
        const ADDR_COMPUTE_SURFACE_ADDRFROMCOORD_INPUT* pIn,
        ADDR_COMPUTE_SURFACE_ADDRFROMCOORD_OUTPUT* pOut) const = 0;

    /// Pure Virtual function for Hwl computing surface coord from address
    virtual ADDR_E_RETURNCODE HwlComputeSurfaceCoordFromAddr(
        const ADDR_COMPUTE_SURFACE_COORDFROMADDR_INPUT* pIn,
        ADDR_COMPUTE_SURFACE_COORDFROMADDR_OUTPUT* pOut) const = 0;

    /// Pure Virtual function for Hwl computing surface tile swizzle
    virtual ADDR_E_RETURNCODE HwlComputeSliceTileSwizzle(
        const ADDR_COMPUTE_SLICESWIZZLE_INPUT* pIn,
        ADDR_COMPUTE_SLICESWIZZLE_OUTPUT* pOut) const = 0;

    /// Pure Virtual function for Hwl extracting bank/pipe swizzle from base256b
    virtual ADDR_E_RETURNCODE HwlExtractBankPipeSwizzle(
        const ADDR_EXTRACT_BANKPIPE_SWIZZLE_INPUT* pIn,
        ADDR_EXTRACT_BANKPIPE_SWIZZLE_OUTPUT* pOut) const = 0;

    /// Pure Virtual function for Hwl combining bank/pipe swizzle
    virtual ADDR_E_RETURNCODE HwlCombineBankPipeSwizzle(
        uint32_t bankSwizzle, uint32_t pipeSwizzle, ADDR_TILEINFO*  pTileInfo,
        uint64_t baseAddr, uint32_t* pTileSwizzle) const = 0;

    /// Pure Virtual function for Hwl computing base swizzle
    virtual ADDR_E_RETURNCODE HwlComputeBaseSwizzle(
        const ADDR_COMPUTE_BASE_SWIZZLE_INPUT* pIn,
        ADDR_COMPUTE_BASE_SWIZZLE_OUTPUT* pOut) const = 0;

    /// Pure Virtual function for Hwl computing HTILE base align
    virtual uint32_t HwlComputeHtileBaseAlign(
        BOOL_32 isTcCompatible, BOOL_32 isLinear, ADDR_TILEINFO* pTileInfo) const = 0;

    /// Pure Virtual function for Hwl computing HTILE bpp
    virtual uint32_t HwlComputeHtileBpp(
        BOOL_32 isWidth8, BOOL_32 isHeight8) const = 0;

    /// Pure Virtual function for Hwl computing HTILE bytes
    virtual uint64_t HwlComputeHtileBytes(
        uint32_t pitch, uint32_t height, uint32_t bpp,
        BOOL_32 isLinear, uint32_t numSlices, uint64_t* pSliceBytes, uint32_t baseAlign) const = 0;

    /// Pure Virtual function for Hwl computing FMASK info
    virtual ADDR_E_RETURNCODE HwlComputeFmaskInfo(
        const ADDR_COMPUTE_FMASK_INFO_INPUT* pIn,
        ADDR_COMPUTE_FMASK_INFO_OUTPUT* pOut) = 0;

    /// Pure Virtual function for Hwl FMASK address from coord
    virtual ADDR_E_RETURNCODE HwlComputeFmaskAddrFromCoord(
        const ADDR_COMPUTE_FMASK_ADDRFROMCOORD_INPUT* pIn,
        ADDR_COMPUTE_FMASK_ADDRFROMCOORD_OUTPUT* pOut) const = 0;

    /// Pure Virtual function for Hwl FMASK coord from address
    virtual ADDR_E_RETURNCODE HwlComputeFmaskCoordFromAddr(
        const ADDR_COMPUTE_FMASK_COORDFROMADDR_INPUT* pIn,
        ADDR_COMPUTE_FMASK_COORDFROMADDR_OUTPUT* pOut) const = 0;

    /// Pure Virtual function for Hwl convert tile info from real value to HW value
    virtual ADDR_E_RETURNCODE HwlConvertTileInfoToHW(
        const ADDR_CONVERT_TILEINFOTOHW_INPUT* pIn,
        ADDR_CONVERT_TILEINFOTOHW_OUTPUT* pOut) const = 0;

    /// Pure Virtual function for Hwl compute mipmap info
    virtual BOOL_32 HwlComputeMipLevel(
        ADDR_COMPUTE_SURFACE_INFO_INPUT* pIn) const = 0;

    /// Pure Virtual function for Hwl compute max cmask blockMax value
    virtual BOOL_32 HwlGetMaxCmaskBlockMax() const = 0;

    /// Pure Virtual function for Hwl compute fmask bits
    virtual uint32_t HwlComputeFmaskBits(
        const ADDR_COMPUTE_FMASK_INFO_INPUT* pIn,
        uint32_t* pNumSamples) const = 0;

    /// Virtual function to get index (not pure then no need to implement this in all hwls
    virtual ADDR_E_RETURNCODE HwlGetTileIndex(
        const ADDR_GET_TILEINDEX_INPUT* pIn,
        ADDR_GET_TILEINDEX_OUTPUT*      pOut) const
    {
        return ADDR_NOTSUPPORTED;
    }

    /// Virtual function for Hwl to compute Dcc info
    virtual ADDR_E_RETURNCODE HwlComputeDccInfo(
        const ADDR_COMPUTE_DCCINFO_INPUT* pIn,
        ADDR_COMPUTE_DCCINFO_OUTPUT* pOut) const
    {
        return ADDR_NOTSUPPORTED;
    }

    /// Virtual function to get cmask address for tc compatible cmask
    virtual ADDR_E_RETURNCODE HwlComputeCmaskAddrFromCoord(
        const ADDR_COMPUTE_CMASK_ADDRFROMCOORD_INPUT* pIn,
        ADDR_COMPUTE_CMASK_ADDRFROMCOORD_OUTPUT* pOut) const
    {
        return ADDR_NOTSUPPORTED;
    }
    // Compute attributes

    // HTILE
    uint32_t    ComputeHtileInfo(
        ADDR_HTILE_FLAGS flags,
        uint32_t pitchIn, uint32_t heightIn, uint32_t numSlices,
        BOOL_32 isLinear, BOOL_32 isWidth8, BOOL_32 isHeight8,
        ADDR_TILEINFO*  pTileInfo,
        uint32_t* pPitchOut, uint32_t* pHeightOut, uint64_t* pHtileBytes,
        uint32_t* pMacroWidth = NULL, uint32_t* pMacroHeight = NULL,
        uint64_t* pSliceSize = NULL, uint32_t* pBaseAlign = NULL) const;

    // CMASK
    ADDR_E_RETURNCODE ComputeCmaskInfo(
        ADDR_CMASK_FLAGS flags,
        uint32_t pitchIn, uint32_t heightIn, uint32_t numSlices, BOOL_32 isLinear,
        ADDR_TILEINFO* pTileInfo, uint32_t* pPitchOut, uint32_t* pHeightOut, uint64_t* pCmaskBytes,
        uint32_t* pMacroWidth, uint32_t* pMacroHeight, uint64_t* pSliceSize = NULL,
        uint32_t* pBaseAlign = NULL, uint32_t* pBlockMax = NULL) const;

    virtual VOID HwlComputeTileDataWidthAndHeightLinear(
        uint32_t* pMacroWidth, uint32_t* pMacroHeight,
        uint32_t bpp, ADDR_TILEINFO* pTileInfo) const;

    // CMASK & HTILE addressing
    virtual uint64_t HwlComputeXmaskAddrFromCoord(
        uint32_t pitch, uint32_t height, uint32_t x, uint32_t y, uint32_t slice,
        uint32_t numSlices, uint32_t factor, BOOL_32 isLinear, BOOL_32 isWidth8,
        BOOL_32 isHeight8, ADDR_TILEINFO* pTileInfo,
        uint32_t* bitPosition) const;

    virtual VOID HwlComputeXmaskCoordFromAddr(
        uint64_t addr, uint32_t bitPosition, uint32_t pitch, uint32_t height, uint32_t numSlices,
        uint32_t factor, BOOL_32 isLinear, BOOL_32 isWidth8, BOOL_32 isHeight8,
        ADDR_TILEINFO* pTileInfo, uint32_t* pX, uint32_t* pY, uint32_t* pSlice) const;

    // Surface mipmap
    VOID    ComputeMipLevel(
        ADDR_COMPUTE_SURFACE_INFO_INPUT* pIn) const;

    /// Pure Virtual function for Hwl checking degrade for base level
    virtual BOOL_32 HwlDegradeBaseLevel(
        const ADDR_COMPUTE_SURFACE_INFO_INPUT* pIn) const = 0;

    virtual BOOL_32 HwlOverrideTileMode(
        const ADDR_COMPUTE_SURFACE_INFO_INPUT* pIn,
        AddrTileMode* pTileMode,
        AddrTileType* pTileType) const
    {
        // not supported in hwl layer, FALSE for not-overrided
        return FALSE;
    }

    AddrTileMode DegradeLargeThickTile(AddrTileMode tileMode, uint32_t bpp) const;

    VOID PadDimensions(
        AddrTileMode tileMode, uint32_t bpp, ADDR_SURFACE_FLAGS flags,
        uint32_t numSamples, ADDR_TILEINFO* pTileInfo, uint32_t padDims, uint32_t mipLevel,
        uint32_t* pPitch, uint32_t pitchAlign, uint32_t* pHeight, uint32_t heightAlign,
        uint32_t* pSlices, uint32_t sliceAlign) const;

    virtual VOID HwlPadDimensions(
        AddrTileMode tileMode, uint32_t bpp, ADDR_SURFACE_FLAGS flags,
        uint32_t numSamples, ADDR_TILEINFO* pTileInfo, uint32_t padDims, uint32_t mipLevel,
        uint32_t* pPitch, uint32_t pitchAlign, uint32_t* pHeight, uint32_t heightAlign,
        uint32_t* pSlices, uint32_t sliceAlign) const
    {
    }

    //
    // Addressing shared for linear/1D tiling
    //
    uint64_t ComputeSurfaceAddrFromCoordLinear(
        uint32_t x, uint32_t y, uint32_t slice, uint32_t sample,
        uint32_t bpp, uint32_t pitch, uint32_t height, uint32_t numSlices,
        uint32_t* pBitPosition) const;

    VOID    ComputeSurfaceCoordFromAddrLinear(
        uint64_t addr, uint32_t bitPosition, uint32_t bpp,
        uint32_t pitch, uint32_t height, uint32_t numSlices,
        uint32_t* pX, uint32_t* pY, uint32_t* pSlice, uint32_t* pSample) const;

    VOID    ComputeSurfaceCoordFromAddrMicroTiled(
        uint64_t addr, uint32_t bitPosition,
        uint32_t bpp, uint32_t pitch, uint32_t height, uint32_t numSamples,
        AddrTileMode tileMode, uint32_t tileBase, uint32_t compBits,
        uint32_t* pX, uint32_t* pY, uint32_t* pSlice, uint32_t* pSample,
        AddrTileType microTileType, BOOL_32 isDepthSampleOrder) const;

    uint32_t ComputePixelIndexWithinMicroTile(
        uint32_t x, uint32_t y, uint32_t z,
        uint32_t bpp, AddrTileMode tileMode, AddrTileType microTileType) const;

    /// Pure Virtual function for Hwl computing coord from offset inside micro tile
    virtual VOID HwlComputePixelCoordFromOffset(
        uint32_t offset, uint32_t bpp, uint32_t numSamples,
        AddrTileMode tileMode, uint32_t tileBase, uint32_t compBits,
        uint32_t* pX, uint32_t* pY, uint32_t* pSlice, uint32_t* pSample,
        AddrTileType microTileType, BOOL_32 isDepthSampleOrder) const = 0;

    //
    // Addressing shared by all
    //
    virtual uint32_t HwlGetPipes(
        const ADDR_TILEINFO* pTileInfo) const;

    uint32_t ComputePipeFromAddr(
        uint64_t addr, uint32_t numPipes) const;

    /// Pure Virtual function for Hwl computing pipe from coord
    virtual uint32_t ComputePipeFromCoord(
        uint32_t x, uint32_t y, uint32_t slice, AddrTileMode tileMode,
        uint32_t pipeSwizzle, BOOL_32 flags, ADDR_TILEINFO* pTileInfo) const = 0;

    /// Pure Virtual function for Hwl computing coord Y for 8 pipe cmask/htile
    virtual uint32_t HwlComputeXmaskCoordYFrom8Pipe(
        uint32_t pipe, uint32_t x) const = 0;

    //
    // Initialization
    //
    /// Pure Virtual function for Hwl computing internal global parameters from h/w registers
    virtual BOOL_32 HwlInitGlobalParams(
        const ADDR_CREATE_INPUT* pCreateIn) = 0;

    /// Pure Virtual function for Hwl converting chip family
    virtual AddrChipFamily HwlConvertChipFamily(uint32_t uChipFamily, uint32_t uChipRevision) = 0;

    //
    // Misc helper
    //
    static const AddrTileModeFlags m_modeFlags[ADDR_TM_COUNT];

    static uint32_t ComputeSurfaceThickness(
        AddrTileMode tileMode);

    // Checking tile mode
    static BOOL_32 IsMacroTiled(AddrTileMode tileMode);
    static BOOL_32 IsMacro3dTiled(AddrTileMode tileMode);
    static BOOL_32 IsLinear(AddrTileMode tileMode);
    static BOOL_32 IsMicroTiled(AddrTileMode tileMode);
    static BOOL_32 IsPrtTileMode(AddrTileMode tileMode);
    static BOOL_32 IsPrtNoRotationTileMode(AddrTileMode tileMode);

    static uint32_t Bits2Number(uint32_t bitNum,...);

    static uint32_t GetNumFragments(uint32_t numSamples, uint32_t numFrags)
    {
        return numFrags != 0 ? numFrags : Max(1u, numSamples);
    }

    /// Returns pointer of AddrElemLib
    AddrElemLib* GetElemLib() const
    {
        return m_pElemLib;
    }

    /// Return TRUE if tile info is needed
    BOOL_32 UseTileInfo() const
    {
        return !m_configFlags.ignoreTileInfo;
    }

    /// Returns fillSizeFields flag
    uint32_t GetFillSizeFieldsFlags() const
    {
        return m_configFlags.fillSizeFields;
    }

    /// Adjusts pitch alignment for flipping surface
    VOID    AdjustPitchAlignment(
        ADDR_SURFACE_FLAGS flags, uint32_t* pPitchAlign) const;

    /// Overwrite tile config according to tile index
    virtual ADDR_E_RETURNCODE HwlSetupTileCfg(
        int32_t index, int32_t macroModeIndex,
        ADDR_TILEINFO* pInfo, AddrTileMode* mode = NULL, AddrTileType* type = NULL) const;

    /// Overwrite macro tile config according to tile index
    virtual int32_t HwlComputeMacroModeIndex(
        int32_t index, ADDR_SURFACE_FLAGS flags, uint32_t bpp, uint32_t numSamples,
        ADDR_TILEINFO* pTileInfo, AddrTileMode *pTileMode = NULL, AddrTileType *pTileType = NULL
        ) const
    {
        return TileIndexNoMacroIndex;
    }

    /// Pre-handler of 3x pitch (96 bit) adjustment
    virtual uint32_t HwlPreHandleBaseLvl3xPitch(
        const ADDR_COMPUTE_SURFACE_INFO_INPUT* pIn, uint32_t expPitch) const;
    /// Post-handler of 3x pitch adjustment
    virtual uint32_t HwlPostHandleBaseLvl3xPitch(
        const ADDR_COMPUTE_SURFACE_INFO_INPUT* pIn, uint32_t expPitch) const;
    /// Check miplevel after surface adjustment
    ADDR_E_RETURNCODE PostComputeMipLevel(
        ADDR_COMPUTE_SURFACE_INFO_INPUT* pIn,
        ADDR_COMPUTE_SURFACE_INFO_OUTPUT* pOut) const;

    /// Quad buffer stereo support, has its implementation in ind. layer
    virtual BOOL_32 ComputeQbStereoInfo(
        ADDR_COMPUTE_SURFACE_INFO_OUTPUT* pOut) const;

    /// Pure virutual function to compute stereo bank swizzle for right eye
    virtual uint32_t HwlComputeQbStereoRightSwizzle(
        ADDR_COMPUTE_SURFACE_INFO_OUTPUT* pOut) const = 0;

private:
    // Disallow the copy constructor
    AddrLib(const AddrLib& a);

    // Disallow the assignment operator
    AddrLib& operator=(const AddrLib& a);

    VOID SetAddrChipFamily(uint32_t uChipFamily, uint32_t uChipRevision);

    uint32_t ComputeCmaskBaseAlign(
        ADDR_CMASK_FLAGS flags, ADDR_TILEINFO*  pTileInfo) const;

    uint64_t ComputeCmaskBytes(
        uint32_t pitch, uint32_t height, uint32_t numSlices) const;

    //
    // CMASK/HTILE shared methods
    //
    VOID    ComputeTileDataWidthAndHeight(
        uint32_t bpp, uint32_t cacheBits, ADDR_TILEINFO* pTileInfo,
        uint32_t* pMacroWidth, uint32_t* pMacroHeight) const;

    uint32_t ComputeXmaskCoordYFromPipe(
        uint32_t pipe, uint32_t x) const;

    VOID SetMinPitchAlignPixels(uint32_t minPitchAlignPixels);

    BOOL_32 DegradeBaseLevel(
        const ADDR_COMPUTE_SURFACE_INFO_INPUT* pIn, AddrTileMode* pTileMode) const;

protected:
    AddrLibClass        m_class;        ///< Store class type (HWL type)

    AddrChipFamily      m_chipFamily;   ///< Chip family translated from the one in atiid.h

    uint32_t            m_chipRevision; ///< Revision id from xxx_id.h

    uint32_t            m_version;      ///< Current version

    //
    // Global parameters
    //
    ADDR_CONFIG_FLAGS   m_configFlags;  ///< Global configuration flags. Note this is setup by
                                        ///  AddrLib instead of Client except forceLinearAligned

    uint32_t            m_pipes;        ///< Number of pipes
    uint32_t            m_banks;        ///< Number of banks
                                        ///  For r800 this is MC_ARB_RAMCFG.NOOFBANK
                                        ///  Keep it here to do default parameter calculation

    uint32_t            m_pipeInterleaveBytes;
                                        ///< Specifies the size of contiguous address space
                                        ///  within each tiling pipe when making linear
                                        ///  accesses. (Formerly Group Size)

    uint32_t            m_rowSize;      ///< DRAM row size, in bytes

    uint32_t            m_minPitchAlignPixels; ///< Minimum pitch alignment in pixels
    uint32_t            m_maxSamples;   ///< Max numSamples
private:
    AddrElemLib*        m_pElemLib;     ///< Element Lib pointer
};

AddrLib* AddrSIHwlInit  (const AddrClient* pClient);
AddrLib* AddrCIHwlInit  (const AddrClient* pClient);

#endif
