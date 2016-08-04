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
* @file  siaddrlib.cpp
* @brief Contains the implementation for the SIAddrLib class.
***************************************************************************************************
*/

#include <stdint.h>

#include "siaddrlib.h"

#include "si_gb_reg.h"

#include "si_ci_vi_merged_enum.h"

#if BRAHMA_BUILD
#include "amdgpu_id.h"
#else
#include "si_id.h"
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

/**
***************************************************************************************************
*   AddrSIHwlInit
*
*   @brief
*       Creates an SIAddrLib object.
*
*   @return
*       Returns an SIAddrLib object pointer.
***************************************************************************************************
*/
AddrLib* AddrSIHwlInit(const AddrClient* pClient)
{
    return SIAddrLib::CreateObj(pClient);
}

/**
***************************************************************************************************
*   SIAddrLib::SIAddrLib
*
*   @brief
*       Constructor
*
***************************************************************************************************
*/
SIAddrLib::SIAddrLib(const AddrClient* pClient) :
    EgBasedAddrLib(pClient),
    m_noOfEntries(0)
{
    m_class = SI_ADDRLIB;
    memset(&m_settings, 0, sizeof(m_settings));
}

/**
***************************************************************************************************
*   SIAddrLib::~SIAddrLib
*
*   @brief
*       Destructor
***************************************************************************************************
*/
SIAddrLib::~SIAddrLib()
{
}

/**
***************************************************************************************************
*   SIAddrLib::HwlGetPipes
*
*   @brief
*       Get number pipes
*   @return
*       num pipes
***************************************************************************************************
*/
uint32_t SIAddrLib::HwlGetPipes(
    const ADDR_TILEINFO* pTileInfo    ///< [in] Tile info
    ) const
{
    uint32_t numPipes;

    if (pTileInfo)
    {
        numPipes = GetPipePerSurf(pTileInfo->pipeConfig);
    }
    else
    {
        ADDR_ASSERT_ALWAYS();
        numPipes = m_pipes; // Suppose we should still have a global pipes
    }

    return numPipes;
}

/**
***************************************************************************************************
*   SIAddrLib::GetPipePerSurf
*   @brief
*       get pipe num base on inputing tileinfo->pipeconfig
*   @return
*       pipe number
***************************************************************************************************
*/
uint32_t SIAddrLib::GetPipePerSurf(
    AddrPipeCfg pipeConfig   ///< [in] pipe config
    ) const
{
    uint32_t numPipes = 0;

    switch (pipeConfig)
    {
        case ADDR_PIPECFG_P2:
            numPipes = 2;
            break;
        case ADDR_PIPECFG_P4_8x16:
        case ADDR_PIPECFG_P4_16x16:
        case ADDR_PIPECFG_P4_16x32:
        case ADDR_PIPECFG_P4_32x32:
            numPipes = 4;
            break;
        case ADDR_PIPECFG_P8_16x16_8x16:
        case ADDR_PIPECFG_P8_16x32_8x16:
        case ADDR_PIPECFG_P8_32x32_8x16:
        case ADDR_PIPECFG_P8_16x32_16x16:
        case ADDR_PIPECFG_P8_32x32_16x16:
        case ADDR_PIPECFG_P8_32x32_16x32:
        case ADDR_PIPECFG_P8_32x64_32x32:
            numPipes = 8;
            break;
        case ADDR_PIPECFG_P16_32x32_8x16:
        case ADDR_PIPECFG_P16_32x32_16x16:
            numPipes = 16;
            break;
        default:
            ADDR_ASSERT(!"Invalid pipe config");
            numPipes = m_pipes;
    }
    return numPipes;
}

/**
***************************************************************************************************
*   SIAddrLib::ComputePipeFromCoord
*
*   @brief
*       Compute pipe number from coordinates
*   @return
*       Pipe number
***************************************************************************************************
*/
uint32_t SIAddrLib::ComputePipeFromCoord(
    uint32_t        x,              ///< [in] x coordinate
    uint32_t        y,              ///< [in] y coordinate
    uint32_t        slice,          ///< [in] slice index
    AddrTileMode    tileMode,       ///< [in] tile mode
    uint32_t        pipeSwizzle,    ///< [in] pipe swizzle
    BOOL_32         ignoreSE,       ///< [in] TRUE if shader engines are ignored
    ADDR_TILEINFO*  pTileInfo       ///< [in] Tile info
    ) const
{
    uint32_t pipe;
    uint32_t pipeBit0 = 0;
    uint32_t pipeBit1 = 0;
    uint32_t pipeBit2 = 0;
    uint32_t pipeBit3 = 0;
    uint32_t sliceRotation;
    uint32_t numPipes = 0;

    uint32_t tx = x / MicroTileWidth;
    uint32_t ty = y / MicroTileHeight;
    uint32_t x3 = _BIT(tx,0);
    uint32_t x4 = _BIT(tx,1);
    uint32_t x5 = _BIT(tx,2);
    uint32_t x6 = _BIT(tx,3);
    uint32_t y3 = _BIT(ty,0);
    uint32_t y4 = _BIT(ty,1);
    uint32_t y5 = _BIT(ty,2);
    uint32_t y6 = _BIT(ty,3);

    switch (pTileInfo->pipeConfig)
    {
        case ADDR_PIPECFG_P2:
            pipeBit0 = x3 ^ y3;
            numPipes = 2;
            break;
        case ADDR_PIPECFG_P4_8x16:
            pipeBit0 = x4 ^ y3;
            pipeBit1 = x3 ^ y4;
            numPipes = 4;
            break;
        case ADDR_PIPECFG_P4_16x16:
            pipeBit0 = x3 ^ y3 ^ x4;
            pipeBit1 = x4 ^ y4;
            numPipes = 4;
            break;
        case ADDR_PIPECFG_P4_16x32:
            pipeBit0 = x3 ^ y3 ^ x4;
            pipeBit1 = x4 ^ y5;
            numPipes = 4;
            break;
        case ADDR_PIPECFG_P4_32x32:
            pipeBit0 = x3 ^ y3 ^ x5;
            pipeBit1 = x5 ^ y5;
            numPipes = 4;
            break;
        case ADDR_PIPECFG_P8_16x16_8x16:
            pipeBit0 = x4 ^ y3 ^ x5;
            pipeBit1 = x3 ^ y5;
            numPipes = 8;
            break;
        case ADDR_PIPECFG_P8_16x32_8x16:
            pipeBit0 = x4 ^ y3 ^ x5;
            pipeBit1 = x3 ^ y4;
            pipeBit2 = x4 ^ y5;
            numPipes = 8;
            break;
        case ADDR_PIPECFG_P8_16x32_16x16:
            pipeBit0 = x3 ^ y3 ^ x4;
            pipeBit1 = x5 ^ y4;
            pipeBit2 = x4 ^ y5;
            numPipes = 8;
            break;
        case ADDR_PIPECFG_P8_32x32_8x16:
            pipeBit0 = x4 ^ y3 ^ x5;
            pipeBit1 = x3 ^ y4;
            pipeBit2 = x5 ^ y5;
            numPipes = 8;
            break;
        case ADDR_PIPECFG_P8_32x32_16x16:
            pipeBit0 = x3 ^ y3 ^ x4;
            pipeBit1 = x4 ^ y4;
            pipeBit2 = x5 ^ y5;
            numPipes = 8;
            break;
        case ADDR_PIPECFG_P8_32x32_16x32:
            pipeBit0 = x3 ^ y3 ^ x4;
            pipeBit1 = x4 ^ y6;
            pipeBit2 = x5 ^ y5;
            numPipes = 8;
            break;
        case ADDR_PIPECFG_P8_32x64_32x32:
            pipeBit0 = x3 ^ y3 ^ x5;
            pipeBit1 = x6 ^ y5;
            pipeBit2 = x5 ^ y6;
            numPipes = 8;
            break;
        case ADDR_PIPECFG_P16_32x32_8x16:
            pipeBit0 = x4 ^ y3;
            pipeBit1 = x3 ^ y4;
            pipeBit2 = x5 ^ y6;
            pipeBit3 = x6 ^ y5;
            numPipes = 16;
            break;
        case ADDR_PIPECFG_P16_32x32_16x16:
            pipeBit0 = x3 ^ y3 ^ x4;
            pipeBit1 = x4 ^ y4;
            pipeBit2 = x5 ^ y6;
            pipeBit3 = x6 ^ y5;
            numPipes = 16;
            break;
        default:
            ADDR_UNHANDLED_CASE();
            break;
    }
    pipe = pipeBit0 | (pipeBit1 << 1) | (pipeBit2 << 2) | (pipeBit3 << 3);

    uint32_t microTileThickness = ComputeSurfaceThickness(tileMode);

    //
    // Apply pipe rotation for the slice.
    //
    switch (tileMode)
    {
        case ADDR_TM_3D_TILED_THIN1:    //fall through thin
        case ADDR_TM_3D_TILED_THICK:    //fall through thick
        case ADDR_TM_3D_TILED_XTHICK:
            sliceRotation =
                Max(1, static_cast<int32_t>(numPipes / 2) - 1) * (slice / microTileThickness);
            break;
        default:
            sliceRotation = 0;
            break;
    }
    pipeSwizzle += sliceRotation;
    pipeSwizzle &= (numPipes - 1);

    pipe = pipe ^ pipeSwizzle;

    return pipe;
}

