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
#ifndef _lnx_common_defs_h_
#define _lnx_common_defs_h_

#if DBG
#include <stdarg.h>                         // We do not have any choice: need variable
                                            // number of parameters support for debug
                                            // build.
#endif                                      // #if DBG

//
// --------------  External functions from Linux kernel driver ----------------
//
// Note: The definitions/declararions below must match the original ones.

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(ATI_API_CALL)
#define ATI_API_CALL __attribute__((regparm(0)))
#endif

#ifdef __cplusplus
}
#endif

//
// --------------------------  C/C++ standard typedefs ----------------------------
//
#ifdef __SIZE_TYPE__
typedef __SIZE_TYPE__       size_t;
#else                                       // #ifdef __SIZE_TYPE__
typedef unsigned int        size_t;
#endif                                      // #ifdef __SIZE_TYPE__

#ifdef __PTRDIFF_TYPE__
typedef __PTRDIFF_TYPE__    ptrdiff_t;
#else                                       // #ifdef __PTRDIFF_TYPE__
typedef int                 ptrdiff_t;
#endif                                      // #ifdef __PTRDIFF_TYPE__

#ifndef NULL
#ifdef __cplusplus
#define NULL    __null
#else
#define NULL    ((void *)0)
#endif
#endif


//
// -------------------------  C/C++ standard macros ---------------------------
//

#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)  // as it is defined in stddef.h
#define CHAR_BIT            8                                   // as it is defined in limits.h

#endif                                      // #ifdef _lnx_common_defs_h_
