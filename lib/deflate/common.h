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

typedef void* voidpf;         /* void * void_ptr */
typedef unsigned char  Byte;  /* 8 bits */
typedef unsigned char  uInt8;  /* 8 bits */
typedef unsigned int   uInt;  /* 16 bits or more */
typedef unsigned long  uLong; /* 32 bits or more */
typedef unsigned long long uLong; /* 32 bits or more */

typedef voidpf (*alloc_func) (voidpf opaque, uInt items, uInt size);
typedef void   (*free_func)  (voidpf opaque, voidpf address);
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

    alloc_func zalloc;  /* used to allocate the internal state */
    free_func  zfree;   /* used to free the internal state */
    voidpf     opaque;  /* private data object passed to zalloc and zfree */

    int     data_type;  /* best guess about the data type: binary or text
                           for deflate, or the decoding state for inflate */
    uLong   adler;      /* Adler-32 or CRC-32 value of the uncompressed data */
    uLong   reserved;   /* reserved for future use */
} h_stream;

typedef h_stream* h_streamptr;

typedef enum {
    HEAD = 0,           // 解析 3bit 头
    COPY,               // copy 模式
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

#ifdef __cplusplus
}
#endif

#endif /* _DEFLATE_COMMON_H__ */