/**
***************************************************************************************************
*   SIAddrLib::ComputeTileCoordFromPipeAndElemIdx
*
*   @brief
*       Compute (x,y) of a tile within a macro tile from address
*   @return
*       Pipe number
***************************************************************************************************
*/
VOID SIAddrLib::ComputeTileCoordFromPipeAndElemIdx(
    uint32_t        elemIdx,          ///< [in] per pipe element index within a macro tile
    uint32_t        pipe,             ///< [in] pipe index
    AddrPipeCfg     pipeCfg,          ///< [in] pipe config
    uint32_t        pitchInMacroTile, ///< [in] surface pitch in macro tile
    uint32_t        x,                ///< [in] x coordinate of the (0,0) tile in a macro tile
    uint32_t        y,                ///< [in] y coordinate of the (0,0) tile in a macro tile
    uint32_t*       pX,               ///< [out] x coordinate
    uint32_t*       pY                ///< [out] y coordinate
    ) const
{
    uint32_t pipebit0 = _BIT(pipe,0);
    uint32_t pipebit1 = _BIT(pipe,1);
    uint32_t pipebit2 = _BIT(pipe,2);
    uint32_t pipebit3 = _BIT(pipe,3);
    uint32_t elemIdx0 = _BIT(elemIdx,0);
    uint32_t elemIdx1 = _BIT(elemIdx,1);
    uint32_t elemIdx2 = _BIT(elemIdx,2);
    uint32_t x3 = 0;
    uint32_t x4 = 0;
    uint32_t x5 = 0;
    uint32_t x6 = 0;
    uint32_t y3 = 0;
    uint32_t y4 = 0;
    uint32_t y5 = 0;
    uint32_t y6 = 0;

    switch(pipeCfg)
    {
        case ADDR_PIPECFG_P2:
            x4 = elemIdx2;
            y4 = elemIdx1 ^ x4;
            y3 = elemIdx0 ^ x4;
            x3 = pipebit0 ^ y3;
            *pY = Bits2Number(2, y4, y3);
            *pX = Bits2Number(2, x4, x3);
            break;
        case ADDR_PIPECFG_P4_8x16:
            x4 = elemIdx1;
            y4 = elemIdx0 ^ x4;
            x3 = pipebit1 ^ y4;
            y3 = pipebit0 ^ x4;
            *pY = Bits2Number(2, y4, y3);
            *pX = Bits2Number(2, x4, x3);
            break;
        case ADDR_PIPECFG_P4_16x16:
            x4 = elemIdx1;
            y3 = elemIdx0 ^ x4;
            y4 = pipebit1 ^ x4;
            x3 = pipebit0 ^ y3 ^ x4;
            *pY = Bits2Number(2, y4, y3);
            *pX = Bits2Number(2, x4, x3);
            break;
        case ADDR_PIPECFG_P4_16x32:
            x3 = elemIdx0 ^ pipebit0;
            y5 = _BIT(y,5);
            x4 = pipebit1 ^ y5;
            y3 = pipebit0 ^ x3 ^ x4;
            y4 = elemIdx1 ^ x4;
            *pY = Bits2Number(2, y4, y3);
            *pX = Bits2Number(2, x4, x3);
            break;
        case ADDR_PIPECFG_P4_32x32:
            x4 = elemIdx2;
            y3 = elemIdx0 ^ x4;
            y4 = elemIdx1 ^ x4;
            if((pitchInMacroTile % 2) == 0)
            {   //even
                y5 = _BIT(y,5);
                x5 = pipebit1 ^ y5;
                x3 = pipebit0 ^ y3 ^ x5;
                *pY = Bits2Number(2, y4, y3);
                *pX = Bits2Number(3, x5, x4, x3);
            }
            else
            {   //odd
                x5 = _BIT(x,5);
                x3 = pipebit0 ^ y3 ^ x5;
                *pY = Bits2Number(2, y4, y3);
                *pX = Bits2Number(2, x4, x3);
            }
            break;
        case ADDR_PIPECFG_P8_16x16_8x16:
            x4 = elemIdx0;
            y5 = _BIT(y,5);
            x5 = _BIT(x,5);
            x3 = pipebit1 ^ y5;
            y4 = pipebit2 ^ x4;
            y3 = pipebit0 ^ x5 ^ x4;
            *pY = Bits2Number(2, y4, y3);
            *pX = Bits2Number(2, x4, x3);
            break;
        case ADDR_PIPECFG_P8_16x32_8x16:
            x3 = elemIdx0;
            y4 = pipebit1 ^ x3;
            y5 = _BIT(y,5);
            x5 = _BIT(x,5);
            x4 = pipebit2 ^ y5;
            y3 = pipebit0 ^ x4 ^ x5;
            *pY = Bits2Number(2, y4, y3);
            *pX = Bits2Number(2, x4, x3);
            break;
        case ADDR_PIPECFG_P8_32x32_8x16:
            x4 = elemIdx1;
            y4 = elemIdx0 ^ x4;
            x3 = pipebit1 ^ y4;
            if((pitchInMacroTile % 2) == 0)
            {  //even
                y5 = _BIT(y,5);
                x5 = _BIT(x,5);
                x5 = pipebit2 ^ y5;
                y3 = pipebit0 ^ x4 ^ x5;
                *pY = Bits2Number(2, y4, y3);
                *pX = Bits2Number(3, x5, x4, x3);
            }
            else
            {  //odd
                x5 = _BIT(x,5);
                y3 = pipebit0 ^ x4 ^ x5;
                *pY = Bits2Number(2, y4, y3);
                *pX = Bits2Number(2, x4, x3);
            }
            break;
        case ADDR_PIPECFG_P8_16x32_16x16:
            x3 = elemIdx0;
            x5 = _BIT(x,5);
            y5 = _BIT(y,5);
            x4 = pipebit2 ^ y5;
            y4 = pipebit1 ^ x5;
            y3 = pipebit0 ^ x3 ^ x4;
            *pY = Bits2Number(2, y4, y3);
            *pX = Bits2Number(2, x4, x3);
            break;
        case ADDR_PIPECFG_P8_32x32_16x16:
            x4 = elemIdx1;
            y3 = elemIdx0 ^ x4;
            x3 = y3^x4^pipebit0;
            y4 = pipebit1 ^ x4;
            if((pitchInMacroTile % 2) == 0)
            {   //even
                y5 = _BIT(y,5);
                x5 = pipebit2 ^ y5;
                *pY = Bits2Number(2, y4, y3);
                *pX = Bits2Number(3, x5, x4, x3);
            }
            else
            {   //odd
                *pY = Bits2Number(2, y4, y3);
                *pX = Bits2Number(2, x4, x3);
            }
            break;
        case ADDR_PIPECFG_P8_32x32_16x32:
            if((pitchInMacroTile % 2) == 0)
            {   //even
                y5 = _BIT(y,5);
                y6 = _BIT(y,6);
                x4 = pipebit1 ^ y6;
                y3 = elemIdx0 ^ x4;
                y4 = elemIdx1 ^ x4;
                x3 = pipebit0 ^ y3 ^ x4;
                x5 = pipebit2 ^ y5;
                *pY = Bits2Number(2, y4, y3);
                *pX = Bits2Number(3, x5, x4, x3);
            }
            else
            {   //odd
                y6 = _BIT(y,6);
                x4 = pipebit1 ^ y6;
                y3 = elemIdx0 ^ x4;
                y4 = elemIdx1 ^ x4;
                x3 = pipebit0 ^ y3 ^ x4;
                *pY = Bits2Number(2, y4, y3);
                *pX = Bits2Number(2, x4, x3);
            }
            break;
        case ADDR_PIPECFG_P8_32x64_32x32:
            x4 = elemIdx2;
            y3 = elemIdx0 ^ x4;
            y4 = elemIdx1 ^ x4;
            if((pitchInMacroTile % 4) == 0)
            {   //multiple of 4
                y5 = _BIT(y,5);
                y6 = _BIT(y,6);
                x5 = pipebit2 ^ y6;
                x6 = pipebit1 ^ y5;
                x3 = pipebit0 ^ y3 ^ x5;
                *pY = Bits2Number(2, y4, y3);
                *pX = Bits2Number(4, x6, x5, x4, x3);
            }
            else
            {
                y6 = _BIT(y,6);
                x5 = pipebit2 ^ y6;
                x3 = pipebit0 ^ y3 ^ x5;
                *pY = Bits2Number(2, y4, y3);
                *pX = Bits2Number(3, x5, x4, x3);
            }
            break;
        case ADDR_PIPECFG_P16_32x32_8x16:
            x4 = elemIdx1;
            y4 = elemIdx0 ^ x4;
            y3 = pipebit0 ^ x4;
            x3 = pipebit1 ^ y4;
            if((pitchInMacroTile % 4) == 0)
            {   //multiple of 4
                y5 = _BIT(y,5);
                y6 = _BIT(y,6);
                x5 = pipebit2 ^ y6;
                x6 = pipebit3 ^ y5;
                *pY = Bits2Number(2, y4, y3);
                *pX = Bits2Number(4, x6, x5,x4, x3);
            }
            else
            {
                y6 = _BIT(y,6);
                x5 = pipebit2 ^ y6;
                *pY = Bits2Number(2, y4, y3);
                *pX = Bits2Number(3, x5, x4, x3);
            }
            break;
        case ADDR_PIPECFG_P16_32x32_16x16:
            x4 = elemIdx1;
            y3 = elemIdx0 ^ x4;
            y4 = pipebit1 ^ x4;
            x3 = pipebit0 ^ y3 ^ x4;
            if((pitchInMacroTile % 4) == 0)
            {   //multiple of 4
                y5 = _BIT(y,5);
                y6 = _BIT(y,6);
                x5 = pipebit2 ^ y6;
                x6 = pipebit3 ^ y5;
                *pY = Bits2Number(2, y4, y3);
                *pX = Bits2Number(4, x6, x5, x4, x3);
            }
            else
            {
                y6 = _BIT(y,6);
                x5 = pipebit2 ^ y6;
                *pY = Bits2Number(2, y4, y3);
                *pX = Bits2Number(3, x5, x4, x3);
            }
            break;
        default:
            ADDR_UNHANDLED_CASE();
    }
}

/**
***************************************************************************************************
*   SIAddrLib::TileCoordToMaskElementIndex
*
*   @brief
*       Compute element index from coordinates in tiles
*   @return
*       Element index
***************************************************************************************************
*/
uint32_t SIAddrLib::TileCoordToMaskElementIndex(
    uint32_t        tx,                 ///< [in] x coord, in Tiles
    uint32_t        ty,                 ///< [in] y coord, in Tiles
    AddrPipeCfg     pipeConfig,         ///< [in] pipe config
    uint32_t*       macroShift,         ///< [out] macro shift
    uint32_t*       elemIdxBits         ///< [out] tile offset bits
    ) const
{
    uint32_t elemIdx = 0;
    uint32_t elemIdx0, elemIdx1, elemIdx2;
    uint32_t tx0, tx1;
    uint32_t ty0, ty1;

    tx0 = _BIT(tx,0);
    tx1 = _BIT(tx,1);
    ty0 = _BIT(ty,0);
    ty1 = _BIT(ty,1);

    switch(pipeConfig)
    {
        case ADDR_PIPECFG_P2:
            *macroShift = 3;
            *elemIdxBits =3;
            elemIdx2 = tx1;
            elemIdx1 = tx1 ^ ty1;
            elemIdx0 = tx1 ^ ty0;
            elemIdx = Bits2Number(3,elemIdx2,elemIdx1,elemIdx0);
            break;
        case ADDR_PIPECFG_P4_8x16:
            *macroShift = 2;
            *elemIdxBits =2;
            elemIdx1 = tx1;
            elemIdx0 = tx1 ^ ty1;
            elemIdx = Bits2Number(2,elemIdx1,elemIdx0);
            break;
        case ADDR_PIPECFG_P4_16x16:
            *macroShift = 2;
            *elemIdxBits =2;
            elemIdx0 = tx1^ty0;
            elemIdx1 = tx1;
            elemIdx = Bits2Number(2, elemIdx1, elemIdx0);
            break;
        case ADDR_PIPECFG_P4_16x32:
            *macroShift = 2;
            *elemIdxBits =2;
            elemIdx0 = tx1^ty0;
            elemIdx1 = tx1^ty1;
            elemIdx = Bits2Number(2, elemIdx1, elemIdx0);
            break;
        case ADDR_PIPECFG_P4_32x32:
            *macroShift = 2;
            *elemIdxBits =3;
            elemIdx0 = tx1^ty0;
            elemIdx1 = tx1^ty1;
            elemIdx2 = tx1;
            elemIdx = Bits2Number(3, elemIdx2, elemIdx1, elemIdx0);
            break;
        case ADDR_PIPECFG_P8_16x16_8x16:
            *macroShift = 1;
            *elemIdxBits =1;
            elemIdx0 = tx1;
            elemIdx = elemIdx0;
            break;
        case ADDR_PIPECFG_P8_16x32_8x16:
            *macroShift = 1;
            *elemIdxBits =1;
            elemIdx0 = tx0;
            elemIdx = elemIdx0;
            break;
        case ADDR_PIPECFG_P8_32x32_8x16:
            *macroShift = 1;
            *elemIdxBits =2;
            elemIdx1 = tx1;
            elemIdx0 = tx1^ty1;
            elemIdx = Bits2Number(2, elemIdx1, elemIdx0);
            break;
        case ADDR_PIPECFG_P8_16x32_16x16:
            *macroShift = 1;
            *elemIdxBits =1;
            elemIdx0 = tx0;
            elemIdx = elemIdx0;
            break;
        case ADDR_PIPECFG_P8_32x32_16x16:
            *macroShift = 1;
            *elemIdxBits =2;
            elemIdx0 = tx1^ty0;
            elemIdx1 = tx1;
            elemIdx = Bits2Number(2, elemIdx1, elemIdx0);
            break;
        case ADDR_PIPECFG_P8_32x32_16x32:
            *macroShift = 1;
            *elemIdxBits =2;
            elemIdx0 =  tx1^ty0;
            elemIdx1 = tx1^ty1;
            elemIdx = Bits2Number(2, elemIdx1, elemIdx0);
            break;
        case ADDR_PIPECFG_P8_32x64_32x32:
            *macroShift = 1;
            *elemIdxBits =3;
            elemIdx0 = tx1^ty0;
            elemIdx1 = tx1^ty1;
            elemIdx2 = tx1;
            elemIdx = Bits2Number(3, elemIdx2, elemIdx1, elemIdx0);
            break;
        case ADDR_PIPECFG_P16_32x32_8x16:
            *macroShift = 0;
            *elemIdxBits =2;
            elemIdx0 = tx1^ty1;
            elemIdx1 = tx1;
            elemIdx = Bits2Number(2, elemIdx1, elemIdx0);
            break;
        case ADDR_PIPECFG_P16_32x32_16x16:
            *macroShift = 0;
            *elemIdxBits =2;
            elemIdx0 = tx1^ty0;
            elemIdx1 = tx1;
            elemIdx = Bits2Number(2, elemIdx1, elemIdx0);
            break;
        default:
            ADDR_UNHANDLED_CASE();
            break;
    }

    return elemIdx;
}

