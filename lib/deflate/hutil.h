/******************************************************/
/*                       hutil.h                      */
/******************************************************/

#include "common.h"
#include <stddef.h>

// memcpy
#define hmemcpy memcpy

// 默认内存申请函数
HEXTERN voidptr hmalloc(voidptr opaque, size_t items, size_t size);

// 默认内存释放函数
HEXTERN void hmfree(voidptr opaque, voidptr ptr);