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
* @file  addrelemlib.h
* @brief Contains the class for element/pixel related functions
***************************************************************************************************
*/

#ifndef __ELEM_LIB_H__
#define __ELEM_LIB_H__

#include <stdint.h>

#include "addrinterface.h"
#include "addrobject.h"
#include "addrcommon.h"

class AddrLib;

// The masks for property bits within the Properties int32_t
union ADDR_COMPONENT_FLAGS
{
    struct
    {
        uint32_t byteAligned    : 1;    ///< all components are byte aligned
        uint32_t exportNorm     : 1;    ///< components support R6xx NORM compression
        uint32_t floatComp      : 1;    ///< there is at least one floating point component
    };

    uint32_t value;
};

// Copy from legacy lib's AddrNumberType
enum AddrNumberType
{
    // The following number types have the range [-1..1]
    ADDR_NO_NUMBER,         // This component doesn't exist and has no default value
    ADDR_EPSILON,           // Force component value to integer 0x00000001
    ADDR_ZERO,              // Force component value to integer 0x00000000
    ADDR_ONE,               // Force component value to floating point 1.0
    // Above values don't have any bits per component (keep ADDR_ONE the last of these)

    ADDR_UNORM,             // Unsigned normalized (repeating fraction) full precision
    ADDR_SNORM,             // Signed normalized (repeating fraction) full precision
    ADDR_GAMMA,             // Gamma-corrected, full precision

    ADDR_UNORM_R5XXRB,      // Unsigned normalized (repeating fraction) for r5xx RB
    ADDR_SNORM_R5XXRB,      // Signed normalized (repeating fraction) for r5xx RB
    ADDR_GAMMA_R5XXRB,      // Gamma-corrected for r5xx RB (note: unnormalized value)
    ADDR_UNORM_R5XXBC,      // Unsigned normalized (repeating fraction) for r5xx BC
    ADDR_SNORM_R5XXBC,      // Signed normalized (repeating fraction) for r5xx BC
    ADDR_GAMMA_R5XXBC,      // Gamma-corrected for r5xx BC (note: unnormalized value)

    ADDR_UNORM_R6XX,        // Unsigned normalized (repeating fraction) for R6xx
    ADDR_UNORM_R6XXDB,      // Unorms for 24-bit depth: one value differs from ADDR_UNORM_R6XX
    ADDR_SNORM_R6XX,        // Signed normalized (repeating fraction) for R6xx
    ADDR_GAMMA8_R6XX,       // Gamma-corrected for r6xx
    ADDR_GAMMA8_R7XX_TP,    // Gamma-corrected for r7xx TP 12bit unorm 8.4.

    ADDR_U4FLOATC,          // Unsigned float: 4-bit exponent, bias=15, no NaN, clamp [0..1]
    ADDR_GAMMA_4SEG,        // Gamma-corrected, four segment approximation
    ADDR_U0FIXED,           // Unsigned 0.N-bit fixed point

    // The following number types have large ranges (LEAVE ADDR_USCALED first or fix Finish routine)
    ADDR_USCALED,           // Unsigned integer converted to/from floating point
    ADDR_SSCALED,           // Signed integer converted to/from floating point
    ADDR_USCALED_R5XXRB,    // Unsigned integer to/from floating point for r5xx RB
    ADDR_SSCALED_R5XXRB,    // Signed integer to/from floating point for r5xx RB
    ADDR_UINT_BITS,         // Keep in unsigned integer form, clamped to specified range
    ADDR_SINT_BITS,         // Keep in signed integer form, clamped to specified range
    ADDR_UINTBITS,          // @@ remove Keep in unsigned integer form, use modulus to reduce bits
    ADDR_SINTBITS,          // @@ remove Keep in signed integer form, use modulus to reduce bits