/**
***************************************************************************************************
*   SIAddrLib::HwlComputeTileDataWidthAndHeightLinear
*
*   @brief
*       Compute the squared cache shape for per-tile data (CMASK and HTILE) for linear layout
*
*   @return
*       N/A
*
*   @note
*       MacroWidth and macroHeight are measured in pixels
***************************************************************************************************
*/
VOID SIAddrLib::HwlComputeTileDataWidthAndHeightLinear(
    uint32_t*       pMacroWidth,     ///< [out] macro tile width
    uint32_t*       pMacroHeight,    ///< [out] macro tile height
    uint32_t        bpp,             ///< [in] bits per pixel
    ADDR_TILEINFO*  pTileInfo        ///< [in] tile info
    ) const
{
    ADDR_ASSERT(pTileInfo != NULL);
    uint32_t macroWidth;
    uint32_t macroHeight;

    /// In linear mode, the htile or cmask buffer must be padded out to 4 tiles
    /// but for P8_32x64_32x32, it must be padded out to 8 tiles
    /// Actually there are more pipe configs which need 8-tile padding but SI family
    /// has a bug which is fixed in CI family
    if ((pTileInfo->pipeConfig == ADDR_PIPECFG_P8_32x64_32x32) ||
        (pTileInfo->pipeConfig == ADDR_PIPECFG_P16_32x32_8x16) ||
        (pTileInfo->pipeConfig == ADDR_PIPECFG_P8_32x32_16x16))
    {
        macroWidth  = 8*MicroTileWidth;
        macroHeight = 8*MicroTileHeight;
    }
    else
    {
        macroWidth  = 4*MicroTileWidth;
        macroHeight = 4*MicroTileHeight;
    }

    *pMacroWidth    = macroWidth;
    *pMacroHeight   = macroHeight;
}

/**
***************************************************************************************************
*   SIAddrLib::HwlComputeHtileBytes
*
*   @brief
*       Compute htile size in bytes
*
*   @return
*       Htile size in bytes
***************************************************************************************************
*/
uint64_t SIAddrLib::HwlComputeHtileBytes(
    uint32_t    pitch,          ///< [in] pitch
    uint32_t    height,         ///< [in] height
    uint32_t    bpp,            ///< [in] bits per pixel
    BOOL_32     isLinear,       ///< [in] if it is linear mode
    uint32_t    numSlices,      ///< [in] number of slices
    uint64_t*   pSliceBytes,    ///< [out] bytes per slice
    uint32_t    baseAlign       ///< [in] base alignments
    ) const
{
    return ComputeHtileBytes(pitch, height, bpp, isLinear, numSlices, pSliceBytes, baseAlign);
}

/**
***************************************************************************************************
*   SIAddrLib::HwlComputeXmaskAddrFromCoord
*
*   @brief
*       Compute address from coordinates for htile/cmask
*   @return
*       Byte address
***************************************************************************************************
*/
uint64_t SIAddrLib::HwlComputeXmaskAddrFromCoord(
    uint32_t       pitch,          ///< [in] pitch
    uint32_t       height,         ///< [in] height
    uint32_t       x,              ///< [in] x coord
    uint32_t       y,              ///< [in] y coord
    uint32_t       slice,          ///< [in] slice/depth index
    uint32_t       numSlices,      ///< [in] number of slices
    uint32_t       factor,         ///< [in] factor that indicates cmask(2) or htile(1)
    BOOL_32        isLinear,       ///< [in] linear or tiled HTILE layout
    BOOL_32        isWidth8,       ///< [in] TRUE if width is 8, FALSE means 4. It's register value
    BOOL_32        isHeight8,      ///< [in] TRUE if width is 8, FALSE means 4. It's register value
    ADDR_TILEINFO* pTileInfo,      ///< [in] Tile info
    uint32_t*      pBitPosition    ///< [out] bit position inside a byte
    ) const
{
    uint32_t tx = x / MicroTileWidth;
    uint32_t ty = y / MicroTileHeight;
    uint32_t newPitch;
    uint32_t newHeight;
    uint64_t totalBytes;
    uint32_t macroWidth;
    uint32_t macroHeight;
    uint64_t pSliceBytes;
    uint32_t pBaseAlign;
    uint32_t tileNumPerPipe;
    uint32_t elemBits;

    if (factor == 2) //CMASK
    {
        ADDR_CMASK_FLAGS flags = {{0}};

        tileNumPerPipe = 256;

        ComputeCmaskInfo(flags,
                         pitch,
                         height,
                         numSlices,
                         isLinear,
                         pTileInfo,
                         &newPitch,
                         &newHeight,
                         &totalBytes,
                         &macroWidth,
                         &macroHeight);
        elemBits = CmaskElemBits;
    }
    else //HTile
    {
        ADDR_HTILE_FLAGS flags = {{0}};

        tileNumPerPipe = 512;

        ComputeHtileInfo(flags,
                         pitch,
                         height,
                         numSlices,
                         isLinear,
                         TRUE,
                         TRUE,
                         pTileInfo,
                         &newPitch,
                         &newHeight,
                         &totalBytes,
                         &macroWidth,
                         &macroHeight,
                         &pSliceBytes,
                         &pBaseAlign);
        elemBits = 32;
    }

    const uint32_t pitchInTile = newPitch / MicroTileWidth;
    const uint32_t heightInTile = newHeight / MicroTileWidth;
    uint64_t macroOffset; // Per pipe starting offset of the macro tile in which this tile lies.
    uint64_t microNumber; // Per pipe starting offset of the macro tile in which this tile lies.
    uint32_t microX;
    uint32_t microY;
    uint64_t microOffset;
    uint32_t microShift;
    uint64_t totalOffset;
    uint32_t elemIdxBits;
    uint32_t elemIdx =
        TileCoordToMaskElementIndex(tx, ty, pTileInfo->pipeConfig, &microShift, &elemIdxBits);

    uint32_t numPipes = HwlGetPipes(pTileInfo);

    if (isLinear)
    {   //linear addressing
        // Linear addressing is extremelly wasting memory if slice > 1, since each pipe has the full
        // slice memory foot print instead of divided by numPipes.
        microX = tx / 4; // Macro Tile is 4x4
        microY = ty / 4 ;
        microNumber = static_cast<uint64_t>(microX + microY * (pitchInTile / 4)) << microShift;

        uint32_t sliceBits = pitchInTile * heightInTile;

        // do htile single slice alignment if the flag is true
        if (m_configFlags.useHtileSliceAlign && (factor == 1))  //Htile
        {
            sliceBits = PowTwoAlign(sliceBits, BITS_TO_BYTES(HtileCacheBits) * numPipes / elemBits);
        }
        macroOffset = slice * (sliceBits / numPipes) * elemBits ;
    }
    else
    {   //tiled addressing
        const uint32_t macroWidthInTile = macroWidth / MicroTileWidth; // Now in unit of Tiles
        const uint32_t macroHeightInTile = macroHeight / MicroTileHeight;
        const uint32_t pitchInCL = pitchInTile / macroWidthInTile;
        const uint32_t heightInCL = heightInTile / macroHeightInTile;

        const uint32_t macroX = x / macroWidth;
        const uint32_t macroY = y / macroHeight;
        const uint32_t macroNumber = macroX + macroY * pitchInCL + slice * pitchInCL * heightInCL;

        // Per pipe starting offset of the cache line in which this tile lies.
        microX = (x % macroWidth) / MicroTileWidth / 4; // Macro Tile is 4x4
        microY = (y % macroHeight) / MicroTileHeight / 4 ;
        microNumber = static_cast<uint64_t>(microX + microY * (macroWidth / MicroTileWidth / 4)) << microShift;

        macroOffset = macroNumber * tileNumPerPipe * elemBits;
    }

    if(elemIdxBits == microShift)
    {
        microNumber += elemIdx;
    }
    else
    {
        microNumber >>= elemIdxBits;
        microNumber <<= elemIdxBits;
        microNumber += elemIdx;
    }

    microOffset = elemBits * microNumber;
    totalOffset = microOffset + macroOffset;

    uint32_t pipe = ComputePipeFromCoord(x, y, 0, ADDR_TM_2D_TILED_THIN1, 0, FALSE, pTileInfo);
    uint64_t addrInBits = totalOffset % (m_pipeInterleaveBytes * 8) +
                   pipe * (m_pipeInterleaveBytes * 8) +
                   totalOffset / (m_pipeInterleaveBytes * 8) * (m_pipeInterleaveBytes * 8) * numPipes;
    *pBitPosition = static_cast<uint32_t>(addrInBits) % 8;
    uint64_t addr = addrInBits / 8;

    return addr;
}

