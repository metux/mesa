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
* @file  addrcommon.h
* @brief Contains the helper function and constants
***************************************************************************************************
*/

#ifndef __ADDR_COMMON_H__
#define __ADDR_COMMON_H__

#include <stdint.h>

#include "addrinterface.h"


// ADDR_LNX_KERNEL_BUILD is for internal build
// Moved from addrinterface.h so __KERNEL__ is not needed any more
#if ADDR_LNX_KERNEL_BUILD // || (defined(__GNUC__) && defined(__KERNEL__))
    #include "lnx_common_defs.h" // ported from cmmqs
#elif !defined(__APPLE__)
    #include <stdlib.h>
    #include <string.h>
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
// Common constants
///////////////////////////////////////////////////////////////////////////////////////////////////
static const uint32_t MicroTileWidth      = 8;       ///< Micro tile width, for 1D and 2D tiling
static const uint32_t MicroTileHeight     = 8;       ///< Micro tile height, for 1D and 2D tiling
static const uint32_t ThickTileThickness  = 4;       ///< Micro tile thickness, for THICK modes
static const uint32_t XThickTileThickness = 8;       ///< Extra thick tiling thickness
static const uint32_t PowerSaveTileBytes  = 64;      ///< Nuber of bytes per tile for power save 64
static const uint32_t CmaskCacheBits      = 1024;    ///< Number of bits for CMASK cache
static const uint32_t CmaskElemBits       = 4;       ///< Number of bits for CMASK element
static const uint32_t HtileCacheBits      = 16384;   ///< Number of bits for HTILE cache 512*32

static const uint32_t MicroTilePixels     = MicroTileWidth * MicroTileHeight;

static const int32_t TileIndexInvalid       = TILEINDEX_INVALID;
static const int32_t TileIndexLinearGeneral = TILEINDEX_LINEAR_GENERAL;
static const int32_t TileIndexNoMacroIndex  = -3;

///////////////////////////////////////////////////////////////////////////////////////////////////
// Common macros
///////////////////////////////////////////////////////////////////////////////////////////////////
#define BITS_PER_BYTE 8
#define BITS_TO_BYTES(x) ( ((x) + (BITS_PER_BYTE-1)) / BITS_PER_BYTE )
#define BYTES_TO_BITS(x) ( (x) * BITS_PER_BYTE )

/// Helper macros to select a single bit from an int (undefined later in section)
#define _BIT(v,b)      (((v) >> (b) ) & 1)

/**
***************************************************************************************************
* @brief Enums to identify AddrLib type
***************************************************************************************************
*/
enum AddrLibClass
{
    BASE_ADDRLIB = 0x0,
    R600_ADDRLIB = 0x6,
    R800_ADDRLIB = 0x8,
    SI_ADDRLIB   = 0xa,
    CI_ADDRLIB   = 0xb,
};

/**
***************************************************************************************************
* AddrChipFamily
*
*   @brief
*       Neutral enums that specifies chip family.
*
***************************************************************************************************
*/
enum AddrChipFamily
{
    ADDR_CHIP_FAMILY_IVLD,    ///< Invalid family
    ADDR_CHIP_FAMILY_R6XX,
    ADDR_CHIP_FAMILY_R7XX,
    ADDR_CHIP_FAMILY_R8XX,
    ADDR_CHIP_FAMILY_NI,
    ADDR_CHIP_FAMILY_SI,
    ADDR_CHIP_FAMILY_CI,
    ADDR_CHIP_FAMILY_VI,
};

/**
***************************************************************************************************
* ADDR_CONFIG_FLAGS
*
*   @brief
*       This structure is used to set addr configuration flags.
***************************************************************************************************
*/
union ADDR_CONFIG_FLAGS
{
    struct
    {
        /// Clients do not need to set these flags except forceLinearAligned.
        /// There flags are set up by AddrLib inside thru AddrInitGlobalParamsFromRegister
        uint32_t optimalBankSwap        : 1;   ///< New bank tiling for RV770 only
        uint32_t noCubeMipSlicesPad     : 1;   ///< Disables faces padding for cubemap mipmaps
        uint32_t fillSizeFields         : 1;   ///< If clients fill size fields in all input and
                                               ///  output structure
        uint32_t ignoreTileInfo         : 1;   ///< Don't use tile info structure
        uint32_t useTileIndex           : 1;   ///< Make tileIndex field in input valid
        uint32_t useCombinedSwizzle     : 1;   ///< Use combined swizzle
        uint32_t checkLast2DLevel       : 1;   ///< Check the last 2D mip sub level
        uint32_t useHtileSliceAlign     : 1;   ///< Do htile single slice alignment
        uint32_t degradeBaseLevel       : 1;   ///< Degrade to 1D modes automatically for base level
        uint32_t allowLargeThickTile    : 1;   ///< Allow 64*thickness*bytesPerPixel > rowSize
        uint32_t reserved               : 22;  ///< Reserved bits for future use
    };