    // The following number types and ADDR_U4FLOATC have exponents
    // (LEAVE ADDR_S8FLOAT first or fix Finish routine)
    ADDR_S8FLOAT,           // Signed floating point with 8-bit exponent, bias=127
    ADDR_S8FLOAT32,         // 32-bit IEEE float, passes through NaN values
    ADDR_S5FLOAT,           // Signed floating point with 5-bit exponent, bias=15
    ADDR_S5FLOATM,          // Signed floating point with 5-bit exponent, bias=15, no NaN/Inf
    ADDR_U5FLOAT,           // Signed floating point with 5-bit exponent, bias=15
    ADDR_U3FLOATM,          // Unsigned floating point with 3-bit exponent, bias=3

    ADDR_S5FIXED,           // Signed 5.N-bit fixed point, with rounding

    ADDR_END_NUMBER         // Used for range comparisons
};

// Copy from legacy lib's AddrElement
enum AddrElemMode
{
    // These formats allow both packing an unpacking
    ADDR_ROUND_BY_HALF,     // add 1/2 and truncate when packing this element
    ADDR_ROUND_TRUNCATE,    // truncate toward 0 for sign/mag, else toward neg
    ADDR_ROUND_DITHER,      // Pack by dithering -- requires (x,y) position

    // These formats only allow unpacking, no packing
    ADDR_UNCOMPRESSED,      // Elements are not compressed: one data element per pixel/texel
    ADDR_EXPANDED,          // Elements are split up and stored in multiple data elements
    ADDR_PACKED_STD,        // Elements are compressed into ExpandX by ExpandY data elements
    ADDR_PACKED_REV,        // Like ADDR_PACKED, but X order of pixels is reverved
    ADDR_PACKED_GBGR,       // Elements are compressed 4:2:2 in G1B_G0R order (high to low)
    ADDR_PACKED_BGRG,       // Elements are compressed 4:2:2 in BG1_RG0 order (high to low)
    ADDR_PACKED_BC1,        // Each data element is uncompressed to a 4x4 pixel/texel array
    ADDR_PACKED_BC2,        // Each data element is uncompressed to a 4x4 pixel/texel array
    ADDR_PACKED_BC3,        // Each data element is uncompressed to a 4x4 pixel/texel array
    ADDR_PACKED_BC4,        // Each data element is uncompressed to a 4x4 pixel/texel array
    ADDR_PACKED_BC5,        // Each data element is uncompressed to a 4x4 pixel/texel array

    // These formats provide various kinds of compression
    ADDR_ZPLANE_R5XX,       // Compressed Zplane using r5xx architecture format
    ADDR_ZPLANE_R6XX,       // Compressed Zplane using r6xx architecture format
    //@@ Fill in the compression modes

    ADDR_END_ELEMENT        // Used for range comparisons
};

enum AddrDepthPlanarType
{
    ADDR_DEPTH_PLANAR_NONE = 0, // No plane z/stencl
    ADDR_DEPTH_PLANAR_R600 = 1, // R600 z and stencil planes are store within a tile
    ADDR_DEPTH_PLANAR_R800 = 2, // R800 has separate z and stencil planes
};

/**
***************************************************************************************************
*   ADDR_PIXEL_FORMATINFO
*
*   @brief
*       Per component info
*
***************************************************************************************************
*/
struct ADDR_PIXEL_FORMATINFO
{
    uint32_t            compBit[4];
    AddrNumberType      numType[4];
    uint32_t            compStart[4];
    AddrElemMode        elemMode;
    uint32_t            comps;          ///< Number of components
};

/**
***************************************************************************************************
* @brief This class contains asic indepentent element related attributes and operations
***************************************************************************************************
*/
class AddrElemLib : public AddrObject
{
protected:
    AddrElemLib(AddrLib* const pAddrLib);

public:

    /// Makes this class virtual
    virtual ~AddrElemLib();

    static AddrElemLib *Create(
        const AddrLib* const pAddrLib);

    /// The implementation is only for R6xx/R7xx, so make it virtual in case we need for R8xx
    BOOL_32 PixGetExportNorm(
        AddrColorFormat colorFmt,
        AddrSurfaceNumber numberFmt, AddrSurfaceSwap swap) const;