/**
***************************************************************************************************
*   SIAddrLib::HwlComputeXmaskCoordFromAddr
*
*   @brief
*       Compute the coord from an address of a cmask/htile
*
*   @return
*       N/A
*
*   @note
*       This method is reused by htile, so rename to Xmask
***************************************************************************************************
*/
VOID SIAddrLib::HwlComputeXmaskCoordFromAddr(
    uint64_t        addr,           ///< [in] address
    uint32_t        bitPosition,    ///< [in] bitPosition in a byte
    uint32_t        pitch,          ///< [in] pitch
    uint32_t        height,         ///< [in] height
    uint32_t        numSlices,      ///< [in] number of slices
    uint32_t        factor,         ///< [in] factor that indicates cmask or htile
    BOOL_32         isLinear,       ///< [in] linear or tiled HTILE layout
    BOOL_32         isWidth8,       ///< [in] Not used by SI
    BOOL_32         isHeight8,      ///< [in] Not used by SI
    ADDR_TILEINFO*  pTileInfo,      ///< [in] Tile info
    uint32_t*       pX,             ///< [out] x coord
    uint32_t*       pY,             ///< [out] y coord
    uint32_t*       pSlice          ///< [out] slice index
    ) const
{
    uint32_t newPitch;
    uint32_t newHeight;
    uint64_t totalBytes;
    uint32_t clWidth;
    uint32_t clHeight;
    uint32_t tileNumPerPipe;
    uint64_t sliceBytes;

    *pX = 0;
    *pY = 0;
    *pSlice = 0;

    if (factor == 2) //CMASK
    {
        ADDR_CMASK_FLAGS flags = {{0}};

        tileNumPerPipe = 256;

        ComputeCmaskInfo(flags,
                         pitch,
                         height,
                         numSlices,
                         isLinear,
                         pTileInfo,
                         &newPitch,
                         &newHeight,
                         &totalBytes,
                         &clWidth,
                         &clHeight);
    }
    else //HTile
    {
        ADDR_HTILE_FLAGS flags = {{0}};

        tileNumPerPipe = 512;

        ComputeHtileInfo(flags,
                         pitch,
                         height,
                         numSlices,
                         isLinear,
                         TRUE,
                         TRUE,
                         pTileInfo,
                         &newPitch,
                         &newHeight,
                         &totalBytes,
                         &clWidth,
                         &clHeight,
                         &sliceBytes);
    }

    const uint32_t pitchInTile = newPitch / MicroTileWidth;
    const uint32_t heightInTile = newHeight / MicroTileWidth;
    const uint32_t pitchInMacroTile = pitchInTile / 4;
    uint32_t macroShift;
    uint32_t elemIdxBits;
    // get macroShift and elemIdxBits
    TileCoordToMaskElementIndex(0, 0, pTileInfo->pipeConfig, &macroShift, &elemIdxBits);

    const uint32_t numPipes = HwlGetPipes(pTileInfo);
    const uint32_t pipe = (uint32_t)((addr / m_pipeInterleaveBytes) % numPipes);
    // per pipe
    uint64_t localOffset = (addr % m_pipeInterleaveBytes) +
        (addr / m_pipeInterleaveBytes / numPipes)* m_pipeInterleaveBytes;

    uint32_t tileIndex;
    if (factor == 2) //CMASK
    {
        tileIndex = (uint32_t)(localOffset * 2 + (bitPosition != 0));
    }
    else
    {
        tileIndex = (uint32_t)(localOffset / 4);
    }

    uint32_t macroOffset;
    if (isLinear)
    {
        uint32_t sliceSizeInTile = pitchInTile * heightInTile;

        // do htile single slice alignment if the flag is true
        if (m_configFlags.useHtileSliceAlign && (factor == 1))  //Htile
        {
            sliceSizeInTile = PowTwoAlign(sliceSizeInTile, static_cast<uint32_t>(sliceBytes) / 64);
        }
        *pSlice = tileIndex / (sliceSizeInTile / numPipes);
        macroOffset = tileIndex % (sliceSizeInTile / numPipes);
    }
    else
    {
        const uint32_t clWidthInTile = clWidth / MicroTileWidth; // Now in unit of Tiles
        const uint32_t clHeightInTile = clHeight / MicroTileHeight;
        const uint32_t pitchInCL = pitchInTile / clWidthInTile;
        const uint32_t heightInCL = heightInTile / clHeightInTile;
        const uint32_t clIndex = tileIndex / tileNumPerPipe;

        uint32_t clX = clIndex % pitchInCL;
        uint32_t clY = (clIndex % (heightInCL * pitchInCL)) / pitchInCL;

        *pX = clX * clWidthInTile * MicroTileWidth;
        *pY = clY * clHeightInTile * MicroTileHeight;
        *pSlice = clIndex / (heightInCL * pitchInCL);

        macroOffset = tileIndex % tileNumPerPipe;
    }

    uint32_t elemIdx = macroOffset & 7;
    macroOffset >>= elemIdxBits;

    if (elemIdxBits != macroShift)
    {
        macroOffset <<= (elemIdxBits - macroShift);

        uint32_t pipebit1 = _BIT(pipe,1);
        uint32_t pipebit2 = _BIT(pipe,2);
        uint32_t pipebit3 = _BIT(pipe,3);
        if (pitchInMacroTile % 2)
        {   //odd
            switch (pTileInfo->pipeConfig)
            {
                case ADDR_PIPECFG_P4_32x32:
                    macroOffset |= pipebit1;
                    break;
                case ADDR_PIPECFG_P8_32x32_8x16:
                case ADDR_PIPECFG_P8_32x32_16x16:
                case ADDR_PIPECFG_P8_32x32_16x32:
                    macroOffset |= pipebit2;
                    break;
                default:
                    break;
            }

        }

        if (pitchInMacroTile % 4)
        {
            if (pTileInfo->pipeConfig == ADDR_PIPECFG_P8_32x64_32x32)
            {
                macroOffset |= (pipebit1<<1);
            }
            if((pTileInfo->pipeConfig == ADDR_PIPECFG_P16_32x32_8x16) ||
               (pTileInfo->pipeConfig == ADDR_PIPECFG_P16_32x32_16x16))
            {
                macroOffset |= (pipebit3<<1);
            }
        }
    }

    uint32_t macroX;
    uint32_t macroY;

    if (isLinear)
    {
        macroX = macroOffset % pitchInMacroTile;
        macroY = macroOffset / pitchInMacroTile;
    }
    else
    {
        const uint32_t clWidthInMacroTile = clWidth / (MicroTileWidth * 4);
        macroX = macroOffset % clWidthInMacroTile;
        macroY = macroOffset / clWidthInMacroTile;
    }

    *pX += macroX * 4 * MicroTileWidth;
    *pY += macroY * 4 * MicroTileHeight;

    uint32_t microX;
    uint32_t microY;
    ComputeTileCoordFromPipeAndElemIdx(elemIdx, pipe, pTileInfo->pipeConfig, pitchInMacroTile,
                                       *pX, *pY, &microX, &microY);

    *pX += microX * MicroTileWidth;
    *pY += microY * MicroTileWidth;
}

/**
***************************************************************************************************
*   SIAddrLib::HwlGetPitchAlignmentLinear
*   @brief
*       Get pitch alignment
*   @return
*       pitch alignment
***************************************************************************************************
*/
uint32_t SIAddrLib::HwlGetPitchAlignmentLinear(
    uint32_t            bpp,    ///< [in] bits per pixel
    ADDR_SURFACE_FLAGS  flags   ///< [in] surface flags
    ) const
{
    uint32_t pitchAlign;

    // Interleaved access requires a 256B aligned pitch, so fall back to pre-SI alignment
    if (flags.interleaved)
    {
        pitchAlign = Max(64u, m_pipeInterleaveBytes / BITS_TO_BYTES(bpp));

    }
    else
    {
        pitchAlign = Max(8u, 64 / BITS_TO_BYTES(bpp));
    }

    return pitchAlign;
}

/**
***************************************************************************************************
*   SIAddrLib::HwlGetSizeAdjustmentLinear
*
*   @brief
*       Adjust linear surface pitch and slice size
*
*   @return
*       Logical slice size in bytes
***************************************************************************************************
*/
uint64_t SIAddrLib::HwlGetSizeAdjustmentLinear(
    AddrTileMode        tileMode,       ///< [in] tile mode
    uint32_t            bpp,            ///< [in] bits per pixel
    uint32_t            numSamples,     ///< [in] number of samples
    uint32_t            baseAlign,      ///< [in] base alignment
    uint32_t            pitchAlign,     ///< [in] pitch alignment
    uint32_t*           pPitch,         ///< [in/out] pointer to pitch
    uint32_t*           pHeight,        ///< [in/out] pointer to height
    uint32_t*           pHeightAlign    ///< [in/out] pointer to height align
    ) const
{
    uint64_t sliceSize;
    if (tileMode == ADDR_TM_LINEAR_GENERAL)
    {
        sliceSize = BITS_TO_BYTES(static_cast<uint64_t>(*pPitch) * (*pHeight) * bpp * numSamples);
    }
    else
    {
        uint32_t pitch  = *pPitch;
        uint32_t height = *pHeight;

        uint32_t pixelsPerPipeInterleave = m_pipeInterleaveBytes / BITS_TO_BYTES(bpp);
        uint32_t sliceAlignInPixel = pixelsPerPipeInterleave < 64 ? 64 : pixelsPerPipeInterleave;

        // numSamples should be 1 in real cases (no MSAA for linear but TGL may pass non 1 value)
        uint64_t pixelPerSlice = static_cast<uint64_t>(pitch) * height * numSamples;

        while (pixelPerSlice % sliceAlignInPixel)
        {
            pitch += pitchAlign;
            pixelPerSlice = static_cast<uint64_t>(pitch) * height * numSamples;
        }

        *pPitch = pitch;

        uint32_t heightAlign = 1;

        while ((pitch * heightAlign) % sliceAlignInPixel)
        {
            heightAlign++;
        }

        *pHeightAlign = heightAlign;

        sliceSize = BITS_TO_BYTES(pixelPerSlice * bpp);
    }

    return sliceSize;
}

/**
***************************************************************************************************
*   SIAddrLib::HwlPreHandleBaseLvl3xPitch
*
*   @brief
*       Pre-handler of 3x pitch (96 bit) adjustment
*
*   @return
*       Expected pitch
***************************************************************************************************
*/
uint32_t SIAddrLib::HwlPreHandleBaseLvl3xPitch(
    const ADDR_COMPUTE_SURFACE_INFO_INPUT*  pIn,        ///< [in] input
    uint32_t                                expPitch    ///< [in] pitch
    ) const
{
    ADDR_ASSERT(pIn->width == expPitch);

    // From SI, if pow2Pad is 1 the pitch is expanded 3x first, then padded to pow2, so nothing to
    // do here
    if (!pIn->flags.pow2Pad)
    {
        AddrLib::HwlPreHandleBaseLvl3xPitch(pIn, expPitch);
    }
    else
    {
        ADDR_ASSERT(IsPow2(expPitch));
    }

    return expPitch;
}

/**
***************************************************************************************************
*   SIAddrLib::HwlPostHandleBaseLvl3xPitch
*
*   @brief
*       Post-handler of 3x pitch adjustment
*
*   @return
*       Expected pitch
***************************************************************************************************
*/
uint32_t SIAddrLib::HwlPostHandleBaseLvl3xPitch(
    const ADDR_COMPUTE_SURFACE_INFO_INPUT*  pIn,        ///< [in] input
    uint32_t                                expPitch    ///< [in] pitch
    ) const
{
    /**
     * @note The pitch will be divided by 3 in the end so the value will look odd but h/w should
     *  be able to compute a correct pitch from it as h/w address library is doing the job.
     */
    // From SI, the pitch is expanded 3x first, then padded to pow2, so no special handler here
    if (!pIn->flags.pow2Pad)
    {
        AddrLib::HwlPostHandleBaseLvl3xPitch(pIn, expPitch);
    }

    return expPitch;
}

