//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// DESCRIPTION:
//	Simple basic typedefs, isolated here to make it easier
//	 separating modules.
//    


#ifndef __DOOMTYPE__
#define __DOOMTYPE__

#include "config.h"

#if defined(_MSC_VER) && !defined(__cplusplus)
#define inline __inline
#endif

// #define macros to provide functions missing in Windows.
// Outside Windows, we use strings.h for str[n]casecmp.


#if !HAVE_DECL_STRCASECMP || !HAVE_DECL_STRNCASECMP

#include <string.h>
#if !HAVE_DECL_STRCASECMP
#define strcasecmp stricmp
#endif
#if !HAVE_DECL_STRNCASECMP
#define strncasecmp strnicmp
#endif

#else

#include <strings.h>

#endif


//
// The packed attribute forces structures to be packed into the minimum 
// space necessary.  If this is not done, the compiler may align structure
// fields differently to optimize memory access, inflating the overall
// structure size.  It is important to use the packed attribute on certain
// structures where alignment is important, particularly data read/written
// to disk.
//

#ifdef __GNUC__

#if defined(_WIN32) && !defined(__clang__)
#define PACKEDATTR __attribute__((packed,gcc_struct))
#else
#define PACKEDATTR __attribute__((packed))
#endif

#ifdef __clang__
#define PRINTF_ATTR(fmt, first)
#else
#define PRINTF_ATTR(fmt, first) __attribute__((format(printf, fmt, first)))
#endif
#define PRINTF_ARG_ATTR(x) __attribute__((format_arg(x)))
#define NORETURN __attribute__((noreturn))

#else
#if defined(_MSC_VER)
#define PACKEDATTR __pragma(pack(pop))
#else
#define PACKEDATTR
#endif
#define PRINTF_ATTR(fmt, first)
#define PRINTF_ARG_ATTR(x)
#define NORETURN
#endif

#ifdef __WATCOMC__
#define PACKEDPREFIX _Packed
#elif defined(_MSC_VER)
#define PACKEDPREFIX __pragma(pack(push,1))
#else
#define PACKEDPREFIX
#endif

#define PACKED_STRUCT(...) PACKEDPREFIX struct __VA_ARGS__ PACKEDATTR

#include <stdint.h>

#if defined(__cplusplus) || defined(__bool_true_false_are_defined)

// Use builtin bool type with C++.

typedef bool boolean;

#else

typedef enum 
{
    false, 
    true
} boolean;

#endif

typedef uint8_t byte;
typedef uint8_t pixel_t;
typedef int16_t dpixel_t;

#include <limits.h>

#ifdef _WIN32

#define DIR_SEPARATOR '\\'
#define DIR_SEPARATOR_S "\\"
#define PATH_SEPARATOR ';'

#else

#define DIR_SEPARATOR '/'
#define DIR_SEPARATOR_S "/"
#define PATH_SEPARATOR ':'

#endif

#define arrlen(array) (sizeof(array) / sizeof(*array))

#endif

