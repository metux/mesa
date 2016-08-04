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
* @file  addrinterface.h
* @brief Contains the addrlib interfaces declaration and parameter defines
***************************************************************************************************
*/
#ifndef __ADDR_INTERFACE_H__
#define __ADDR_INTERFACE_H__

#include <stdint.h>

#include "addrtypes.h"

#if defined(__cplusplus)
extern "C"
{
#endif

#define ADDRLIB_VERSION_MAJOR 5
#define ADDRLIB_VERSION_MINOR 25
#define ADDRLIB_VERSION ((ADDRLIB_VERSION_MAJOR << 16) | ADDRLIB_VERSION_MINOR)

/// Virtually all interface functions need ADDR_HANDLE as first parameter
typedef VOID*   ADDR_HANDLE;

/// Client handle used in callbacks
typedef VOID*   ADDR_CLIENT_HANDLE;

/**
* /////////////////////////////////////////////////////////////////////////////////////////////////
* //                                  Callback functions
* /////////////////////////////////////////////////////////////////////////////////////////////////
*    typedef VOID* (ADDR_API* ADDR_ALLOCSYSMEM)(
*         const ADDR_ALLOCSYSMEM_INPUT* pInput);
*    typedef ADDR_E_RETURNCODE (ADDR_API* ADDR_FREESYSMEM)(
*         VOID* pVirtAddr);
*    typedef ADDR_E_RETURNCODE (ADDR_API* ADDR_DEBUGPRINT)(
*         const ADDR_DEBUGPRINT_INPUT* pInput);
*
* /////////////////////////////////////////////////////////////////////////////////////////////////
* //                               Create/Destroy/Config functions
* /////////////////////////////////////////////////////////////////////////////////////////////////
*     AddrCreate()
*     AddrDestroy()
*
* /////////////////////////////////////////////////////////////////////////////////////////////////
* //                                  Surface functions
* /////////////////////////////////////////////////////////////////////////////////////////////////
*     AddrComputeSurfaceInfo()
*     AddrComputeSurfaceAddrFromCoord()
*     AddrComputeSurfaceCoordFromAddr()
*
* /////////////////////////////////////////////////////////////////////////////////////////////////
* //                                   HTile functions
* /////////////////////////////////////////////////////////////////////////////////////////////////
*     AddrComputeHtileInfo()
*     AddrComputeHtileAddrFromCoord()
*     AddrComputeHtileCoordFromAddr()
*
* /////////////////////////////////////////////////////////////////////////////////////////////////
* //                                   C-mask functions
* /////////////////////////////////////////////////////////////////////////////////////////////////
*     AddrComputeCmaskInfo()
*     AddrComputeCmaskAddrFromCoord()
*     AddrComputeCmaskCoordFromAddr()
*
* /////////////////////////////////////////////////////////////////////////////////////////////////
* //                                   F-mask functions
* /////////////////////////////////////////////////////////////////////////////////////////////////
*     AddrComputeFmaskInfo()
*     AddrComputeFmaskAddrFromCoord()
*     AddrComputeFmaskCoordFromAddr()
*
* /////////////////////////////////////////////////////////////////////////////////////////////////
* //                               Element/Utility functions
* /////////////////////////////////////////////////////////////////////////////////////////////////
*     ElemFlt32ToDepthPixel()
*     ElemFlt32ToColorPixel()
*     AddrExtractBankPipeSwizzle()
*     AddrCombineBankPipeSwizzle()
*     AddrComputeSliceSwizzle()
*     AddrConvertTileInfoToHW()
*     AddrConvertTileIndex()
*     AddrConvertTileIndex1()
*     AddrGetTileIndex()
*     AddrComputeBaseSwizzle()
*     AddrUseTileIndex()
*     AddrUseCombinedSwizzle()
*
* /////////////////////////////////////////////////////////////////////////////////////////////////
* //                                    Dump functions
* /////////////////////////////////////////////////////////////////////////////////////////////////
*     AddrDumpSurfaceInfo()
*     AddrDumpFmaskInfo()
*     AddrDumpCmaskInfo()
*     AddrDumpHtileInfo()
*
**/

///////////////////////////////////////////////////////////////////////////////////////////////////
//                                      Callback functions
///////////////////////////////////////////////////////////////////////////////////////////////////

/**
***************************************************************************************************
* @brief Alloc system memory flags.
* @note These flags are reserved for future use and if flags are added will minimize the impact
*       of the client.
***************************************************************************************************
*/
typedef union _ADDR_ALLOCSYSMEM_FLAGS
{
    struct
    {
        uint32_t reserved    : 32;  ///< Reserved for future use.
    } fields;
    uint32_t value;

} ADDR_ALLOCSYSMEM_FLAGS;

/**
***************************************************************************************************
* @brief Alloc system memory input structure
***************************************************************************************************
*/
typedef struct _ADDR_ALLOCSYSMEM_INPUT
{
    uint32_t                size;           ///< Size of this structure in bytes

    ADDR_ALLOCSYSMEM_FLAGS  flags;          ///< System memory flags.
    uint32_t                sizeInBytes;    ///< System memory allocation size in bytes.
    ADDR_CLIENT_HANDLE      hClient;        ///< Client handle
} ADDR_ALLOCSYSMEM_INPUT;

/**
***************************************************************************************************
* ADDR_ALLOCSYSMEM
*   @brief
*       Allocate system memory callback function. Returns valid pointer on success.
***************************************************************************************************
*/
typedef VOID* (ADDR_API* ADDR_ALLOCSYSMEM)(
    const ADDR_ALLOCSYSMEM_INPUT* pInput);

/**
***************************************************************************************************
* @brief Free system memory input structure
***************************************************************************************************
*/
typedef struct _ADDR_FREESYSMEM_INPUT
{
    uint32_t                size;           ///< Size of this structure in bytes

    VOID*                   pVirtAddr;      ///< Virtual address
    ADDR_CLIENT_HANDLE      hClient;        ///< Client handle
} ADDR_FREESYSMEM_INPUT;

/**
***************************************************************************************************
* ADDR_FREESYSMEM
*   @brief
*       Free system memory callback function.
*       Returns ADDR_OK on success.
***************************************************************************************************
*/
typedef ADDR_E_RETURNCODE (ADDR_API* ADDR_FREESYSMEM)(
    const ADDR_FREESYSMEM_INPUT* pInput);

/**
***************************************************************************************************
* @brief Print debug message input structure
***************************************************************************************************
*/
typedef struct _ADDR_DEBUGPRINT_INPUT
{
    uint32_t            size;           ///< Size of this structure in bytes

    CHAR*               pDebugString;   ///< Debug print string
    va_list             ap;             ///< Variable argument list
    ADDR_CLIENT_HANDLE  hClient;        ///< Client handle
} ADDR_DEBUGPRINT_INPUT;

/**
***************************************************************************************************
* ADDR_DEBUGPRINT
*   @brief
*       Print debug message callback function.
*       Returns ADDR_OK on success.
***************************************************************************************************
*/
typedef ADDR_E_RETURNCODE (ADDR_API* ADDR_DEBUGPRINT)(
    const ADDR_DEBUGPRINT_INPUT* pInput);

/**
***************************************************************************************************
* ADDR_CALLBACKS
*
*   @brief
*       Address Library needs client to provide system memory alloc/free routines.
***************************************************************************************************
*/
typedef struct _ADDR_CALLBACKS
{
    ADDR_ALLOCSYSMEM allocSysMem;   ///< Routine to allocate system memory
    ADDR_FREESYSMEM  freeSysMem;    ///< Routine to free system memory
    ADDR_DEBUGPRINT  debugPrint;    ///< Routine to print debug message
} ADDR_CALLBACKS;

///////////////////////////////////////////////////////////////////////////////////////////////////
//                               Create/Destroy functions
///////////////////////////////////////////////////////////////////////////////////////////////////

/**
***************************************************************************************************
* ADDR_CREATE_FLAGS
*
*   @brief
*       This structure is used to pass some setup in creation of AddrLib
*   @note
***************************************************************************************************
*/
typedef union _ADDR_CREATE_FLAGS
{
    struct
    {
        uint32_t noCubeMipSlicesPad     : 1;    ///< Turn cubemap faces padding off
        uint32_t fillSizeFields         : 1;    ///< If clients fill size fields in all input and
                                               ///  output structure
        uint32_t useTileIndex           : 1;    ///< Make tileIndex field in input valid
        uint32_t useCombinedSwizzle     : 1;    ///< Use combined tile swizzle
        uint32_t checkLast2DLevel       : 1;    ///< Check the last 2D mip sub level
        uint32_t useHtileSliceAlign     : 1;    ///< Do htile single slice alignment
        uint32_t degradeBaseLevel       : 1;    ///< Degrade to 1D modes automatically for base level
        uint32_t allowLargeThickTile    : 1;    ///< Allow 64*thickness*bytesPerPixel > rowSize
        uint32_t reserved               : 24;   ///< Reserved bits for future use
    };

    uint32_t value;
} ADDR_CREATE_FLAGS;

/**
***************************************************************************************************
*   ADDR_REGISTER_VALUE
*
*   @brief
*       Data from registers to setup AddrLib global data, used in AddrCreate
***************************************************************************************************
*/
typedef struct _ADDR_REGISTER_VALUE
{
    uint32_t gbAddrConfig;       ///< For R8xx, use GB_ADDR_CONFIG register value.
                                 ///  For R6xx/R7xx, use GB_TILING_CONFIG.
                                 ///  But they can be treated as the same.
                                 ///  if this value is 0, use chip to set default value
    uint32_t backendDisables;    ///< 1 bit per backend, starting with LSB. 1=disabled,0=enabled.
                                 ///  Register value of CC_RB_BACKEND_DISABLE.BACKEND_DISABLE

                                 ///  R800 registers-----------------------------------------------
    uint32_t noOfBanks;          ///< Number of h/w ram banks - For r800: MC_ARB_RAMCFG.NOOFBANK
                                 ///  No enums for this value in h/w header files
                                 ///  0: 4
                                 ///  1: 8
                                 ///  2: 16
    uint32_t noOfRanks;          ///  MC_ARB_RAMCFG.NOOFRANK
                                 ///  0: 1
                                 ///  1: 2
                                 ///  SI (R1000) registers-----------------------------------------
    const uint32_t* pTileConfig;      ///< Global tile setting tables
    uint32_t  noOfEntries;            ///< Number of entries in pTileConfig

                                      ///< CI registers-------------------------------------------------
    const uint32_t* pMacroTileConfig; ///< Global macro tile mode table
    uint32_t  noOfMacroEntries;       ///< Number of entries in pMacroTileConfig

} ADDR_REGISTER_VALUE;

/**
***************************************************************************************************
* ADDR_CREATE_INPUT
*
*   @brief
*       Parameters use to create an AddrLib Object. Caller must provide all fields.
*
***************************************************************************************************
*/
typedef struct _ADDR_CREATE_INPUT
{
    uint32_t            size;                ///< Size of this structure in bytes

    uint32_t            chipEngine;          ///< Chip Engine
    uint32_t            chipFamily;          ///< Chip Family
    uint32_t            chipRevision;        ///< Chip Revision
    ADDR_CALLBACKS      callbacks;           ///< Callbacks for sysmem alloc/free/print
    ADDR_CREATE_FLAGS   createFlags;         ///< Flags to setup AddrLib
    ADDR_REGISTER_VALUE regValue;            ///< Data from registers to setup AddrLib global data
    ADDR_CLIENT_HANDLE  hClient;             ///< Client handle
    uint32_t            minPitchAlignPixels; ///< Minimum pitch alignment in pixels
} ADDR_CREATE_INPUT;

/**
***************************************************************************************************
* ADDR_CREATEINFO_OUTPUT
*
*   @brief
*       Return AddrLib handle to client driver
*
***************************************************************************************************
*/
typedef struct _ADDR_CREATE_OUTPUT
{
    uint32_t    size;    ///< Size of this structure in bytes

    ADDR_HANDLE hLib;    ///< Address lib handle
} ADDR_CREATE_OUTPUT;

/**
***************************************************************************************************
*   AddrCreate
*
*   @brief
*       Create AddrLib object, must be called before any interface calls
*
*   @return
*       ADDR_OK if successful
***************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API AddrCreate(
    const ADDR_CREATE_INPUT*    pAddrCreateIn,
    ADDR_CREATE_OUTPUT*         pAddrCreateOut);



/**
***************************************************************************************************
*   AddrDestroy
*
*   @brief
*       Destroy AddrLib object, must be called to free internally allocated resources.
*
*   @return
*      ADDR_OK if successful
***************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API AddrDestroy(
    ADDR_HANDLE hLib);



///////////////////////////////////////////////////////////////////////////////////////////////////
//                                    Surface functions
///////////////////////////////////////////////////////////////////////////////////////////////////

/**
***************************************************************************************************
* @brief
*       Bank/tiling parameters. On function input, these can be set as desired or
*       left 0 for AddrLib to calculate/default. On function output, these are the actual
*       parameters used.
* @note
*       Valid bankWidth/bankHeight value:
*       1,2,4,8. They are factors instead of pixels or bytes.
*
*       The bank number remains constant across each row of the
*       macro tile as each pipe is selected, so the number of
*       tiles in the x direction with the same bank number will
*       be bank_width * num_pipes.
***************************************************************************************************
*/
typedef struct _ADDR_TILEINFO
{
    ///  Any of these parameters can be set to 0 to use the HW default.
    uint32_t    banks;              ///< Number of banks, numerical value
    uint32_t    bankWidth;          ///< Number of tiles in the X direction in the same bank
    uint32_t    bankHeight;         ///< Number of tiles in the Y direction in the same bank
    uint32_t    macroAspectRatio;   ///< Macro tile aspect ratio. 1-1:1, 2-4:1, 4-16:1, 8-64:1
    uint32_t    tileSplitBytes;     ///< Tile split size, in bytes
    AddrPipeCfg pipeConfig;         ///< Pipe Config = HW enum + 1
} ADDR_TILEINFO;

// Create a define to avoid client change. The removal of R800 is because we plan to implement SI
// within 800 HWL - An AddrPipeCfg is added in above data structure
typedef ADDR_TILEINFO ADDR_R800_TILEINFO;

/**
***************************************************************************************************
* @brief
*       Information needed by quad buffer stereo support
***************************************************************************************************
*/
typedef struct _ADDR_QBSTEREOINFO
{
    uint32_t        eyeHeight;          ///< Height (in pixel rows) to right eye
    uint32_t        rightOffset;        ///< Offset (in bytes) to right eye
    uint32_t        rightSwizzle;       ///< TileSwizzle for right eyes
} ADDR_QBSTEREOINFO;

/**
***************************************************************************************************
*   ADDR_SURFACE_FLAGS
*
*   @brief
*       Surface flags
***************************************************************************************************
*/
typedef union _ADDR_SURFACE_FLAGS
{
    struct
    {
        uint32_t color            : 1; ///< Flag indicates this is a color buffer
        uint32_t depth            : 1; ///< Flag indicates this is a depth/stencil buffer
        uint32_t stencil          : 1; ///< Flag indicates this is a stencil buffer
        uint32_t texture          : 1; ///< Flag indicates this is a texture
        uint32_t cube             : 1; ///< Flag indicates this is a cubemap

        uint32_t volume           : 1; ///< Flag indicates this is a volume texture
        uint32_t fmask            : 1; ///< Flag indicates this is an fmask
        uint32_t cubeAsArray      : 1; ///< Flag indicates if treat cubemap as arrays
        uint32_t compressZ        : 1; ///< Flag indicates z buffer is compressed
        uint32_t overlay          : 1; ///< Flag indicates this is an overlay surface
        uint32_t noStencil        : 1; ///< Flag indicates this depth has no separate stencil
        uint32_t display          : 1; ///< Flag indicates this should match display controller req.
        uint32_t opt4Space        : 1; ///< Flag indicates this surface should be optimized for space
                                       ///  i.e. save some memory but may lose performance
        uint32_t prt              : 1; ///< Flag for partially resident texture
        uint32_t qbStereo         : 1; ///< Quad buffer stereo surface
        uint32_t pow2Pad          : 1; ///< SI: Pad to pow2, must set for mipmap (include level0)
        uint32_t interleaved      : 1; ///< Special flag for interleaved YUV surface padding
        uint32_t degrade4Space    : 1; ///< Degrade base level's tile mode to save memory
        uint32_t tcCompatible     : 1; ///< Flag indicates surface needs to be shader readable
        uint32_t dispTileType     : 1; ///< NI: force display Tiling for 128 bit shared resoruce
        uint32_t dccCompatible    : 1; ///< VI: whether to support dcc fast clear
        uint32_t czDispCompatible : 1; ///< SI+: CZ family (Carrizo) has a HW bug needs special alignment.
                                       ///<      This flag indicates we need to follow the alignment with
                                       ///<      CZ families or other ASICs under PX configuration + CZ.
        uint32_ reserved          :10; ///< Reserved bits
    };

    uint32_t value;
} ADDR_SURFACE_FLAGS;

/**
***************************************************************************************************
*   ADDR_COMPUTE_SURFACE_INFO_INPUT
*
*   @brief
*       Input structure for AddrComputeSurfaceInfo
***************************************************************************************************
*/
typedef struct _ADDR_COMPUTE_SURFACE_INFO_INPUT
{
    uint32_t            size;               ///< Size of this structure in bytes

    AddrTileMode        tileMode;           ///< Tile mode
    AddrFormat          format;             ///< If format is set to valid one, bpp/width/height
                                            ///  might be overwritten
    uint32_t            bpp;                ///< Bits per pixel
    uint32_t            numSamples;         ///< Number of samples
    uint32_t            width;              ///< Width, in pixels
    uint32_t            height;             ///< Height, in pixels
    uint32_t            numSlices;          ///< Number surface slice/depth,
                                            ///  Note:
                                            ///  For cubemap, driver clients usually set numSlices
                                            ///  to 1 in per-face calc.
                                            ///  For 7xx and above, we need pad faces as slices.
                                            ///  In this case, clients should set numSlices to 6 and
                                            ///  this is also can be turned off by createFlags when
                                            ///  calling AddrCreate
    uint32_t            slice;              ///< Slice index
    uint32_t            mipLevel;           ///< Current mipmap level.
                                            ///  Padding/tiling have different rules for level0 and
                                            ///  sublevels
    ADDR_SURFACE_FLAGS  flags;              ///< Surface type flags
    uint32_t            numFrags;           ///< Number of fragments, leave it zero or the same as
                                            ///  number of samples for normal AA; Set it to the
                                            ///  number of fragments for EQAA
    /// r800 and later HWL parameters
    // Needed by 2D tiling, for linear and 1D tiling, just keep them 0's
    ADDR_TILEINFO*      pTileInfo;          ///< 2D tile parameters. Set to 0 to default/calculate
    AddrTileType        tileType;           ///< Micro tiling type, not needed when tileIndex != -1
    int32_t             tileIndex;          ///< Tile index, MUST be -1 if you don't want to use it
                                            ///  while the global useTileIndex is set to 1
    uint32_t            basePitch;          ///< Base level pitch in pixels, 0 means ignored, is a
                                            ///  must for mip levels from SI+.
                                            ///  Don't use pitch in blocks for compressed formats!
} ADDR_COMPUTE_SURFACE_INFO_INPUT;

/**
***************************************************************************************************
*   ADDR_COMPUTE_SURFACE_INFO_OUTPUT
*
*   @brief
*       Output structure for AddrComputeSurfInfo
*   @note
        Element: AddrLib unit for computing. e.g. BCn: 4x4 blocks; R32B32B32: 32bit with 3x pitch
        Pixel: Original pixel
***************************************************************************************************
*/
typedef struct _ADDR_COMPUTE_SURFACE_INFO_OUTPUT
{
    uint32_t        size;           ///< Size of this structure in bytes

    uint32_t        pitch;          ///< Pitch in elements (in blocks for compressed formats)
    uint32_t        height;         ///< Height in elements (in blocks for compressed formats)
    uint32_t        depth;          ///< Number of slice/depth
    uint64_t        surfSize;       ///< Surface size in bytes
    AddrTileMode    tileMode;       ///< Actual tile mode. May differ from that in input
    uint32_t        baseAlign;      ///< Base address alignment
    uint32_t        pitchAlign;     ///< Pitch alignment, in elements
    uint32_t        heightAlign;    ///< Height alignment, in elements
    uint32_t        depthAlign;     ///< Depth alignment, aligned to thickness, for 3d texture
    uint32_t        bpp;            ///< Bits per elements (e.g. blocks for BCn, 1/3 for 96bit)
    uint32_t        pixelPitch;     ///< Pitch in original pixels
    uint32_t        pixelHeight;    ///< Height in original pixels
    uint32_t        pixelBits;      ///< Original bits per pixel, passed from input
    uint64_t        sliceSize;      ///< Size of slice specified by input's slice
                                    ///  The result is controlled by surface flags & createFlags
                                    ///  By default this value equals to surfSize for volume
    uint32_t        pitchTileMax;   ///< PITCH_TILE_MAX value for h/w register
    uint32_t        heightTileMax;  ///< HEIGHT_TILE_MAX value for h/w register
    uint32_t        sliceTileMax;   ///< SLICE_TILE_MAX value for h/w register

    uint32_t        numSamples;     ///< Pass the effective numSamples processed in this call

    /// r800 and later HWL parameters
    ADDR_TILEINFO*  pTileInfo;      ///< Tile parameters used. Filled in if 0 on input
    AddrTileType    tileType;       ///< Micro tiling type, only valid when tileIndex != -1
    int32_t         tileIndex;      ///< Tile index, MAY be "downgraded"

    int32_t         macroModeIndex; ///< Index in macro tile mode table if there is one (CI)
    /// Special information to work around SI mipmap swizzle bug UBTS #317508
    BOOL_32         last2DLevel;    ///< TRUE if this is the last 2D(3D) tiled
                                    ///< Only meaningful when create flag checkLast2DLevel is set
    /// Stereo info
    ADDR_QBSTEREOINFO*  pStereoInfo;///< Stereo information, needed when .qbStereo flag is TRUE
} ADDR_COMPUTE_SURFACE_INFO_OUTPUT;

/**
***************************************************************************************************
*   AddrComputeSurfaceInfo
*
*   @brief
*       Compute surface width/height/depth/alignments and suitable tiling mode
***************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API AddrComputeSurfaceInfo(
    ADDR_HANDLE                             hLib,
    const ADDR_COMPUTE_SURFACE_INFO_INPUT*  pIn,
    ADDR_COMPUTE_SURFACE_INFO_OUTPUT*       pOut);



/**
***************************************************************************************************
*   ADDR_COMPUTE_SURFACE_ADDRFROMCOORD_INPUT
*
*   @brief
*       Input structure for AddrComputeSurfaceAddrFromCoord
***************************************************************************************************
*/
typedef struct _ADDR_COMPUTE_SURFACE_ADDRFROMCOORD_INPUT
{
    uint32_t        size;               ///< Size of this structure in bytes

    uint32_t        x;                  ///< X coordinate
    uint32_t        y;                  ///< Y coordinate
    uint32_t        slice;              ///< Slice index
    uint32_t        sample;             ///< Sample index, use fragment index for EQAA

    uint32_t        bpp;                ///< Bits per pixel
    uint32_t        pitch;              ///< Surface pitch, in pixels
    uint32_t        height;             ///< Surface height, in pixels
    uint32_t        numSlices;          ///< Surface depth
    uint32_t        numSamples;         ///< Number of samples

    AddrTileMode    tileMode;           ///< Tile mode
    BOOL_32         isDepth;            ///< TRUE if the surface uses depth sample ordering within
                                        ///  micro tile. Textures can also choose depth sample order
    uint32_t        tileBase;           ///< Base offset (in bits) inside micro tile which handles
                                        ///  the case that components are stored separately
    uint32_t        compBits;           ///< The component bits actually needed(for planar surface)

    uint32_t        numFrags;           ///< Number of fragments, leave it zero or the same as
                                        ///  number of samples for normal AA; Set it to the
                                        ///  number of fragments for EQAA
    /// r800 and later HWL parameters
    // Used for 1D tiling above
    AddrTileType    tileType;           ///< See defintion of AddrTileType
    struct
    {
        uint32_t    ignoreSE : 1;       ///< TRUE if shader engines are ignored. This is texture
                                        ///  only flag. Only non-RT texture can set this to TRUE
        uint32_t    reserved :31;       ///< Reserved for future use.
    };
    // 2D tiling needs following structure
    ADDR_TILEINFO*  pTileInfo;          ///< 2D tile parameters. Client must provide all data
    int32_t         tileIndex;          ///< Tile index, MUST be -1 if you don't want to use it
                                        ///  while the global useTileIndex is set to 1
    union
    {
        struct
        {
            uint32_t bankSwizzle;       ///< Bank swizzle
            uint32_t pipeSwizzle;       ///< Pipe swizzle
        };
        uint32_t    tileSwizzle;        ///< Combined swizzle, if useCombinedSwizzle is TRUE
    };

#if ADDR_AM_BUILD // These two fields are not valid in SW blt since no HTILE access
    uint32_t        addr5Swizzle;       ///< ADDR5_SWIZZLE_MASK of DB_DEPTH_INFO
    BOOL_32         is32ByteTile;       ///< Caller must have access to HTILE buffer and know if
                                        ///  this tile is compressed to 32B
#endif
} ADDR_COMPUTE_SURFACE_ADDRFROMCOORD_INPUT;

/**
***************************************************************************************************
*   ADDR_COMPUTE_SURFACE_ADDRFROMCOORD_OUTPUT
*
*   @brief
*       Output structure for AddrComputeSurfaceAddrFromCoord
***************************************************************************************************
*/
typedef struct _ADDR_COMPUTE_SURFACE_ADDRFROMCOORD_OUTPUT
{
    uint32_t size;           ///< Size of this structure in bytes

    uint64_t addr;           ///< Byte address
    uint32_t bitPosition;    ///< Bit position within surfaceAddr, 0-7.
                             ///  For surface bpp < 8, e.g. FMT_1.
    uint32_t prtBlockIndex;  ///< Index of a PRT tile (64K block)
} ADDR_COMPUTE_SURFACE_ADDRFROMCOORD_OUTPUT;

/**
***************************************************************************************************
*   AddrComputeSurfaceAddrFromCoord
*
*   @brief
*       Compute surface address from a given coordinate.
***************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API AddrComputeSurfaceAddrFromCoord(
    ADDR_HANDLE                                     hLib,
    const ADDR_COMPUTE_SURFACE_ADDRFROMCOORD_INPUT* pIn,
    ADDR_COMPUTE_SURFACE_ADDRFROMCOORD_OUTPUT*      pOut);



/**
***************************************************************************************************
*   ADDR_COMPUTE_SURFACE_COORDFROMADDR_INPUT
*
*   @brief
*       Input structure for AddrComputeSurfaceCoordFromAddr
***************************************************************************************************
*/
typedef struct _ADDR_COMPUTE_SURFACE_COORDFROMADDR_INPUT
{
    uint32_t        size;               ///< Size of this structure in bytes

    uint64_t        addr;               ///< Address in bytes
    uint32_t        bitPosition;        ///< Bit position in addr. 0-7. for surface bpp < 8,
                                        ///  e.g. FMT_1;
    uint32_t        bpp;                ///< Bits per pixel
    uint32_t        pitch;              ///< Pitch, in pixels
    uint32_t        height;             ///< Height in pixels
    uint32_t        numSlices;          ///< Surface depth
    uint32_t        numSamples;         ///< Number of samples

    AddrTileMode    tileMode;           ///< Tile mode
    BOOL_32         isDepth;            ///< Surface uses depth sample ordering within micro tile.
                                        ///  Note: Textures can choose depth sample order as well.
    uint32_t        tileBase;           ///< Base offset (in bits) inside micro tile which handles
                                        ///  the case that components are stored separately
    uint32_t        compBits;           ///< The component bits actually needed(for planar surface)

    uint32_t        numFrags;           ///< Number of fragments, leave it zero or the same as
                                        ///  number of samples for normal AA; Set it to the
                                        ///  number of fragments for EQAA
    /// r800 and later HWL parameters
    // Used for 1D tiling above
    AddrTileType    tileType;           ///< See defintion of AddrTileType
    struct
    {
        uint32_t    ignoreSE : 1;       ///< TRUE if shader engines are ignored. This is texture
                                        ///  only flag. Only non-RT texture can set this to TRUE
        uint32_t    reserved :31;       ///< Reserved for future use.
    };
    // 2D tiling needs following structure
    ADDR_TILEINFO*  pTileInfo;          ///< 2D tile parameters. Client must provide all data
    int32_t         tileIndex;          ///< Tile index, MUST be -1 if you don't want to use it
                                        ///  while the global useTileIndex is set to 1
    union
    {
        struct
        {
            uint32_t bankSwizzle;       ///< Bank swizzle
            uint32_t pipeSwizzle;       ///< Pipe swizzle
        };
        uint32_t    tileSwizzle;        ///< Combined swizzle, if useCombinedSwizzle is TRUE
    };
} ADDR_COMPUTE_SURFACE_COORDFROMADDR_INPUT;

/**
***************************************************************************************************
*   ADDR_COMPUTE_SURFACE_COORDFROMADDR_OUTPUT
*
*   @brief
*       Output structure for AddrComputeSurfaceCoordFromAddr
***************************************************************************************************
*/
typedef struct _ADDR_COMPUTE_SURFACE_COORDFROMADDR_OUTPUT
{
    uint32_t size;   ///< Size of this structure in bytes

    uint32_t x;      ///< X coordinate
    uint32_t y;      ///< Y coordinate
    uint32_t slice;  ///< Index of slices
    uint32_t sample; ///< Index of samples, means fragment index for EQAA
} ADDR_COMPUTE_SURFACE_COORDFROMADDR_OUTPUT;

/**
***************************************************************************************************
*   AddrComputeSurfaceCoordFromAddr
*
*   @brief
*       Compute coordinate from a given surface address
***************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API AddrComputeSurfaceCoordFromAddr(
    ADDR_HANDLE                                     hLib,
    const ADDR_COMPUTE_SURFACE_COORDFROMADDR_INPUT* pIn,
    ADDR_COMPUTE_SURFACE_COORDFROMADDR_OUTPUT*      pOut);

///////////////////////////////////////////////////////////////////////////////////////////////////
//                                   HTile functions
///////////////////////////////////////////////////////////////////////////////////////////////////

/**
***************************************************************************************************
*   ADDR_HTILE_FLAGS
*
*   @brief
*       HTILE flags
***************************************************************************************************
*/
typedef union _ADDR_HTILE_FLAGS
{
    struct
    {
        uint32_t tcCompatible  : 1; ///< Flag indicates surface needs to be shader readable
        uint32_t reserved      :31; ///< Reserved bits
    };

    uint32_t value;
} ADDR_HTILE_FLAGS;

/**
***************************************************************************************************
*   ADDR_COMPUTE_HTILE_INFO_INPUT
*
*   @brief
*       Input structure of AddrComputeHtileInfo
***************************************************************************************************
*/
typedef struct _ADDR_COMPUTE_HTILE_INFO_INPUT
{
    uint32_t           size;            ///< Size of this structure in bytes

    ADDR_HTILE_FLAGS   flags;           ///< HTILE flags
    uint32_t           pitch;           ///< Surface pitch, in pixels
    uint32_t           height;          ///< Surface height, in pixels
    uint32_t           numSlices;       ///< Number of slices
    BOOL_32            isLinear;        ///< Linear or tiled HTILE layout
    AddrHtileBlockSize blockWidth;      ///< 4 or 8. EG above only support 8
    AddrHtileBlockSize blockHeight;     ///< 4 or 8. EG above only support 8
    ADDR_TILEINFO*     pTileInfo;       ///< Tile info

    int32_t            tileIndex;       ///< Tile index, MUST be -1 if you don't want to use it
                                        ///  while the global useTileIndex is set to 1
    int32_t            macroModeIndex;  ///< Index in macro tile mode table if there is one (CI)
                                        ///< README: When tileIndex is not -1, this must be valid
} ADDR_COMPUTE_HTILE_INFO_INPUT;

/**
***************************************************************************************************
*   ADDR_COMPUTE_HTILE_INFO_OUTPUT
*
*   @brief
*       Output structure of AddrComputeHtileInfo
***************************************************************************************************
*/
typedef struct _ADDR_COMPUTE_HTILE_INFO_OUTPUT
{
    uint32_t size;          ///< Size of this structure in bytes

    uint32_t pitch;         ///< Pitch in pixels of depth buffer represented in this
                            ///  HTile buffer. This might be larger than original depth
                            ///  buffer pitch when called with an unaligned pitch.
    uint32_t height;        ///< Height in pixels, as above
    uint64_t htileBytes;    ///< Size of HTILE buffer, in bytes
    uint32_t baseAlign;     ///< Base alignment
    uint32_t bpp;           ///< Bits per pixel for HTILE is how many bits for an 8x8 block!
    uint32_t macroWidth;    ///< Macro width in pixels, actually squared cache shape
    uint32_t macroHeight;   ///< Macro height in pixels
    uint64_t sliceSize;     ///< Slice size, in bytes.
} ADDR_COMPUTE_HTILE_INFO_OUTPUT;

/**
***************************************************************************************************
*   AddrComputeHtileInfo
*
*   @brief
*       Compute Htile pitch, height, base alignment and size in bytes
***************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API AddrComputeHtileInfo(
    ADDR_HANDLE                             hLib,
    const ADDR_COMPUTE_HTILE_INFO_INPUT*    pIn,
    ADDR_COMPUTE_HTILE_INFO_OUTPUT*         pOut);



/**
***************************************************************************************************
*   ADDR_COMPUTE_HTILE_ADDRFROMCOORD_INPUT
*
*   @brief
*       Input structure for AddrComputeHtileAddrFromCoord
***************************************************************************************************
*/
typedef struct _ADDR_COMPUTE_HTILE_ADDRFROMCOORD_INPUT
{
    uint32_t           size;            ///< Size of this structure in bytes

    uint32_t           pitch;           ///< Pitch, in pixels
    uint32_t           height;          ///< Height in pixels
    uint32_t           x;               ///< X coordinate
    uint32_t           y;               ///< Y coordinate
    uint32_t           slice;           ///< Index of slice
    uint32_t           numSlices;       ///< Number of slices
    BOOL_32            isLinear;        ///< Linear or tiled HTILE layout
    AddrHtileBlockSize blockWidth;      ///< 4 or 8. 1 means 8, 0 means 4. EG above only support 8
    AddrHtileBlockSize blockHeight;     ///< 4 or 8. 1 means 8, 0 means 4. EG above only support 8
    ADDR_TILEINFO*     pTileInfo;       ///< Tile info

    int32_t            tileIndex;       ///< Tile index, MUST be -1 if you don't want to use it
                                        ///  while the global useTileIndex is set to 1
    int32_t            macroModeIndex;  ///< Index in macro tile mode table if there is one (CI)
                                        ///< README: When tileIndex is not -1, this must be valid
} ADDR_COMPUTE_HTILE_ADDRFROMCOORD_INPUT;

/**
***************************************************************************************************
*   ADDR_COMPUTE_HTILE_ADDRFROMCOORD_OUTPUT
*
*   @brief
*       Output structure for AddrComputeHtileAddrFromCoord
***************************************************************************************************
*/
typedef struct _ADDR_COMPUTE_HTILE_ADDRFROMCOORD_OUTPUT
{
    uint32_t size;          ///< Size of this structure in bytes

    uint64_t addr;          ///< Address in bytes
    uint32_t bitPosition;   ///< Bit position, 0 or 4. CMASK and HTILE shares some lib method.
                            ///  So we keep bitPosition for HTILE as well
} ADDR_COMPUTE_HTILE_ADDRFROMCOORD_OUTPUT;

/**
***************************************************************************************************
*   AddrComputeHtileAddrFromCoord
*
*   @brief
*       Compute Htile address according to coordinates (of depth buffer)
***************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API AddrComputeHtileAddrFromCoord(
    ADDR_HANDLE                                     hLib,
    const ADDR_COMPUTE_HTILE_ADDRFROMCOORD_INPUT*   pIn,
    ADDR_COMPUTE_HTILE_ADDRFROMCOORD_OUTPUT*        pOut);



/**
***************************************************************************************************
*   ADDR_COMPUTE_HTILE_COORDFROMADDR_INPUT
*
*   @brief
*       Input structure for AddrComputeHtileCoordFromAddr
***************************************************************************************************
*/
typedef struct _ADDR_COMPUTE_HTILE_COORDFROMADDR_INPUT
{
    uint32_t           size;            ///< Size of this structure in bytes

    uint64_t           addr;            ///< Address
    uint32_t           bitPosition;     ///< Bit position 0 or 4. CMASK and HTILE share some methods
                                        ///  so we keep bitPosition for HTILE as well
    uint32_t           pitch;           ///< Pitch, in pixels
    uint32_t           height;          ///< Height, in pixels
    uint32_t           numSlices;       ///< Number of slices
    BOOL_32            isLinear;        ///< Linear or tiled HTILE layout
    AddrHtileBlockSize blockWidth;      ///< 4 or 8. 1 means 8, 0 means 4. R8xx/R9xx only support 8
    AddrHtileBlockSize blockHeight;     ///< 4 or 8. 1 means 8, 0 means 4. R8xx/R9xx only support 8
    ADDR_TILEINFO*     pTileInfo;       ///< Tile info

    int32_t            tileIndex;       ///< Tile index, MUST be -1 if you don't want to use it
                                        ///  while the global useTileIndex is set to 1
    int32_t            macroModeIndex;  ///< Index in macro tile mode table if there is one (CI)
                                        ///< README: When tileIndex is not -1, this must be valid
} ADDR_COMPUTE_HTILE_COORDFROMADDR_INPUT;

/**
***************************************************************************************************
*   ADDR_COMPUTE_HTILE_COORDFROMADDR_OUTPUT
*
*   @brief
*       Output structure for AddrComputeHtileCoordFromAddr
***************************************************************************************************
*/
typedef struct _ADDR_COMPUTE_HTILE_COORDFROMADDR_OUTPUT
{
    uint32_t size;   ///< Size of this structure in bytes

    uint32_t x;      ///< X coordinate
    uint32_t y;      ///< Y coordinate
    uint32_t slice;  ///< Slice index
} ADDR_COMPUTE_HTILE_COORDFROMADDR_OUTPUT;

/**
***************************************************************************************************
*   AddrComputeHtileCoordFromAddr
*
*   @brief
*       Compute coordinates within depth buffer (1st pixel of a micro tile) according to
*       Htile address
***************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API AddrComputeHtileCoordFromAddr(
    ADDR_HANDLE                                     hLib,
    const ADDR_COMPUTE_HTILE_COORDFROMADDR_INPUT*   pIn,
    ADDR_COMPUTE_HTILE_COORDFROMADDR_OUTPUT*        pOut);



///////////////////////////////////////////////////////////////////////////////////////////////////
//                                     C-mask functions
///////////////////////////////////////////////////////////////////////////////////////////////////

/**
***************************************************************************************************
*   ADDR_CMASK_FLAGS
*
*   @brief
*       CMASK flags
***************************************************************************************************
*/
typedef union _ADDR_CMASK_FLAGS
{
    struct
    {
        uint32_t tcCompatible  : 1; ///< Flag indicates surface needs to be shader readable
        uint32_t reserved      :31; ///< Reserved bits
    };

    uint32_t value;
} ADDR_CMASK_FLAGS;

/**
***************************************************************************************************
*   ADDR_COMPUTE_CMASK_INFO_INPUT
*
*   @brief
*       Input structure of AddrComputeCmaskInfo
***************************************************************************************************
*/
typedef struct _ADDR_COMPUTE_CMASKINFO_INPUT
{
    uint32_t            size;            ///< Size of this structure in bytes

    ADDR_CMASK_FLAGS    flags;           ///< CMASK flags
    uint32_t            pitch;           ///< Pitch, in pixels, of color buffer
    uint32_t            height;          ///< Height, in pixels, of color buffer
    uint32_t            numSlices;       ///< Number of slices, of color buffer
    BOOL_32             isLinear;        ///< Linear or tiled layout, Only SI can be linear
    ADDR_TILEINFO*      pTileInfo;       ///< Tile info

    int32_t             tileIndex;       ///< Tile index, MUST be -1 if you don't want to use it
                                         ///  while the global useTileIndex is set to 1
    int32_t             macroModeIndex;  ///< Index in macro tile mode table if there is one (CI)
                                         ///< README: When tileIndex is not -1, this must be valid
} ADDR_COMPUTE_CMASK_INFO_INPUT;

/**
***************************************************************************************************
*   ADDR_COMPUTE_CMASK_INFO_OUTPUT
*
*   @brief
*       Output structure of AddrComputeCmaskInfo
***************************************************************************************************
*/
typedef struct _ADDR_COMPUTE_CMASK_INFO_OUTPUT
{
    uint32_t size;          ///< Size of this structure in bytes

    uint32_t pitch;         ///< Pitch in pixels of color buffer which
                            ///  this Cmask matches. The size might be larger than
                            ///  original color buffer pitch when called with
                            ///  an unaligned pitch.
    uint32_t height;        ///< Height in pixels, as above
    uint64_t cmaskBytes;    ///< Size in bytes of CMask buffer
    uint32_t baseAlign;     ///< Base alignment
    uint32_t blockMax;      ///< Cmask block size. Need this to set CB_COLORn_MASK register
    uint32_t macroWidth;    ///< Macro width in pixels, actually squared cache shape
    uint32_t macroHeight;   ///< Macro height in pixels
    uint64_t sliceSize;     ///< Slice size, in bytes.
} ADDR_COMPUTE_CMASK_INFO_OUTPUT;

/**
***************************************************************************************************
*   AddrComputeCmaskInfo
*
*   @brief
*       Compute Cmask pitch, height, base alignment and size in bytes from color buffer
*       info
***************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API AddrComputeCmaskInfo(
    ADDR_HANDLE                             hLib,
    const ADDR_COMPUTE_CMASK_INFO_INPUT*    pIn,
    ADDR_COMPUTE_CMASK_INFO_OUTPUT*         pOut);



/**
***************************************************************************************************
*   ADDR_COMPUTE_CMASK_ADDRFROMCOORD_INPUT
*
*   @brief
*       Input structure for AddrComputeCmaskAddrFromCoord
*
***************************************************************************************************
*/
typedef struct _ADDR_COMPUTE_CMASK_ADDRFROMCOORD_INPUT
{
    uint32_t         size;           ///< Size of this structure in bytes
    uint32_t         x;              ///< X coordinate
    uint32_t         y;              ///< Y coordinate
    uint64_t         fmaskAddr;      ///< Fmask addr for tc compatible Cmask
    uint32_t         slice;          ///< Slice index
    uint32_t         pitch;          ///< Pitch in pixels, of color buffer
    uint32_t         height;         ///< Height in pixels, of color buffer
    uint32_t         numSlices;      ///< Number of slices
    uint32_t         bpp;
    BOOL_32          isLinear;       ///< Linear or tiled layout, Only SI can be linear
    ADDR_CMASK_FLAGS flags;          ///< CMASK flags
    ADDR_TILEINFO*   pTileInfo;      ///< Tile info

    int32_t          tileIndex;      ///< Tile index, MUST be -1 if you don't want to use it
                                     ///< while the global useTileIndex is set to 1
    int32_t          macroModeIndex; ///< Index in macro tile mode table if there is one (CI)
                                     ///< README: When tileIndex is not -1, this must be valid
} ADDR_COMPUTE_CMASK_ADDRFROMCOORD_INPUT;

/**
***************************************************************************************************
*   ADDR_COMPUTE_CMASK_ADDRFROMCOORD_OUTPUT
*
*   @brief
*       Output structure for AddrComputeCmaskAddrFromCoord
***************************************************************************************************
*/
typedef struct _ADDR_COMPUTE_CMASK_ADDRFROMCOORD_OUTPUT
{
    uint32_t size;          ///< Size of this structure in bytes

    uint64_t addr;          ///< CMASK address in bytes
    uint32_t bitPosition;   ///< Bit position within addr, 0-7. CMASK is 4 bpp,
                            ///  so the address may be located in bit 0 (0) or 4 (4)
} ADDR_COMPUTE_CMASK_ADDRFROMCOORD_OUTPUT;

/**
***************************************************************************************************
*   AddrComputeCmaskAddrFromCoord
*
*   @brief
*       Compute Cmask address according to coordinates (of MSAA color buffer)
***************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API AddrComputeCmaskAddrFromCoord(
    ADDR_HANDLE                                     hLib,
    const ADDR_COMPUTE_CMASK_ADDRFROMCOORD_INPUT*   pIn,
    ADDR_COMPUTE_CMASK_ADDRFROMCOORD_OUTPUT*        pOut);



/**
***************************************************************************************************
*   ADDR_COMPUTE_CMASK_COORDFROMADDR_INPUT
*
*   @brief
*       Input structure for AddrComputeCmaskCoordFromAddr
***************************************************************************************************
*/
typedef struct _ADDR_COMPUTE_CMASK_COORDFROMADDR_INPUT
{
    uint32_t       size;            ///< Size of this structure in bytes

    uint64_t       addr;            ///< CMASK address in bytes
    uint32_t       bitPosition;     ///< Bit position within addr, 0-7. CMASK is 4 bpp,
                                    ///  so the address may be located in bit 0 (0) or 4 (4)
    uint32_t       pitch;           ///< Pitch, in pixels
    uint32_t       height;          ///< Height in pixels
    uint32_t       numSlices;       ///< Number of slices
    BOOL_32        isLinear;        ///< Linear or tiled layout, Only SI can be linear
    ADDR_TILEINFO* pTileInfo;       ///< Tile info

    int32_t        tileIndex;       ///< Tile index, MUST be -1 if you don't want to use it
                                    ///  while the global useTileIndex is set to 1
    int32_t        macroModeIndex;  ///< Index in macro tile mode table if there is one (CI)
                                    ///< README: When tileIndex is not -1, this must be valid
} ADDR_COMPUTE_CMASK_COORDFROMADDR_INPUT;

/**
***************************************************************************************************
*   ADDR_COMPUTE_CMASK_COORDFROMADDR_OUTPUT
*
*   @brief
*       Output structure for AddrComputeCmaskCoordFromAddr
***************************************************************************************************
*/
typedef struct _ADDR_COMPUTE_CMASK_COORDFROMADDR_OUTPUT
{
    uint32_t size;   ///< Size of this structure in bytes

    uint32_t x;      ///< X coordinate
    uint32_t y;      ///< Y coordinate
    uint32_t slice;  ///< Slice index
} ADDR_COMPUTE_CMASK_COORDFROMADDR_OUTPUT;

/**
***************************************************************************************************
*   AddrComputeCmaskCoordFromAddr
*
*   @brief
*       Compute coordinates within color buffer (1st pixel of a micro tile) according to
*       Cmask address
***************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API AddrComputeCmaskCoordFromAddr(
    ADDR_HANDLE                                     hLib,
    const ADDR_COMPUTE_CMASK_COORDFROMADDR_INPUT*   pIn,
    ADDR_COMPUTE_CMASK_COORDFROMADDR_OUTPUT*        pOut);



///////////////////////////////////////////////////////////////////////////////////////////////////
//                                     F-mask functions
///////////////////////////////////////////////////////////////////////////////////////////////////

/**
***************************************************************************************************
*   ADDR_COMPUTE_FMASK_INFO_INPUT
*
*   @brief
*       Input structure for AddrComputeFmaskInfo
***************************************************************************************************
*/
typedef struct _ADDR_COMPUTE_FMASK_INFO_INPUT
{
    uint32_t        size;               ///< Size of this structure in bytes

    AddrTileMode    tileMode;           ///< Tile mode
    uint32_t        pitch;              ///< Surface pitch, in pixels
    uint32_t        height;             ///< Surface height, in pixels
    uint32_t        numSlices;          ///< Number of slice/depth
    uint32_t        numSamples;         ///< Number of samples
    uint32_t        numFrags;           ///< Number of fragments, leave it zero or the same as
                                        ///  number of samples for normal AA; Set it to the
                                        ///  number of fragments for EQAA
    /// r800 and later HWL parameters
    struct
    {
        uint32_t resolved:   1;         ///< TRUE if the surface is for resolved fmask, only used
                                        ///  by H/W clients. S/W should always set it to FALSE.
        uint32_t reserved:  31;         ///< Reserved for future use.
    };
    ADDR_TILEINFO*  pTileInfo;          ///< 2D tiling parameters. Clients must give valid data
    int32_t         tileIndex;          ///< Tile index, MUST be -1 if you don't want to use it
                                        ///  while the global useTileIndex is set to 1
} ADDR_COMPUTE_FMASK_INFO_INPUT;

/**
***************************************************************************************************
*   ADDR_COMPUTE_FMASK_INFO_OUTPUT
*
*   @brief
*       Output structure for AddrComputeFmaskInfo
***************************************************************************************************
*/
typedef struct _ADDR_COMPUTE_FMASK_INFO_OUTPUT
{
    uint32_t        size;           ///< Size of this structure in bytes

    uint32_t        pitch;          ///< Pitch of fmask in pixels
    uint32_t        height;         ///< Height of fmask in pixels
    uint32_t        numSlices;      ///< Slices of fmask
    uint64_t        fmaskBytes;     ///< Size of fmask in bytes
    uint32_t        baseAlign;      ///< Base address alignment
    uint32_t        pitchAlign;     ///< Pitch alignment
    uint32_t        heightAlign;    ///< Height alignment
    uint32_t        bpp;            ///< Bits per pixel of FMASK is: number of bit planes
    uint32_t        numSamples;     ///< Number of samples, used for dump, export this since input
                                    ///  may be changed in 9xx and above
    /// r800 and later HWL parameters
    ADDR_TILEINFO*  pTileInfo;      ///< Tile parameters used. Fmask can have different
                                    ///  bank_height from color buffer
    int32_t         tileIndex;      ///< Tile index, MUST be -1 if you don't want to use it
                                    ///  while the global useTileIndex is set to 1
    int32_t         macroModeIndex; ///< Index in macro tile mode table if there is one (CI)
    uint64_t        sliceSize;      ///< Size of slice in bytes
} ADDR_COMPUTE_FMASK_INFO_OUTPUT;

/**
***************************************************************************************************
*   AddrComputeFmaskInfo
*
*   @brief
*       Compute Fmask pitch/height/depth/alignments and size in bytes
***************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API AddrComputeFmaskInfo(
    ADDR_HANDLE                             hLib,
    const ADDR_COMPUTE_FMASK_INFO_INPUT*    pIn,
    ADDR_COMPUTE_FMASK_INFO_OUTPUT*         pOut);



/**
***************************************************************************************************
*   ADDR_COMPUTE_FMASK_ADDRFROMCOORD_INPUT
*
*   @brief
*       Input structure for AddrComputeFmaskAddrFromCoord
***************************************************************************************************
*/
typedef struct _ADDR_COMPUTE_FMASK_ADDRFROMCOORD_INPUT
{
    uint32_t        size;               ///< Size of this structure in bytes

    uint32_t        x;                  ///< X coordinate
    uint32_t        y;                  ///< Y coordinate
    uint32_t        slice;              ///< Slice index
    uint32_t        plane;              ///< Plane number
    uint32_t        sample;             ///< Sample index (fragment index for EQAA)

    uint32_t        pitch;              ///< Surface pitch, in pixels
    uint32_t        height;             ///< Surface height, in pixels
    uint32_t        numSamples;         ///< Number of samples
    uint32_t        numFrags;           ///< Number of fragments, leave it zero or the same as
                                        ///  number of samples for normal AA; Set it to the
                                        ///  number of fragments for EQAA

    AddrTileMode    tileMode;           ///< Tile mode
    union
    {
        struct
        {
            uint32_t bankSwizzle;       ///< Bank swizzle
            uint32_t pipeSwizzle;       ///< Pipe swizzle
        };
        uint32_t    tileSwizzle;        ///< Combined swizzle, if useCombinedSwizzle is TRUE
    };

    /// r800 and later HWL parameters
    struct
    {
        uint32_t resolved:   1;         ///< TRUE if this is a resolved fmask, used by H/W clients
        uint32_t ignoreSE:   1;         ///< TRUE if shader engines are ignored.
        uint32_t reserved:  30;         ///< Reserved for future use.
    };
    ADDR_TILEINFO*  pTileInfo;          ///< 2D tiling parameters. Client must provide all data

} ADDR_COMPUTE_FMASK_ADDRFROMCOORD_INPUT;

/**
***************************************************************************************************
*   ADDR_COMPUTE_FMASK_ADDRFROMCOORD_OUTPUT
*
*   @brief
*       Output structure for AddrComputeFmaskAddrFromCoord
***************************************************************************************************
*/
typedef struct _ADDR_COMPUTE_FMASK_ADDRFROMCOORD_OUTPUT
{
    uint32_t size;           ///< Size of this structure in bytes

    uint64_t addr;           ///< Fmask address
    uint32_t bitPosition;    ///< Bit position within fmaskAddr, 0-7.
} ADDR_COMPUTE_FMASK_ADDRFROMCOORD_OUTPUT;

/**
***************************************************************************************************
*   AddrComputeFmaskAddrFromCoord
*
*   @brief
*       Compute Fmask address according to coordinates (x,y,slice,sample,plane)
***************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API AddrComputeFmaskAddrFromCoord(
    ADDR_HANDLE                                     hLib,
    const ADDR_COMPUTE_FMASK_ADDRFROMCOORD_INPUT*   pIn,
    ADDR_COMPUTE_FMASK_ADDRFROMCOORD_OUTPUT*        pOut);



/**
***************************************************************************************************
*   ADDR_COMPUTE_FMASK_COORDFROMADDR_INPUT
*
*   @brief
*       Input structure for AddrComputeFmaskCoordFromAddr
***************************************************************************************************
*/
typedef struct _ADDR_COMPUTE_FMASK_COORDFROMADDR_INPUT
{
    uint32_t        size;               ///< Size of this structure in bytes

    uint64_t        addr;               ///< Address
    uint32_t        bitPosition;        ///< Bit position within addr, 0-7.

    uint32_t        pitch;              ///< Pitch, in pixels
    uint32_t        height;             ///< Height in pixels
    uint32_t        numSamples;         ///< Number of samples
    uint32_t        numFrags;           ///< Number of fragments
    AddrTileMode    tileMode;           ///< Tile mode
    union
    {
        struct
        {
            uint32_t bankSwizzle;       ///< Bank swizzle
            uint32_t pipeSwizzle;       ///< Pipe swizzle
        };
        uint32_t    tileSwizzle;        ///< Combined swizzle, if useCombinedSwizzle is TRUE
    };

    /// r800 and later HWL parameters
    struct
    {
        uint32_t resolved:   1;         ///< TRUE if this is a resolved fmask, used by HW components
        uint32_t ignoreSE:   1;         ///< TRUE if shader engines are ignored.
        uint32_t reserved:  30;         ///< Reserved for future use.
    };
    ADDR_TILEINFO*  pTileInfo;          ///< 2D tile parameters. Client must provide all data

} ADDR_COMPUTE_FMASK_COORDFROMADDR_INPUT;

/**
***************************************************************************************************
*   ADDR_COMPUTE_FMASK_COORDFROMADDR_OUTPUT
*
*   @brief
*       Output structure for AddrComputeFmaskCoordFromAddr
***************************************************************************************************
*/
typedef struct _ADDR_COMPUTE_FMASK_COORDFROMADDR_OUTPUT
{
    uint32_t size;       ///< Size of this structure in bytes

    uint32_t x;          ///< X coordinate
    uint32_t y;          ///< Y coordinate
    uint32_t slice;      ///< Slice index
    uint32_t plane;      ///< Plane number
    uint32_t sample;     ///< Sample index (fragment index for EQAA)
} ADDR_COMPUTE_FMASK_COORDFROMADDR_OUTPUT;

/**
***************************************************************************************************
*   AddrComputeFmaskCoordFromAddr
*
*   @brief
*       Compute FMASK coordinate from an given address
***************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API AddrComputeFmaskCoordFromAddr(
    ADDR_HANDLE                                     hLib,
    const ADDR_COMPUTE_FMASK_COORDFROMADDR_INPUT*   pIn,
    ADDR_COMPUTE_FMASK_COORDFROMADDR_OUTPUT*        pOut);



///////////////////////////////////////////////////////////////////////////////////////////////////
//                          Element/utility functions
///////////////////////////////////////////////////////////////////////////////////////////////////

/**
***************************************************************************************************
*   AddrGetVersion
*
*   @brief
*       Get AddrLib version number
***************************************************************************************************
*/
uint32_t ADDR_API AddrGetVersion(ADDR_HANDLE hLib);

/**
***************************************************************************************************
*   AddrUseTileIndex
*
*   @brief
*       Return TRUE if tileIndex is enabled in this address library
***************************************************************************************************
*/
BOOL_32 ADDR_API AddrUseTileIndex(ADDR_HANDLE hLib);

/**
***************************************************************************************************
*   AddrUseCombinedSwizzle
*
*   @brief
*       Return TRUE if combined swizzle is enabled in this address library
***************************************************************************************************
*/
BOOL_32 ADDR_API AddrUseCombinedSwizzle(ADDR_HANDLE hLib);

/**
***************************************************************************************************
*   ADDR_EXTRACT_BANKPIPE_SWIZZLE_INPUT
*
*   @brief
*       Input structure of AddrExtractBankPipeSwizzle
***************************************************************************************************
*/
typedef struct _ADDR_EXTRACT_BANKPIPE_SWIZZLE_INPUT
{
    uint32_t        size;           ///< Size of this structure in bytes

    uint32_t        base256b;       ///< Base256b value

    /// r800 and later HWL parameters
    ADDR_TILEINFO*  pTileInfo;      ///< 2D tile parameters. Client must provide all data

    int32_t         tileIndex;      ///< Tile index, MUST be -1 if you don't want to use it
                                    ///  while the global useTileIndex is set to 1
    int32_t         macroModeIndex; ///< Index in macro tile mode table if there is one (CI)
                                    ///< README: When tileIndex is not -1, this must be valid
} ADDR_EXTRACT_BANKPIPE_SWIZZLE_INPUT;

/**
***************************************************************************************************
*   ADDR_EXTRACT_BANKPIPE_SWIZZLE_OUTPUT
*
*   @brief
*       Output structure of AddrExtractBankPipeSwizzle
***************************************************************************************************
*/
typedef struct _ADDR_EXTRACT_BANKPIPE_SWIZZLE_OUTPUT
{
    uint32_t size;           ///< Size of this structure in bytes

    uint32_t bankSwizzle;    ///< Bank swizzle
    uint32_t pipeSwizzle;    ///< Pipe swizzle
} ADDR_EXTRACT_BANKPIPE_SWIZZLE_OUTPUT;

/**
***************************************************************************************************
*   AddrExtractBankPipeSwizzle
*
*   @brief
*       Extract Bank and Pipe swizzle from base256b
*   @return
*       ADDR_OK if no error
***************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API AddrExtractBankPipeSwizzle(
    ADDR_HANDLE                                 hLib,
    const ADDR_EXTRACT_BANKPIPE_SWIZZLE_INPUT*  pIn,
    ADDR_EXTRACT_BANKPIPE_SWIZZLE_OUTPUT*       pOut);


/**
***************************************************************************************************
*   ADDR_COMBINE_BANKPIPE_SWIZZLE_INPUT
*
*   @brief
*       Input structure of AddrCombineBankPipeSwizzle
***************************************************************************************************
*/
typedef struct _ADDR_COMBINE_BANKPIPE_SWIZZLE_INPUT
{
    uint32_t        size;           ///< Size of this structure in bytes

    uint32_t        bankSwizzle;    ///< Bank swizzle
    uint32_t        pipeSwizzle;    ///< Pipe swizzle
    uint64_t        baseAddr;       ///< Base address (leave it zero for driver clients)

    /// r800 and later HWL parameters
    ADDR_TILEINFO*  pTileInfo;      ///< 2D tile parameters. Client must provide all data

    int32_t         tileIndex;      ///< Tile index, MUST be -1 if you don't want to use it
                                    ///  while the global useTileIndex is set to 1
    int32_t         macroModeIndex; ///< Index in macro tile mode table if there is one (CI)
                                    ///< README: When tileIndex is not -1, this must be valid
} ADDR_COMBINE_BANKPIPE_SWIZZLE_INPUT;

/**
***************************************************************************************************
*   ADDR_COMBINE_BANKPIPE_SWIZZLE_OUTPUT
*
*   @brief
*       Output structure of AddrCombineBankPipeSwizzle
***************************************************************************************************
*/
typedef struct _ADDR_COMBINE_BANKPIPE_SWIZZLE_OUTPUT
{
    uint32_t size;           ///< Size of this structure in bytes

    uint32_t tileSwizzle;    ///< Combined swizzle
} ADDR_COMBINE_BANKPIPE_SWIZZLE_OUTPUT;

/**
***************************************************************************************************
*   AddrCombineBankPipeSwizzle
*
*   @brief
*       Combine Bank and Pipe swizzle
*   @return
*       ADDR_OK if no error
*   @note
*       baseAddr here is full MCAddress instead of base256b
***************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API AddrCombineBankPipeSwizzle(
    ADDR_HANDLE                                 hLib,
    const ADDR_COMBINE_BANKPIPE_SWIZZLE_INPUT*  pIn,
    ADDR_COMBINE_BANKPIPE_SWIZZLE_OUTPUT*       pOut);



/**
***************************************************************************************************
*   ADDR_COMPUTE_SLICESWIZZLE_INPUT
*
*   @brief
*       Input structure of AddrComputeSliceSwizzle
***************************************************************************************************
*/
typedef struct _ADDR_COMPUTE_SLICESWIZZLE_INPUT
{
    uint32_t        size;               ///< Size of this structure in bytes

    AddrTileMode    tileMode;           ///< Tile Mode
    uint32_t        baseSwizzle;        ///< Base tile swizzle
    uint32_t        slice;              ///< Slice index
    uint64_t        baseAddr;           ///< Base address, driver should leave it 0 in most cases

    /// r800 and later HWL parameters
    ADDR_TILEINFO*  pTileInfo;          ///< 2D tile parameters. Actually banks needed here!

    int32_t         tileIndex;          ///< Tile index, MUST be -1 if you don't want to use it
                                        ///  while the global useTileIndex is set to 1
    int32_t         macroModeIndex;     ///< Index in macro tile mode table if there is one (CI)
                                        ///< README: When tileIndex is not -1, this must be valid
} ADDR_COMPUTE_SLICESWIZZLE_INPUT;



/**
***************************************************************************************************
*   ADDR_COMPUTE_SLICESWIZZLE_OUTPUT
*
*   @brief
*       Output structure of AddrComputeSliceSwizzle
***************************************************************************************************
*/
typedef struct _ADDR_COMPUTE_SLICESWIZZLE_OUTPUT
{
    uint32_t  size;           ///< Size of this structure in bytes

    uint32_t  tileSwizzle;    ///< Recalculated tileSwizzle value
} ADDR_COMPUTE_SLICESWIZZLE_OUTPUT;

/**
***************************************************************************************************
*   AddrComputeSliceSwizzle
*
*   @brief
*       Extract Bank and Pipe swizzle from base256b
*   @return
*       ADDR_OK if no error
***************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API AddrComputeSliceSwizzle(
    ADDR_HANDLE                             hLib,
    const ADDR_COMPUTE_SLICESWIZZLE_INPUT*  pIn,
    ADDR_COMPUTE_SLICESWIZZLE_OUTPUT*       pOut);


/**
***************************************************************************************************
*   AddrSwizzleGenOption
*
*   @brief
*       Which swizzle generating options: legacy or linear
***************************************************************************************************
*/
typedef enum _AddrSwizzleGenOption
{
    ADDR_SWIZZLE_GEN_DEFAULT    = 0,    ///< As is in client driver implemention for swizzle
    ADDR_SWIZZLE_GEN_LINEAR     = 1,    ///< Using a linear increment of swizzle
} AddrSwizzleGenOption;

/**
***************************************************************************************************
*   AddrSwizzleOption
*
*   @brief
*       Controls how swizzle is generated
***************************************************************************************************
*/
typedef union _ADDR_SWIZZLE_OPTION
{
    struct
    {
        uint32_t genOption       : 1;    ///< The way swizzle is generated, see AddrSwizzleGenOption
        uint32_t reduceBankBit   : 1;    ///< TRUE if we need reduce swizzle bits
        uint32_t reserved        :30;    ///< Reserved bits
    };

    uint32_t value;

} ADDR_SWIZZLE_OPTION;

/**
***************************************************************************************************
*   ADDR_COMPUTE_BASE_SWIZZLE_INPUT
*
*   @brief
*       Input structure of AddrComputeBaseSwizzle
***************************************************************************************************
*/
typedef struct _ADDR_COMPUTE_BASE_SWIZZLE_INPUT
{
    uint32_t            size;           ///< Size of this structure in bytes

    ADDR_SWIZZLE_OPTION option;         ///< Swizzle option
    uint32_t            surfIndex;      ///< Index of this surface type
    AddrTileMode        tileMode;       ///< Tile Mode

    /// r800 and later HWL parameters
    ADDR_TILEINFO*      pTileInfo;      ///< 2D tile parameters. Actually banks needed here!

    int32_t             tileIndex;      ///< Tile index, MUST be -1 if you don't want to use it
                                        ///  while the global useTileIndex is set to 1
    int32_t             macroModeIndex; ///< Index in macro tile mode table if there is one (CI)
                                        ///< README: When tileIndex is not -1, this must be valid
} ADDR_COMPUTE_BASE_SWIZZLE_INPUT;

/**
***************************************************************************************************
*   ADDR_COMPUTE_BASE_SWIZZLE_OUTPUT
*
*   @brief
*       Output structure of AddrComputeBaseSwizzle
***************************************************************************************************
*/
typedef struct _ADDR_COMPUTE_BASE_SWIZZLE_OUTPUT
{
    uint32_t size;           ///< Size of this structure in bytes

    uint32_t tileSwizzle;    ///< Combined swizzle
} ADDR_COMPUTE_BASE_SWIZZLE_OUTPUT;

/**
***************************************************************************************************
*   AddrComputeBaseSwizzle
*
*   @brief
*       Return a Combined Bank and Pipe swizzle base on surface based on surface type/index
*   @return
*       ADDR_OK if no error
***************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API AddrComputeBaseSwizzle(
    ADDR_HANDLE                             hLib,
    const ADDR_COMPUTE_BASE_SWIZZLE_INPUT*  pIn,
    ADDR_COMPUTE_BASE_SWIZZLE_OUTPUT*       pOut);



/**
***************************************************************************************************
*   ELEM_GETEXPORTNORM_INPUT
*
*   @brief
*       Input structure for ElemGetExportNorm
*
***************************************************************************************************
*/
typedef struct _ELEM_GETEXPORTNORM_INPUT
{
    uint32_t            size;       ///< Size of this structure in bytes

    AddrColorFormat     format;     ///< Color buffer format; Client should use ColorFormat
    AddrSurfaceNumber   num;        ///< Surface number type; Client should use NumberType
    AddrSurfaceSwap     swap;       ///< Surface swap byte swap; Client should use SurfaceSwap
    uint32_t            numSamples; ///< Number of samples
} ELEM_GETEXPORTNORM_INPUT;

/**
***************************************************************************************************
*  ElemGetExportNorm
*
*   @brief
*       Helper function to check one format can be EXPORT_NUM, which is a register
*       CB_COLOR_INFO.SURFACE_FORMAT. FP16 can be reported as EXPORT_NORM for rv770 in r600
*       family
*   @note
*       The implementation is only for r600.
*       00 - EXPORT_FULL: PS exports are 4 pixels with 4 components with 32-bits-per-component. (two
*       clocks per export)
*       01 - EXPORT_NORM: PS exports are 4 pixels with 4 components with 16-bits-per-component. (one
*       clock per export)
*
***************************************************************************************************
*/
BOOL_32 ADDR_API ElemGetExportNorm(
    ADDR_HANDLE                     hLib,
    const ELEM_GETEXPORTNORM_INPUT* pIn);



/**
***************************************************************************************************
*   ELEM_FLT32TODEPTHPIXEL_INPUT
*
*   @brief
*       Input structure for addrFlt32ToDepthPixel
*
***************************************************************************************************
*/
typedef struct _ELEM_FLT32TODEPTHPIXEL_INPUT
{
    uint32_t        size;           ///< Size of this structure in bytes

    AddrDepthFormat format;         ///< Depth buffer format
    ADDR_FLT_32     comps[2];       ///< Component values (Z/stencil)
} ELEM_FLT32TODEPTHPIXEL_INPUT;

/**
***************************************************************************************************
*   ELEM_FLT32TODEPTHPIXEL_INPUT
*
*   @brief
*       Output structure for ElemFlt32ToDepthPixel
*
***************************************************************************************************
*/
typedef struct _ELEM_FLT32TODEPTHPIXEL_OUTPUT
{
    uint32_t size;          ///< Size of this structure in bytes

    uint8_t* pPixel;        ///< Real depth value. Same data type as depth buffer.
                            ///  Client must provide enough storage for this type.
    uint32_t depthBase;     ///< Tile base in bits for depth bits
    uint32_t stencilBase;   ///< Tile base in bits for stencil bits
    uint32_t depthBits;     ///< Bits for depth
    uint32_t stencilBits;   ///< Bits for stencil
} ELEM_FLT32TODEPTHPIXEL_OUTPUT;

/**
***************************************************************************************************
*   ElemFlt32ToDepthPixel
*
*   @brief
*       Convert a FLT_32 value to a depth/stencil pixel value
*
*   @return
*       Return code
*
***************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API ElemFlt32ToDepthPixel(
    ADDR_HANDLE                         hLib,
    const ELEM_FLT32TODEPTHPIXEL_INPUT* pIn,
    ELEM_FLT32TODEPTHPIXEL_OUTPUT*      pOut);



/**
***************************************************************************************************
*   ELEM_FLT32TOCOLORPIXEL_INPUT
*
*   @brief
*       Input structure for addrFlt32ToColorPixel
*
***************************************************************************************************
*/
typedef struct _ELEM_FLT32TOCOLORPIXEL_INPUT
{
    uint32_t           size;           ///< Size of this structure in bytes

    AddrColorFormat    format;         ///< Color buffer format
    AddrSurfaceNumber  surfNum;        ///< Surface number
    AddrSurfaceSwap    surfSwap;       ///< Surface swap
    ADDR_FLT_32        comps[4];       ///< Component values (r/g/b/a)
} ELEM_FLT32TOCOLORPIXEL_INPUT;

/**
***************************************************************************************************
*   ELEM_FLT32TOCOLORPIXEL_INPUT
*
*   @brief
*       Output structure for ElemFlt32ToColorPixel
*
***************************************************************************************************
*/
typedef struct _ELEM_FLT32TOCOLORPIXEL_OUTPUT
{
    uint32_t size;       ///< Size of this structure in bytes

    uint8_t* pPixel;     ///< Real color value. Same data type as color buffer.
                        ///  Client must provide enough storage for this type.
} ELEM_FLT32TOCOLORPIXEL_OUTPUT;

/**
***************************************************************************************************
*   ElemFlt32ToColorPixel
*
*   @brief
*       Convert a FLT_32 value to a red/green/blue/alpha pixel value
*
*   @return
*       Return code
*
***************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API ElemFlt32ToColorPixel(
    ADDR_HANDLE                         hLib,
    const ELEM_FLT32TOCOLORPIXEL_INPUT* pIn,
    ELEM_FLT32TOCOLORPIXEL_OUTPUT*      pOut);


/**
***************************************************************************************************
*   ADDR_CONVERT_TILEINFOTOHW_INPUT
*
*   @brief
*       Input structure for AddrConvertTileInfoToHW
*   @note
*       When reverse is TRUE, indices are igonred
***************************************************************************************************
*/
typedef struct _ADDR_CONVERT_TILEINFOTOHW_INPUT
{
    uint32_t        size;               ///< Size of this structure in bytes
    BOOL_32         reverse;            ///< Convert control flag.
                                        ///  FALSE: convert from real value to HW value;
                                        ///  TRUE: convert from HW value to real value.

    /// r800 and later HWL parameters
    ADDR_TILEINFO*  pTileInfo;          ///< Tile parameters with real value

    int32_t         tileIndex;          ///< Tile index, MUST be -1 if you don't want to use it
                                        ///  while the global useTileIndex is set to 1
    int32_t         macroModeIndex;     ///< Index in macro tile mode table if there is one (CI)
                                        ///< README: When tileIndex is not -1, this must be valid
} ADDR_CONVERT_TILEINFOTOHW_INPUT;

/**
***************************************************************************************************
*   ADDR_CONVERT_TILEINFOTOHW_OUTPUT
*
*   @brief
*       Output structure for AddrConvertTileInfoToHW
***************************************************************************************************
*/
typedef struct _ADDR_CONVERT_TILEINFOTOHW_OUTPUT
{
    uint32_t            size;               ///< Size of this structure in bytes

    /// r800 and later HWL parameters
    ADDR_TILEINFO*      pTileInfo;          ///< Tile parameters with hardware register value

} ADDR_CONVERT_TILEINFOTOHW_OUTPUT;

/**
***************************************************************************************************
*   AddrConvertTileInfoToHW
*
*   @brief
*       Convert tile info from real value to hardware register value
***************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API AddrConvertTileInfoToHW(
    ADDR_HANDLE                             hLib,
    const ADDR_CONVERT_TILEINFOTOHW_INPUT*  pIn,
    ADDR_CONVERT_TILEINFOTOHW_OUTPUT*       pOut);



/**
***************************************************************************************************
*   ADDR_CONVERT_TILEINDEX_INPUT
*
*   @brief
*       Input structure for AddrConvertTileIndex
***************************************************************************************************
*/
typedef struct _ADDR_CONVERT_TILEINDEX_INPUT
{
    uint32_t        size;               ///< Size of this structure in bytes

    int32_t         tileIndex;          ///< Tile index
    int32_t         macroModeIndex;     ///< Index in macro tile mode table if there is one (CI)
    BOOL_32         tileInfoHw;         ///< Set to TRUE if client wants HW enum, otherwise actual
} ADDR_CONVERT_TILEINDEX_INPUT;

/**
***************************************************************************************************
*   ADDR_CONVERT_TILEINDEX_OUTPUT
*
*   @brief
*       Output structure for AddrConvertTileIndex
***************************************************************************************************
*/
typedef struct _ADDR_CONVERT_TILEINDEX_OUTPUT
{
    uint32_t            size;           ///< Size of this structure in bytes

    AddrTileMode        tileMode;       ///< Tile mode
    AddrTileType        tileType;       ///< Tile type
    ADDR_TILEINFO*      pTileInfo;      ///< Tile info

} ADDR_CONVERT_TILEINDEX_OUTPUT;

/**
***************************************************************************************************
*   AddrConvertTileIndex
*
*   @brief
*       Convert tile index to tile mode/type/info
***************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API AddrConvertTileIndex(
    ADDR_HANDLE                         hLib,
    const ADDR_CONVERT_TILEINDEX_INPUT* pIn,
    ADDR_CONVERT_TILEINDEX_OUTPUT*      pOut);



/**
***************************************************************************************************
*   ADDR_CONVERT_TILEINDEX1_INPUT
*
*   @brief
*       Input structure for AddrConvertTileIndex1 (without macro mode index)
***************************************************************************************************
*/
typedef struct _ADDR_CONVERT_TILEINDEX1_INPUT
{
    uint32_t        size;               ///< Size of this structure in bytes

    int32_t         tileIndex;          ///< Tile index
    uint32_t        bpp;                ///< Bits per pixel
    uint32_t        numSamples;         ///< Number of samples
    BOOL_32         tileInfoHw;         ///< Set to TRUE if client wants HW enum, otherwise actual
} ADDR_CONVERT_TILEINDEX1_INPUT;

/**
***************************************************************************************************
*   AddrConvertTileIndex1
*
*   @brief
*       Convert tile index to tile mode/type/info
***************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API AddrConvertTileIndex1(
    ADDR_HANDLE                             hLib,
    const ADDR_CONVERT_TILEINDEX1_INPUT*    pIn,
    ADDR_CONVERT_TILEINDEX_OUTPUT*          pOut);



/**
***************************************************************************************************
*   ADDR_GET_TILEINDEX_INPUT
*
*   @brief
*       Input structure for AddrGetTileIndex
***************************************************************************************************
*/
typedef struct _ADDR_GET_TILEINDEX_INPUT
{
    uint32_t        size;           ///< Size of this structure in bytes

    AddrTileMode    tileMode;       ///< Tile mode
    AddrTileType    tileType;       ///< Tile-type: disp/non-disp/...
    ADDR_TILEINFO*  pTileInfo;      ///< Pointer to tile-info structure, can be NULL for linear/1D
} ADDR_GET_TILEINDEX_INPUT;

/**
***************************************************************************************************
*   ADDR_GET_TILEINDEX_OUTPUT
*
*   @brief
*       Output structure for AddrGetTileIndex
***************************************************************************************************
*/
typedef struct _ADDR_GET_TILEINDEX_OUTPUT
{
    uint32_t        size;           ///< Size of this structure in bytes

    int32_t         index;          ///< index in table
} ADDR_GET_TILEINDEX_OUTPUT;

/**
***************************************************************************************************
*   AddrGetTileIndex
*
*   @brief
*       Get the tiling mode index in table
***************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API AddrGetTileIndex(
    ADDR_HANDLE                     hLib,
    const ADDR_GET_TILEINDEX_INPUT* pIn,
    ADDR_GET_TILEINDEX_OUTPUT*      pOut);




/**
***************************************************************************************************
*   ADDR_PRT_INFO_INPUT
*
*   @brief
*       Input structure for AddrComputePrtInfo
***************************************************************************************************
*/
typedef struct _ADDR_PRT_INFO_INPUT
{
    AddrFormat          format;        ///< Surface format
    uint32_t            baseMipWidth;  ///< Base mipmap width
    uint32_t            baseMipHeight; ///< Base mipmap height
    uint32_t            baseMipDepth;  ///< Base mipmap depth
    uint32_t            numFrags;      ///< Number of fragments,
} ADDR_PRT_INFO_INPUT;

/**
***************************************************************************************************
*   ADDR_PRT_INFO_OUTPUT
*
*   @brief
*       Input structure for AddrComputePrtInfo
***************************************************************************************************
*/
typedef struct _ADDR_PRT_INFO_OUTPUT
{
    uint32_t             prtTileWidth;
    uint32_t             prtTileHeight;
} ADDR_PRT_INFO_OUTPUT;

/**
***************************************************************************************************
*   AddrComputePrtInfo
*
*   @brief
*       Compute prt surface related information
***************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API AddrComputePrtInfo(
    ADDR_HANDLE                 hLib,
    const ADDR_PRT_INFO_INPUT*  pIn,
    ADDR_PRT_INFO_OUTPUT*       pOut);

///////////////////////////////////////////////////////////////////////////////////////////////////
//                                     DCC key functions
///////////////////////////////////////////////////////////////////////////////////////////////////

/**
***************************************************************************************************
*   _ADDR_COMPUTE_DCCINFO_INPUT
*
*   @brief
*       Input structure of AddrComputeDccInfo
***************************************************************************************************
*/
typedef struct _ADDR_COMPUTE_DCCINFO_INPUT
{
    uint32_t            size;            ///< Size of this structure in bytes
    uint32_t            bpp;             ///< BitPP of color surface
    uint32_t            numSamples;      ///< Sample number of color surface
    uint64_t            colorSurfSize;   ///< Size of color surface to which dcc key is bound
    AddrTileMode        tileMode;        ///< Tile mode of color surface
    ADDR_TILEINFO       tileInfo;        ///< Tile info of color surface
    uint32_t            tileSwizzle;     ///< Tile swizzle
    int32_t             tileIndex;       ///< Tile index of color surface,
                                         ///< MUST be -1 if you don't want to use it
                                         ///< while the global useTileIndex is set to 1
    int32_t             macroModeIndex;  ///< Index in macro tile mode table if there is one (CI)
                                         ///< README: When tileIndex is not -1, this must be valid
} ADDR_COMPUTE_DCCINFO_INPUT;

/**
***************************************************************************************************
*   ADDR_COMPUTE_DCCINFO_OUTPUT
*
*   @brief
*       Output structure of AddrComputeDccInfo
***************************************************************************************************
*/
typedef struct _ADDR_COMPUTE_DCCINFO_OUTPUT
{
    uint32_t size;                ///< Size of this structure in bytes
    uint64_t dccRamBaseAlign;     ///< Base alignment of dcc key
    uint64_t dccRamSize;          ///< Size of dcc key
    uint64_t dccFastClearSize;    ///< Size of dcc key portion that can be fast cleared
    BOOL_32 subLvlCompressible;   ///< whether sub resource is compressiable
} ADDR_COMPUTE_DCCINFO_OUTPUT;

/**
***************************************************************************************************
*   AddrComputeDccInfo
*
*   @brief
*       Compute DCC key size, base alignment
*       info
***************************************************************************************************
*/
ADDR_E_RETURNCODE ADDR_API AddrComputeDccInfo(
    ADDR_HANDLE                             hLib,
    const ADDR_COMPUTE_DCCINFO_INPUT*       pIn,
    ADDR_COMPUTE_DCCINFO_OUTPUT*            pOut);

#if defined(__cplusplus)
}
#endif

#endif // __ADDR_INTERFACE_H__
