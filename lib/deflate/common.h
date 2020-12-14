/**************************************************************
 * 
 * DEFLATE common.h
 * 
***************************************************************/
#ifndef _DEFLATE_COMMON_H__
#define _DEFLATE_COMMON_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifndef HEXTERN
#  define HEXTERN extern
#endif

#define LOCAL static inline
#define H_NULL 0

typedef void* voidptr;         /* void * void_ptr */
typedef unsigned char  Byte;  /* 8 bits */
typedef unsigned char  uInt8;  /* 8 bits */
typedef unsigned int   uInt;  /* 16 bits or more */
typedef unsigned long  uLong; /* 32 bits or more */
typedef unsigned long long uLong; /* 32 bits or more */

typedef voidptr (*alloc_func)(voidptr opaque, uInt items, uInt size);
typedef void    (*free_func)(voidptr opaque, voidptr address);
struct internal_state;

typedef struct h_stream_s {
    const Byte *next_in;     /* next input byte */
    uInt     avail_in;       /* number of bytes available at next_in */
    uLong    total_in;       /* total number of input bytes read so far */

    Byte    *next_out;       /* next output byte will go here */
    uInt     avail_out;      /* remaining free space at next_out */
    uLong    total_out;      /* total number of bytes output so far */

    const char *msg;         /* last error message, NULL if no error */
    struct internal_state *state; /* not visible by applications */

    alloc_func halloc;  /* used to allocate the internal state */
    free_func  hfree;   /* used to free the internal state */
    voidptr     opaque;  /* private data object passed to zalloc and zfree NOT USE*/

    int     data_type;  /* best guess about the data type: binary or text
                           for deflate, or the decoding state for inflate */
    uLong   adler;      /* Adler-32 or CRC-32 value of the uncompressed data */
    uLong   reserved;   /* reserved for future use */
} h_stream;

typedef h_stream* h_streamptr;

typedef enum {
    HEAD = 0,           // 解析 3bit 头
    STORED,             // STORED 模式
    HLIT,               // 字符和匹配长度树sym数 5bits （257-286）
    HDIST,              // 偏移距离树sym数 5bits （1-32）
    HCLEN,              // 游程树sym数 4bits （4-19）
    RUNLTREE,           // 解码 游程树
    RUNL,               // 解码游程数据
    LLTREE,             // 解码 字符和匹配长度树
    LENEXT,             // 匹配长度额外bit
    DTREE,              // 解码偏移量树
    DEXT,               // 偏移距离额外bit
    DATA_STREAM         // 解码压缩数据流
} h_state;

typedef enum {
    CONTINUE_BLOCK = 0,           // 非最后block
    LAST_BLOCK                    // 最后block
} h_block_type;

typedef struct h_tree_s{
    uInt counts[16];  // 记录各个huffman码长的个数，huffman码长最大不会超过15bits
    uInt symbols[288]; // 三棵huffman树最大的即为静态 lltree 288个叶子节点（dtree较小，动态树小于等于静态树）
    uInt max_symbol;   // 记录最大的 symbol 大于最大symbol的symbols索引都无效
} h_tree;

typedef h_tree* h_treePtr;

enum {  // return value
    H_OK = 0,           // OK
    H_STREAM_END,       // 
    H_NEED_DICT,        // 需要更多输入
    H_ERRNO,            // 通用ERROR
    H_STREAM_ERROR,     // 数据结构体错误
    H_DATA_ERROR,       // 数据错误
    H_MEM_ERROR,        // 内存错误
    H_BUF_ERROR         // 缓存错误
}


#ifdef __cplusplus
}
#endif

#endif /* _DEFLATE_COMMON_H__ */