    /// Below method are asic independent, so make them just static.
    /// Remove static if we need different operation in hwl.

    VOID    Flt32ToDepthPixel(
        AddrDepthFormat format, const ADDR_FLT_32 comps[2], uint8_t *pPixel) const;

    VOID    Flt32ToColorPixel(
        AddrColorFormat format, AddrSurfaceNumber surfNum, AddrSurfaceSwap surfSwap,
        const ADDR_FLT_32 comps[4], uint8_t *pPixel) const;

    static VOID    Flt32sToInt32s(
        ADDR_FLT_32 value, uint32_t bits, AddrNumberType numberType, uint32_t* pResult);

    static VOID    Int32sToPixel(
        uint32_t numComps, uint32_t* pComps, uint32_t* pCompBits, uint32_t* pCompStart,
        ADDR_COMPONENT_FLAGS properties, uint32_t resultBits, uint8_t* pPixel);

    VOID    PixGetColorCompInfo(
        AddrColorFormat format, AddrSurfaceNumber number, AddrSurfaceSwap swap,
        ADDR_PIXEL_FORMATINFO* pInfo) const;

    VOID    PixGetDepthCompInfo(
        AddrDepthFormat format, ADDR_PIXEL_FORMATINFO* pInfo) const;

    uint32_t GetBitsPerPixel(
        AddrFormat format, AddrElemMode* pElemMode,
        uint32_t* pExpandX = NULL, uint32_t* pExpandY = NULL, uint32_t* pBitsUnused = NULL);

    static VOID    SetClearComps(
        ADDR_FLT_32 comps[4], BOOL_32 clearColor, BOOL_32 float32);

    VOID    AdjustSurfaceInfo(
        AddrElemMode elemMode, uint32_t expandX, uint32_t expandY,
        uint32_t* pBpp, uint32_t* pBasePitch, uint32_t* pWidth, uint32_t* pHeight);

    VOID    RestoreSurfaceInfo(
        AddrElemMode elemMode, uint32_t expandX, uint32_t expandY,
        uint32_t* pBpp, uint32_t* pWidth, uint32_t* pHeight);

    /// Checks if depth and stencil are planar inside a tile
    BOOL_32 IsDepthStencilTilePlanar()
    {
        return (m_depthPlanarType == ADDR_DEPTH_PLANAR_R600) ? TRUE : FALSE;
    }

    /// Sets m_configFlags, copied from AddrLib
    VOID    SetConfigFlags(ADDR_CONFIG_FLAGS flags)
    {
        m_configFlags = flags;
    }

    static BOOL_32 IsCompressed(AddrFormat format);
    static BOOL_32 IsBlockCompressed(AddrFormat format);
    static BOOL_32 IsExpand3x(AddrFormat format);

protected:

    static VOID    GetCompBits(
        uint32_t c0, uint32_t c1, uint32_t c2, uint32_t c3,
        ADDR_PIXEL_FORMATINFO* pInfo,
        AddrElemMode elemMode = ADDR_ROUND_BY_HALF);

    static VOID    GetCompType(
        AddrColorFormat format, AddrSurfaceNumber numType,
        ADDR_PIXEL_FORMATINFO* pInfo);

    static VOID    GetCompSwap(
        AddrSurfaceSwap swap, ADDR_PIXEL_FORMATINFO* pInfo);

    static VOID    SwapComps(
        uint32_t c0, uint32_t c1, ADDR_PIXEL_FORMATINFO* pInfo);

private:

    uint32_t             m_fp16ExportNorm;   ///< If allow FP16 to be reported as EXPORT_NORM
    AddrDepthPlanarType m_depthPlanarType;

    ADDR_CONFIG_FLAGS   m_configFlags;      ///< Copy of AddrLib's configFlags
    AddrLib* const      m_pAddrLib;         ///< Pointer to parent addrlib instance
};

#endif
