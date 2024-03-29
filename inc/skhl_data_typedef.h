#ifndef __SKHL_DATA_TYPEDEF_H__
#define __SKHL_DATA_TYPEDEF_H__

#ifdef SK_WINDOWS
#include "stdlib.h"
#endif

#define ARRAY_SIZE(arr)         (sizeof(arr) / sizeof(arr[0]))

typedef unsigned char uint8_t;
typedef unsigned short int uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long int uint64_t;

#ifdef SK_WINDOWS
typedef char int8_t;
typedef short int16_t;
typedef int int32_t;
typedef long int int64_t;
#else
//typedef char int8_t;
typedef short int int16_t;
typedef int int32_t;
//typedef long int int64_t;
#endif

typedef int32_t* skhl_handle;
typedef int32_t skhl_result;


#define TRUE    1
#define FALSE   0

#endif