/**
***************************************************************************************************
*   SIAddrLib::HwlGetPitchAlignmentMicroTiled
*
*   @brief
*       Compute 1D tiled surface pitch alignment
*
*   @return
*       pitch alignment
***************************************************************************************************
*/
uint32_t SIAddrLib::HwlGetPitchAlignmentMicroTiled(
    AddrTileMode        tileMode,          ///< [in] tile mode
    uint32_t            bpp,               ///< [in] bits per pixel
    ADDR_SURFACE_FLAGS  flags,             ///< [in] surface flags
    uint32_t            numSamples         ///< [in] number of samples
    ) const
{
    uint32_t pitchAlign;

    if (flags.qbStereo)
    {
        pitchAlign = EgBasedAddrLib::HwlGetPitchAlignmentMicroTiled(tileMode,bpp,flags,numSamples);
    }
    else
    {
        pitchAlign = 8;
    }

    return pitchAlign;
}

/**
***************************************************************************************************
*   SIAddrLib::HwlGetSizeAdjustmentMicroTiled
*
*   @brief
*       Adjust 1D tiled surface pitch and slice size
*
*   @return
*       Logical slice size in bytes
***************************************************************************************************
*/
uint64_t SIAddrLib::HwlGetSizeAdjustmentMicroTiled(
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

    uint32_t pitch  = *pPitch;
    uint32_t height = *pHeight;

    // Logical slice: pitch * height * bpp * numSamples (no 1D MSAA so actually numSamples == 1)
    logicalSliceSize = BITS_TO_BYTES(static_cast<uint64_t>(pitch) * height * bpp * numSamples);

    // Physical slice: multiplied by thickness
    physicalSliceSize =  logicalSliceSize * thickness;

    // Pitch alignment is always 8, so if slice size is not padded to base alignment
    // (pipe_interleave_size), we need to increase pitch
    while ((physicalSliceSize % baseAlign) != 0)
    {
        pitch += pitchAlign;

        logicalSliceSize = BITS_TO_BYTES(static_cast<uint64_t>(pitch) * height * bpp * numSamples);

        physicalSliceSize =  logicalSliceSize * thickness;
    }

#if !ALT_TEST
    //
    // Special workaround for depth/stencil buffer, use 8 bpp to align depth buffer again since
    // the stencil plane may have larger pitch if the slice size is smaller than base alignment.
    //
    // Note: this actually does not work for mipmap but mipmap depth texture is not really
    // sampled with mipmap.
    //
    if (flags.depth && !flags.noStencil)
    {
        ADDR_ASSERT(numSamples == 1);

        uint64_t logicalSiceSizeStencil = static_cast<uint64_t>(pitch) * height; // 1 byte stencil

        while ((logicalSiceSizeStencil % baseAlign) != 0)
        {
            pitch += pitchAlign; // Stencil plane's pitch alignment is the same as depth plane's

            logicalSiceSizeStencil = static_cast<uint64_t>(pitch) * height;
        }

        if (pitch != *pPitch)
        {
            // If this is a mipmap, this padded one cannot be sampled as a whole mipmap!
            logicalSliceSize = logicalSiceSizeStencil * BITS_TO_BYTES(bpp);
        }
    }
#endif
    *pPitch = pitch;

    // No adjust for pHeight

    return logicalSliceSize;
}

/**
***************************************************************************************************
*   SIAddrLib::HwlConvertChipFamily
*
*   @brief
*       Convert familyID defined in atiid.h to AddrChipFamily and set m_chipFamily/m_chipRevision
*   @return
*       AddrChipFamily
***************************************************************************************************
*/
AddrChipFamily SIAddrLib::HwlConvertChipFamily(
    uint32_t uChipFamily,       ///< [in] chip family defined in atiih.h
    uint32_t uChipRevision)     ///< [in] chip revision defined in "asic_family"_id.h
{
    AddrChipFamily family = ADDR_CHIP_FAMILY_SI;

    switch (uChipFamily)
    {
        case FAMILY_SI:
            m_settings.isSouthernIsland = 1;
            m_settings.isTahiti     = ASICREV_IS_TAHITI_P(uChipRevision);
            m_settings.isPitCairn   = ASICREV_IS_PITCAIRN_PM(uChipRevision);
            m_settings.isCapeVerde  = ASICREV_IS_CAPEVERDE_M(uChipRevision);
            m_settings.isOland      = ASICREV_IS_OLAND_M(uChipRevision);
            m_settings.isHainan     = ASICREV_IS_HAINAN_V(uChipRevision);
            break;
        default:
            ADDR_ASSERT(!"This should be a Fusion");
            break;
    }

    return family;
}

/**
***************************************************************************************************
*   SIAddrLib::HwlSetupTileInfo
*
*   @brief
*       Setup default value of tile info for SI
***************************************************************************************************
*/
VOID SIAddrLib::HwlSetupTileInfo(
    AddrTileMode                        tileMode,       ///< [in] Tile mode
    ADDR_SURFACE_FLAGS                  flags,          ///< [in] Surface type flags
    uint32_t                            bpp,            ///< [in] Bits per pixel
    uint32_t                            pitch,          ///< [in] Pitch in pixels
    uint32_t                            height,         ///< [in] Height in pixels
    uint32_t                            numSamples,     ///< [in] Number of samples
    ADDR_TILEINFO*                      pTileInfoIn,    ///< [in] Tile info input: NULL for default
    ADDR_TILEINFO*                      pTileInfoOut,   ///< [out] Tile info output
    AddrTileType                        inTileType,     ///< [in] Tile type
    ADDR_COMPUTE_SURFACE_INFO_OUTPUT*   pOut            ///< [out] Output
    ) const
{
    uint32_t thickness = ComputeSurfaceThickness(tileMode);
    ADDR_TILEINFO* pTileInfo = pTileInfoOut;
    INT index = TileIndexInvalid;

    // Fail-safe code
    if (!IsLinear(tileMode))
    {
        // 128 bpp/thick tiling must be non-displayable.
        // Fmask reuse color buffer's entry but bank-height field can be from another entry
        // To simplify the logic, fmask entry should be picked from non-displayable ones
        if (bpp == 128 || thickness > 1 || flags.fmask || flags.prt)
        {
            inTileType = ADDR_NON_DISPLAYABLE;
        }

        if (flags.depth || flags.stencil)
        {
            inTileType = ADDR_DEPTH_SAMPLE_ORDER;
        }
    }

    // Partial valid fields are not allowed for SI.
    if (IsTileInfoAllZero(pTileInfo))
    {
        if (IsMacroTiled(tileMode))
        {
            if (flags.prt)
            {
                if (numSamples == 1)
                {
                    if (flags.depth)
                    {
                        switch (bpp)
                        {
                            case 16:
                                index = 3;
                                break;
                            case 32:
                                index = 6;
                                break;
                            default:
                                ADDR_ASSERT_ALWAYS();
                                break;
                        }
                    }
                    else
                    {
                        switch (bpp)
                        {
                            case 8:
                                index = 21;
                                break;
                            case 16:
                                index = 22;
                                break;
                            case 32:
                                index = 23;
                                break;
                            case 64:
                                index = 24;
                                break;
                            case 128:
                                index = 25;
                                break;
                            default:
                                break;
                        }

                        if (thickness > 1)
                        {
                            ADDR_ASSERT(bpp != 128);
                            index += 5;
                        }
                    }
                }
                else
                {
                    ADDR_ASSERT(numSamples == 4);

                    if (flags.depth)
                    {
                        switch (bpp)
                        {
                            case 16:
                                index = 5;
                                break;
                            case 32:
                                index = 7;
                                break;
                            default:
                                ADDR_ASSERT_ALWAYS();
                                break;
                        }
                    }
                    else
                    {
                        switch (bpp)
                        {
                            case 8:
                                index = 23;
                                break;
                            case 16:
                                index = 24;
                                break;
                            case 32:
                                index = 25;
                                break;
                            case 64:
                                index = 30;
                                break;
                            default:
                                ADDR_ASSERT_ALWAYS();
                                break;
                        }
                    }
                }
            }//end of PRT part
            // See table entries 0-7
            else if (flags.depth || flags.stencil)
            {
                if (flags.compressZ)
                {
                    if (flags.stencil)
                    {
                        index = 0;
                    }
                    else
                    {
                        // optimal tile index for compressed depth/stencil.
                        switch (numSamples)
                        {
                            case 1:
                                index = 0;
                                break;
                            case 2:
                            case 4:
                                index = 1;
                                break;
                            case 8:
                                index = 2;
                                break;
                            default:
                                break;
                        }
                    }
                }
                else // unCompressZ
                {
                    index = 3;
                }
            }
            else //non PRT & non Depth & non Stencil
            {
                // See table entries 9-12
                if (inTileType == ADDR_DISPLAYABLE)
                {
                    switch (bpp)
                    {
                        case 8:
                            index = 10;
                            break;
                        case 16:
                            index = 11;
                            break;
                        case 32:
                            index = 12;
                            break;
                        case 64:
                            index = 12;
                            break;
                        default:
                            break;
                    }
                }
                else
                {
                    // See table entries 13-17
                    if (thickness == 1)
                    {
                        if (flags.fmask)
                        {
                            uint32_t fmaskPixelSize = bpp * numSamples;

                            switch (fmaskPixelSize)
                            {
                                case 8:
                                    index = 14;
                                    break;
                                case 16:
                                    index = 15;
                                    break;
                                case 32:
                                    index = 16;
                                    break;
                                case 64:
                                    index = 17;
                                    break;
                                default:
                                    ADDR_ASSERT_ALWAYS();
                            }
                        }
                        else
                        {
                            switch (bpp)
                            {
                                case 8:
                                    index = 14;
                                    break;
                                case 16:
                                    index = 15;
                                    break;
                                case 32:
                                    index = 16;
                                    break;
                                case 64:
                                    index = 17;
                                    break;
                                case 128:
                                    index = 17;
                                    break;
                                default:
                                    break;
                            }
                        }
                    }
                    else // thick tiling - entries 18-20
                    {
                        switch (thickness)
                        {
                            case 4:
                                index = 20;
                                break;
                            case 8:
                                index = 19;
                                break;
                            default:
                                break;
                        }
                    }
                }
            }
        }
        else
        {
            if (tileMode == ADDR_TM_LINEAR_ALIGNED)
            {
                index = 8;
            }
            else if (tileMode == ADDR_TM_LINEAR_GENERAL)
            {
                index = TileIndexLinearGeneral;
            }
            else
            {
                if (flags.depth || flags.stencil)
                {
                    index = 4;
                }
                else if (inTileType == ADDR_DISPLAYABLE)
                {
                    index = 9;
                }
                else if (thickness == 1)
                {
                    index = 13;
                }
                else
                {
                    index = 18;
                }
            }
        }

        if (index >= 0 && index <= 31)
        {
            *pTileInfo      = m_tileTable[index].info;
            pOut->tileType  = m_tileTable[index].type;
        }

        if (index == TileIndexLinearGeneral)
        {
            *pTileInfo      = m_tileTable[8].info;
            pOut->tileType  = m_tileTable[8].type;
        }
    }
    else
    {
        if (pTileInfoIn)
        {
            if (flags.stencil && pTileInfoIn->tileSplitBytes == 0)
            {
                // Stencil always uses index 0
                *pTileInfo = m_tileTable[0].info;
            }
        }
        // Pass through tile type
        pOut->tileType = inTileType;
    }

    pOut->tileIndex = index;
}

