#ifndef _STDINT_H_
#define _STDINT_H_

typedef unsigned char uint8_t;
typedef signed char int8_t;

typedef unsigned short uint16_t;
typedef signed short int16_t;

typedef unsigned int uint32_t;
typedef signed int int32_t;

typedef unsigned long long uint64_t;
typedef signed long long int64_t;

typedef uint32_t size_t;

typedef uint32_t uintptr_t;
typedef int32_t ptrdiff_t;
typedef int32_t intptr_t;

typedef int64_t intmax_t;

#define NULL ((void *) 0)

#endif
