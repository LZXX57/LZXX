/******************************************************/
/*                       hutil.c                      */
/******************************************************/

#include "hutil.h"

HEXTERN voidptr hmalloc(voidptr opaque, size_t items, size_t size){
    opaque = (voidptr)H_NULL;
    return (voidptr)malloc(items * size);
}

HEXTERN void hmfree(voidptr opaque, voidptr ptr){
    opaque = (voidptr)H_NULL;
    free(ptr);
}