/**
***************************************************************************************************
*   SIAddrLib::DecodeGbRegs
*
*   @brief
*       Decodes GB_ADDR_CONFIG and noOfBanks/noOfRanks
*
*   @return
*       TRUE if all settings are valid
*
***************************************************************************************************
*/
BOOL_32 SIAddrLib::DecodeGbRegs(
    const ADDR_REGISTER_VALUE* pRegValue) ///< [in] create input
{
    GB_ADDR_CONFIG  reg;
    BOOL_32         valid = TRUE;

    reg.val = pRegValue->gbAddrConfig;

    switch (reg.f.pipe_interleave_size)
    {
        case ADDR_CONFIG_PIPE_INTERLEAVE_256B:
            m_pipeInterleaveBytes = ADDR_PIPEINTERLEAVE_256B;
            break;
        case ADDR_CONFIG_PIPE_INTERLEAVE_512B:
            m_pipeInterleaveBytes = ADDR_PIPEINTERLEAVE_512B;
            break;
        default:
            valid = FALSE;
            ADDR_UNHANDLED_CASE();
            break;
    }

    switch (reg.f.row_size)
    {
        case ADDR_CONFIG_1KB_ROW:
            m_rowSize = ADDR_ROWSIZE_1KB;
            break;
        case ADDR_CONFIG_2KB_ROW:
            m_rowSize = ADDR_ROWSIZE_2KB;
            break;
        case ADDR_CONFIG_4KB_ROW:
            m_rowSize = ADDR_ROWSIZE_4KB;
            break;
        default:
            valid = FALSE;
            ADDR_UNHANDLED_CASE();
            break;
    }

    switch (pRegValue->noOfBanks)
    {
        case 0:
            m_banks = 4;
            break;
        case 1:
            m_banks = 8;
            break;
        case 2:
            m_banks = 16;
            break;
        default:
            valid = FALSE;
            ADDR_UNHANDLED_CASE();
            break;
    }

    switch (pRegValue->noOfRanks)
    {
        case 0:
            m_ranks = 1;
            break;
        case 1:
            m_ranks = 2;
            break;
        default:
            valid = FALSE;
            ADDR_UNHANDLED_CASE();
            break;
    }

    m_logicalBanks = m_banks * m_ranks;

    ADDR_ASSERT(m_logicalBanks <= 16);

    return valid;
}

/**
***************************************************************************************************
*   SIAddrLib::HwlInitGlobalParams
*
*   @brief
*       Initializes global parameters
*
*   @return
*       TRUE if all settings are valid
*
***************************************************************************************************
*/
BOOL_32 SIAddrLib::HwlInitGlobalParams(
    const ADDR_CREATE_INPUT* pCreateIn) ///< [in] create input
{
    BOOL_32 valid = TRUE;
    const ADDR_REGISTER_VALUE* pRegValue = &pCreateIn->regValue;

    valid = DecodeGbRegs(pRegValue);

    if (valid)
    {
        if (m_settings.isTahiti || m_settings.isPitCairn)
        {
            m_pipes = 8;
        }
        else if (m_settings.isCapeVerde || m_settings.isOland)
        {
            m_pipes = 4;
        }
        else
        {
            // Hainan is 2-pipe (m_settings.isHainan == 1)
            m_pipes = 2;
        }

        valid = InitTileSettingTable(pRegValue->pTileConfig, pRegValue->noOfEntries);

        m_maxSamples = 16;
    }

    return valid;
}

/**
***************************************************************************************************
*   SIAddrLib::HwlConvertTileInfoToHW
*   @brief
*       Entry of si's ConvertTileInfoToHW
*   @return
*       ADDR_E_RETURNCODE
***************************************************************************************************
*/
ADDR_E_RETURNCODE SIAddrLib::HwlConvertTileInfoToHW(
    const ADDR_CONVERT_TILEINFOTOHW_INPUT* pIn, ///< [in] input structure
    ADDR_CONVERT_TILEINFOTOHW_OUTPUT* pOut      ///< [out] output structure
    ) const
{
    ADDR_E_RETURNCODE retCode   = ADDR_OK;

    retCode = EgBasedAddrLib::HwlConvertTileInfoToHW(pIn, pOut);

    if (retCode == ADDR_OK)
    {
        if (pIn->reverse == FALSE)
        {
            if (pIn->pTileInfo->pipeConfig == ADDR_PIPECFG_INVALID)
            {
                retCode = ADDR_INVALIDPARAMS;
            }
            else
            {
                pOut->pTileInfo->pipeConfig =
                    static_cast<AddrPipeCfg>(pIn->pTileInfo->pipeConfig - 1);
            }
        }
        else
        {
            pOut->pTileInfo->pipeConfig =
                static_cast<AddrPipeCfg>(pIn->pTileInfo->pipeConfig + 1);
        }
    }

    return retCode;
}

/**
***************************************************************************************************
*   SIAddrLib::HwlComputeXmaskCoordYFrom8Pipe
*
*   @brief
*       Compute the Y coord which will be added to Xmask Y
*       coord.
*   @return
*       Y coord
***************************************************************************************************
*/
uint32_t SIAddrLib::HwlComputeXmaskCoordYFrom8Pipe(
    uint32_t        pipe,       ///< [in] pipe id
    uint32_t        x           ///< [in] tile coord x, which is original x coord / 8
    ) const
{
    // This function should never be called since it is 6xx/8xx specfic.
    // Keep this empty implementation to avoid any mis-use.
    ADDR_ASSERT_ALWAYS();

    return 0;
}

/**
***************************************************************************************************
*   SIAddrLib::HwlComputeSurfaceCoord2DFromBankPipe
*
*   @brief
*       Compute surface x,y coordinates from bank/pipe info
*   @return
*       N/A
***************************************************************************************************
*/
VOID SIAddrLib::HwlComputeSurfaceCoord2DFromBankPipe(
    AddrTileMode        tileMode,   ///< [in] tile mode
    uint32_t*           pX,         ///< [in/out] x coordinate
    uint32_t*           pY,         ///< [in/out] y coordinate
    uint32_t            slice,      ///< [in] slice index
    uint32_t            bank,       ///< [in] bank number
    uint32_t            pipe,       ///< [in] pipe number
    uint32_t            bankSwizzle,///< [in] bank swizzle
    uint32_t            pipeSwizzle,///< [in] pipe swizzle
    uint32_t            tileSlices, ///< [in] slices in a micro tile
    BOOL_32             ignoreSE,   ///< [in] TRUE if shader engines are ignored
    ADDR_TILEINFO*      pTileInfo   ///< [in] bank structure. **All fields to be valid on entry**
    ) const
{
    uint32_t xBit;
    uint32_t yBit;
    uint32_t yBit3 = 0;
    uint32_t yBit4 = 0;
    uint32_t yBit5 = 0;
    uint32_t yBit6 = 0;

    uint32_t xBit3 = 0;
    uint32_t xBit4 = 0;
    uint32_t xBit5 = 0;

    uint32_t numPipes = GetPipePerSurf(pTileInfo->pipeConfig);

    CoordFromBankPipe xyBits = {0};
    ComputeSurfaceCoord2DFromBankPipe(tileMode, *pX, *pY, slice, bank, pipe,
                                      bankSwizzle, pipeSwizzle, tileSlices, pTileInfo,
                                      &xyBits);
    yBit3 = xyBits.yBit3;
    yBit4 = xyBits.yBit4;
    yBit5 = xyBits.yBit5;
    yBit6 = xyBits.yBit6;

    xBit3 = xyBits.xBit3;
    xBit4 = xyBits.xBit4;
    xBit5 = xyBits.xBit5;

    yBit = xyBits.yBits;

    uint32_t yBitTemp = 0;

    if ((pTileInfo->pipeConfig == ADDR_PIPECFG_P4_32x32) ||
        (pTileInfo->pipeConfig == ADDR_PIPECFG_P8_32x64_32x32))
    {
        ADDR_ASSERT(pTileInfo->bankWidth == 1 && pTileInfo->macroAspectRatio > 1);
        uint32_t yBitToCheck = QLog2(pTileInfo->banks) - 1;

        ADDR_ASSERT(yBitToCheck <= 3);

        yBitTemp = _BIT(yBit, yBitToCheck);

        xBit3 = 0;
    }

    yBit = Bits2Number(4, yBit6, yBit5, yBit4, yBit3);
    xBit = Bits2Number(3, xBit5, xBit4, xBit3);

    *pY += yBit * pTileInfo->bankHeight * MicroTileHeight;
    *pX += xBit * numPipes * pTileInfo->bankWidth * MicroTileWidth;

    //calculate the bank and pipe bits in x, y
    uint32_t xTile; //x in micro tile
    uint32_t x3 = 0;
    uint32_t x4 = 0;
    uint32_t x5 = 0;
    uint32_t x6 = 0;
    uint32_t y = *pY;

    uint32_t pipeBit0 = _BIT(pipe,0);
    uint32_t pipeBit1 = _BIT(pipe,1);
    uint32_t pipeBit2 = _BIT(pipe,2);

    uint32_t y3 = _BIT(y, 3);
    uint32_t y4 = _BIT(y, 4);
    uint32_t y5 = _BIT(y, 5);
    uint32_t y6 = _BIT(y, 6);

    // bankbit0 after ^x4^x5
    uint32_t bankBit00 = _BIT(bank,0);
    uint32_t bankBit0 = 0;

    switch (pTileInfo->pipeConfig)
    {
        case ADDR_PIPECFG_P2:
            x3 = pipeBit0 ^ y3;
            break;
        case ADDR_PIPECFG_P4_8x16:
            x4 = pipeBit0 ^ y3;
            x3 = pipeBit0 ^ y4;
            break;
        case ADDR_PIPECFG_P4_16x16:
            x4 = pipeBit1 ^ y4;
            x3 = pipeBit0 ^ y3 ^ x4;
            break;
        case ADDR_PIPECFG_P4_16x32:
            x4 = pipeBit1 ^ y4;
            x3 = pipeBit0 ^ y3 ^ x4;
            break;
        case ADDR_PIPECFG_P4_32x32:
            x5 = pipeBit1 ^ y5;
            x3 = pipeBit0 ^ y3 ^ x5;
            bankBit0 = yBitTemp ^ x5;
            x4 = bankBit00 ^ x5 ^ bankBit0;
            *pX += x5 * 4 * 1 * 8; // x5 * num_pipes * bank_width * 8;
            break;
        case ADDR_PIPECFG_P8_16x16_8x16:
            x3 = pipeBit1 ^ y5;
            x4 = pipeBit2 ^ y4;
            x5 = pipeBit0 ^ y3 ^ x4;
            break;
        case ADDR_PIPECFG_P8_16x32_8x16:
            x3 = pipeBit1 ^ y4;
            x4 = pipeBit2 ^ y5;
            x5 = pipeBit0 ^ y3 ^ x4;
            break;
        case ADDR_PIPECFG_P8_32x32_8x16:
            x3 = pipeBit1 ^ y4;
            x5 = pipeBit2 ^ y5;
            x4 = pipeBit0 ^ y3 ^ x5;
            break;
        case ADDR_PIPECFG_P8_16x32_16x16:
            x4 = pipeBit2 ^ y5;
            x5 = pipeBit1 ^ y4;
            x3 = pipeBit0 ^ y3 ^ x4;
            break;
        case ADDR_PIPECFG_P8_32x32_16x16:
            x5 = pipeBit2 ^ y5;
            x4 = pipeBit1 ^ y4;
            x3 = pipeBit0 ^ y3 ^ x4;
            break;
        case ADDR_PIPECFG_P8_32x32_16x32:
            x5 = pipeBit2 ^ y5;
            x4 = pipeBit1 ^ y6;
            x3 = pipeBit0 ^ y3 ^ x4;
            break;
        case ADDR_PIPECFG_P8_32x64_32x32:
            x6 = pipeBit1 ^ y5;
            x5 = pipeBit2 ^ y6;
            x3 = pipeBit0 ^ y3 ^ x5;
            bankBit0 = yBitTemp ^ x6;
            x4 = bankBit00 ^ x5 ^ bankBit0;
            *pX += x6 * 8 * 1 * 8; // x6 * num_pipes * bank_width * 8;
            break;
        default:
            ADDR_ASSERT_ALWAYS();
    }

    xTile = Bits2Number(3, x5, x4, x3);

    *pX += xTile << 3;
}