    uint32_t value;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// Platform specific debug break defines
///////////////////////////////////////////////////////////////////////////////////////////////////
#if DEBUG
    #if defined(__GNUC__)
        #define ADDR_DBG_BREAK()
    #elif defined(__APPLE__)
        #define ADDR_DBG_BREAK()    { IOPanic("");}
    #else
        #define ADDR_DBG_BREAK()    { __debugbreak(); }
    #endif
#else
    #define ADDR_DBG_BREAK()
#endif
///////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
// Debug assertions used in AddrLib
///////////////////////////////////////////////////////////////////////////////////////////////////
#if DEBUG
#define ADDR_ASSERT(__e) if ( !((__e) ? TRUE : FALSE)) { ADDR_DBG_BREAK(); }
#define ADDR_ASSERT_ALWAYS() ADDR_DBG_BREAK()
#define ADDR_UNHANDLED_CASE() ADDR_ASSERT(!"Unhandled case")
#define ADDR_NOT_IMPLEMENTED() ADDR_ASSERT(!"Not implemented");
#else //DEBUG
#define ADDR_ASSERT(__e)
#define ADDR_ASSERT_ALWAYS()
#define ADDR_UNHANDLED_CASE()
#define ADDR_NOT_IMPLEMENTED()
#endif //DEBUG
///////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
// Debug print macro from legacy address library
///////////////////////////////////////////////////////////////////////////////////////////////////
#if DEBUG

#define ADDR_PRNT(a)    AddrObject::DebugPrint a

/// @brief Macro for reporting informational messages
/// @ingroup util
///
/// This macro optionally prints an informational message to stdout.
/// The first parameter is a condition -- if it is true, nothing is done.
/// The second pararmeter MUST be a parenthesis-enclosed list of arguments,
/// starting with a string. This is passed to printf() or an equivalent
/// in order to format the informational message. For example,
/// ADDR_INFO(0, ("test %d",3) ); prints out "test 3".
///
#define ADDR_INFO(cond, a)         \
{ if (!(cond)) { ADDR_PRNT(a); } }


/// @brief Macro for reporting error warning messages
/// @ingroup util
///
/// This macro optionally prints an error warning message to stdout,
/// followed by the file name and line number where the macro was called.
/// The first parameter is a condition -- if it is true, nothing is done.
/// The second pararmeter MUST be a parenthesis-enclosed list of arguments,
/// starting with a string. This is passed to printf() or an equivalent
/// in order to format the informational message. For example,
/// ADDR_WARN(0, ("test %d",3) ); prints out "test 3" followed by
/// a second line with the file name and line number.
///
#define ADDR_WARN(cond, a)         \
{ if (!(cond))                     \
  { ADDR_PRNT(a);                  \
    ADDR_PRNT(("  WARNING in file %s, line %d\n", __FILE__, __LINE__)); \
} }


/// @brief Macro for reporting fatal error conditions
/// @ingroup util
///
/// This macro optionally stops execution of the current routine
/// after printing an error warning message to stdout,
/// followed by the file name and line number where the macro was called.
/// The first parameter is a condition -- if it is true, nothing is done.
/// The second pararmeter MUST be a parenthesis-enclosed list of arguments,
/// starting with a string. This is passed to printf() or an equivalent
/// in order to format the informational message. For example,
/// ADDR_EXIT(0, ("test %d",3) ); prints out "test 3" followed by
/// a second line with the file name and line number, then stops execution.
///
#define ADDR_EXIT(cond, a)         \
{ if (!(cond))                     \
  { ADDR_PRNT(a); ADDR_DBG_BREAK();\
} }

#else // DEBUG

#define ADDRDPF 1 ? (void)0 : (void)

#define ADDR_PRNT(a)

#define ADDR_DBG_BREAK()

#define ADDR_INFO(cond, a)

#define ADDR_WARN(cond, a)

#define ADDR_EXIT(cond, a)

#endif // DEBUG
///////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
// Misc helper functions
////////////////////////////////////////////////////////////////////////////////////////////////////

/**
***************************************************************************************************
*   AddrXorReduce
*
*   @brief
*       Xor the right-side numberOfBits bits of x.
***************************************************************************************************
*/
static inline uint32_t XorReduce(
    uint32_t x,
    uint32_t numberOfBits)
{
    uint32_t i;
    uint32_t result = x & 1;

    for (i=1; i<numberOfBits; i++)
    {
        result ^= ((x>>i) & 1);
    }

    return result;
}

/**
***************************************************************************************************
*   IsPow2
*
*   @brief
*       Check if the size (uint32_t) is pow 2
***************************************************************************************************
*/
static inline uint32_t IsPow2(
    uint32_t dim)        ///< [in] dimension of miplevel
{
    ADDR_ASSERT(dim > 0);
    return !(dim & (dim - 1));
}

/**
***************************************************************************************************
*   IsPow2
*
*   @brief
*       Check if the size (uint64_t) is pow 2
***************************************************************************************************
*/
static inline int64_t IsPow2(
    uint64_t dim)       ///< [in] dimension of miplevel
{
    ADDR_ASSERT(dim > 0);
    return !(dim & (dim - 1));
}

/**
***************************************************************************************************
*   ByteAlign
*
*   @brief
*       Align uint32_t "x" to "align" alignment, "align" should be power of 2
***************************************************************************************************
*/
static inline uint32_t PowTwoAlign(
    uint32_t x,
    uint32_t align)
{
    //
    // Assert that x is a power of two.
    //
    ADDR_ASSERT(IsPow2(align));
    return (x + (align - 1)) & (~(align - 1));
}

/**
***************************************************************************************************
*   ByteAlign
*
*   @brief
*       Align uint64_t "x" to "align" alignment, "align" should be power of 2
***************************************************************************************************
*/
static inline uint64_t PowTwoAlign(
    uint64_t x,
    uint64_t align)
{
    //
    // Assert that x is a power of two.
    //
    ADDR_ASSERT(IsPow2(align));
    return (x + (align - 1)) & (~(align - 1));
}

/**
***************************************************************************************************
*   Min
*
*   @brief
*       Get the min value between two unsigned values
***************************************************************************************************
*/
static inline uint32_t Min(
    uint32_t value1,
    uint32_t value2)
{
    return ((value1 < (value2)) ? (value1) : value2);
}

/**
***************************************************************************************************
*   Min
*
*   @brief
*       Get the min value between two signed values
***************************************************************************************************
*/
static inline int32_t Min(
    int32_t value1,
    int32_t value2)
{
    return ((value1 < (value2)) ? (value1) : value2);
}

/**
***************************************************************************************************
*   Max
*
*   @brief
*       Get the max value between two unsigned values
***************************************************************************************************
*/
static inline uint32_t Max(
    uint32_t value1,
    uint32_t value2)
{
    return ((value1 > (value2)) ? (value1) : value2);
}

/**
***************************************************************************************************
*   Max
*
*   @brief
*       Get the max value between two signed values
***************************************************************************************************
*/
static inline int32_t Max(
    int32_t value1,
    int32_t value2)
{
    return ((value1 > (value2)) ? (value1) : value2);
}

/**
***************************************************************************************************
*   NextPow2
*
*   @brief
*       Compute the mipmap's next level dim size
***************************************************************************************************
*/
static inline uint32_t NextPow2(
    uint32_t dim)        ///< [in] dimension of miplevel
{
    uint32_t newDim;

    newDim = 1;

    if (dim > 0x7fffffff)
    {
        ADDR_ASSERT_ALWAYS();
        newDim = 0x80000000;
    }
    else
    {
        while (newDim < dim)
        {
            newDim <<= 1;
        }
    }

    return newDim;
}

/**
***************************************************************************************************
*   Log2
*
*   @brief
*       Compute log of base 2
***************************************************************************************************
*/
static inline uint32_t Log2(
    uint32_t x)      ///< [in] the value should calculate log based 2
{
    uint32_t y;

    //
    // Assert that x is a power of two.
    //
    ADDR_ASSERT(IsPow2(x));

    y = 0;
    while (x > 1)
    {
        x >>= 1;
        y++;
    }

    return y;
}

/**
***************************************************************************************************
*   QLog2
*
*   @brief
*       Compute log of base 2 quickly (<= 16)
***************************************************************************************************
*/
static inline uint32_t QLog2(
    uint32_t x)      ///< [in] the value should calculate log based 2
{
    ADDR_ASSERT(x <= 16);

    uint32_t y = 0;

    switch (x)
    {
        case 1:
            y = 0;
            break;
        case 2:
            y = 1;
            break;
        case 4:
            y = 2;
            break;
        case 8:
            y = 3;
            break;
        case 16:
            y = 4;
            break;
        default:
            ADDR_ASSERT_ALWAYS();
    }

    return y;
}

/**
***************************************************************************************************
*   SafeAssign
*
*   @brief
*       NULL pointer safe assignment
***************************************************************************************************
*/
static inline VOID SafeAssign(
    uint32_t*    pLVal,  ///< [in] Pointer to left val
    uint32_t     rVal)   ///< [in] Right value
{
    if (pLVal)
    {
        *pLVal = rVal;
    }
}

/**
***************************************************************************************************
*   SafeAssign
*
*   @brief
*       NULL pointer safe assignment for 64bit values
***************************************************************************************************
*/
static inline VOID SafeAssign(
    uint64_t*   pLVal,  ///< [in] Pointer to left val
    uint64_t    rVal)   ///< [in] Right value
{
    if (pLVal)
    {
        *pLVal = rVal;
    }
}

/**
***************************************************************************************************
*   SafeAssign
*
*   @brief
*       NULL pointer safe assignment for AddrTileMode
***************************************************************************************************
*/
static inline VOID SafeAssign(
    AddrTileMode*    pLVal, ///< [in] Pointer to left val
    AddrTileMode     rVal)  ///< [in] Right value
{
    if (pLVal)
    {
        *pLVal = rVal;
    }
}

#endif // __ADDR_COMMON_H__