/**
***************************************************************************************************
*   SIAddrLib::HwlPreAdjustBank
*
*   @brief
*       Adjust bank before calculating address acoording to bank/pipe
*   @return
*       Adjusted bank
***************************************************************************************************
*/
uint32_t SIAddrLib::HwlPreAdjustBank(
    uint32_t        tileX,      ///< [in] x coordinate in unit of tile
    uint32_t        bank,       ///< [in] bank
    ADDR_TILEINFO*  pTileInfo   ///< [in] tile info
    ) const
{
    if (((pTileInfo->pipeConfig == ADDR_PIPECFG_P4_32x32) ||
        (pTileInfo->pipeConfig == ADDR_PIPECFG_P8_32x64_32x32)) && (pTileInfo->bankWidth == 1))
    {
        uint32_t bankBit0 = _BIT(bank, 0);
        uint32_t x4 = _BIT(tileX, 1);
        uint32_t x5 = _BIT(tileX, 2);

        bankBit0 = bankBit0 ^ x4 ^ x5;
        bank |= bankBit0;

        ADDR_ASSERT(pTileInfo->macroAspectRatio > 1)
    }

    return bank;
}

/**
***************************************************************************************************
*   SIAddrLib::HwlComputeSurfaceInfo
*
*   @brief
*       Entry of si's ComputeSurfaceInfo
*   @return
*       ADDR_E_RETURNCODE
***************************************************************************************************
*/
ADDR_E_RETURNCODE SIAddrLib::HwlComputeSurfaceInfo(
    const ADDR_COMPUTE_SURFACE_INFO_INPUT*  pIn,    ///< [in] input structure
    ADDR_COMPUTE_SURFACE_INFO_OUTPUT*       pOut    ///< [out] output structure
    ) const
{
    pOut->tileIndex = pIn->tileIndex;

    return EgBasedAddrLib::HwlComputeSurfaceInfo(pIn,pOut);
}

/**
***************************************************************************************************
*   SIAddrLib::HwlComputeMipLevel
*   @brief
*       Compute MipLevel info (including level 0)
*   @return
*       TRUE if HWL's handled
***************************************************************************************************
*/
BOOL_32 SIAddrLib::HwlComputeMipLevel(
    ADDR_COMPUTE_SURFACE_INFO_INPUT* pIn ///< [in/out] Input structure
    ) const
{
    // basePitch is calculated from level 0 so we only check this for mipLevel > 0
    if (pIn->mipLevel > 0)
    {
        // Note: Don't check expand 3x formats(96 bit) as the basePitch is not pow2 even if
        // we explicity set pow2Pad flag. The 3x base pitch is padded to pow2 but after being
        // divided by expandX factor (3) - to program texture pitch, the basePitch is never pow2.
        if (!AddrElemLib::IsExpand3x(pIn->format))
        {
            // Sublevel pitches are generated from base level pitch instead of width on SI
            // If pow2Pad is 0, we don't assert - as this is not really used for a mip chain
            ADDR_ASSERT(!pIn->flags.pow2Pad || ((pIn->basePitch != 0) && IsPow2(pIn->basePitch)));
        }

        if (pIn->basePitch != 0)
        {
            pIn->width = Max(1u, pIn->basePitch >> pIn->mipLevel);
        }
    }

    // pow2Pad is done in PostComputeMipLevel

    return TRUE;
}

/**
***************************************************************************************************
*   SIAddrLib::HwlCheckLastMacroTiledLvl
*
*   @brief
*       Sets pOut->last2DLevel to TRUE if it is
*   @note
*
***************************************************************************************************
*/
VOID SIAddrLib::HwlCheckLastMacroTiledLvl(
    const ADDR_COMPUTE_SURFACE_INFO_INPUT* pIn, ///< [in] Input structure
    ADDR_COMPUTE_SURFACE_INFO_OUTPUT* pOut      ///< [in/out] Output structure (used as input, too)
    ) const
{
    // pow2Pad covers all mipmap cases
    if (pIn->flags.pow2Pad)
    {
        ADDR_ASSERT(IsMacroTiled(pIn->tileMode));

        uint32_t nextPitch;
        uint32_t nextHeight;
        uint32_t nextSlices;

        AddrTileMode nextTileMode;

        if (pIn->mipLevel == 0 || pIn->basePitch == 0)
        {
            // Base level or fail-safe case (basePitch == 0)
            nextPitch = pOut->pitch >> 1;
        }
        else
        {
            // Sub levels
            nextPitch = pIn->basePitch >> (pIn->mipLevel + 1);
        }

        // nextHeight must be shifted from this level's original height rather than a pow2 padded
        // one but this requires original height stored somewhere (pOut->height)
        ADDR_ASSERT(pOut->height != 0);

        // next level's height is just current level's >> 1 in pixels
        nextHeight = pOut->height >> 1;
        // Special format such as FMT_1 and FMT_32_32_32 can be linear only so we consider block
        // compressed foramts
        if (AddrElemLib::IsBlockCompressed(pIn->format))
        {
            nextHeight = (nextHeight + 3) / 4;
        }
        nextHeight = NextPow2(nextHeight);

        // nextSlices may be 0 if this level's is 1
        if (pIn->flags.volume)
        {
            nextSlices = Max(1u, pIn->numSlices >> 1);
        }
        else
        {
            nextSlices = pIn->numSlices;
        }

        nextTileMode = ComputeSurfaceMipLevelTileMode(pIn->tileMode,
                                                      pIn->bpp,
                                                      nextPitch,
                                                      nextHeight,
                                                      nextSlices,
                                                      pIn->numSamples,
                                                      pOut->pitchAlign,
                                                      pOut->heightAlign,
                                                      pOut->pTileInfo);

        pOut->last2DLevel = IsMicroTiled(nextTileMode);
    }
}

/**
***************************************************************************************************
*   SIAddrLib::HwlDegradeThickTileMode
*
*   @brief
*       Degrades valid tile mode for thick modes if needed
*
*   @return
*       Suitable tile mode
***************************************************************************************************
*/
AddrTileMode SIAddrLib::HwlDegradeThickTileMode(
    AddrTileMode        baseTileMode,   ///< [in] base tile mode
    uint32_t            numSlices,      ///< [in] current number of slices
    uint32_t*           pBytesPerTile   ///< [in/out] pointer to bytes per slice
    ) const
{
    return EgBasedAddrLib::HwlDegradeThickTileMode(baseTileMode, numSlices, pBytesPerTile);
}

/**
***************************************************************************************************
*   SIAddrLib::HwlTileInfoEqual
*
*   @brief
*       Return TRUE if all field are equal
*   @note
*       Only takes care of current HWL's data
***************************************************************************************************
*/
BOOL_32 SIAddrLib::HwlTileInfoEqual(
    const ADDR_TILEINFO* pLeft, ///<[in] Left compare operand
    const ADDR_TILEINFO* pRight ///<[in] Right compare operand
    ) const
{
    BOOL_32 equal = FALSE;

    if (pLeft->pipeConfig == pRight->pipeConfig)
    {
        equal =  EgBasedAddrLib::HwlTileInfoEqual(pLeft, pRight);
    }

    return equal;
}

/**
***************************************************************************************************
*   SIAddrLib::GetTileSettings
*
*   @brief
*       Get tile setting infos by index.
*   @return
*       Tile setting info.
***************************************************************************************************
*/
const ADDR_TILECONFIG* SIAddrLib::GetTileSetting(
    uint32_t index         ///< [in] Tile index
    ) const
{
    ADDR_ASSERT(index < m_noOfEntries);
    return &m_tileTable[index];
}

/**
***************************************************************************************************
*   SIAddrLib::HwlPostCheckTileIndex
*
*   @brief
*       Map a tile setting to index if curIndex is invalid, otherwise check if curIndex matches
*       tile mode/type/info and change the index if needed
*   @return
*       Tile index.
***************************************************************************************************
*/
int32_t SIAddrLib::HwlPostCheckTileIndex(
    const ADDR_TILEINFO* pInfo,     ///< [in] Tile Info
    AddrTileMode         mode,      ///< [in] Tile mode
    AddrTileType         type,      ///< [in] Tile type
    INT                  curIndex   ///< [in] Current index assigned in HwlSetupTileInfo
    ) const
{
    int32_t index = curIndex;

    if (mode == ADDR_TM_LINEAR_GENERAL)
    {
        index = TileIndexLinearGeneral;
    }
    else
    {
        BOOL_32 macroTiled = IsMacroTiled(mode);

        // We need to find a new index if either of them is true
        // 1. curIndex is invalid
        // 2. tile mode is changed
        // 3. tile info does not match for macro tiled
        if ((index == TileIndexInvalid         ||
            (mode != m_tileTable[index].mode)  ||
            (macroTiled && !HwlTileInfoEqual(pInfo, &m_tileTable[index].info))))
        {
            for (index = 0; index < static_cast<int32_t>(m_noOfEntries); index++)
            {
                if (macroTiled)
                {
                    // macro tile modes need all to match
                    if (HwlTileInfoEqual(pInfo, &m_tileTable[index].info) &&
                        (mode == m_tileTable[index].mode)                 &&
                        (type == m_tileTable[index].type))
                    {
                        break;
                    }
                }
                else if (mode == ADDR_TM_LINEAR_ALIGNED)
                {
                    // linear mode only needs tile mode to match
                    if (mode == m_tileTable[index].mode)
                    {
                        break;
                    }
                }
                else
                {
                    // micro tile modes only need tile mode and tile type to match
                    if (mode == m_tileTable[index].mode &&
                        type == m_tileTable[index].type)
                    {
                        break;
                    }
                }
            }
        }
    }

    ADDR_ASSERT(index < static_cast<int32_t>(m_noOfEntries));

    if (index >= static_cast<int32_t>(m_noOfEntries))
    {
        index = TileIndexInvalid;
    }

    return index;
}

/**
***************************************************************************************************
*   SIAddrLib::HwlSetupTileCfg
*
*   @brief
*       Map tile index to tile setting.
*   @return
*       ADDR_E_RETURNCODE
***************************************************************************************************
*/
ADDR_E_RETURNCODE SIAddrLib::HwlSetupTileCfg(
    int32_t         index,          ///< [in] Tile index
    int32_t         macroModeIndex, ///< [in] Index in macro tile mode table(CI)
    ADDR_TILEINFO*  pInfo,          ///< [out] Tile Info
    AddrTileMode*   pMode,          ///< [out] Tile mode
    AddrTileType*   pType          ///< [out] Tile type
    ) const
{
    ADDR_E_RETURNCODE returnCode = ADDR_OK;

    // Global flag to control usage of tileIndex
    if (UseTileIndex(index))
    {
        if (index == TileIndexLinearGeneral)
        {
            if (pMode)
            {
                *pMode = ADDR_TM_LINEAR_GENERAL;
            }

            if (pType)
            {
                *pType = ADDR_DISPLAYABLE;
            }

            if (pInfo)
            {
                pInfo->banks = 2;
                pInfo->bankWidth = 1;
                pInfo->bankHeight = 1;
                pInfo->macroAspectRatio = 1;
                pInfo->tileSplitBytes = 64;
                pInfo->pipeConfig = ADDR_PIPECFG_P2;
            }
        }
        else if (static_cast<uint32_t>(index) >= m_noOfEntries)
        {
            returnCode = ADDR_INVALIDPARAMS;
        }
        else
        {
            const ADDR_TILECONFIG* pCfgTable = GetTileSetting(index);

            if (pInfo)
            {
                *pInfo = pCfgTable->info;
            }
            else
            {
                if (IsMacroTiled(pCfgTable->mode))
                {
                    returnCode = ADDR_INVALIDPARAMS;
                }
            }

            if (pMode)
            {
                *pMode = pCfgTable->mode;
            }

            if (pType)
            {
                *pType = pCfgTable->type;
            }
        }
    }

    return returnCode;
}

/**
***************************************************************************************************
*   SIAddrLib::ReadGbTileMode
*
*   @brief
*       Convert GB_TILE_MODE HW value to ADDR_TILE_CONFIG.
*   @return
*       NA.
***************************************************************************************************
*/
VOID SIAddrLib::ReadGbTileMode(
    uint32_t            regValue,   ///< [in] GB_TILE_MODE register
    ADDR_TILECONFIG*    pCfg        ///< [out] output structure
    ) const
{
    GB_TILE_MODE gbTileMode;
    gbTileMode.val = regValue;

    pCfg->type = static_cast<AddrTileType>(gbTileMode.f.micro_tile_mode);
    pCfg->info.bankHeight = 1 << gbTileMode.f.bank_height;
    pCfg->info.bankWidth = 1 << gbTileMode.f.bank_width;
    pCfg->info.banks = 1 << (gbTileMode.f.num_banks + 1);
    pCfg->info.macroAspectRatio = 1 << gbTileMode.f.macro_tile_aspect;
    pCfg->info.tileSplitBytes = 64 << gbTileMode.f.tile_split;
    pCfg->info.pipeConfig = static_cast<AddrPipeCfg>(gbTileMode.f.pipe_config + 1);

    uint32_t regArrayMode = gbTileMode.f.array_mode;

    pCfg->mode = static_cast<AddrTileMode>(regArrayMode);

    if (regArrayMode == 8) //ARRAY_2D_TILED_XTHICK
    {
        pCfg->mode = ADDR_TM_2D_TILED_XTHICK;
    }
    else if (regArrayMode >= 14) //ARRAY_3D_TILED_XTHICK
    {
        pCfg->mode = static_cast<AddrTileMode>(pCfg->mode + 3);
    }
}

/**
***************************************************************************************************
*   SIAddrLib::InitTileSettingTable
*
*   @brief
*       Initialize the ADDR_TILE_CONFIG table.
*   @return
*       TRUE if tile table is correctly initialized
***************************************************************************************************
*/
BOOL_32 SIAddrLib::InitTileSettingTable(
    const uint32_t* pCfg,           ///< [in] Pointer to table of tile configs
    uint32_t        noOfEntries     ///< [in] Numbe of entries in the table above
    )
{
    BOOL_32 initOk = TRUE;

    ADDR_ASSERT(noOfEntries <= TileTableSize);

    memset(m_tileTable, 0, sizeof(m_tileTable));

    if (noOfEntries != 0)
    {
        m_noOfEntries = noOfEntries;
    }
    else
    {
        m_noOfEntries = TileTableSize;
    }

    if (pCfg) // From Client
    {
        for (uint32_t i = 0; i < m_noOfEntries; i++)
        {
            ReadGbTileMode(*(pCfg + i), &m_tileTable[i]);
        }
    }
    else
    {
        ADDR_ASSERT_ALWAYS();
        initOk = FALSE;
    }

    if (initOk)
    {
        ADDR_ASSERT(m_tileTable[TILEINDEX_LINEAR_ALIGNED].mode == ADDR_TM_LINEAR_ALIGNED);
    }

    return initOk;
}

/**
***************************************************************************************************
*   SIAddrLib::HwlGetTileIndex
*
*   @brief
*       Return the virtual/real index for given mode/type/info
*   @return
*       ADDR_OK if successful.
***************************************************************************************************
*/
ADDR_E_RETURNCODE SIAddrLib::HwlGetTileIndex(
    const ADDR_GET_TILEINDEX_INPUT* pIn,
    ADDR_GET_TILEINDEX_OUTPUT*      pOut) const
{
    ADDR_E_RETURNCODE returnCode = ADDR_OK;

    pOut->index = HwlPostCheckTileIndex(pIn->pTileInfo, pIn->tileMode, pIn->tileType);

    return returnCode;
}

/**
***************************************************************************************************
*   SIAddrLib::HwlFmaskPreThunkSurfInfo
*
*   @brief
*       Some preparation before thunking a ComputeSurfaceInfo call for Fmask
*   @return
*       ADDR_E_RETURNCODE
***************************************************************************************************
*/
VOID SIAddrLib::HwlFmaskPreThunkSurfInfo(
    const ADDR_COMPUTE_FMASK_INFO_INPUT*    pFmaskIn,   ///< [in] Input of fmask info
    const ADDR_COMPUTE_FMASK_INFO_OUTPUT*   pFmaskOut,  ///< [in] Output of fmask info
    ADDR_COMPUTE_SURFACE_INFO_INPUT*        pSurfIn,    ///< [out] Input of thunked surface info
    ADDR_COMPUTE_SURFACE_INFO_OUTPUT*       pSurfOut    ///< [out] Output of thunked surface info
    ) const
{
    pSurfIn->tileIndex = pFmaskIn->tileIndex;
}

/**
***************************************************************************************************
*   SIAddrLib::HwlFmaskPostThunkSurfInfo
*
*   @brief
*       Copy hwl extra field after calling thunked ComputeSurfaceInfo
*   @return
*       ADDR_E_RETURNCODE
***************************************************************************************************
*/
VOID SIAddrLib::HwlFmaskPostThunkSurfInfo(
    const ADDR_COMPUTE_SURFACE_INFO_OUTPUT* pSurfOut,   ///< [in] Output of surface info
    ADDR_COMPUTE_FMASK_INFO_OUTPUT* pFmaskOut           ///< [out] Output of fmask info
    ) const
{
    pFmaskOut->macroModeIndex = TileIndexInvalid;
    pFmaskOut->tileIndex = pSurfOut->tileIndex;
}

/**
***************************************************************************************************
*   SIAddrLib::HwlComputeFmaskBits
*   @brief
*       Computes fmask bits
*   @return
*       Fmask bits
***************************************************************************************************
*/
uint32_t SIAddrLib::HwlComputeFmaskBits(
    const ADDR_COMPUTE_FMASK_INFO_INPUT* pIn,
    uint32_t* pNumSamples
    ) const
{
    uint32_t numSamples = pIn->numSamples;
    uint32_t numFrags = GetNumFragments(numSamples, pIn->numFrags);
    uint32_t bpp;

    if (numFrags != numSamples) // EQAA
    {
        ADDR_ASSERT(numFrags <= 8);

        if (!pIn->resolved)
        {
            if (numFrags == 1)
            {
                bpp          = 1;
                numSamples   = numSamples == 16 ? 16 : 8;
            }
            else if (numFrags == 2)
            {
                ADDR_ASSERT(numSamples >= 4);

                bpp          = 2;
                numSamples   = numSamples;
            }
            else if (numFrags == 4)
            {
                ADDR_ASSERT(numSamples >= 4);

                bpp          = 4;
                numSamples   = numSamples;
            }
            else // numFrags == 8
            {
                ADDR_ASSERT(numSamples == 16);

                bpp          = 4;
                numSamples   = numSamples;
            }
        }
        else
        {
            if (numFrags == 1)
            {
                bpp          = (numSamples == 16) ? 16 : 8;
                numSamples   = 1;
            }
            else if (numFrags == 2)
            {
                ADDR_ASSERT(numSamples >= 4);

                bpp          = numSamples*2;
                numSamples   = 1;
            }
            else if (numFrags == 4)
            {
                ADDR_ASSERT(numSamples >= 4);

                bpp          = numSamples*4;
                numSamples   = 1;
            }
            else // numFrags == 8
            {
                ADDR_ASSERT(numSamples >= 16);

                bpp          = 16*4;
                numSamples   = 1;
            }
        }
    }
    else // Normal AA
    {
        if (!pIn->resolved)
        {
            bpp          = ComputeFmaskNumPlanesFromNumSamples(numSamples);
            numSamples   = numSamples == 2 ? 8 : numSamples;
        }
        else
        {
            // The same as 8XX
            bpp          = ComputeFmaskResolvedBppFromNumSamples(numSamples);
            numSamples   = 1; // 1x sample
        }
    }

    SafeAssign(pNumSamples, numSamples);

    return bpp;
}

/**
***************************************************************************************************
*   SIAddrLib::HwlOverrideTileMode
*
*   @brief
*       Override tile modes (for PRT only, avoid client passes in an invalid PRT mode for SI.
*
*   @return
*       Suitable tile mode
*
***************************************************************************************************
*/
BOOL_32 SIAddrLib::HwlOverrideTileMode(
    const ADDR_COMPUTE_SURFACE_INFO_INPUT*  pIn,       ///< [in] input structure
    AddrTileMode*                           pTileMode, ///< [in/out] pointer to the tile mode
    AddrTileType*                           pTileType  ///< [in/out] pointer to the tile type
    ) const
{
    BOOL_32 bOverrided = FALSE;
    AddrTileMode tileMode = *pTileMode;

    switch (tileMode)
    {
        case ADDR_TM_PRT_TILED_THIN1:
            tileMode    = ADDR_TM_2D_TILED_THIN1;
            break;

        case ADDR_TM_PRT_TILED_THICK:
            tileMode    = ADDR_TM_2D_TILED_THICK;
            break;

        case ADDR_TM_PRT_2D_TILED_THICK:
            tileMode    = ADDR_TM_2D_TILED_THICK;
            break;

        case ADDR_TM_PRT_3D_TILED_THICK:
            tileMode    = ADDR_TM_3D_TILED_THICK;
            break;

        default:
            break;
    }

    if (tileMode != *pTileMode)
    {
        *pTileMode = tileMode;
        bOverrided = TRUE;
        ADDR_ASSERT(pIn->flags.prt == TRUE);
    }

    return bOverrided;
}
